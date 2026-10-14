// SPDX-License-Identifier: MS-PL
// Tinting and fading sprites through the colour argument of SpriteBatch::Draw.
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

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp,
                                nullptr, nullptr);
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

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                const std::array<Color, 4> opaqueExpected = {
                    Color::White, Color::Red, Color::Lime, Color(128, 128, 255)
                };
                bool opaqueTints = true;
                for (int i = 0; i < 4; ++i)
                {
                    // These points map well inside the checker's opaque white top-left square.
                    opaqueTints = opaqueTints &&
                        CnaDemos::Near(pixels.At(25 + i * 180, 25),
                                       opaqueExpected[static_cast<std::size_t>(i)], 0);
                }
                Context().Check(opaqueTints,
                                "opaque white texels are multiplied by all four RGB tints");

                // Color::Multiply truncates 255 * opacity before premultiplied AlphaBlend adds
                // the uncovered share of the (105,105,105) DimGray background.
                const std::array<Color, 4> fadeExpected = {
                    Color(255, 255, 255), Color(217, 217, 217),
                    Color(180, 180, 180), Color(142, 142, 142)
                };
                bool fadesOverBackground = true;
                for (int i = 0; i < 4; ++i)
                {
                    fadesOverBackground = fadesOverBackground &&
                        CnaDemos::Near(pixels.At(25 + i * 180, 245),
                                       fadeExpected[static_cast<std::size_t>(i)], 2);
                }
                Context().Check(fadesOverBackground,
                                "premultiplied opacity blends white texels over DimGray");
            }

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
    .description = "Point-samples white texels to verify opaque RGB tint multiplication, then fades the same texels "
                   "over DimGray with premultiplied Color::White * opacity.",
    .apis = {"SpriteBatch::Draw", "Color::op_Multiply"},
    .tags = {"sprite", "tint", "alpha", "fade", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/checker.png"},
    .restrictions = "Representative tint and fade pixels are checked programmatically; no screenshot or manual visual inspection is claimed.",
})
