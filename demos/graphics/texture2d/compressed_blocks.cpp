// SPDX-License-Identifier: MS-PL
// Direct DXT Texture2D transfers use complete 4x4 blocks, including sub-4x4 mip levels.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/ArgumentException.hpp"
#include "System/NotSupportedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    using Block = std::array<std::uint8_t, 8>;
    using AlphaBlock = std::array<std::uint8_t, 16>;

    Block SolidBlock(std::uint16_t rgb565)
    {
        return {static_cast<std::uint8_t>(rgb565), static_cast<std::uint8_t>(rgb565 >> 8),
                0, 0, 0, 0, 0, 0};
    }

    AlphaBlock SolidAlphaBlock(SurfaceFormat format, std::uint8_t alphaCode,
                               std::uint16_t rgb565)
    {
        AlphaBlock block{};
        if (format == SurfaceFormat::Dxt3)
        {
            const std::uint8_t nibbles =
                static_cast<std::uint8_t>((alphaCode << 4) | alphaCode);
            std::fill_n(block.begin(), 8, nibbles);
        }
        else
        {
            block[0] = alphaCode;
            block[1] = alphaCode;
        }
        const Block color = SolidBlock(rgb565);
        std::copy(color.begin(), color.end(), block.begin() + 8);
        return block;
    }

    class CompressedBlocksDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            try
            {
                dxt1_ = std::make_unique<Texture2D>(device, 8, 8, true, SurfaceFormat::Dxt1);
                dxt3_ = std::make_unique<Texture2D>(device, 4, 4, false, SurfaceFormat::Dxt3);
                dxt5_ = std::make_unique<Texture2D>(device, 4, 4, false, SurfaceFormat::Dxt5);
            }
            catch (const System::NotSupportedException&)
            {
                Context().Skip("the live device cannot create every declared DXT texture format");
                return;
            }

            Context().Check(dxt1_->getWidthProperty() == 8 && dxt1_->getHeightProperty() == 8 &&
                                dxt1_->getLevelCountProperty() == 4 &&
                                dxt1_->getFormatProperty() == SurfaceFormat::Dxt1,
                            "an 8x8 DXT1 texture exposes 8x8 4x4 2x2 and 1x1 levels");

            bool npotRejected = false;
            try
            {
                Texture2D npot(device, 10, 6, false, SurfaceFormat::Dxt1);
            }
            catch (const System::ArgumentException&)
            {
                npotRejected = true;
            }
            catch (const System::NotSupportedException&)
            {
                // XNA validates the DXT dimensions before asking the device for format support.
            }
            Context().Check(npotRejected,
                            "DXT texture dimensions that are not multiples of four are rejected");

            const std::array<Block, 4> blocks{SolidBlock(0xf800), SolidBlock(0x07e0),
                                               SolidBlock(0x001f), SolidBlock(0xffe0)};
            std::array<std::uint8_t, 32> expected{};
            for (std::size_t index = 0; index < blocks.size(); ++index)
                std::copy(blocks[index].begin(), blocks[index].end(),
                          expected.begin() + static_cast<std::ptrdiff_t>(index * 8));
            dxt1_->SetData(expected.data(), static_cast<int>(expected.size()));
            std::array<std::uint8_t, 32> observed{};
            dxt1_->GetData(observed.data(), static_cast<int>(observed.size()));
            Context().Check(observed == expected,
                            "the 8x8 base level round-trips four authored DXT1 blocks in row order");

            const Rectangle bottomRight(4, 4, 4, 4);
            const Block patch = SolidBlock(0xf81f);
            std::array<std::uint8_t, 10> source{};
            source.fill(0xa5);
            std::copy(patch.begin(), patch.end(), source.begin() + 1);
            const auto originalSource = source;
            dxt1_->SetData(0, &bottomRight, source.data(), 1, 8);
            std::copy(patch.begin(), patch.end(), expected.begin() + 24);

            std::array<std::uint8_t, 12> destination{};
            destination.fill(0x5a);
            auto expectedDestination = destination;
            std::copy(patch.begin(), patch.end(), expectedDestination.begin() + 2);
            dxt1_->GetData(0, &bottomRight, destination.data(), 2, 8);
            Context().Check(source == originalSource && destination == expectedDestination,
                            "a 4x4 region transfers one block through guarded source and destination windows");

            const std::array<Block, 3> smallMips{SolidBlock(0x07ff), SolidBlock(0xffe0),
                                                  SolidBlock(0x001f)};
            bool smallLevels = true;
            for (int level = 1; level < 4; ++level)
            {
                dxt1_->SetData(level, nullptr, smallMips[level - 1].data(), 0, 8);
                Block mipObserved{};
                dxt1_->GetData(level, nullptr, mipObserved.data(), 0, 8);
                smallLevels = smallLevels && mipObserved == smallMips[level - 1];
            }
            Context().Check(smallLevels,
                            "4x4 2x2 and 1x1 mip levels each retain one complete padded DXT1 block");

            bool shortWriteRejected = false;
            try
            {
                const Block replacement = SolidBlock(0xffff);
                dxt1_->SetData(0, &bottomRight, replacement.data(), 0, 7);
            }
            catch (const System::ArgumentException&)
            {
                shortWriteRejected = true;
            }
            observed.fill(0);
            dxt1_->GetData(observed.data(), static_cast<int>(observed.size()));
            Context().Check(shortWriteRejected && observed == expected,
                            "a seven-byte write is refused before changing its block or any neighbor");

            const AlphaBlock dxt3Block = SolidAlphaBlock(SurfaceFormat::Dxt3, 8, 0x07e0);
            const AlphaBlock dxt5Block = SolidAlphaBlock(SurfaceFormat::Dxt5, 64, 0x001f);
            dxt3_->SetData(dxt3Block.data(), static_cast<int>(dxt3Block.size()));
            dxt5_->SetData(dxt5Block.data(), static_cast<int>(dxt5Block.size()));
            AlphaBlock dxt3Observed{}, dxt5Observed{};
            dxt3_->GetData(dxt3Observed.data(), static_cast<int>(dxt3Observed.size()));
            dxt5_->GetData(dxt5Observed.data(), static_cast<int>(dxt5Observed.size()));
            Context().Check(dxt3Observed == dxt3Block && dxt5Observed == dxt5Block,
                            "DXT3 and DXT5 retain their distinct sixteen-byte alpha-plus-color blocks exactly");

            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            Context().Print("DXT1 bytes: base=32; mip sizes 4,2,1 each use 8; region=(4,4,4,4).");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            if (!spriteBatch_)
            {
                Game::Draw(gameTime);
                return;
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque,
                                &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*dxt1_, Rectangle(32, 80, 320, 320), Color::White);
            spriteBatch_->Draw(*dxt3_, Rectangle(416, 80, 144, 144), Color::White);
            spriteBatch_->Draw(*dxt5_, Rectangle(608, 80, 144, 144), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const std::array<Color, 4> dxt1Pixels{pixels.At(112, 160), pixels.At(272, 160),
                                                       pixels.At(112, 320), pixels.At(272, 320)};
                const Color dxt3Pixel = pixels.At(488, 152);
                const Color dxt5Pixel = pixels.At(680, 152);
                Context().Print("sampled DXT1 blocks: {} {} {} {}", dxt1Pixels[0].ToString(),
                                dxt1Pixels[1].ToString(), dxt1Pixels[2].ToString(),
                                dxt1Pixels[3].ToString());
                Context().Print("sampled DXT3 {}, DXT5 {}", dxt3Pixel.ToString(),
                                dxt5Pixel.ToString());
                Context().Check(dxt1Pixels == std::array<Color, 4>{Color::Red, Color::Lime,
                                                                   Color::Blue, Color::Magenta},
                                "sampling exposes all four directly uploaded DXT1 blocks in row order");
                Context().Check(CnaDemos::Near(dxt3Pixel, Color(0, 255, 0, 136), 1) &&
                                    CnaDemos::Near(dxt5Pixel, Color(0, 0, 255, 64), 1),
                                "sampling decodes explicit DXT3 and endpoint DXT5 alpha before output");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> dxt1_;
        std::unique_ptr<Texture2D> dxt3_;
        std::unique_ptr<Texture2D> dxt5_;
    };
}

CNA_DEMOS_GAME(CompressedBlocksDemo, {
    .id = "graphics.texture2d.compressed_blocks",
    .title = "Compressed texture block transfers",
    .description = "Transfers exact DXT1 blocks through whole levels, guarded regions and padded small mips, "
                   "round-trips DXT3/DXT5 alpha blocks, samples all three formats and checks XNA's dimension rule.",
    .apis = {"Texture2D::Texture2D", "Texture2D::SetData", "Texture2D::GetData",
             "Texture::LevelCount", "Texture::Format", "SurfaceFormat::Dxt1",
             "SurfaceFormat::Dxt3", "SurfaceFormat::Dxt5", "SpriteBatch::Draw"},
    .tags = {"texture", "compression", "mipmap", "sampling", "array"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "Requires Dxt1, Dxt3 and Dxt5 texture creation; only constructor capability refusal is skipped. "
                    "Pixel checks use programmatic back-buffer readback; no screenshot or manual inspection is claimed.",
})
