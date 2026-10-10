// SPDX-License-Identifier: MS-PL
// A StorageContainer's lifetime: DisplayName, StorageDevice, the Disposing event and IsDisposed.
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageContainer.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "System/EventArgs.hpp"
#include "System/IAsyncResult.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework::Storage;

namespace
{
    void Lifetime(CnaDemos::DemoContext& ctx)
    {
        StorageDevice::SetAppNameEXT("cna-demos");
        const std::unique_ptr<System::IAsyncResult> selector = StorageDevice::BeginShowSelector(nullptr, nullptr);
        const std::unique_ptr<StorageDevice> device = StorageDevice::EndShowSelector(selector.get());

        const std::unique_ptr<System::IAsyncResult> opening = device->BeginOpenContainer("cna-demos lifetime", nullptr, nullptr);
        std::unique_ptr<StorageContainer> container = device->EndOpenContainer(opening.get());
        ctx.Check(container->getDisplayNameProperty() == "cna-demos lifetime", "DisplayName is the name it was opened with");
        ctx.Check(&container->getStorageDeviceProperty() == device.get(), "StorageDevice is the device that opened it");

        // Disposing is raised once, before the container closes -- the last moment to write.
        int disposing = 0;
        container->Disposing.Add([&disposing](System::Object*, const System::EventArgs&) { ++disposing; });
        ctx.Check(!container->getIsDisposedProperty(), "an open container is not disposed");
        container->Dispose();
        ctx.Check(container->getIsDisposedProperty() && disposing == 1, "Dispose raises Disposing and sets IsDisposed");
        container->Dispose();
        ctx.Check(disposing == 1, "a second Dispose does nothing");

        container.reset();
        device->DeleteContainer("cna-demos lifetime");
    }
}

CNA_DEMOS_CONSOLE(Lifetime, {
    .id = "storage.storage_container.lifetime",
    .title = "Container lifetime",
    .description = "Opens a StorageContainer, reads DisplayName and StorageDevice, and shows that Dispose raises "
                   "the Disposing event once and sets IsDisposed.",
    .apis = {"StorageContainer::DisplayName", "StorageContainer::StorageDevice", "StorageContainer::Disposing",
             "StorageContainer::IsDisposed", "StorageContainer::Dispose"},
    .tags = {"storage", "dispose", "event", "lifetime"},
})
