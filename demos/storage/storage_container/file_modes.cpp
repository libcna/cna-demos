// SPDX-License-Identifier: MS-PL
// StorageContainer::OpenFile with FileMode and FileAccess: create, append, truncate, read-only.
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageContainer.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "System/IAsyncResult.hpp"
#include "System/IO/FileAccess.hpp"
#include "System/IO/FileMode.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/IO/IOException.hpp"
#include "System/IO/Stream.hpp"
#include "System/IO/StreamReader.hpp"
#include "System/IO/StreamWriter.hpp"

using namespace Microsoft::Xna::Framework::Storage;
using System::IO::FileAccess;
using System::IO::FileMode;

namespace
{
    std::string ReadAll(StorageContainer& container, const std::string& file)
    {
        const std::unique_ptr<System::IO::Stream> stream = container.OpenFile(file, FileMode::Open, FileAccess::Read);
        System::IO::StreamReader reader(stream.get());
        return reader.ReadToEnd();
    }

    void Append(StorageContainer& container, const std::string& file, FileMode mode, const std::string& line)
    {
        const std::unique_ptr<System::IO::Stream> stream = container.OpenFile(file, mode, FileAccess::Write);
        System::IO::StreamWriter writer(stream.get());
        writer.Write(line + "\n");
    }

    void FileModes(CnaDemos::DemoContext& ctx)
    {
        StorageDevice::SetAppNameEXT("cna-demos");
        const std::unique_ptr<System::IAsyncResult> selector = StorageDevice::BeginShowSelector(nullptr, nullptr);
        const std::unique_ptr<StorageDevice> device = StorageDevice::EndShowSelector(selector.get());
        {
            const std::unique_ptr<System::IAsyncResult> opening = device->BeginOpenContainer("cna-demos file modes", nullptr, nullptr);
            const std::unique_ptr<StorageContainer> container = device->EndOpenContainer(opening.get());

            Append(*container, "log.txt", FileMode::Create, "first");       // new or replaced
            Append(*container, "log.txt", FileMode::Append, "second");      // added at the end
            ctx.Print("after Create and Append: \"{}\"", ReadAll(*container, "log.txt"));
            ctx.Check(ReadAll(*container, "log.txt") == "first\nsecond\n", "Append adds to what Create wrote");

            bool refused = false;
            try
            {
                Append(*container, "log.txt", FileMode::CreateNew, "third");  // only for files that do not exist
            }
            catch (const System::IO::IOException& e)
            {
                refused = true;
                ctx.Print("CreateNew on an existing file: {}", e.what());
            }
            ctx.Check(refused, "CreateNew refuses an existing file with IOException");

            Append(*container, "log.txt", FileMode::Truncate, "fresh");     // emptied, then written
            ctx.Check(ReadAll(*container, "log.txt") == "fresh\n", "Truncate empties the file first");

            bool missing = false;
            try
            {
                (void)container->OpenFile("missing.txt", FileMode::Open);
            }
            catch (const System::IO::FileNotFoundException&)
            {
                missing = true;
            }
            ctx.Check(missing, "Open on a missing file throws FileNotFoundException");
            Append(*container, "missing.txt", FileMode::OpenOrCreate, "made");
            ctx.Check(container->FileExists("missing.txt"), "OpenOrCreate creates it");

            const std::unique_ptr<System::IO::Stream> readOnly =
                container->OpenFile("log.txt", FileMode::Open, FileAccess::Read);
            ctx.Check(readOnly->getCanReadProperty() && !readOnly->getCanWriteProperty(),
                      "FileAccess::Read gives a stream that can read but not write");
        }
        device->DeleteContainer("cna-demos file modes");
    }
}

CNA_DEMOS_CONSOLE(FileModes, {
    .id = "storage.storage_container.file_modes",
    .title = "File modes and access",
    .description = "Opens one file in a StorageContainer with each FileMode -- Create, Append, CreateNew (refused for "
                   "an existing file), Truncate, Open (missing file), OpenOrCreate -- and with FileAccess::Read.",
    .apis = {"StorageContainer::OpenFile", "StorageContainer::FileExists", "StorageDevice::DeleteContainer",
             "FileMode", "FileAccess", "StorageDevice::BeginOpenContainer", "StorageDevice::EndOpenContainer"},
    .tags = {"storage", "file", "append", "truncate", "text"},
})
