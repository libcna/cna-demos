// SPDX-License-Identifier: MS-PL
// A StorageContainer's lifetime: DisplayName, StorageDevice, the Disposing event and IsDisposed.
#include <any>
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageContainer.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "System/EventArgs.hpp"
#include "System/IAsyncResult.hpp"
#include "System/Object.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/IO/FileAccess.hpp"
#include "System/IO/FileMode.hpp"
#include "System/IO/FileShare.hpp"
#include "System/Threading/WaitHandle.hpp"

using namespace Microsoft::Xna::Framework::Storage;

namespace
{
    template <typename Action>
    bool ThrowsObjectDisposed(Action action)
    {
        try
        {
            action();
        }
        catch (const System::ObjectDisposedException&)
        {
            return true;
        }
        catch (...)
        {
        }
        return false;
    }

    void Lifetime(CnaDemos::DemoContext& ctx)
    {
        StorageDevice::SetAppNameEXT("cna-demos");
        const std::unique_ptr<System::IAsyncResult> selector = StorageDevice::BeginShowSelector(nullptr, nullptr);
        const std::unique_ptr<StorageDevice> device = StorageDevice::EndShowSelector(selector.get());

        int openState = 73;
        bool callbackRan = false;
        bool callbackSawState = false;
        const std::unique_ptr<System::IAsyncResult> opening = device->BeginOpenContainer(
            "cna-demos lifetime",
            [&callbackRan, &callbackSawState, &openState](System::IAsyncResult* result) {
                callbackRan = true;
                callbackSawState = std::any_cast<void*>(result->getAsyncStateProperty()) == &openState;
            },
            &openState);
        ctx.Check(callbackRan && callbackSawState && opening->getIsCompletedProperty() &&
                      opening->getCompletedSynchronouslyProperty() &&
                      std::any_cast<void*>(opening->getAsyncStateProperty()) == &openState &&
                      opening->getAsyncWaitHandleProperty().WaitOne(0),
                  "BeginOpenContainer completes synchronously with its callback, state and signalled wait handle");
        std::unique_ptr<StorageContainer> container = device->EndOpenContainer(opening.get());
        ctx.Check(container->getDisplayNameProperty() == "cna-demos lifetime", "DisplayName is the name it was opened with");
        ctx.Check(&container->getStorageDeviceProperty() == device.get(), "StorageDevice is the device that opened it");
        ctx.Check(container->GetTypeName() == "Microsoft.Xna.Framework.Storage.StorageContainer",
                  "GetTypeName returns the canonical storage-container identity");

        container->CreateDirectory("empty");
        (void)container->CreateFile("fixture.bin");

        // Disposing is raised once; IsDisposed is already true inside the callback.
        int disposing = 0;
        container->Disposing.Add([&disposing](System::Object*, const System::EventArgs&) { ++disposing; });
        ctx.Check(!container->getIsDisposedProperty(), "an open container is not disposed");
        container->Dispose();
        ctx.Check(container->getIsDisposedProperty() && disposing == 1, "Dispose raises Disposing and sets IsDisposed");
        container->Dispose();
        ctx.Check(disposing == 1, "a second Dispose does nothing");

        using System::IO::FileAccess;
        using System::IO::FileMode;
        using System::IO::FileShare;
        const bool displayNameDisposed = ThrowsObjectDisposed([&] { (void)container->getDisplayNameProperty(); });
        const bool storageDeviceDisposed = ThrowsObjectDisposed([&] { (void)container->getStorageDeviceProperty(); });
        ctx.Check(displayNameDisposed && storageDeviceDisposed,
                  "disposed container properties throw ObjectDisposedException");
        const bool createDirectoryDisposed =
            ThrowsObjectDisposed([&] { container->CreateDirectory("created-after-dispose"); });
        const bool directoryExistsDisposed = ThrowsObjectDisposed([&] { (void)container->DirectoryExists("empty"); });
        const bool deleteDirectoryDisposed = ThrowsObjectDisposed([&] { container->DeleteDirectory("empty"); });
        ctx.Check(createDirectoryDisposed && directoryExistsDisposed && deleteDirectoryDisposed,
                  "disposed directory mutation and queries throw ObjectDisposedException");
        const bool directoryNamesDisposed = ThrowsObjectDisposed([&] { (void)container->GetDirectoryNames(); });
        const bool filteredDirectoryNamesDisposed =
            ThrowsObjectDisposed([&] { (void)container->GetDirectoryNames("*"); });
        ctx.Check(directoryNamesDisposed && filteredDirectoryNamesDisposed,
                  "disposed directory listings throw ObjectDisposedException");
        const bool createFileDisposed =
            ThrowsObjectDisposed([&] { (void)container->CreateFile("created-after-dispose.bin"); });
        const bool fileExistsDisposed = ThrowsObjectDisposed([&] { (void)container->FileExists("fixture.bin"); });
        ctx.Check(createFileDisposed && fileExistsDisposed,
                  "disposed file creation and queries throw ObjectDisposedException");
        const bool fileNamesDisposed = ThrowsObjectDisposed([&] { (void)container->GetFileNames(); });
        const bool filteredFileNamesDisposed = ThrowsObjectDisposed([&] { (void)container->GetFileNames("*"); });
        ctx.Check(fileNamesDisposed && filteredFileNamesDisposed,
                  "disposed file listings throw ObjectDisposedException");
        const bool openFileDisposed =
            ThrowsObjectDisposed([&] { (void)container->OpenFile("fixture.bin", FileMode::Open); });
        const bool openFileAccessDisposed = ThrowsObjectDisposed(
            [&] { (void)container->OpenFile("fixture.bin", FileMode::Open, FileAccess::Read); });
        const bool openFileShareDisposed = ThrowsObjectDisposed([&] {
            (void)container->OpenFile("fixture.bin", FileMode::Open, FileAccess::Read, FileShare::Read);
        });
        ctx.Check(openFileDisposed && openFileAccessDisposed && openFileShareDisposed,
                  "every disposed OpenFile overload throws ObjectDisposedException");
        const bool deleteFileDisposed = ThrowsObjectDisposed([&] { container->DeleteFile("fixture.bin"); });
        ctx.Check(deleteFileDisposed,
                  "disposed file deletion throws ObjectDisposedException");

        const int disposedRefusals = static_cast<int>(displayNameDisposed) + static_cast<int>(storageDeviceDisposed) +
            static_cast<int>(createDirectoryDisposed) + static_cast<int>(directoryExistsDisposed) +
            static_cast<int>(deleteDirectoryDisposed) + static_cast<int>(directoryNamesDisposed) +
            static_cast<int>(filteredDirectoryNamesDisposed) + static_cast<int>(createFileDisposed) +
            static_cast<int>(fileExistsDisposed) + static_cast<int>(fileNamesDisposed) +
            static_cast<int>(filteredFileNamesDisposed) + static_cast<int>(openFileDisposed) +
            static_cast<int>(openFileAccessDisposed) + static_cast<int>(openFileShareDisposed) +
            static_cast<int>(deleteFileDisposed);
        ctx.Print("post-dispose ObjectDisposedException results: {}/15", disposedRefusals);

        container.reset();
        device->DeleteContainer("cna-demos lifetime");
    }
}

CNA_DEMOS_CONSOLE(Lifetime, {
    .id = "storage.storage_container.lifetime",
    .title = "Container lifetime",
    .description = "Opens a StorageContainer with the complete synchronous async-result contract, reads its identity "
                   "and ownership, and verifies one-shot disposal plus XNA's post-dispose operation boundary.",
    .apis = {"StorageContainer::DisplayName", "StorageContainer::StorageDevice", "StorageContainer::Disposing",
             "StorageContainer::IsDisposed", "StorageContainer::Dispose", "StorageContainer::GetTypeName",
             "StorageContainer::CreateDirectory", "StorageContainer::DirectoryExists",
             "StorageContainer::DeleteDirectory", "StorageContainer::GetDirectoryNames",
             "StorageContainer::CreateFile", "StorageContainer::FileExists", "StorageContainer::DeleteFile",
             "StorageContainer::GetFileNames", "StorageContainer::OpenFile",
             "StorageDevice::BeginOpenContainer", "StorageDevice::EndOpenContainer"},
    .tags = {"storage", "dispose", "event", "lifetime"},
})
