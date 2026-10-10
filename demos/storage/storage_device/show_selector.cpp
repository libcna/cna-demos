// SPDX-License-Identifier: MS-PL
// StorageDevice::BeginShowSelector/EndShowSelector: XNA's asynchronous way to get a save device.
#include <any>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "System/IAsyncResult.hpp"

using Microsoft::Xna::Framework::Storage::StorageDevice;

namespace
{
    void ShowSelector(CnaDemos::DemoContext& ctx)
    {
        // CNAEXT: the folder this program's saves live in (the default is "game").
        StorageDevice::SetAppNameEXT("cna-demos");

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
                   "EndShowSelector, then reads IsConnected, FreeSpace and TotalSpace.",
    .apis = {"StorageDevice::BeginShowSelector", "StorageDevice::EndShowSelector", "StorageDevice::IsConnected",
             "StorageDevice::FreeSpace", "StorageDevice::TotalSpace", "StorageDevice::SetAppNameEXT"},
    .tags = {"storage", "save", "device", "async", "selector"},
})
