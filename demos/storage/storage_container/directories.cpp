// SPDX-License-Identifier: MS-PL
// Directories and file listings inside a StorageContainer, with search patterns.
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageContainer.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "System/IAsyncResult.hpp"
#include "System/IO/IOException.hpp"
#include "System/IO/DirectoryNotFoundException.hpp"
#include "System/IO/Stream.hpp"
#include "System/UnauthorizedAccessException.hpp"

using namespace Microsoft::Xna::Framework::Storage;

namespace
{
    std::string Join(std::vector<std::string> names)
    {
        std::sort(names.begin(), names.end());  // the order of a listing is not defined
        std::string text;
        for (const std::string& name : names)
            text += (text.empty() ? "" : ", ") + name;
        return text;
    }

    void Directories(CnaDemos::DemoContext& ctx)
    {
        StorageDevice::SetAppNameEXT("cna-demos");
        const std::unique_ptr<System::IAsyncResult> selector = StorageDevice::BeginShowSelector(nullptr, nullptr);
        const std::unique_ptr<StorageDevice> device = StorageDevice::EndShowSelector(selector.get());
        {
            const std::unique_ptr<System::IAsyncResult> opening = device->BeginOpenContainer("cna-demos directories", nullptr, nullptr);
            const std::unique_ptr<StorageContainer> container = device->EndOpenContainer(opening.get());

            // Paths inside a container are relative; "/" separates directories.
            container->CreateDirectory("replays");
            container->CreateDirectory("screenshots");
            ctx.Check(container->DirectoryExists("replays"), "DirectoryExists sees the new directory");
            for (const char* file : {"slot1.sav", "slot2.sav", "settings.cfg", "replays/race1.rpl"})
                (void)container->CreateFile(file);  // an empty file is enough here

            ctx.Print("directories: {}", Join(container->GetDirectoryNames()));
            ctx.Print("files: {}", Join(container->GetFileNames()));
            ctx.Print("files matching *.sav: {}", Join(container->GetFileNames("*.sav")));
            ctx.Check(Join(container->GetFileNames()) == "settings.cfg, slot1.sav, slot2.sav",
                      "GetFileNames lists the files of the container's top level only");
            ctx.Check(Join(container->GetFileNames("*.sav")) == "slot1.sav, slot2.sav",
                      "a search pattern filters the names, * matching any text");
            ctx.Check(Join(container->GetDirectoryNames("s*")) == "screenshots", "directory names can be filtered too");
            ctx.Check(container->FileExists("replays/race1.rpl"), "files in subdirectories are addressed by path");

            bool existingFileRefused = false;
            try { container->CreateDirectory("settings.cfg"); }
            catch (const System::IO::IOException&) { existingFileRefused = true; }
            ctx.Check(existingFileRefused, "CreateDirectory refuses an existing file with IOException");
            ctx.Check(container->FileExists("settings.cfg"), "CreateDirectory preserves the existing file");
            bool fileParentRefused = false;
            try { container->CreateDirectory("settings.cfg/child"); }
            catch (const System::IO::IOException&) { fileParentRefused = true; }
            ctx.Check(fileParentRefused, "CreateDirectory refuses a file used as a parent with IOException");

            // DeleteFile must never substitute a directory deletion (ISSUE-010).
            bool directoryRefused = false;
            try { container->DeleteFile("screenshots"); }
            catch (const System::UnauthorizedAccessException&) { directoryRefused = true; }
            ctx.Check(directoryRefused, "DeleteFile refuses a directory with UnauthorizedAccessException");
            ctx.Check(container->DirectoryExists("screenshots"), "DeleteFile preserves an empty directory");
            bool parentRefused = false;
            try { container->DeleteFile("missing-parent/save.bin"); }
            catch (const System::IO::DirectoryNotFoundException&) { parentRefused = true; }
            ctx.Check(parentRefused, "DeleteFile distinguishes a missing parent from a missing file");
            container->DeleteFile("missing-file.bin");
            ctx.Check(!container->FileExists("missing-file.bin"), "DeleteFile accepts a missing file in an existing directory");

            // A directory must be empty before it can be deleted.
            bool refused = false;
            try
            {
                container->DeleteDirectory("replays");
            }
            catch (const std::exception& e)
            {
                refused = true;
                ctx.Print("DeleteDirectory on a non-empty directory: {}", e.what());
            }
            ctx.Check(refused, "deleting a directory that still holds a file is refused");
            container->DeleteFile("replays/race1.rpl");
            container->DeleteDirectory("replays");
            ctx.Check(!container->DirectoryExists("replays"), "after DeleteFile the directory can be deleted");
        }
        device->DeleteContainer("cna-demos directories");
        {
            auto opening = device->BeginOpenContainer("cna-demos vanished directories", nullptr, nullptr);
            auto container = device->EndOpenContainer(opening.get());
            device->DeleteContainer("cna-demos vanished directories");
            const auto missingRoot = [&](const auto& action, const char* message) {
                bool refused = false;
                try { action(); }
                catch (const System::IO::DirectoryNotFoundException&) { refused = true; }
                ctx.Check(refused, message);
            };
            missingRoot([&] { (void)container->GetFileNames(); },
                        "GetFileNames reports a vanished root with DirectoryNotFoundException");
            missingRoot([&] { (void)container->GetFileNames("*"); },
                        "filtered GetFileNames reports a vanished root with DirectoryNotFoundException");
            missingRoot([&] { (void)container->GetDirectoryNames(); },
                        "GetDirectoryNames reports a vanished root with DirectoryNotFoundException");
            missingRoot([&] { (void)container->GetDirectoryNames("*"); },
                        "filtered GetDirectoryNames reports a vanished root with DirectoryNotFoundException");
        }
    }
}

CNA_DEMOS_CONSOLE(Directories, {
    .id = "storage.storage_container.directories",
    .title = "Directories and file listings",
    .description = "Creates directories and files in a StorageContainer, lists them with GetDirectoryNames and "
                   "GetFileNames (with and without search patterns), and deletes a file and then its directory.",
    .apis = {"StorageContainer::CreateDirectory", "StorageContainer::DirectoryExists",
             "StorageContainer::DeleteDirectory", "StorageContainer::GetDirectoryNames",
             "StorageContainer::GetFileNames", "StorageContainer::CreateFile", "StorageContainer::DeleteFile",
             "StorageContainer::FileExists"},
    .tags = {"storage", "directory", "folder", "search", "file"},
})
