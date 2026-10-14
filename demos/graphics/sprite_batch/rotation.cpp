// SPDX-License-Identifier: MS-PL
// Rotating an asymmetric sprite about its centre and top-left origins.
#include <array>
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
#include "Microsoft/Xna/Framework/MathHelper.hpp"
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

    class RotationDemo final : public CnaDemos::DemoGame
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

            // The origin is the point of the texture that sits at `position` and that the sprite
            // rotates about. Positive angles turn clockwise on screen because Y points down.
            const Vector2 centre(arrow_.getWidthProperty() / 2.0f, arrow_.getHeightProperty() / 2.0f);
            const std::array<Vector2, 4> pivots = {
                Vector2(100.0f, 150.0f), Vector2(300.0f, 150.0f),
                Vector2(500.0f, 150.0f), Vector2(700.0f, 150.0f)
            };
            const std::array<float, 4> angles = {
                0.0f, MathHelper::PiOver2, MathHelper::Pi, 3.0f * MathHelper::PiOver2
            };

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp,
                                nullptr, nullptr);
            for (std::size_t i = 0; i < pivots.size(); ++i)
            {
                spriteBatch_->Draw(arrow_, pivots[i], std::nullopt, Color::White, angles[i], centre, 1.0f,
                                   SpriteEffects::None, 0.0f);
            }
            // At 90 degrees the same texture pivots differently when its top-left texel is the origin.
            spriteBatch_->Draw(arrow_, Vector2(250.0f, 350.0f), std::nullopt, Color::White,
                               MathHelper::PiOver2, centre, 1.0f, SpriteEffects::None, 0.0f);
            spriteBatch_->Draw(arrow_, Vector2(550.0f, 350.0f), std::nullopt, Color::White,
                               MathHelper::PiOver2, Vector2::Zero, 1.0f,
                               SpriteEffects::None, 0.0f);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                bool centresStayFixed = true;
                for (const Vector2& pivot : pivots)
                {
                    centresStayFixed = centresStayFixed &&
                        HasColorNear(pixels, static_cast<int>(pivot.X), static_cast<int>(pivot.Y),
                                     Color(255, 200, 40), 1);
                }
                Context().Check(centresStayFixed,
                                "the centre origin stays on each position through four rotations");

                const std::array<Vector2, 4> redMarkerPositions = {
                    Vector2(76.0f, 126.0f), Vector2(324.0f, 126.0f),
                    Vector2(524.0f, 174.0f), Vector2(676.0f, 174.0f)
                };
                bool cardinalTurns = true;
                for (const Vector2& marker : redMarkerPositions)
                {
                    cardinalTurns = cardinalTurns &&
                        HasColorNear(pixels, static_cast<int>(marker.X), static_cast<int>(marker.Y),
                                     Color(220, 40, 40));
                }
                Context().Check(cardinalTurns,
                                "the asymmetric red marker turns clockwise through 0, 90, 180 and 270 degrees");

                Context().Check(
                    HasColorNear(pixels, 274, 326, Color(220, 40, 40)) &&
                        HasColorNear(pixels, 542, 358, Color(220, 40, 40)) &&
                        pixels.At(550, 350) == Color::CornflowerBlue,
                    "changing the origin moves the same 90-degree rotation around a different pivot");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D arrow_;
    };
}

CNA_DEMOS_GAME(RotationDemo, {
    .id = "graphics.sprite_batch.rotation",
    .title = "Rotate a sprite",
    .description = "Draws 0/90/180/270-degree rotations around a sprite's centre, then contrasts a centre origin "
                   "with a top-left origin at the same 90-degree angle.",
    .apis = {"SpriteBatch::Draw", "MathHelper::Pi", "MathHelper::PiOver2"},
    .tags = {"sprite", "rotation", "origin", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/arrow.png"},
    .restrictions = "Pivots and representative marker pixels are checked programmatically; no screenshot or manual visual inspection is claimed.",
})
