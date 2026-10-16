// SPDX-License-Identifier: MS-PL
// CNAEXT Clipboard: both public wrappers share one system clipboard and preserve its prior state.
#include <memory>
#include <optional>
#include <string>

#include "CNA/Devices/Clipboard.hpp"
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
using DeviceClipboard = CNA::Devices::Clipboard;
using InputClipboard = CNA::Input::Clipboard;

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
                InputClipboard::HasTextEXT() ? std::optional(InputClipboard::GetTextEXT()) : std::nullopt;
            const std::optional<std::string> previousPrimary =
                InputClipboard::HasPrimarySelectionTextEXT()
                    ? std::optional(InputClipboard::GetPrimarySelectionTextEXT())
                    : std::nullopt;

            const std::string copied = "Score: 12345 -- copied by cna-demos";
            Context().Check(DeviceClipboard::setTextProperty(copied),
                            "the CNA::Devices compatibility wrapper sets UTF-8 clipboard text");
            Context().Check(DeviceClipboard::getHasTextProperty() &&
                                DeviceClipboard::getTextProperty() == copied,
                            "the CNA::Devices compatibility properties read back the text");
            Context().Check(InputClipboard::HasTextEXT() && InputClipboard::GetTextEXT() == copied,
                            "CNA::Input observes text written through CNA::Devices");

            read_ = "Shared by CNA::Input and CNA::Devices";
            InputClipboard::SetTextEXT(read_);
            Context().Check(DeviceClipboard::getHasTextProperty() &&
                                DeviceClipboard::getTextProperty() == read_,
                            "CNA::Devices observes text written through CNA::Input");
            Context().Print("shared clipboard text: \"{}\"", read_);

            // X11 and Wayland also have a "primary selection": the last selected text, pasted
            // with the middle mouse button. Elsewhere it is not available.
            InputClipboard::SetPrimarySelectionTextEXT("selected text");
            primary_ = InputClipboard::HasPrimarySelectionTextEXT();
            Context().Print("primary selection {}",
                            primary_ ? "available: \"" + InputClipboard::GetPrimarySelectionTextEXT() + "\""
                                     : std::string("not available on this platform"));

            Context().Check(DeviceClipboard::setTextProperty(previous.value_or("")) &&
                                DeviceClipboard::getHasTextProperty() == previous.has_value() &&
                                (!previous || DeviceClipboard::getTextProperty() == *previous),
                            "the prior clipboard text or empty state is restored exactly");
            InputClipboard::SetPrimarySelectionTextEXT(previousPrimary.value_or(""));
            Context().Check(InputClipboard::HasPrimarySelectionTextEXT() == previousPrimary.has_value() &&
                                (!previousPrimary ||
                                 InputClipboard::GetPrimarySelectionTextEXT() == *previousPrimary),
                            "the prior primary-selection text or empty state is restored exactly");
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
    .description = "Writes and reads text across CNA::Devices::Clipboard and CNA::Input::Clipboard, tries the "
                   "X11/Wayland primary selection, then restores both prior text states including empty ones.",
    .apis = {"CNA::Input::Clipboard::SetTextEXT", "CNA::Input::Clipboard::GetTextEXT",
             "CNA::Input::Clipboard::HasTextEXT", "CNA::Input::Clipboard::SetPrimarySelectionTextEXT",
             "CNA::Input::Clipboard::GetPrimarySelectionTextEXT",
             "CNA::Input::Clipboard::HasPrimarySelectionTextEXT", "CNA::Devices::Clipboard::Text",
             "CNA::Devices::Clipboard::HasText"},
    .tags = {"clipboard", "copy", "paste", "text", "cnaext"},
    .needs = CnaDemos::Need::Window,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
