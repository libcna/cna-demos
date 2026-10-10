// SPDX-License-Identifier: MS-PL
// Rotating a sprite about its centre: SpriteBatch::Draw with rotation and origin.
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class RotationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            arrow_ = getContentProperty().Load<Texture2D>("textures/arrow");
        }

        void Update(GameTime& gameTime) override
        {
            // Half a turn per second of game time. Angles are radians; positive turns clockwise on
            // screen (Y points down).
            angle_ = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty()) * MathHelper::Pi;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);

            // The origin is the point of the texture that sits at `position` and that the sprite
            // rotates about -- here the texture's centre.
            const Vector2 centre(arrow_.getWidthProperty() / 2.0f, arrow_.getHeightProperty() / 2.0f);
            spriteBatch_->Begin();
            spriteBatch_->Draw(arrow_, Vector2(250.0f, 240.0f), std::nullopt, Color::White, angle_, centre, 2.0f,
                               SpriteEffects::None, 0.0f);
            // The same sprite with origin (0, 0) swings around its top-left corner instead.
            spriteBatch_->Draw(arrow_, Vector2(550.0f, 240.0f), std::nullopt, Color::White, angle_, Vector2::Zero, 2.0f,
                               SpriteEffects::None, 0.0f);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D arrow_;
        float angle_ = 0.0f;
    };
}

CNA_DEMOS_GAME(RotationDemo, {
    .id = "graphics.sprite_batch.rotation",
    .title = "Rotate a sprite",
    .description = "Spins a sprite with SpriteBatch::Draw's rotation argument, once about its centre (origin = half "
                   "the size) and once about its top-left corner.",
    .apis = {"SpriteBatch::Draw", "MathHelper::Pi"},
    .tags = {"sprite", "rotation", "origin", "2d"},
    .frames = 20,
    .assets = {"textures/arrow.png"},
})
