// SPDX-License-Identifier: MS-PL
// DDS cube decoding across complete, offset, short-read and truncated streams.
#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/FormatException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace System::IO;

namespace
{
    // Original RGB24 DDS: six faces, independently colored 4x4/2x2/1x1 levels.
    // The final blue byte is 255, so silently zero-padding a truncated input changes a texel.
    std::vector<bytecs> CubeDds()
    {
        std::vector<bytecs> bytes(128, 0);
        bytes[0] = 'D'; bytes[1] = 'D'; bytes[2] = 'S'; bytes[3] = ' ';
        const auto word = [&bytes](int offset, std::uint32_t value) {
            for (int i = 0; i < 4; ++i) bytes[offset + i] = static_cast<bytecs>(value >> (8 * i));
        };
        word(4, 124); word(8, 0x2100f); word(12, 4); word(16, 4); word(20, 12);
        word(28, 3); word(76, 32); word(80, 0x40); word(88, 24);
        word(92, 0xff); word(96, 0xff00); word(100, 0xff0000);
        word(108, 0x401008); word(112, 0xfe00);
        const std::array<std::array<bytecs, 3>, 6> colors{{
            {255, 0, 0}, {0, 255, 255}, {0, 0, 255},
            {255, 255, 0}, {0, 255, 0}, {255, 0, 255}}};
        for (int face = 0; face < 6; ++face)
            for (int level = 0; level < 3; ++level)
                for (int texel = 0; texel < (4 >> level) * (4 >> level); ++texel)
                    bytes.insert(bytes.end(), colors[(face + level) % 6].begin(), colors[(face + level) % 6].end());
        return bytes;
    }

    class ChunkedStream final : public MemoryStream
    {
    public:
        using MemoryStream::MemoryStream;
        intcs Read(bytecs buffer[], intcs offset, intcs count) override
        {
            ++calls;
            return MemoryStream::Read(buffer, offset, std::min<intcs>(count, 7));
        }
        int calls = 0;
    };

    bool Matches(TextureCube& cube)
    {
        if (cube.getSizeProperty() != 4 || cube.getLevelCountProperty() != 3 ||
            cube.getFormatProperty() != SurfaceFormat::Color) return false;
        const std::array<Color, 6> colors{Color::Red, Color::Cyan, Color::Blue,
                                         Color::Yellow, Color::Lime, Color::Magenta};
        for (int face = 0; face < 6; ++face)
            for (int level = 0; level < 3; ++level)
            {
                std::vector<Color> pixels((4 >> level) * (4 >> level), Color::Orange);
                cube.GetData(static_cast<CubeMapFace>(face), level, nullptr, pixels.data(), 0,
                             static_cast<int>(pixels.size()));
                if (!std::all_of(pixels.begin(), pixels.end(), [&](const Color& pixel) {
                    return pixel == colors[(face + level) % 6];
                })) return false;
            }
        return true;
    }

    class DdsStreamsDemo final : public CnaDemos::DemoGame
    {
        void LoadContent() override
        {
            auto bytes = CubeDds();
            Context().Print("DDS fixture: {} bytes, six faces with authored 4x4/2x2/1x1 levels", bytes.size());
            auto& device = getGraphicsDeviceProperty();
            MemoryStream complete(bytes.data(), static_cast<intcs>(bytes.size()), false);
            auto cube = TextureCube::DDSFromStreamEXT(device, complete);
            Context().Check(Matches(cube), "RGB24 DDS decodes every face and authored mip to opaque Color");

            std::vector<bytecs> prefixed(16, 0x5a);
            prefixed.insert(prefixed.end(), bytes.begin(), bytes.end());
            MemoryStream offset(prefixed.data(), static_cast<intcs>(prefixed.size()), false);
            offset.setPositionProperty(16);
            auto offsetCube = TextureCube::DDSFromStreamEXT(device, offset);
            Context().Check(Matches(offsetCube), "DDS decoding begins at the current stream position");

            ChunkedStream chunks(bytes.data(), static_cast<intcs>(bytes.size()), false);
            bool chunkedMatches = false;
            try { auto chunkCube = TextureCube::DDSFromStreamEXT(device, chunks); chunkedMatches = Matches(chunkCube); }
            catch (const std::exception& error) { Context().Print("Chunked stream refused: {}", error.what()); }
            Context().Print("Chunked stream Read calls: {}", chunks.calls);
            Context().Check(chunkedMatches && chunks.calls > 1,
                            "short successful reads are accumulated until the DDS payload is complete");

            // Both inputs lose the same final nonzero byte; prefix size must not mask truncation.
            bytes.pop_back(); prefixed.pop_back();
            for (int prefix : {0, 16})
            {
                const auto& data = prefix == 0 ? bytes : prefixed;
                MemoryStream truncated(data.data(), static_cast<intcs>(data.size()), false);
                truncated.setPositionProperty(prefix);
                bool refused = false;
                try {
                    auto invalid = TextureCube::DDSFromStreamEXT(device, truncated);
                    Context().Print("Truncated DDS at offset {} accepted; matches full fixture: {}", prefix, Matches(invalid));
                }
                catch (const System::FormatException& error) {
                    refused = true; Context().Print("Truncated DDS at offset {} refused: {}", prefix, error.what());
                }
                Context().Check(refused, "truncated DDS is refused at stream offset " + std::to_string(prefix));
            }
        }
        void Draw(const GameTime& time) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            DemoGame::Draw(time);
        }
    };
}

CNA_DEMOS_GAME(DdsStreamsDemo, {
    .id = "graphics.texture_cube.dds_streams",
    .title = "DDS cube stream boundaries",
    .description = "Decodes an original RGB24 cube with authored mip levels, then checks current-position, short-read "
                   "and truncated-input stream behavior.",
    .apis = {"TextureCube::DDSFromStreamEXT", "TextureCube::GetData", "TextureCube::Size", "Texture::LevelCount"},
    .tags = {"texture", "cubemap", "dds", "stream", "mipmap"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Reads all decoded faces and mips through public GetData; no cube sampling or image-comparison claim.",
})
