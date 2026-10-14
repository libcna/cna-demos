// SPDX-License-Identifier: MS-PL
// Mirroring sprites with SpriteEffects::FlipHorizontally and FlipVertically.
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
                      Color expected, int radius = 3)
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
                                             SpriteEffects::FlipHorizontally | SpriteEffects::FlipVertically};
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp,
                                nullptr, nullptr);
            for (int i = 0; i < 4; ++i)
            {
                // Flipping mirrors the texture in place; the sprite is not moved.
                spriteBatch_->Draw(arrow_, Vector2(40.0f + i * 190.0f, 176.0f), std::nullopt, Color::White, 0.0f,
                                   Vector2::Zero, 2.0f, effects[i], 0.0f);
            }
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                const Color marker(220, 40, 40);
                const Color arrow(255, 200, 40);
                Context().Check(
                    HasColorNear(pixels, 56, 192, marker) &&
                        HasColorNear(pixels, 340, 192, marker) &&
                        HasColorNear(pixels, 436, 286, marker) &&
                        HasColorNear(pixels, 720, 286, marker),
                    "the red corner marker mirrors to all four expected corners in place");

                Context().Check(
                    HasColorNear(pixels, 152, 240, arrow) &&
                        HasColorNear(pixels, 244, 240, arrow) &&
                        HasColorNear(pixels, 532, 240, arrow) &&
                        HasColorNear(pixels, 624, 240, arrow),
                    "horizontal flipping reverses the arrow direction while vertical flipping retains it");

                bool fixedBounds = true;
                for (int i = 0; i < 4; ++i)
                {
                    const int x = 40 + i * 190;
                    fixedBounds = fixedBounds &&
                        pixels.At(x - 1, 240) == Color::CornflowerBlue &&
                        pixels.At(x + 128, 240) == Color::CornflowerBlue &&
                        pixels.At(x + 64, 175) == Color::CornflowerBlue &&
                        pixels.At(x + 64, 304) == Color::CornflowerBlue;
                }
                Context().Check(fixedBounds,
                                "all four effects retain the same 128 by 128 destination bounds");
            }

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
    .description = "Point-samples an asymmetric sprite with None, horizontal, vertical and combined flips, proving "
                   "marker mirroring inside unchanged destination bounds.",
    .apis = {"SpriteBatch::Draw", "SpriteEffects::None", "SpriteEffects::FlipHorizontally",
             "SpriteEffects::FlipVertically"},
    .tags = {"sprite", "flip", "mirror", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/arrow.png"},
    .restrictions = "Mirrored marker and boundary pixels are checked programmatically; no screenshot or manual visual inspection is claimed.",
})
