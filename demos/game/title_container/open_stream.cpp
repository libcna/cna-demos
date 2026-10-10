// SPDX-License-Identifier: MS-PL
// Reading a file shipped with the game: TitleContainer::OpenStream.
#include <array>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "System/IO/Stream.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void TitleContainerOpenStream(CnaDemos::DemoContext& ctx)
    {
        // Paths are relative to the title's install folder (next to the executable), read-only.
        const std::unique_ptr<System::IO::Stream> stream = TitleContainer::OpenStream("assets/textures/checker.png");
        ctx.Print("opened assets/textures/checker.png: {} bytes", stream->getLengthProperty());

        std::array<std::uint8_t, 8> header{};
        const int read = stream->Read(header.data(), 0, static_cast<int>(header.size()));
        ctx.Print("first bytes: {:02X} {:02X} {:02X} {:02X}", header[0], header[1], header[2], header[3]);

        ctx.Check(read == 8, "eight bytes were read");
        ctx.Check(header[1] == 'P' && header[2] == 'N' && header[3] == 'G', "the file starts with the PNG signature");
        ctx.Check(stream->getLengthProperty() > 8, "Length reports the file size");
    }
}

CNA_DEMOS_CONSOLE(TitleContainerOpenStream, {
    .id = "game.title_container.open_stream",
    .title = "Open a title file",
    .description = "Opens a file that ships with the game through TitleContainer::OpenStream and reads its first "
                   "bytes from the returned Stream.",
    .apis = {"TitleContainer::OpenStream"},
    .tags = {"file", "stream", "title"},
    .assets = {"textures/checker.png"},
})
