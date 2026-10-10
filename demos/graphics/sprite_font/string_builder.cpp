// SPDX-License-Identifier: MS-PL
// DrawString with a StringBuilder: text that changes every frame, built in one reused buffer.
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
#include "System/Text/StringBuilder.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class StringBuilderDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Draw(const GameTime& gameTime) override
        {
            // The XNA idiom for per-frame text (scores, timers): clear and refill one StringBuilder
            // instead of building a new string every frame.
            text_.Clear();
            text_.Append("Frame ");
            text_.Append(FramesDrawn() + 1);
            text_.Append("\nUpdates so far: ");
            text_.Append(updates_);

            getGraphicsDeviceProperty().Clear(Color::DarkGreen);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, text_, Vector2(60.0f, 60.0f), Color::White, 0.0f, Vector2::Zero, 3.0f,
                                     SpriteEffects::None, 0.0f);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                // MeasureString takes the StringBuilder too.
                const Vector2 size = font_->MeasureString(text_);
                Context().Print("\"{}\" measures {}", text_.ToString(), size.ToString());
                Context().Check(size == font_->MeasureString(text_.ToString()),
                                "MeasureString(StringBuilder) equals MeasureString of its string");
            }

            Game::Draw(gameTime);
        }

        void Update(GameTime& gameTime) override
        {
            ++updates_;
            Game::Update(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        System::Text::StringBuilder text_;
        int updates_ = 0;
    };
}

CNA_DEMOS_GAME(StringBuilderDemo, {
    .id = "graphics.sprite_font.string_builder",
    .title = "Text from a StringBuilder",
    .description = "Refills one System::Text::StringBuilder every frame and draws it with the StringBuilder overloads "
                   "of DrawString and MeasureString.",
    .apis = {"SpriteBatch::DrawString", "SpriteFont::MeasureString"},
    .tags = {"text", "font", "stringbuilder", "hud", "score"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
})
