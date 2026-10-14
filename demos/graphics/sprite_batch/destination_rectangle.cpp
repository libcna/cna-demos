// SPDX-License-Identifier: MS-PL
// Stretching a texture into a rectangle: SpriteBatch::Draw(texture, Rectangle, Color).
#include <array>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DestinationRectangleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);

            // The 64x64 texture is scaled to fill each rectangle, whatever its size or shape.
            const std::array<Rectangle, 4> destinations = {
                Rectangle(20, 20, 64, 64), Rectangle(120, 20, 256, 128),
                Rectangle(420, 20, 48, 400), Rectangle(520, 200, 240, 240)
            };
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp,
                                nullptr, nullptr);
            spriteBatch_->Draw(checker_, destinations[0], Color::White); // native size
            spriteBatch_->Draw(checker_, destinations[1], Color::White); // stretched horizontally
            spriteBatch_->Draw(checker_, destinations[2], Color::White); // stretched vertically
            spriteBatch_->Draw(checker_, destinations[3], Color::White); // enlarged
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                bool exactBounds = true;
                for (const Rectangle& destination : destinations)
                {
                    exactBounds = exactBounds &&
                        pixels.At(destination.X + destination.Width / 16,
                                  destination.Y + destination.Height / 16) == Color::White &&
                        pixels.At(destination.X - 1,
                                  destination.Y + destination.Height / 2) == Color::CornflowerBlue &&
                        pixels.At(destination.X + destination.Width,
                                  destination.Y + destination.Height / 2) == Color::CornflowerBlue &&
                        pixels.At(destination.X + destination.Width / 2,
                                  destination.Y - 1) == Color::CornflowerBlue &&
                        pixels.At(destination.X + destination.Width / 2,
                                  destination.Y + destination.Height) == Color::CornflowerBlue;
                }
                Context().Check(exactBounds,
                                "all four rectangles fill their bounds and leave adjacent pixels untouched");

                const Color ink(72, 61, 139);
                const bool horizontalStretch =
                    pixels.At(138, 29) == Color::White && pixels.At(170, 29) == ink &&
                    pixels.At(170, 45) == Color::White;
                const bool verticalStretch =
                    pixels.At(423, 48) == Color::White && pixels.At(429, 48) == ink &&
                    pixels.At(429, 98) == Color::White;
                Context().Check(horizontalStretch && verticalStretch,
                                "nonuniform rectangles preserve the whole checker's normalized pattern");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
    };
}

CNA_DEMOS_GAME(DestinationRectangleDemo, {
    .id = "graphics.sprite_batch.destination_rectangle",
    .title = "Draw into a rectangle",
    .description = "Point-scales a complete checker texture into destination rectangles of different sizes and "
                   "aspect ratios, checking their exact bounds and normalized pattern.",
    .apis = {"SpriteBatch::Draw"},
    .tags = {"sprite", "stretch", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/checker.png"},
    .restrictions = "Rectangle bounds and representative nonuniform-scale pixels are checked programmatically; no screenshot or manual visual inspection is claimed.",
})
