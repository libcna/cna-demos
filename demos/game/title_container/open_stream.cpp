// SPDX-License-Identifier: MS-PL
// Reading a file shipped with the game: TitleContainer::OpenStream.
#include <array>
#include <filesystem>
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/IO/Stream.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class TitleStateRestorer final
    {
    public:
        TitleStateRestorer()
            : path_(TitleLocation::Path()), currentDirectory_(std::filesystem::current_path())
        {
        }

        ~TitleStateRestorer()
        {
            std::error_code ignored;
            std::filesystem::current_path(currentDirectory_, ignored);
            TitleLocation::setPathProperty(path_);
        }

        TitleStateRestorer(const TitleStateRestorer&) = delete;
        TitleStateRestorer& operator=(const TitleStateRestorer&) = delete;

    private:
        std::string path_;
        std::filesystem::path currentDirectory_;
    };

    void TitleContainerOpenStream(CnaDemos::DemoContext& ctx)
    {
        TitleStateRestorer restore;
        const std::string titlePath = TitleLocation::Path();

        // Paths are relative to the title's install folder (next to the executable), read-only.
        const std::unique_ptr<System::IO::Stream> stream = TitleContainer::OpenStream("assets/textures/checker.png");
        ctx.Print("opened assets/textures/checker.png: {} bytes", stream->getLengthProperty());

        std::array<std::uint8_t, 8> header{};
        const int read = stream->Read(header.data(), 0, static_cast<int>(header.size()));
        ctx.Print("first bytes: {:02X} {:02X} {:02X} {:02X}", header[0], header[1], header[2], header[3]);

        ctx.Check(read == 8, "eight bytes were read");
        ctx.Check(header[1] == 'P' && header[2] == 'N' && header[3] == 'G', "the file starts with the PNG signature");
        ctx.Check(stream->getLengthProperty() > 8, "Length reports the file size");
        ctx.Check(stream->getCanReadProperty() && stream->getCanSeekProperty() && !stream->getCanWriteProperty(),
                  "OpenStream returns a readable, seekable, read-only stream");

        // Resolution uses the title location, not the process working directory. Backslashes and
        // dot segments are accepted in XNA title-relative names.
        std::filesystem::current_path(std::filesystem::temp_directory_path());
        const auto backslash = TitleContainer::OpenStream("assets\\textures\\checker.png");
        const auto dotted = TitleContainer::OpenStream("assets/./textures/../textures/checker.png");
        ctx.Check(backslash->getLengthProperty() == stream->getLengthProperty() &&
                      dotted->getLengthProperty() == stream->getLengthProperty(),
                  "relative paths stay title-rooted and normalize separators and dot segments");

        // CNA exposes XNA's internal title location as a public helper; changing it immediately
        // changes the root used by OpenStream. The guard restores both pieces of process state.
        ctx.Check(&TitleLocation::Path() == &TitleLocation::getPathProperty() && !titlePath.empty(),
                  "the two title-location accessors expose the same initialized path");
        TitleLocation::setPathProperty(titlePath + "/assets");
        const auto relocated = TitleContainer::OpenStream("textures/checker.png");
        ctx.Check(relocated->getLengthProperty() == stream->getLengthProperty(),
                  "the CNA title-location setter changes relative OpenStream resolution");
        TitleLocation::setPathProperty(titlePath);

        bool absoluteRejected = false;
        try
        {
            (void)TitleContainer::OpenStream(titlePath + "/assets/textures/checker.png");
        }
        catch (const System::ArgumentException&)
        {
            absoluteRejected = true;
        }
        ctx.Check(absoluteRejected, "XNA rejects an absolute name with ArgumentException");

        bool missingUsesXnaException = false;
        try
        {
            (void)TitleContainer::OpenStream("assets/textures/cna-demos-missing.png");
        }
        catch (const System::IO::FileNotFoundException&)
        {
            missingUsesXnaException = true;
        }
        catch (const std::exception&)
        {
        }
        ctx.Check(missingUsesXnaException, "a missing title file throws FileNotFoundException");

        bool emptyRejected = false;
        try
        {
            (void)TitleContainer::OpenStream("");
        }
        catch (const System::ArgumentNullException&)
        {
            emptyRejected = true;
        }
        catch (const std::exception&)
        {
        }
        ctx.Check(emptyRejected, "XNA rejects an empty name with ArgumentNullException");
    }
}

CNA_DEMOS_CONSOLE(TitleContainerOpenStream, {
    .id = "game.title_container.open_stream",
    .title = "Open a title file",
    .description = "Opens a title-relative file through TitleContainer::OpenStream, inspects its read-only stream, "
                   "and exercises title-root resolution and path validation.",
    .apis = {"TitleContainer::OpenStream", "TitleLocation::Path", "TitleLocation::setPathProperty"},
    .tags = {"file", "stream", "title"},
    .assets = {"textures/checker.png"},
    .restrictions = "CNA-DEMOS-ISSUE-145 records absolute-name acceptance and non-XNA exceptions for missing or empty names.",
    .knownIssue = "CNA-DEMOS-ISSUE-145",
})
