// SPDX-License-Identifier: MS-PL
// CNAEXT StorageDevice::SetAppNameEXT/GetStorageRootEXT: where saves live on disk, and the layout
// below that folder (container, then AllPlayers or PlayerN).
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageContainer.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "System/IAsyncResult.hpp"
#include "System/IO/Stream.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Storage;

namespace
{
    std::unique_ptr<StorageDevice> Device(std::optional<PlayerIndex> player)
    {
        const std::unique_ptr<System::IAsyncResult> result =
            player ? StorageDevice::BeginShowSelector(*player, nullptr, nullptr)
                   : StorageDevice::BeginShowSelector(nullptr, nullptr);
        return StorageDevice::EndShowSelector(result.get());
    }

    void Touch(StorageDevice& device, const std::string& container, const std::string& file)
    {
        const std::unique_ptr<System::IAsyncResult> opening = device.BeginOpenContainer(container, nullptr, nullptr);
        (void)device.EndOpenContainer(opening.get())->CreateFile(file);
    }

    void StorageRoot(CnaDemos::DemoContext& ctx)
    {
        // The application name picks the folder: on Linux $XDG_DATA_HOME/<name> (or
        // ~/.local/share/<name>), on Windows %LOCALAPPDATA%\<name>, on macOS
        // ~/Library/Application Support/<name>.
        StorageDevice::SetAppNameEXT("cna-demos");
        const std::filesystem::path root = StorageDevice::GetStorageRootEXT();
        ctx.Print("storage root: {}", root.string());
        ctx.Check(root.filename() == "cna-demos", "the storage root ends in the application name");

        // A device chosen for a player keeps that player's saves apart from the shared ones.
        const std::unique_ptr<StorageDevice> shared = Device(std::nullopt);
        const std::unique_ptr<StorageDevice> playerTwo = Device(PlayerIndex::Two);
        Touch(*shared, "cna-demos layout", "options.cfg");
        Touch(*playerTwo, "cna-demos layout", "profile.sav");

        const std::filesystem::path container = root / "cna-demos layout";
        for (const auto& entry : std::filesystem::recursive_directory_iterator(container))
            ctx.Print("  {}", std::filesystem::relative(entry.path(), root).generic_string());
        ctx.Check(std::filesystem::exists(container / "AllPlayers" / "options.cfg"),
                  "shared saves are in <root>/<container>/AllPlayers");
        ctx.Check(std::filesystem::exists(container / "Player2" / "profile.sav"),
                  "player two's saves are in <root>/<container>/Player2");

        shared->DeleteContainer("cna-demos layout");
        ctx.Check(!std::filesystem::exists(container), "DeleteContainer removes the container with every player's files");
    }
}

CNA_DEMOS_CONSOLE(StorageRoot, {
    .id = "storage.storage_device.storage_root",
    .title = "Where saves are stored (CNAEXT)",
    .description = "Names the save folder with StorageDevice::SetAppNameEXT, prints GetStorageRootEXT, and shows the "
                   "layout below it: a container per display name, with AllPlayers and Player2 folders for a shared "
                   "device and one selected for player two.",
    .apis = {"StorageDevice::SetAppNameEXT", "StorageDevice::GetStorageRootEXT", "StorageDevice::BeginShowSelector",
             "StorageDevice::EndShowSelector", "StorageDevice::DeleteContainer", "PlayerIndex",
             "StorageDevice::BeginOpenContainer", "StorageDevice::EndOpenContainer"},
    .tags = {"storage", "save", "folder", "path", "player", "cnaext"},
})
