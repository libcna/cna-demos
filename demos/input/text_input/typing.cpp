// SPDX-License-Identifier: MS-PL
// CNAEXT TextInputEXT: typed characters as events -- layout, Shift and key repeat already applied.
#include <deque>
#include <memory>
#include <optional>
#include <string>

#include "CNA/Input/Clipboard.hpp"
#include "CNA/Input/TextInputType.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/TextInputEXT.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/MulticastAction.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    // The demo font has printable ASCII only; anything else is shown as '?'.
    std::string Printable(const std::u16string& text)
    {
        std::string out;
        for (const char16_t c : text)
            out += (c >= u' ' && c <= u'~') ? static_cast<char>(c) : '?';
        return out;
    }

    // Clipboard text is UTF-8; the line holds UTF-16 like the TextInput event. Characters outside
    // the Basic Multilingual Plane become '?' here.
    std::u16string FromUtf8(const std::string& text)
    {
        std::u16string out;
        for (std::size_t i = 0; i < text.size();)
        {
            const auto lead = static_cast<unsigned char>(text[i]);
            const std::size_t length = lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
            if (i + length > text.size())
                break;
            char32_t code = length == 1 ? lead : length == 2 ? lead & 0x1F : length == 3 ? lead & 0x0F : lead & 0x07;
            for (std::size_t k = 1; k < length; ++k)
                code = (code << 6) | (static_cast<unsigned char>(text[i + k]) & 0x3F);
            out += code <= 0xFFFF ? static_cast<char16_t>(code) : u'?';
            i += length;
        }
        return out;
    }

    class TypingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // Characters arrive through the TextInput event, and only between StartTextInput and
            // StopTextInput (on phones, starting it also brings up the on-screen keyboard).
            textToken_ = TextInputEXT::TextInput.Add([this](SharpRuntime::charcs c) { OnCharacter(c); });
            TextInputEXT::StartTextInput();
            Context().Check(TextInputEXT::IsTextInputActive(), "StartTextInput turns text input on");
            TextInputEXT::StopTextInput();
            TextInputEXT::StartTextInputWithTypeEXT(CNA::Input::TextInputTypeEXT::TextEmail);
            Context().Check(TextInputEXT::IsTextInputActive(),
                            "StartTextInputWithTypeEXT activates text input with an email-layout hint");
        }

        void UnloadContent() override
        {
            TextInputEXT::StopTextInput();
            Context().Check(!TextInputEXT::IsTextInputActive(), "StopTextInput turns it off again");
            // TextInput is static: unsubscribe before this game, which the handler uses, is gone.
            TextInputEXT::TextInput.Remove(textToken_);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Type a line, Enter sends it. Home/End/Backspace/Delete edit,",
                                     Vector2(40.0f, 20.0f), Color::White);
            spriteBatch_->DrawString(*font_, "Ctrl+V pastes the clipboard.", Vector2(40.0f, 45.0f), Color::White);

            spriteBatch_->Draw(*pixel_, Rectangle(40, 90, 720, 40), Color::Black);
            const std::string shown = Printable(line_);
            spriteBatch_->DrawString(*font_, shown, Vector2(50.0f, 100.0f), Color::White);
            // The caret blinks after the character it follows.
            const float caretX = 50.0f + font_->MeasureString(shown.substr(0, caret_)).X;
            if (static_cast<int>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty() * 2.0) % 2 == 0)
                spriteBatch_->Draw(*pixel_, Rectangle(static_cast<int>(caretX), 96, 2, 28), Color::Yellow);

            float y = 160.0f;
            for (const std::u16string& sent : sent_)
            {
                spriteBatch_->DrawString(*font_, "> " + Printable(sent), Vector2(50.0f, y), Color::LightGray);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void OnCharacter(SharpRuntime::charcs c)
        {
            // Editing keys arrive as control characters (FNA's convention).
            switch (c)
            {
            case 2:  // Home
                caret_ = 0;
                break;
            case 3:  // End
                caret_ = line_.size();
                break;
            case 8:  // Backspace
                if (caret_ > 0)
                    line_.erase(--caret_, 1);
                break;
            case 127:  // Delete
                if (caret_ < line_.size())
                    line_.erase(caret_, 1);
                break;
            case 13:  // Enter
                sent_.push_front(line_);
                if (sent_.size() > 10)
                    sent_.pop_back();
                line_.clear();
                caret_ = 0;
                break;
            case 22:  // Ctrl+V
                for (const char16_t pasted : FromUtf8(CNA::Input::Clipboard::GetTextEXT()))
                    Insert(pasted);
                break;
            case 9:  // Tab
                Insert(u' ');
                break;
            default:
                if (c >= u' ')
                    Insert(c);
                break;
            }
        }

        void Insert(char16_t c)
        {
            if (c < u' ' || line_.size() >= 50)
                return;
            line_.insert(caret_++, 1, c);
        }

        System::MulticastAction<SharpRuntime::charcs>::Token textToken_ =
            System::MulticastAction<SharpRuntime::charcs>::InvalidToken;
        std::u16string line_;
        std::size_t caret_ = 0;
        std::deque<std::u16string> sent_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(TypingDemo, {
    .id = "input.text_input.typing",
    .title = "Typing text (CNAEXT)",
    .description = "A one-line text box fed by the TextInputEXT::TextInput event between StartTextInput and "
                   "StopTextInput, including the TextEmail on-screen-keyboard hint; editing keys arrive as control "
                   "characters (Backspace 8, Enter 13, Home 2, End 3, Delete 127, Ctrl+V 22).",
    .apis = {"TextInputEXT::TextInput", "TextInputEXT::StartTextInput",
             "TextInputEXT::StartTextInputWithTypeEXT", "TextInputEXT::StopTextInput",
             "TextInputEXT::IsTextInputActive", "CNA::Input::TextInputTypeEXT",
             "CNA::Input::Clipboard::GetTextEXT"},
    .tags = {"text", "typing", "text-box", "chat", "keyboard", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard | CnaDemos::Need::Window,
    .assets = {"fonts/pixel_font.cnb"},
})
