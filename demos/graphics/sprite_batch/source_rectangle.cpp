// SPDX-License-Identifier: MS-PL
// Drawing part of a texture: the sourceRectangle argument of SpriteBatch::Draw.
#include <array>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
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

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp,
                                nullptr, nullptr);
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

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                const std::array<Color, 4> frameColors = {
                    Color(230, 80, 80), Color(80, 200, 90),
                    Color(80, 120, 230), Color(230, 200, 60)
                };
                bool selectedFrames = true;
                bool transparentCorners = true;
                bool wholeStrip = true;
                for (int frame = 0; frame < 4; ++frame)
                {
                    const int enlargedX = 20 + frame * 140;
                    selectedFrames = selectedFrames &&
                        pixels.At(enlargedX + 34, 154) == frameColors[static_cast<std::size_t>(frame)];
                    transparentCorners = transparentCorners &&
                        pixels.At(enlargedX + 18, 138) == Color::DarkSlateGray;
                    wholeStrip = wholeStrip &&
                        pixels.At(28 + frame * 32, 308) == frameColors[static_cast<std::size_t>(frame)];
                }
                Context().Check(selectedFrames,
                                "each source rectangle selects its own solid-color atlas frame");
                Context().Check(transparentCorners,
                                "transparent texels in every selected frame preserve the background");
                Context().Check(wholeStrip,
                                "std::nullopt draws all four atlas frames in their original order");
            }

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
    .description = "Selects and point-samples single transparent frames from a sprite strip with source rectangles; "
                   "std::nullopt draws the complete atlas in its original order.",
    .apis = {"SpriteBatch::Draw"},
    .tags = {"sprite", "sprite-sheet", "atlas", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/sprite_strip.png"},
    .restrictions = "Frame colors and transparent background pixels are checked programmatically; no screenshot or manual visual inspection is claimed.",
})
