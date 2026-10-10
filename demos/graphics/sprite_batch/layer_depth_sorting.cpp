// SPDX-License-Identifier: MS-PL
// Draw order: SpriteSortMode::Deferred (call order) versus BackToFront/FrontToBack (layerDepth).
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
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

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(LayerDepthDemo, {
    .id = "graphics.sprite_batch.layer_depth_sorting",
    .title = "Layer depth and sort modes",
    .description = "Draws the same three overlapping sprites with SpriteSortMode Deferred, BackToFront and "
                   "FrontToBack to show when the layerDepth argument decides what is on top.",
    .apis = {"SpriteBatch::Begin", "SpriteBatch::Draw", "SpriteSortMode::Deferred", "SpriteSortMode::BackToFront",
             "SpriteSortMode::FrontToBack"},
    .tags = {"sprite", "sorting", "depth", "2d"},
    .frames = 5,
    .deterministic = true,
})
