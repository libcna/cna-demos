// SPDX-License-Identifier: MS-PL
// Drawing a texture with SpriteBatch at a position.
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DrawTextureDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());

            // A 64x64 two-colour checkerboard, generated so the demo needs no asset file.
            std::vector<Color> pixels(64 * 64);
            for (int y = 0; y < 64; ++y)
            {
                for (int x = 0; x < 64; ++x)
                {
                    pixels[y * 64 + x] = ((x / 8 + y / 8) % 2 == 0) ? Color::White : Color::DarkSlateBlue;
                }
            }
            texture_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 64, 64);
            texture_->SetData(pixels.data(), static_cast<int>(pixels.size()));
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);

            spriteBatch_->Begin();
            // The XNA overload takes a Vector2 position.
            spriteBatch_->Draw(*texture_, Vector2(100.0f, 80.0f), Color::White);
            // CNA also provides the convenient NOXNA float x/y form; it produces the same
            // placement semantics without constructing a Vector2 at the call site.
            spriteBatch_->Draw(*texture_, 100.0f, 80.0f);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> texture_;
    };
}

CNA_DEMOS_GAME(DrawTextureDemo, {
    .id = "graphics.sprite_batch.draw_texture",
    .title = "Draw a texture",
    .description = "Draws a Texture2D at the same pixel position with both the XNA Vector2 overload and CNA's float "
                   "x/y overload between SpriteBatch::Begin and SpriteBatch::End.",
    .apis = {"SpriteBatch::SpriteBatch", "SpriteBatch::Begin", "SpriteBatch::Draw",
             "SpriteBatch::Draw(float,float)", "SpriteBatch::End"},
    .tags = {"sprite", "2d", "position", "overload"},
    .frames = 10,
    .deterministic = true,
})
