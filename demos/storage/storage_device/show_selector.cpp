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

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDeviceNotConnectedException.hpp"
#include "System/IAsyncResult.hpp"

using Microsoft::Xna::Framework::Storage::StorageDevice;
using Microsoft::Xna::Framework::Storage::StorageDeviceNotConnectedException;

namespace
{
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
        std::exception_ptr unexpected;
        try
        {
            StorageDevice::SetAppNameEXT("unavailable-storage-probe");
        }
        catch (const StorageDeviceNotConnectedException& error)
        {
            unavailableRootReported = std::string(error.what()) == "Unable to create the storage directory." &&
                error.getInnerExceptionProperty() != nullptr;
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
#else
        // CNAEXT: the folder this program's saves live in (the default is "game").
        StorageDevice::SetAppNameEXT("cna-demos");
#endif

        // On an Xbox this showed a picker (hard drive or memory unit) and finished later, so the
        // API is asynchronous: Begin..., a callback or polling IsCompleted, then End....
        // On PC there is one device and the call completes at once -- the pattern stays.
        // The size and directory count say how much room the game will need.
        bool callbackRan = false;
        const std::unique_ptr<System::IAsyncResult> result = StorageDevice::BeginShowSelector(
            64 * 1024, 1, [&callbackRan](System::IAsyncResult*) { callbackRan = true; }, nullptr);
        ctx.Print("IsCompleted {}, CompletedSynchronously {}", result->getIsCompletedProperty(),
                  result->getCompletedSynchronouslyProperty());
        ctx.Check(result->getIsCompletedProperty() && callbackRan,
                  "on this platform the selector completes at once and calls back");

        const std::unique_ptr<StorageDevice> device = StorageDevice::EndShowSelector(result.get());
        ctx.Check(device != nullptr && device->getIsConnectedProperty(), "EndShowSelector returns a connected device");

        const long long free = device->getFreeSpaceProperty();
        const long long total = device->getTotalSpaceProperty();
        ctx.Print("FreeSpace {} MiB of TotalSpace {} MiB", free / (1024 * 1024), total / (1024 * 1024));
        ctx.Check(free > 0 && free <= total, "FreeSpace is positive and at most TotalSpace");
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
             "StorageDeviceNotConnectedException::StorageDeviceNotConnectedException"},
    .tags = {"storage", "save", "device", "async", "selector"},
    .ownProcess = true,
})
