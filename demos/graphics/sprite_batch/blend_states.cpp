// SPDX-License-Identifier: MS-PL
// The built-in blend states passed to SpriteBatch::Begin.
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class BlendStatesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
        }

        // Two overlapping half-transparent squares (premultiplied colours) in a column.
        void DrawPair(int x, const BlendState& state)
        {
            spriteBatch_->Begin(SpriteSortMode::Deferred, state);
            spriteBatch_->Draw(*pixel_, Rectangle(x, 120, 120, 120), Color::Red * 0.6f);
            spriteBatch_->Draw(*pixel_, Rectangle(x + 50, 180, 120, 120), Color::Blue * 0.6f);
            spriteBatch_->End();
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkGreen);
            DrawPair(20, BlendState::AlphaBlend);         // premultiplied alpha: the default
            DrawPair(220, BlendState::Additive);          // adds light: overlaps get brighter
            DrawPair(420, BlendState::NonPremultiplied);  // for straight-alpha colours
            DrawPair(620, BlendState::Opaque);            // ignores alpha entirely
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(BlendStatesDemo, {
    .id = "graphics.sprite_batch.blend_states",
    .title = "Sprite blend states",
    .description = "Draws the same overlapping translucent sprites with BlendState::AlphaBlend, Additive, "
                   "NonPremultiplied and Opaque passed to SpriteBatch::Begin.",
    .apis = {"SpriteBatch::Begin", "BlendState::AlphaBlend", "BlendState::Additive", "BlendState::NonPremultiplied",
             "BlendState::Opaque"},
    .tags = {"sprite", "blending", "alpha", "2d"},
    .frames = 5,
    .deterministic = true,
})
