// SPDX-License-Identifier: MS-PL
// SpriteFont::Characters and DefaultCharacter: what happens to text the font cannot draw.
#include <algorithm>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/ArgumentException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // Text with characters outside the font (it only has printable ASCII). The accented letters
    // are intentional: they are the subject of this demo.
    constexpr const char* Text = "Café über © 2026";

    class DefaultCharacterDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            const auto& characters = font_->getCharactersProperty();
            const bool hasE = std::ranges::find(characters, u'e') != characters.end();
            const bool hasEAcute = std::ranges::find(characters, u'é') != characters.end();
            Context().Check(hasE && !hasEAcute, "Characters lists 'e' but not 'é'");

            // Without a DefaultCharacter, text with a missing character is an error.
            Context().Check(!font_->getDefaultCharacterProperty().has_value(), "this font has no DefaultCharacter");
            bool threw = false;
            try
            {
                static_cast<void>(font_->MeasureString(Text));
            }
            catch (const System::ArgumentException& e)
            {
                threw = true;
                Context().Print("without DefaultCharacter: ArgumentException: {}", e.what());
            }
            Context().Check(threw, "measuring (or drawing) a missing character throws ArgumentException");

            // With one, every missing character is drawn as it instead.
            fallback_.emplace(*font_);
            fallback_->setDefaultCharacterProperty(u'?');
            Context().Check(fallback_->MeasureString(Text).X == fallback_->MeasureString("Caf? ?ber ? 2026").X,
                            "with DefaultCharacter '?' each missing character takes a '?'");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*fallback_, "DefaultCharacter = '?':", Vector2(40.0f, 140.0f), Color::Gray);
            spriteBatch_->DrawString(*fallback_, Text, Vector2(40.0f, 180.0f), Color::White, 0.0f, Vector2::Zero, 3.0f,
                                     SpriteEffects::None, 0.0f);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::optional<SpriteFont> fallback_;
    };
}

CNA_DEMOS_GAME(DefaultCharacterDemo, {
    .id = "graphics.sprite_font.default_character",
    .title = "Missing characters",
    .description = "Checks SpriteFont::Characters, shows the ArgumentException for a character the font lacks, then "
                   "sets DefaultCharacter so missing characters are drawn as '?'.",
    .apis = {"SpriteFont::Characters", "SpriteFont::DefaultCharacter", "SpriteFont::MeasureString",
             "SpriteBatch::DrawString"},
    .tags = {"text", "font", "unicode", "fallback"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
