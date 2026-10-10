// SPDX-License-Identifier: MS-PL
// A mipmapped Texture2D: LevelCount, SetData per level, and which level a scaled-down sprite uses.
#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Size = 256;
    // One colour per level so the level the sampler picks is visible.
    const std::array<Color, 9> LevelColors = {Color::Red,    Color::Orange, Color::Yellow, Color::Lime,  Color::Cyan,
                                              Color::Blue,   Color::Purple, Color::Magenta, Color::White};

    class MipmapsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());

            // mipMap = true allocates the full chain: 256, 128, 64, ... 1 -> 9 levels.
            texture_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), Size, Size, true, SurfaceFormat::Color);
            Context().Print("LevelCount = {}", texture_->getLevelCountProperty());
            Context().Check(texture_->getLevelCountProperty() == 9, "a 256x256 texture has 9 mip levels");

            // Each level is filled separately. Normally they would be smaller copies of the same image;
            // here every level gets its own colour.
            for (int level = 0; level < texture_->getLevelCountProperty(); ++level)
            {
                const int size = std::max(1, Size >> level);
                const std::vector<Color> pixels(static_cast<std::size_t>(size) * size,
                                                LevelColors[static_cast<std::size_t>(level)]);
                texture_->SetData(level, nullptr, pixels.data(), 0, static_cast<int>(pixels.size()));
            }

            // Reading a whole level back: elementCount must be that level's pixel count (32 x 32).
            std::vector<Color> level3(32 * 32);
            texture_->GetData(3, nullptr, level3.data(), 0, static_cast<int>(level3.size()));
            Context().Check(level3.front() == LevelColors[3] && level3.back() == LevelColors[3],
                            "GetData reads mip level 3 back");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            // Drawn at 256, 128, 64, 32 and 16 pixels: the smaller the sprite, the smaller the mip
            // level the sampler reads. PointClamp picks the nearest level, so each copy shows one colour.
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            int x = 16;
            for (int size = Size; size >= 16; size /= 2)
            {
                spriteBatch_->Draw(*texture_, Rectangle(x, 112, size, size), Color::White);
                x += size + 16;
            }
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const int width = device.getPresentationParametersProperty().getBackBufferWidthProperty();
                std::vector<Color> pixels(static_cast<std::size_t>(width) *
                                          device.getPresentationParametersProperty().getBackBufferHeightProperty());
                device.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
                const auto at = [&](int px, int py) { return pixels[static_cast<std::size_t>(py) * width + px]; };
                Context().Check(at(16 + 128, 112 + 128) == LevelColors[0], "the full-size copy shows level 0");
                Context().Check(at(288 + 64, 112 + 64) == LevelColors[1], "the half-size copy shows level 1");
                Context().Check(at(432 + 32, 112 + 32) == LevelColors[2], "the quarter-size copy shows level 2");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> texture_;
    };
}

CNA_DEMOS_GAME(MipmapsDemo, {
    .id = "graphics.texture2d.mipmaps",
    .title = "Mipmap levels",
    .description = "Creates a mipmapped Texture2D, fills each level with its own colour through SetData(level, ...) "
                   "and draws it at decreasing sizes to show which level the sampler uses.",
    .apis = {"Texture2D::Texture2D", "Texture::LevelCount", "Texture2D::SetData", "Texture2D::GetData",
             "SamplerState::PointClamp"},
    .tags = {"texture", "mipmap", "filtering", "lod"},
    .frames = 3,
    .deterministic = true,
})
