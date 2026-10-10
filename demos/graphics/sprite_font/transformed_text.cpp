// SPDX-License-Identifier: MS-PL
// Rotated and scaled text: the full DrawString overload with rotation, origin and scale.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
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

            // Scale enlarges the glyphs around the origin (here the text's top-left corner).
            spriteBatch_->DrawString(*font_, "Scale 3", Vector2(40.0f, 30.0f), Color::Gold, 0.0f, Vector2::Zero, 3.0f,
                                     SpriteEffects::None, 0.0f);
            // A Vector2 scale stretches the axes independently.
            spriteBatch_->DrawString(*font_, "Scale (4, 1.5)", Vector2(40.0f, 110.0f), Color::LightGreen, 0.0f,
                                     Vector2::Zero, Vector2(4.0f, 1.5f), SpriteEffects::None, 0.0f);

            // The origin is in the text's own unscaled coordinates: its centre, from MeasureString,
            // makes the text turn around its middle and sit centred on the position.
            const std::string spin = "Rotation";
            const Vector2 centre = font_->MeasureString(spin) / 2.0f;
            const float angles[4] = {0.0f, MathHelper::PiOver4, MathHelper::PiOver2, -MathHelper::Pi / 6.0f};
            const Vector2 positions[4] = {Vector2(530.0f, 220.0f), Vector2(700.0f, 220.0f), Vector2(530.0f, 380.0f),
                                          Vector2(700.0f, 380.0f)};
            for (int i = 0; i < 4; ++i)
                spriteBatch_->DrawString(*font_, spin, positions[i], Color::Orange, angles[i], centre, 1.5f,
                                         SpriteEffects::None, 0.0f);

            // Mirrored text: the whole string is mirrored, not each glyph in place.
            spriteBatch_->DrawString(*font_, "Mirrored", Vector2(40.0f, 260.0f), Color::LightSkyBlue, 0.0f,
                                     Vector2::Zero, 2.0f, SpriteEffects::FlipHorizontally, 0.0f);
            spriteBatch_->DrawString(*font_, "Upside down", Vector2(40.0f, 340.0f), Color::Plum, 0.0f, Vector2::Zero,
                                     2.0f, SpriteEffects::FlipVertically, 0.0f);

            spriteBatch_->End();
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
