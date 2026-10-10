// SPDX-License-Identifier: MS-PL
// Drawing part of a texture: the sourceRectangle argument of SpriteBatch::Draw.
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class SourceRectangleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            // A strip of four 32x32 frames side by side.
            strip_ = getContentProperty().Load<Texture2D>("textures/sprite_strip");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);

            spriteBatch_->Begin();
            // The whole strip, for reference.
            spriteBatch_->Draw(strip_, Vector2(20.0f, 20.0f), Color::White);
            // Each frame separately: the source rectangle selects the texels to draw.
            for (int frame = 0; frame < 4; ++frame)
            {
                const Rectangle source(frame * 32, 0, 32, 32);
                spriteBatch_->Draw(strip_, Rectangle(20 + frame * 140, 120, 128, 128), std::optional<Rectangle>(source),
                                   Color::White);
            }
            // std::nullopt means "the whole texture".
            spriteBatch_->Draw(strip_, Vector2(20.0f, 300.0f), std::nullopt, Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D strip_;
    };
}

CNA_DEMOS_GAME(SourceRectangleDemo, {
    .id = "graphics.sprite_batch.source_rectangle",
    .title = "Draw part of a texture",
    .description = "Selects single frames of a sprite strip with SpriteBatch::Draw's sourceRectangle argument; "
                   "std::nullopt draws the whole texture.",
    .apis = {"SpriteBatch::Draw"},
    .tags = {"sprite", "sprite-sheet", "atlas", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/sprite_strip.png"},
})
