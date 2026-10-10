// SPDX-License-Identifier: MS-PL
// CNAEXT Clipboard: put text on the system clipboard and read it back.
#include <memory>
#include <optional>
#include <string>

#include "CNA/Input/Clipboard.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using CNA::Input::Clipboard;

namespace
{
    class ClipboardTextDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // The clipboard belongs to the desktop, so keep what the person had copied.
            const std::optional<std::string> previous =
                Clipboard::HasTextEXT() ? std::optional(Clipboard::GetTextEXT()) : std::nullopt;

            const std::string copied = "Score: 12345 -- copied by cna-demos";
            Clipboard::SetTextEXT(copied);
            Context().Check(Clipboard::HasTextEXT(), "after SetTextEXT the clipboard has text");
            read_ = Clipboard::GetTextEXT();
            Context().Print("GetTextEXT: \"{}\"", read_);
            Context().Check(read_ == copied, "GetTextEXT returns the text that was set (UTF-8)");

            // X11 and Wayland also have a "primary selection": the last selected text, pasted
            // with the middle mouse button. Elsewhere it is not available.
            Clipboard::SetPrimarySelectionTextEXT("selected text");
            primary_ = Clipboard::HasPrimarySelectionTextEXT();
            Context().Print("primary selection {}", primary_ ? "available: \"" + Clipboard::GetPrimarySelectionTextEXT() + "\""
                                                             : std::string("not available on this platform"));

            if (previous)
                Clipboard::SetTextEXT(*previous);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Copied and read back:", Vector2(40.0f, 40.0f), Color::White);
            spriteBatch_->DrawString(*font_, read_, Vector2(60.0f, 75.0f), Color::LightGreen);
            spriteBatch_->DrawString(*font_, primary_ ? "Primary selection: available" : "Primary selection: none",
                                     Vector2(40.0f, 130.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, "The previous clipboard text was put back.", Vector2(40.0f, 160.0f),
                                     Color::LightGray);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::string read_;
        bool primary_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(ClipboardTextDemo, {
    .id = "input.clipboard.text",
    .title = "Clipboard text (CNAEXT)",
    .description = "Copies text with Clipboard::SetTextEXT, reads it back with GetTextEXT and HasTextEXT, tries the "
                   "X11/Wayland primary selection, then restores the text that was on the clipboard before.",
    .apis = {"CNA::Input::Clipboard::SetTextEXT", "CNA::Input::Clipboard::GetTextEXT",
             "CNA::Input::Clipboard::HasTextEXT", "CNA::Input::Clipboard::SetPrimarySelectionTextEXT",
             "CNA::Input::Clipboard::GetPrimarySelectionTextEXT",
             "CNA::Input::Clipboard::HasPrimarySelectionTextEXT"},
    .tags = {"clipboard", "copy", "paste", "text", "cnaext"},
    .needs = CnaDemos::Need::Window,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
