// SPDX-License-Identifier: MS-PL
// CNAEXT TextInputEXT composition: draft text from an input method (IME) before it is committed.
#include <memory>
#include <optional>
#include <string>
#include <vector>

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
    // The demo font has printable ASCII only; other characters (and UTF-8 bytes) show as '?'.
    std::string Printable(const std::string& utf8)
    {
        std::string out;
        for (std::size_t i = 0; i < utf8.size(); ++i)
        {
            const auto byte = static_cast<unsigned char>(utf8[i]);
            if (byte >= 0x20 && byte <= 0x7E)
                out += static_cast<char>(byte);
            else if (byte < 0x80 || byte >= 0xC0)  // one '?' per character, not per byte
                out += '?';
        }
        return out;
    }

    const Rectangle TextBox(40, 120, 720, 40);

    class CompositionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // Committed characters still come through TextInput.
            textToken_ = TextInputEXT::TextInput.Add([this](SharpRuntime::charcs c) {
                if (c >= u' ' && c != 127)
                    committed_ += c <= u'~' ? static_cast<char>(c) : '?';
                else if (c == 8 && !committed_.empty())
                    committed_.pop_back();
            });
            // While a word is being composed, TextEditing reports the draft (UTF-8) and the
            // selected part of it; an empty draft means the composition ended.
            editingToken_ = TextInputEXT::TextEditing.Add([this](const std::string& text, int start, int length) {
                draft_ = text;
                Context().Print("TextEditing: \"{}\" start {} length {}", text, start, length);
            });
            // Some input methods let the game draw the candidate list itself.
            candidatesToken_ = TextInputEXT::TextEditingCandidatesEXT.Add(
                [this](const std::vector<std::string>& candidates, int selected, bool) {
                    candidates_ = candidates;
                    selected_ = selected;
                });

            TextInputEXT::StartTextInput();
            // Tell the input method where the text is, so it puts its own window next to it.
            TextInputEXT::SetInputRectangle(TextBox);
        }

        void UnloadContent() override
        {
            TextInputEXT::StopTextInput();
            TextInputEXT::TextInput.Remove(textToken_);
            TextInputEXT::TextEditing.Remove(editingToken_);
            TextInputEXT::TextEditingCandidatesEXT.Remove(candidatesToken_);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Switch to an input method (for example Japanese or", Vector2(40.0f, 20.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "Chinese) and type: the draft appears underlined.", Vector2(40.0f, 45.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "Drafts are also printed on the console.", Vector2(40.0f, 70.0f),
                                     Color::LightGray);

            spriteBatch_->Draw(*pixel_, TextBox, Color::Black);
            const std::string committed = Printable(committed_);
            const std::string draft = Printable(draft_);
            spriteBatch_->DrawString(*font_, committed, Vector2(50.0f, 130.0f), Color::White);
            const float draftX = 50.0f + font_->MeasureString(committed).X;
            spriteBatch_->DrawString(*font_, draft, Vector2(draftX, 130.0f), Color::Gold);
            if (!draft.empty())
                spriteBatch_->Draw(*pixel_,
                                   Rectangle(static_cast<int>(draftX), 152,
                                             static_cast<int>(font_->MeasureString(draft).X), 2),
                                   Color::Gold);

            float y = 190.0f;
            for (std::size_t i = 0; i < candidates_.size() && i < 8; ++i)
            {
                const bool chosen = static_cast<int>(i) == selected_;
                spriteBatch_->DrawString(*font_, std::to_string(i + 1) + ". " + Printable(candidates_[i]),
                                         Vector2(60.0f, y), chosen ? Color::Yellow : Color::LightGray);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        System::MulticastAction<SharpRuntime::charcs>::Token textToken_ = 0;
        System::MulticastAction<const std::string&, int, int>::Token editingToken_ = 0;
        System::MulticastAction<const std::vector<std::string>&, int, bool>::Token candidatesToken_ = 0;
        std::string committed_;
        std::string draft_;
        std::vector<std::string> candidates_;
        int selected_ = -1;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(CompositionDemo, {
    .id = "input.text_input.composition",
    .title = "Input method composition (CNAEXT)",
    .description = "Shows the draft text an input method is composing (TextInputEXT::TextEditing), its candidate "
                   "list (TextEditingCandidatesEXT) and the committed text (TextInput); SetInputRectangle places the "
                   "input method's window next to the text box.",
    .apis = {"TextInputEXT::TextEditing", "TextInputEXT::TextEditingCandidatesEXT", "TextInputEXT::SetInputRectangle",
             "TextInputEXT::TextInput", "TextInputEXT::StartTextInput", "TextInputEXT::StopTextInput"},
    .tags = {"text", "ime", "input-method", "composition", "cjk", "localization", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard | CnaDemos::Need::Window,
    .assets = {"fonts/pixel_font.cnb"},
})
