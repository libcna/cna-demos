// SPDX-License-Identifier: MS-PL
// Uniform and non-uniform sprite scaling with SpriteBatch::Draw.
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    bool HasColorNear(const CnaDemos::BackBufferPixels& pixels, int centreX, int centreY,
                      Color expected, int radius = 2)
    {
        for (int y = centreY - radius; y <= centreY + radius; ++y)
        {
            for (int x = centreX - radius; x <= centreX + radius; ++x)
            {
                if (pixels.At(x, y) == expected)
                    return true;
            }
        }
        return false;
    }

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

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp,
                                nullptr, nullptr);
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

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                const Color marker(220, 40, 40);
                const Color arrow(255, 200, 40);
                Context().Check(
                    HasColorNear(pixels, 24, 44, marker) && HasColorNear(pixels, 48, 56, arrow) &&
                        HasColorNear(pixels, 88, 48, marker) && HasColorNear(pixels, 136, 72, arrow) &&
                        HasColorNear(pixels, 184, 64, marker) && HasColorNear(pixels, 328, 136, arrow),
                    "scalar 0.5, 1 and 3 scales move two authored texels by proportional distances");

                Context().Check(
                    HasColorNear(pixels, 440, 48, marker) && HasColorNear(pixels, 680, 72, arrow) &&
                        HasColorNear(pixels, 408, 160, marker) && HasColorNear(pixels, 456, 280, arrow),
                    "Vector2 scales stretch the same marker pair independently on X and Y");
            }

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
    .description = "Point-samples a sprite at half, native and triple uniform size, then stretches the same authored "
                   "marker pair independently with Vector2 scale.",
    .apis = {"SpriteBatch::Draw"},
    .tags = {"sprite", "scale", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/arrow.png"},
    .restrictions = "Representative authored marker positions are checked programmatically; no screenshot or manual visual inspection is claimed.",
})
