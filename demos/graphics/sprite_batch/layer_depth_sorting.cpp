// SPDX-License-Identifier: MS-PL
// Draw order: SpriteSortMode::Deferred (call order) versus BackToFront/FrontToBack (layerDepth).
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class LayerDepthDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        // Three overlapping squares, drawn red, green, blue in that order, with layer depths
        // 0.0 (front), 0.5 and 1.0 (back).
        void DrawSquares(Vector2 offset)
        {
            const Color colors[] = {Color::Red, Color::Lime, Color::Blue};
            const float depths[] = {0.0f, 0.5f, 1.0f};
            for (int i = 0; i < 3; ++i)
            {
                spriteBatch_->Draw(*pixel_, offset + Vector2(i * 40.0f, i * 40.0f), std::nullopt, colors[i], 0.0f,
                                   Vector2::Zero, 120.0f, SpriteEffects::None, depths[i]);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Gray);

            // Deferred: drawn in call order, layerDepth ignored -- blue (drawn last) is on top.
            spriteBatch_->Begin(SpriteSortMode::Deferred, BlendState::AlphaBlend);
            DrawSquares(Vector2(40.0f, 120.0f));
            spriteBatch_->End();

            // BackToFront: sorted by depth, 1.0 first -- red (depth 0) ends on top.
            spriteBatch_->Begin(SpriteSortMode::BackToFront, BlendState::AlphaBlend);
            DrawSquares(Vector2(300.0f, 120.0f));
            spriteBatch_->End();

            // FrontToBack: the opposite order -- blue (depth 1) on top again.
            spriteBatch_->Begin(SpriteSortMode::FrontToBack, BlendState::AlphaBlend);
            DrawSquares(Vector2(560.0f, 120.0f));
            spriteBatch_->End();

            // DrawString queues every glyph through the same layer-depth sorting path as Draw.
            // On the left, the text is submitted first but its lower depth brings it to the front.
            // On the right, the text is submitted last but its higher depth puts it behind.
            spriteBatch_->Begin(SpriteSortMode::BackToFront, &BlendState::AlphaBlend,
                                &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "TEXT", Vector2(70.0f, 378.0f), Color::White,
                                     0.0f, Vector2::Zero, 3.0f, SpriteEffects::None, 0.25f);
            spriteBatch_->Draw(*pixel_, Rectangle(40, 350, 320, 100), std::nullopt, Color::DarkRed,
                               0.0f, Vector2::Zero, SpriteEffects::None, 0.5f);
            spriteBatch_->Draw(*pixel_, Rectangle(440, 350, 320, 100), std::nullopt, Color::DarkGreen,
                               0.0f, Vector2::Zero, SpriteEffects::None, 0.5f);
            spriteBatch_->DrawString(*font_, "TEXT", Vector2(470.0f, 378.0f), Color::White,
                                     0.0f, Vector2::Zero, 3.0f, SpriteEffects::None, 0.75f);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                Context().Check(pixels.At(140, 220) == Color::Blue,
                                "Deferred keeps call order: the last blue sprite covers the overlap");
                Context().Check(pixels.At(400, 220) == Color::Red,
                                "BackToFront draws depth zero last: red covers the overlap");
                Context().Check(pixels.At(660, 220) == Color::Blue,
                                "FrontToBack draws depth one last: blue covers the overlap");
                bool allColorsDrawn = true;
                for (const int x : {40, 300, 560})
                    allColorsDrawn = allColorsDrawn && pixels.At(x + 20, 140) == Color::Red &&
                        pixels.At(x + 60, 260) == Color::Lime && pixels.At(x + 180, 300) == Color::Blue;
                Context().Check(allColorsDrawn, "all three sprites remain visible outside the shared overlap in every mode");

                int frontTextPixels = 0;
                int backTextPixels = 0;
                for (int y = 350; y < 450; ++y)
                {
                    for (int x = 40; x < 360; ++x)
                        frontTextPixels += pixels.At(x, y) == Color::White ? 1 : 0;
                    for (int x = 440; x < 760; ++x)
                        backTextPixels += pixels.At(x, y) == Color::White ? 1 : 0;
                }
                Context().Check(frontTextPixels > 20 && backTextPixels == 0,
                                "nonzero DrawString depth sorts glyphs with ordinary sprites independent of call order");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> pixel_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(LayerDepthDemo, {
    .id = "graphics.sprite_batch.layer_depth_sorting",
    .title = "Layer depth and sort modes",
    .description = "Draws overlapping sprites with SpriteSortMode Deferred, BackToFront and FrontToBack, then "
                   "mixes text and a sprite to prove DrawString layer depth enters the same sorted queue.",
    .apis = {"SpriteBatch::Begin", "SpriteBatch::Draw", "SpriteBatch::DrawString", "SpriteSortMode::Deferred",
             "SpriteSortMode::BackToFront", "SpriteSortMode::FrontToBack"},
    .tags = {"sprite", "text", "sorting", "depth", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
