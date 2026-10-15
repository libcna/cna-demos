// SPDX-License-Identifier: MS-PL
// DXT1 cube regions use padded 4x4 blocks, including sub-4x4 mip levels.
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <utility>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "System/ArgumentException.hpp"
#include "System/NotSupportedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    using Block = std::array<std::uint8_t, 8>;
    using Face = std::array<std::uint8_t, 32>;
    // Original opaque DXT1 block: authored RGB565 endpoint 0, black endpoint 1, all selectors zero.
    Block SolidBlock(std::uint16_t rgb565)
    {
        return {static_cast<std::uint8_t>(rgb565), static_cast<std::uint8_t>(rgb565 >> 8),
                0, 0, 0, 0, 0, 0};
    }

    using AlphaBlock = std::array<std::uint8_t, 16>;

    AlphaBlock SolidAlphaBlock(SurfaceFormat format, std::uint16_t rgb565)
    {
        AlphaBlock block{};
        if (format == SurfaceFormat::Dxt3)
        {
            // Sixteen explicit four-bit alpha values, all opaque.
            std::fill_n(block.begin(), 8, std::uint8_t{0xff});
        }
        else
        {
            // Equal opaque endpoints plus selectors that all choose endpoint zero.
            block[0] = 0xff;
            block[1] = 0xff;
        }
        const Block color = SolidBlock(rgb565);
        std::copy(color.begin(), color.end(), block.begin() + 8);
        return block;
    }

    class CompressedCubeDemo final : public CnaDemos::DemoGame
    {
        void LoadContent() override
        {
            std::optional<TextureCube> cube;
            try { cube.emplace(getGraphicsDeviceProperty(), 8, true, SurfaceFormat::Dxt1); }
            catch (const System::NotSupportedException&)
            { Context().Skip("the live device cannot create the declared Dxt1 cube format"); }
            if (!cube) return;
            Context().Check(cube->getSizeProperty() == 8 && cube->getLevelCountProperty() == 4 &&
                            cube->getFormatProperty() == SurfaceFormat::Dxt1,
                            "an 8x8 DXT1 cube exposes 8x8 4x4 2x2 and 1x1 levels");
            const std::array<std::uint16_t, 6> endpoints{0xf800, 0x07e0, 0x001f, 0xffe0, 0x07ff, 0xf81f};
            std::array<Face, 6> expected;
            bool fullFaces = true;
            for (std::size_t face = 0; face < expected.size(); ++face)
            {
                const Block block = SolidBlock(endpoints[face]);
                for (int index = 0; index < 4; ++index)
                    std::copy(block.begin(), block.end(), expected[face].begin() + index * 8);
                cube->SetData(static_cast<CubeMapFace>(face), expected[face].data(), 32);
                Face observed{};
                cube->GetData(static_cast<CubeMapFace>(face), observed.data(), 32);
                fullFaces = fullFaces && observed == expected[face];
            }
            Context().Check(fullFaces, "each 8x8 face transfers exactly four authored eight-byte blocks");

            const Rectangle topRight(4, 0, 4, 4);
            const Block patch = SolidBlock(0x07e0);
            std::array<std::uint8_t, 10> source;
            source.fill(0xa5);
            std::copy(patch.begin(), patch.end(), source.begin() + 1);
            const auto originalSource = source;
            cube->SetData(CubeMapFace::PositiveY, 0, &topRight, source.data(), 1, 8);
            std::copy(patch.begin(), patch.end(), expected[2].begin() + 8);
            std::array<std::uint8_t, 12> destination, expectedDestination;
            destination.fill(0x5a); expectedDestination = destination;
            std::copy(patch.begin(), patch.end(), expectedDestination.begin() + 2);
            cube->GetData(CubeMapFace::PositiveY, 0, &topRight, destination.data(), 2, 8);
            Context().Check(destination == expectedDestination && source == originalSource,
                            "a 4x4 region transfers eight bytes through independent guarded source and destination windows");

            std::array<Block, 3> smallMips{SolidBlock(0xffe0), SolidBlock(0xf81f), SolidBlock(0x07ff)};
            bool smallLevels = true;
            for (int level = 1; level < 4; ++level)
            {
                cube->SetData(CubeMapFace::PositiveY, level, nullptr, smallMips[level - 1].data(), 0, 8);
                Block observed{};
                cube->GetData(CubeMapFace::PositiveY, level, nullptr, observed.data(), 0, 8);
                smallLevels = smallLevels && observed == smallMips[level - 1];
            }
            Context().Check(smallLevels, "4x4 2x2 and 1x1 mip faces each retain one complete padded DXT1 block");
            Context().Print("DXT1 cube bytes: base face=32; mip sizes 4,2,1 each use 8; patched +Y rectangle=(4,0,4,4).");

            const Block replacement = SolidBlock(0xf800);
            bool shortWrite = false;
            try { cube->SetData(CubeMapFace::PositiveY, 0, &topRight, replacement.data(), 0, 7); }
            catch (const System::ArgumentException&) { shortWrite = true; }
            Context().Check(shortWrite, "a seven-byte logical write cannot replace a complete eight-byte DXT1 block");
            const Rectangle misaligned(1, 0, 4, 4);
            bool refusedWrite = false;
            try { cube->SetData(CubeMapFace::PositiveY, 0, &misaligned, replacement.data(), 0, 8); }
            catch (const System::ArgumentException&) { refusedWrite = true; }
            Context().Check(refusedWrite, "a compressed write whose x origin is not block-aligned is refused");
            destination.fill(0x5a); expectedDestination = destination;
            bool refusedRead = false;
            try { cube->GetData(CubeMapFace::PositiveY, 0, &misaligned, destination.data(), 2, 8); }
            catch (const System::ArgumentException&) { refusedRead = true; }
            Context().Check(refusedRead && destination == expectedDestination,
                            "a refused misaligned compressed read leaves every destination and guard byte unchanged");

            bool isolated = true;
            for (std::size_t face = 0; face < expected.size(); ++face)
            {
                Face observed{};
                cube->GetData(static_cast<CubeMapFace>(face), observed.data(), 32);
                isolated = isolated && observed == expected[face];
            }
            for (int level = 1; level < 4; ++level)
            {
                Block observed{};
                cube->GetData(CubeMapFace::PositiveY, level, nullptr, observed.data(), 0, 8);
                isolated = isolated && observed == smallMips[level - 1];
            }
            Context().Check(isolated, "the patch and refused transfers preserve neighboring blocks other faces and authored small mips");

            bool alphaFormats = true;
            for (const auto& [format, endpoint] :
                 {std::pair{SurfaceFormat::Dxt3, std::uint16_t{0x07e0}},
                  std::pair{SurfaceFormat::Dxt5, std::uint16_t{0x001f}}})
            {
                std::optional<TextureCube> alphaCube;
                try { alphaCube.emplace(getGraphicsDeviceProperty(), 4, false, format); }
                catch (const System::NotSupportedException&)
                {
                    Context().Skip("the live device cannot create every declared DXT cube format");
                    return;
                }
                const AlphaBlock authored = SolidAlphaBlock(format, endpoint);
                AlphaBlock observed{};
                alphaCube->SetData(CubeMapFace::PositiveX, authored.data(), 16);
                alphaCube->GetData(CubeMapFace::PositiveX, observed.data(), 16);
                alphaFormats = alphaFormats && alphaCube->getFormatProperty() == format &&
                               Texture::GetFormatSizeEXT(format) == 16 && observed == authored;
            }
            Context().Check(alphaFormats,
                            "Dxt3 and Dxt5 cubes retain their distinct sixteen-byte alpha-plus-color blocks exactly");
        }
    };
}

CNA_DEMOS_GAME(CompressedCubeDemo, {
    .id = "graphics.texture_cube.compressed_blocks",
    .title = "Compressed cube block windows and small mips",
    .description = "Transfers original DXT1 cube blocks through guarded byte windows, authors padded small mip levels, "
                   "checks exact DXT3/DXT5 alpha-plus-color blocks, and verifies refused transfers preserve all data.",
    .apis = {"TextureCube::TextureCube", "TextureCube::SetData", "TextureCube::GetData", "Texture::LevelCount", "Texture::Format",
             "TextureCube::Size", "SurfaceFormat::Dxt1", "SurfaceFormat::Dxt3", "SurfaceFormat::Dxt5"},
    .tags = {"texture", "cube", "compression", "mipmap", "array"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Requires Dxt1, Dxt3 and Dxt5 cube creation; only constructor capability refusal is skipped. "
                    "Checks public compressed bytes, not direct GPU readback, cube sampling or DDS decoding.",
})
