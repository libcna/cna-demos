// SPDX-License-Identifier: MS-PL
// StorageDevice::BeginShowSelector/EndShowSelector: XNA's asynchronous way to get a save device.
#include <any>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDeviceNotConnectedException.hpp"
#include "System/IAsyncResult.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Runtime/Serialization/SerializationInfo.hpp"
#include "System/Runtime/Serialization/StreamingContext.hpp"
#include "System/Threading/WaitHandle.hpp"

using Microsoft::Xna::Framework::Storage::StorageDevice;
using Microsoft::Xna::Framework::Storage::StorageDeviceNotConnectedException;

namespace
{
    template<class Exception, class Action>
    bool ThrowsExactly(Action action)
    {
        try { action(); }
        catch (const Exception& error) { return typeid(error) == typeid(Exception); }
        catch (...) { return false; }
        return false;
    }

    void ShowSelector(CnaDemos::DemoContext& ctx)
    {
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
        // Force the real storage-root creation path through a file where a directory is required.
        // Restore the caller's environment before returning so this remains a self-contained probe.
        const std::filesystem::path blocker =
            std::filesystem::temp_directory_path() / "cna-demos-storage-device-not-connected";
        std::error_code ignored;
        std::filesystem::remove_all(blocker, ignored);
        {
            std::ofstream file(blocker);
            if (!file)
            {
                throw std::runtime_error("could not create the storage-device blocker file");
            }
        }
        const char* configuredRoot = std::getenv("XDG_DATA_HOME");
        const std::optional<std::string> previousRoot =
            configuredRoot != nullptr ? std::optional<std::string>(configuredRoot) : std::nullopt;
        if (setenv("XDG_DATA_HOME", blocker.c_str(), 1) != 0)
        {
            std::filesystem::remove(blocker, ignored);
            throw std::runtime_error("could not override XDG_DATA_HOME");
        }

        bool unavailableRootReported = false;
        bool unavailableRootSerialized = false;
        std::exception_ptr unexpected;
        try
        {
            StorageDevice::SetAppNameEXT("unavailable-storage-probe");
        }
        catch (const StorageDeviceNotConnectedException& error)
        {
            unavailableRootReported = std::string(error.what()) == "Unable to create the storage directory." &&
                error.getInnerExceptionProperty() != nullptr;
            System::Runtime::Serialization::SerializationInfo info;
            const System::Runtime::Serialization::StreamingContext context;
            error.GetObjectData(info, context);
            unavailableRootSerialized = info.getMemberCountProperty() == 2 &&
                info.GetString("Message") == "Unable to create the storage directory." &&
                info.GetValue<std::exception_ptr>("InnerException") != nullptr;
        }
        catch (...)
        {
            unexpected = std::current_exception();
        }

        if (previousRoot)
            setenv("XDG_DATA_HOME", previousRoot->c_str(), 1);
        else
            unsetenv("XDG_DATA_HOME");
        std::filesystem::remove(blocker, ignored);
        StorageDevice::SetAppNameEXT("cna-demos");
        if (unexpected)
        {
            std::rethrow_exception(unexpected);
        }
        ctx.Check(unavailableRootReported,
                  "an unusable storage root raises StorageDeviceNotConnectedException with its filesystem cause");
        ctx.Check(unavailableRootSerialized,
                  "GetObjectData preserves the real disconnected exception's message and inner cause");
#else
        // CNAEXT: the folder this program's saves live in (the default is "game").
        StorageDevice::SetAppNameEXT("cna-demos");
#endif

        // On an Xbox this showed a picker (hard drive or memory unit) and finished later, so the
        // API is asynchronous: Begin..., a callback or polling IsCompleted, then End....
        // On PC there is one device and the call completes at once -- the pattern stays.
        // The size and directory count say how much room the game will need.
        int selectorState = 91;
        bool callbackRan = false;
        bool callbackSawState = false;
        const std::unique_ptr<System::IAsyncResult> result = StorageDevice::BeginShowSelector(
            64 * 1024, 1,
            [&callbackRan, &callbackSawState, &selectorState](System::IAsyncResult* callbackResult) {
                callbackRan = true;
                callbackSawState =
                    std::any_cast<void*>(callbackResult->getAsyncStateProperty()) == &selectorState;
            },
            &selectorState);
        ctx.Print("IsCompleted {}, CompletedSynchronously {}", result->getIsCompletedProperty(),
                  result->getCompletedSynchronouslyProperty());
        ctx.Check(result->getIsCompletedProperty() && callbackRan,
                  "on this platform the selector completes at once and calls back");
        ctx.Check(result->getCompletedSynchronouslyProperty() && callbackSawState &&
                      std::any_cast<void*>(result->getAsyncStateProperty()) == &selectorState &&
                      result->getAsyncWaitHandleProperty().WaitOne(0),
                  "the selector preserves AsyncState and exposes a signalled completion handle");

        const std::unique_ptr<StorageDevice> device = StorageDevice::EndShowSelector(result.get());
        ctx.Check(device != nullptr && device->getIsConnectedProperty(), "EndShowSelector returns a connected device");

        const long long free = device->getFreeSpaceProperty();
        const long long total = device->getTotalSpaceProperty();
        ctx.Print("FreeSpace {} MiB of TotalSpace {} MiB", free / (1024 * 1024), total / (1024 * 1024));
        ctx.Check(free > 0 && free <= total, "FreeSpace is positive and at most TotalSpace");

        // Genuine XNA public-call oracle: docs/repair-evidence/storage/StorageDeviceAsyncOracle.cs.
        bool invalidCallback = false;
        ctx.Check(ThrowsExactly<System::ArgumentOutOfRangeException>([&] {
            (void)StorageDevice::BeginShowSelector(-1, 0,
                [&](System::IAsyncResult*) { invalidCallback = true; }, nullptr);
        }) && !invalidCallback, "negative size is refused before invoking the selector callback");
        ctx.Check(ThrowsExactly<System::ArgumentOutOfRangeException>([] {
            (void)StorageDevice::BeginShowSelector(static_cast<Microsoft::Xna::Framework::PlayerIndex>(4),
                nullptr, nullptr);
        }), "invalid player is refused by the short selector overload");
        ctx.Check(ThrowsExactly<System::InvalidOperationException>([&] {
            (void)StorageDevice::EndShowSelector(result.get());
        }), "a selector result can only be ended once");

        auto freshSelector = StorageDevice::BeginShowSelector(nullptr, nullptr);
        const bool wrongKind = ThrowsExactly<System::ArgumentNullException>([&] {
            (void)device->EndOpenContainer(freshSelector.get());
        });
        auto otherDevice = StorageDevice::EndShowSelector(freshSelector.get());
        ctx.Check(wrongKind && otherDevice != nullptr &&
            ThrowsExactly<System::ArgumentNullException>([] { (void)StorageDevice::EndShowSelector(nullptr); }),
            "null and wrong-kind results use ArgumentNullException without consuming another operation");
        auto empty = device->BeginOpenContainer("", nullptr, nullptr);
        const bool emptyName = ThrowsExactly<System::ArgumentNullException>([&] {
            (void)device->EndOpenContainer(empty.get());
        });
        ctx.Check(emptyName && ThrowsExactly<System::InvalidOperationException>([&] {
            (void)device->EndOpenContainer(empty.get());
        }), "empty-name validation occurs in End and a failed End still consumes the result");
        auto wrongOwner = device->BeginOpenContainer("", nullptr, nullptr);
        const bool wrongDevice = ThrowsExactly<System::ArgumentException>([&] {
            (void)otherDevice->EndOpenContainer(wrongOwner.get());
        });
        ctx.Check(wrongDevice && ThrowsExactly<System::InvalidOperationException>([&] {
            (void)device->EndOpenContainer(wrongOwner.get());
        }), "wrong-owner refusal precedes name validation and consumes the container result");
    }
}

CNA_DEMOS_CONSOLE(ShowSelector, {
    .id = "storage.storage_device.show_selector",
    .title = "Choose a storage device",
    .description = "Gets a StorageDevice with BeginShowSelector (asking for 64 KiB and one directory), a callback and "
                   "EndShowSelector, reads IsConnected, FreeSpace and TotalSpace, and on Linux proves the exact "
                   "failure reported for an unusable storage root.",
    .apis = {"StorageDevice::BeginShowSelector", "StorageDevice::EndShowSelector", "StorageDevice::IsConnected",
             "StorageDevice::FreeSpace", "StorageDevice::TotalSpace", "StorageDevice::SetAppNameEXT",
             "StorageDeviceNotConnectedException::StorageDeviceNotConnectedException",
             "StorageDeviceNotConnectedException::GetObjectData"},
    .tags = {"storage", "save", "device", "async", "selector"},
    .ownProcess = true,
})
