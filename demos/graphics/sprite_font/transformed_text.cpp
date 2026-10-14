// SPDX-License-Identifier: MS-PL
// Rotated and scaled text: the full DrawString overload with rotation, origin and scale.
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    struct InkBounds
    {
        int minX = std::numeric_limits<int>::max();
        int minY = std::numeric_limits<int>::max();
        int maxX = std::numeric_limits<int>::min();
        int maxY = std::numeric_limits<int>::min();
        int count = 0;

        [[nodiscard]] int Width() const { return maxX - minX + 1; }
        [[nodiscard]] int Height() const { return maxY - minY + 1; }
    };

    InkBounds FindInk(const CnaDemos::BackBufferPixels& pixels, Color ink,
                      int left, int top, int right, int bottom)
    {
        InkBounds bounds;
        for (int y = top; y < bottom; ++y)
        {
            for (int x = left; x < right; ++x)
            {
                if (pixels.At(x, y) != ink) continue;
                bounds.minX = std::min(bounds.minX, x);
                bounds.minY = std::min(bounds.minY, y);
                bounds.maxX = std::max(bounds.maxX, x);
                bounds.maxY = std::max(bounds.maxY, y);
                ++bounds.count;
            }
        }
        return bounds;
    }

    class TransformedTextDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);

            // Per-axis scale applies to glyph size and to spacing across the whole string.
            const std::string scaleText = "Scale";
            spriteBatch_->DrawString(*font_, scaleText, Vector2(40.0f, 30.0f), Color::Gold, 0.0f, Vector2::Zero, 2.0f,
                                     SpriteEffects::None, 0.0f);
            spriteBatch_->DrawString(*font_, scaleText, Vector2(300.0f, 30.0f), Color::LightGreen, 0.0f,
                                     Vector2::Zero, Vector2(4.0f, 2.0f), SpriteEffects::None, 0.0f);

            // The origin is in the text's own unscaled coordinates: its centre, from MeasureString,
            // makes the text turn around its middle and sit centred on the position.
            const std::string spin = "Rotation";
            const Vector2 centre = font_->MeasureString(spin) / 2.0f;
            constexpr Vector2 unrotatedPosition(180.0f, 230.0f);
            constexpr Vector2 quarterTurnPosition(600.0f, 230.0f);
            spriteBatch_->DrawString(*font_, spin, unrotatedPosition, Color::Orange, 0.0f, centre, 2.0f,
                                     SpriteEffects::None, 0.0f);
            spriteBatch_->DrawString(*font_, spin, quarterTurnPosition, Color::Orange, MathHelper::PiOver2, centre,
                                     2.0f, SpriteEffects::None, 0.0f);

            // Mirroring applies to the complete layout. Distinct L/J glyphs make a per-glyph-only
            // implementation observably different from the expected whole-string reflection.
            const std::string mirrorText = "LJ";
            constexpr float mirrorScale = 4.0f;
            constexpr float mirrorY = 380.0f;
            const Vector2 mirrorPositions[4] = {
                Vector2(40.0f, mirrorY), Vector2(220.0f, mirrorY),
                Vector2(400.0f, mirrorY), Vector2(580.0f, mirrorY)};
            const SpriteEffects mirrorEffects[4] = {
                SpriteEffects::None, SpriteEffects::FlipHorizontally, SpriteEffects::FlipVertically,
                SpriteEffects::FlipHorizontally | SpriteEffects::FlipVertically};
            for (int i = 0; i < 4; ++i)
                spriteBatch_->DrawString(*font_, mirrorText, mirrorPositions[i], Color::LightSkyBlue, 0.0f,
                                         Vector2::Zero, mirrorScale, mirrorEffects[i], 0.0f);

            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());

                const InkBounds uniform = FindInk(pixels, Color::Gold, 40, 20, 260, 100);
                const InkBounds stretched = FindInk(pixels, Color::LightGreen, 300, 20, 780, 100);
                Context().Check(uniform.count > 0 && stretched.count > 0 &&
                                    std::abs(stretched.Width() - uniform.Width() * 2) <= 2 &&
                                    std::abs(stretched.Height() - uniform.Height()) <= 1,
                                "Vector2 scale doubles the complete text width while preserving its height");

                const InkBounds unrotated = FindInk(pixels, Color::Orange, 20, 130, 340, 330);
                const InkBounds quarterTurn = FindInk(pixels, Color::Orange, 500, 70, 700, 390);
                const int unrotatedCentreX2 = unrotated.minX + unrotated.maxX + 1;
                const int unrotatedCentreY2 = unrotated.minY + unrotated.maxY + 1;
                const int quarterCentreX2 = quarterTurn.minX + quarterTurn.maxX + 1;
                const int quarterCentreY2 = quarterTurn.minY + quarterTurn.maxY + 1;
                const int dx2 = unrotatedCentreX2 - static_cast<int>(unrotatedPosition.X * 2.0f);
                const int dy2 = unrotatedCentreY2 - static_cast<int>(unrotatedPosition.Y * 2.0f);
                const int turnedDx2 = quarterCentreX2 - static_cast<int>(quarterTurnPosition.X * 2.0f);
                const int turnedDy2 = quarterCentreY2 - static_cast<int>(quarterTurnPosition.Y * 2.0f);
                Context().Check(unrotated.count > 0 && quarterTurn.count > 0 &&
                                    std::abs(quarterTurn.Width() - unrotated.Height()) <= 2 &&
                                    std::abs(quarterTurn.Height() - unrotated.Width()) <= 2 &&
                                    std::abs(turnedDx2 + dy2) <= 2 && std::abs(turnedDy2 - dx2) <= 2,
                                "a 90-degree DrawString rotation turns the measured centre and ink bounds together");

                const Vector2 mirrorSize = font_->MeasureString(mirrorText) * mirrorScale;
                const int mirrorWidth = static_cast<int>(mirrorSize.X);
                const int mirrorHeight = static_cast<int>(mirrorSize.Y);
                const auto matchesMirror = [&](int candidate, bool flipX, bool flipY)
                {
                    for (int y = 0; y < mirrorHeight; ++y)
                    {
                        for (int x = 0; x < mirrorWidth; ++x)
                        {
                            const bool normal = pixels.At(static_cast<int>(mirrorPositions[0].X) + x,
                                                          static_cast<int>(mirrorPositions[0].Y) + y) ==
                                                Color::LightSkyBlue;
                            const int candidateX = flipX ? mirrorWidth - 1 - x : x;
                            const int candidateY = flipY ? mirrorHeight - 1 - y : y;
                            const bool reflected = pixels.At(static_cast<int>(mirrorPositions[candidate].X) + candidateX,
                                                             static_cast<int>(mirrorPositions[candidate].Y) + candidateY) ==
                                                   Color::LightSkyBlue;
                            if (normal != reflected) return false;
                        }
                    }
                    return true;
                };
                Context().Check(matchesMirror(1, true, false) && matchesMirror(2, false, true) &&
                                    matchesMirror(3, true, true),
                                "horizontal, vertical and combined flags mirror the complete multi-glyph layout");
            }
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(TransformedTextDemo, {
    .id = "graphics.sprite_font.transformed_text",
    .title = "Rotated, scaled and mirrored text",
    .description = "Draws text with the full DrawString overload: uniform and per-axis scale, rotation about the "
                   "text's centre (origin from MeasureString) and SpriteEffects mirroring.",
    .apis = {"SpriteBatch::DrawString", "SpriteFont::MeasureString", "SpriteEffects::FlipHorizontally",
             "SpriteEffects::FlipVertically"},
    .tags = {"text", "font", "rotation", "scale", "mirror"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
