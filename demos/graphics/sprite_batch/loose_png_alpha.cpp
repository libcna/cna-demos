// SPDX-License-Identifier: MS-PL
// Straight-alpha PNGs loaded at run time need BlendState::NonPremultiplied.
#include <memory>

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
    class LoosePngAlphaDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            // A white disc whose edge fades out, saved with straight (not premultiplied) alpha.
            // A built XNB/CNB texture is premultiplied by the content pipeline; a loose PNG loaded
            // at run time is used as it is in the file.
            circle_ = getContentProperty().Load<Texture2D>("textures/soft_circle");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);

            // Wrong pairing: AlphaBlend assumes premultiplied texels and adds their RGB in full.
            // Straight-alpha texels keep their colour where alpha is low or zero (white in this
            // file), so the whole quad comes out white instead of a soft disc.
            spriteBatch_->Begin(SpriteSortMode::Deferred, BlendState::AlphaBlend);
            spriteBatch_->Draw(circle_, Rectangle(100, 100, 256, 256), Color::White);
            spriteBatch_->End();

            // Right pairing: NonPremultiplied multiplies by alpha while blending -- a soft edge.
            spriteBatch_->Begin(SpriteSortMode::Deferred, BlendState::NonPremultiplied);
            spriteBatch_->Draw(circle_, Rectangle(444, 100, 256, 256), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D circle_;
    };
}

CNA_DEMOS_GAME(LoosePngAlphaDemo, {
    .id = "graphics.sprite_batch.loose_png_alpha",
    .title = "Alpha of loose PNG files",
    .description = "Draws a straight-alpha PNG loaded at run time with BlendState::AlphaBlend (wrong: transparent "
                   "texels show their colour) and BlendState::NonPremultiplied (right: a soft-edged disc).",
    .apis = {"ContentManager::Load", "BlendState::NonPremultiplied", "BlendState::AlphaBlend", "SpriteBatch::Begin"},
    .tags = {"alpha", "premultiplied", "png", "blending", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/soft_circle.png"},
})
