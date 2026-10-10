// SPDX-License-Identifier: MS-PL
// Tinting and fading sprites through the colour argument of SpriteBatch::Draw.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class TintDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DimGray);

            spriteBatch_->Begin();
            // The texture's colours are multiplied by the tint: White leaves them unchanged.
            spriteBatch_->Draw(checker_, Rectangle(20, 20, 160, 160), Color::White);
            spriteBatch_->Draw(checker_, Rectangle(200, 20, 160, 160), Color::Red);
            spriteBatch_->Draw(checker_, Rectangle(380, 20, 160, 160), Color::Lime);
            spriteBatch_->Draw(checker_, Rectangle(560, 20, 160, 160), Color(0.5f, 0.5f, 1.0f));

            // Fading: scale the whole (premultiplied) colour, alpha included.
            for (int i = 0; i < 4; ++i)
            {
                const float opacity = 1.0f - 0.25f * static_cast<float>(i);
                spriteBatch_->Draw(checker_, Rectangle(20 + i * 180, 240, 160, 160), Color::White * opacity);
            }
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
    };
}

CNA_DEMOS_GAME(TintDemo, {
    .id = "graphics.sprite_batch.tint_and_transparency",
    .title = "Tint and fade sprites",
    .description = "Multiplies a sprite's colours by a tint and fades it out with Color::White * opacity, using the "
                   "colour argument of SpriteBatch::Draw.",
    .apis = {"SpriteBatch::Draw", "Color::op_Multiply"},
    .tags = {"sprite", "tint", "alpha", "fade", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
