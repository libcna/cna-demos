// SPDX-License-Identifier: MS-PL
// Uniform and non-uniform sprite scaling with SpriteBatch::Draw.
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ScaleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            arrow_ = getContentProperty().Load<Texture2D>("textures/arrow");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);

            spriteBatch_->Begin();
            // float scale: the same factor on both axes.
            spriteBatch_->Draw(arrow_, Vector2(20.0f, 40.0f), std::nullopt, Color::White, 0.0f, Vector2::Zero, 0.5f,
                               SpriteEffects::None, 0.0f);
            spriteBatch_->Draw(arrow_, Vector2(80.0f, 40.0f), std::nullopt, Color::White, 0.0f, Vector2::Zero, 1.0f,
                               SpriteEffects::None, 0.0f);
            spriteBatch_->Draw(arrow_, Vector2(160.0f, 40.0f), std::nullopt, Color::White, 0.0f, Vector2::Zero, 3.0f,
                               SpriteEffects::None, 0.0f);
            // Vector2 scale: different factors per axis.
            spriteBatch_->Draw(arrow_, Vector2(400.0f, 40.0f), std::nullopt, Color::White, 0.0f, Vector2::Zero,
                               Vector2(5.0f, 1.0f), SpriteEffects::None, 0.0f);
            spriteBatch_->Draw(arrow_, Vector2(400.0f, 120.0f), std::nullopt, Color::White, 0.0f, Vector2::Zero,
                               Vector2(1.0f, 5.0f), SpriteEffects::None, 0.0f);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D arrow_;
    };
}

CNA_DEMOS_GAME(ScaleDemo, {
    .id = "graphics.sprite_batch.scale",
    .title = "Scale a sprite",
    .description = "Draws a sprite at several sizes with the float scale overload of SpriteBatch::Draw and stretches "
                   "it per axis with the Vector2 scale overload.",
    .apis = {"SpriteBatch::Draw"},
    .tags = {"sprite", "scale", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/arrow.png"},
})
