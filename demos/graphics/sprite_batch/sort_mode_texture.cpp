// SPDX-License-Identifier: MS-PL
// SpriteSortMode::Texture: sprites are grouped by texture, so call order no longer decides overlaps.
#include <array>
#include <cstddef>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class SortModeTextureDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            // Two different textures (solid red and solid blue).
            red_ = MakeSolid(device, Color::Red);
            blue_ = MakeSolid(device, Color::Blue);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            // The same chain of overlapping squares twice, textures alternating red, blue, red, blue.
            DrawChain(SpriteSortMode::Deferred, 80);  // call order: each square covers the previous one
            DrawChain(SpriteSortMode::Texture, 280);  // grouped: all squares of one texture, then the other

            if (FramesDrawn() == 0)
                CheckOverlaps(device);

            Game::Draw(gameTime);
        }

    private:
        static std::unique_ptr<Texture2D> MakeSolid(GraphicsDevice& device, Color color)
        {
            auto texture = std::make_unique<Texture2D>(device, 1, 1);
            texture->SetData(&color, 1);
            return texture;
        }

        void DrawChain(SpriteSortMode mode, int y)
        {
            spriteBatch_->Begin(mode, BlendState::Opaque);
            for (int i = 0; i < 6; ++i)
                spriteBatch_->Draw(i % 2 == 0 ? *red_ : *blue_, Rectangle(100 + i * 100, y, 140, 140), Color::White);
            spriteBatch_->End();
        }

        void CheckOverlaps(GraphicsDevice& device)
        {
            const int width = device.getPresentationParametersProperty().getBackBufferWidthProperty();
            std::vector<Color> pixels(static_cast<std::size_t>(width) *
                                      device.getPresentationParametersProperty().getBackBufferHeightProperty());
            device.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
            const auto at = [&](int x, int y) { return pixels[static_cast<std::size_t>(y) * width + x]; };

            // The overlaps are at x = 200..240, 300..340, ...: square i+1 over square i.
            std::array<Color, 5> deferred{};
            std::array<Color, 5> textured{};
            for (int i = 0; i < 5; ++i)
            {
                deferred[static_cast<std::size_t>(i)] = at(220 + i * 100, 150);
                textured[static_cast<std::size_t>(i)] = at(220 + i * 100, 350);
            }
            bool alternates = true;
            for (int i = 0; i < 5; ++i)
                alternates = alternates && deferred[static_cast<std::size_t>(i)] == (i % 2 == 0 ? Color::Blue : Color::Red);
            Context().Check(alternates, "Deferred: every overlap shows the square drawn later");

            bool same = true;
            for (const Color& c : textured)
                same = same && c == textured[0];
            Context().Check(same, "Texture: every overlap shows the same texture -- one group was drawn after the other");
            Context().Print("Texture mode drew the {} group last (texture order is not defined)",
                            textured[0] == Color::Red ? "red" : "blue");
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> red_;
        std::unique_ptr<Texture2D> blue_;
    };
}

CNA_DEMOS_GAME(SortModeTextureDemo, {
    .id = "graphics.sprite_batch.sort_mode_texture",
    .title = "Sort sprites by texture",
    .description = "Draws the same overlapping chain with SpriteSortMode::Deferred (call order) and "
                   "SpriteSortMode::Texture, which groups sprites by texture to save state changes.",
    .apis = {"SpriteBatch::Begin", "SpriteSortMode::Texture", "SpriteSortMode::Deferred"},
    .tags = {"sprite", "sorting", "batching", "performance"},
    .frames = 3,
    .restrictions = "Which texture group comes first is not defined, so the picture may differ between runs.",
})
