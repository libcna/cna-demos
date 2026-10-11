// SPDX-License-Identifier: MS-PL
// A SpriteFont authored as CNJ: an artist's glyph atlas plus a JSON document that names each
// glyph's rectangle, cropping and kerning, compiled by CNA's content pipeline.
#include <algorithm>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Color Lcd(90, 230, 120);
    constexpr float Scale = 3.0f;

    class CnjSpriteFontDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            // content/fonts/score_digits.cnj names score_digits_atlas.png and, per glyph, its
            // source rectangle, cropping and kerning (tools/generate_assets.py writes both). The
            // pipeline compiles it to fonts/score_digits.cnb; the atlas also builds as a texture.
            digits_.emplace(getContentProperty().Load<SpriteFont>("fonts/score_digits"));
            atlas_.emplace(getContentProperty().Load<Texture2D>("fonts/score_digits_atlas"));

            const auto& characters = digits_->getCharactersProperty();
            ctx.Print("{} characters, LineSpacing {}, Spacing {}", characters.size(),
                      digits_->getLineSpacingProperty(), digits_->getSpacingProperty());
            ctx.Check(characters.size() == 11 && characters.front() == u'0' && characters.back() == u':',
                      "the font has the eleven authored characters, 0-9 and ':'");
            ctx.Check(digits_->getLineSpacingProperty() == 36, "LineSpacing is the authored 36");
            ctx.Check(digits_->getDefaultCharacterProperty() == u'0', "DefaultCharacter is the authored '0'");

            // A digit advances 2 + 20 + 2 pixels (left bearing, width, right bearing), the colon 2 + 4 + 2.
            const Vector2 size = digits_->MeasureString("12:34");
            ctx.Print("MeasureString(\"12:34\") = {}", size.ToString());
            ctx.Check(size.X == 4.0f * 24.0f + 8.0f && size.Y == 36.0f,
                      "MeasureString adds the authored kerning: 104 x 36");
            ctx.Check(digits_->MeasureString("A") == digits_->MeasureString("0"),
                      "a character outside the font measures as the DefaultCharacter");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(16, 20, 18));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // The atlas as the artist drew it, and the score drawn with the font, three times larger.
            spriteBatch_->Draw(*atlas_, Vector2(298.0f, 40.0f), Color::Gray);
            const Vector2 size = digits_->MeasureString("12:34") * Scale;
            spriteBatch_->DrawString(*digits_, "12:34", Vector2((800.0f - size.X) / 2.0f, 150.0f), Lcd, 0.0f,
                                     Vector2::Zero, Scale, SpriteEffects::None, 0.0f);
            spriteBatch_->DrawString(*digits_, "0123456789", Vector2(282.0f, 320.0f), Lcd);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                // The '1' of "12:34": its right-hand segments (b, c) lit, its left side dark.
                const CnaDemos::BackBufferPixels p(device);
                const int left = static_cast<int>((800.0f - size.X) / 2.0f);
                const Color segmentB = p.At(left + static_cast<int>(Scale * (2 + 17)), 150 + static_cast<int>(Scale * 8));
                const Color unlit = p.At(left + static_cast<int>(Scale * (2 + 1)), 150 + static_cast<int>(Scale * 8));
                Context().Print("'1': segment b {}, left side {}", segmentB.ToString(), unlit.ToString());
                Context().Check(segmentB == Lcd, "the 1's right segment is drawn from the atlas glyph");
                Context().Check(unlit == Color(16, 20, 18), "and its left side is empty");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> digits_;
        std::optional<Texture2D> atlas_;
    };
}

CNA_DEMOS_GAME(CnjSpriteFontDemo, {
    .id = "content.cnj.sprite_font",
    .title = "Bitmap font from CNJ content",
    .description = "Loads a seven-segment SpriteFont authored as CNJ (a glyph atlas plus each glyph's rectangle, "
                   "cropping and kerning) and compiled by the content pipeline; MeasureString follows the authored "
                   "kerning and DefaultCharacter stands in for missing characters.",
    .apis = {"ContentManager::Load", "SpriteFont::Characters", "SpriteFont::LineSpacing", "SpriteFont::DefaultCharacter",
             "SpriteFont::MeasureString", "SpriteBatch::DrawString"},
    .tags = {"content", "cnj", "json", "font", "bitmap-font", "score"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/score_digits.cnb", "fonts/score_digits_atlas.cnb"},
})
