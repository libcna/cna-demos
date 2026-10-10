// SPDX-License-Identifier: MS-PL
// Save and load a game: a file in a StorageContainer, written and read with BinaryWriter/Reader.
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageContainer.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "System/IAsyncResult.hpp"
#include "System/IO/BinaryReader.hpp"
#include "System/IO/BinaryWriter.hpp"
#include "System/IO/FileMode.hpp"
#include "System/IO/Stream.hpp"

using namespace Microsoft::Xna::Framework::Storage;

namespace
{
    struct SaveGame
    {
        std::string player;
        int level = 0;
        float score = 0.0f;
        bool hardMode = false;
    };

    std::unique_ptr<StorageContainer> Open(StorageDevice& device, const std::string& name)
    {
        // Containers open asynchronously too (on an Xbox it could take a moment).
        const std::unique_ptr<System::IAsyncResult> result = device.BeginOpenContainer(name, nullptr, nullptr);
        return device.EndOpenContainer(result.get());
    }

    void SaveAndLoad(CnaDemos::DemoContext& ctx)
    {
        StorageDevice::SetAppNameEXT("cna-demos");
        const std::unique_ptr<System::IAsyncResult> selector = StorageDevice::BeginShowSelector(nullptr, nullptr);
        const std::unique_ptr<StorageDevice> device = StorageDevice::EndShowSelector(selector.get());

        const SaveGame saved{"Ada", 7, 12345.5f, true};
        {
            // CreateFile replaces an existing file. Disposing the container commits the save,
            // so keep it open only as long as needed -- XNA allows one open container at a time.
            const std::unique_ptr<StorageContainer> container = Open(*device, "cna-demos save games");
            const std::unique_ptr<System::IO::Stream> stream = container->CreateFile("slot1.sav");
            System::IO::BinaryWriter writer(stream.get());
            writer.Write(saved.player);
            writer.Write(static_cast<int>(saved.level));
            writer.Write(saved.score);
            writer.Write(saved.hardMode);
            writer.Flush();
            ctx.Print("saved {} bytes to slot1.sav", stream->getLengthProperty());
        }

        SaveGame loaded;
        {
            const std::unique_ptr<StorageContainer> container = Open(*device, "cna-demos save games");
            ctx.Check(container->FileExists("slot1.sav"), "the save file exists after reopening the container");
            ctx.Check(!container->FileExists("slot2.sav"), "an unused slot does not");
            const std::unique_ptr<System::IO::Stream> stream =
                container->OpenFile("slot1.sav", System::IO::FileMode::Open);
            System::IO::BinaryReader reader(stream.get());
            // Read in the order of writing: the file holds the values, not their names.
            loaded.player = reader.ReadString();
            loaded.level = reader.ReadInt32();
            loaded.score = reader.ReadSingle();
            loaded.hardMode = reader.ReadBoolean();
        }
        ctx.Print("loaded: {} on level {}, score {}, hard mode {}", loaded.player, loaded.level, loaded.score,
                  loaded.hardMode);
        ctx.Check(loaded.player == saved.player && loaded.level == saved.level && loaded.score == saved.score &&
                      loaded.hardMode == saved.hardMode,
                  "the loaded game equals the saved one");

        // Remove the demo's saves again (DeleteContainer deletes the whole container).
        device->DeleteContainer("cna-demos save games");
    }
}

CNA_DEMOS_CONSOLE(SaveAndLoad, {
    .id = "storage.storage_container.save_and_load",
    .title = "Save and load a game",
    .description = "Opens a StorageContainer with BeginOpenContainer/EndOpenContainer, writes a save game with "
                   "CreateFile and BinaryWriter, reopens it, checks FileExists and reads it back with OpenFile and "
                   "BinaryReader; the container is deleted at the end.",
    .apis = {"StorageDevice::BeginOpenContainer", "StorageDevice::EndOpenContainer", "StorageDevice::DeleteContainer",
             "StorageContainer::CreateFile", "StorageContainer::OpenFile", "StorageContainer::FileExists",
             "StorageContainer::Dispose"},
    .tags = {"storage", "save", "load", "savegame", "file", "binary"},
})
