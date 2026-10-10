// SPDX-License-Identifier: MS-PL
// Mirroring sprites with SpriteEffects::FlipHorizontally and FlipVertically.
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
    class SpriteEffectsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            arrow_ = getContentProperty().Load<Texture2D>("textures/arrow");   // points right, red dot top-left
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);

            const SpriteEffects effects[] = {SpriteEffects::None, SpriteEffects::FlipHorizontally,
                                             SpriteEffects::FlipVertically,
                                             static_cast<SpriteEffects>(static_cast<int>(SpriteEffects::FlipHorizontally)
                                                                        | static_cast<int>(SpriteEffects::FlipVertically))};
            spriteBatch_->Begin();
            for (int i = 0; i < 4; ++i)
            {
                // Flipping mirrors the texture in place; the sprite is not moved.
                spriteBatch_->Draw(arrow_, Vector2(40.0f + i * 190.0f, 176.0f), std::nullopt, Color::White, 0.0f,
                                   Vector2::Zero, 2.0f, effects[i], 0.0f);
            }
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D arrow_;
    };
}

CNA_DEMOS_GAME(SpriteEffectsDemo, {
    .id = "graphics.sprite_batch.sprite_effects",
    .title = "Flip sprites",
    .description = "Draws the same sprite with SpriteEffects::None, FlipHorizontally, FlipVertically and both flags "
                   "combined.",
    .apis = {"SpriteBatch::Draw", "SpriteEffects::None", "SpriteEffects::FlipHorizontally",
             "SpriteEffects::FlipVertically"},
    .tags = {"sprite", "flip", "mirror", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/arrow.png"},
})
