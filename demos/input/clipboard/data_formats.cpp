// SPDX-License-Identifier: MS-PL
// CNAEXT Clipboard data: copy bytes under a MIME type, or one item in several formats at once.
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "CNA/Input/Clipboard.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;
using CNA::Input::Clipboard;

namespace
{
    // A level editor's own format: what another instance of the game can paste.
    const std::string TileFormat = "application/x-cna-demos-tiles";

    std::vector<std::uint8_t> Bytes(const std::string& text) { return {text.begin(), text.end()}; }

    class DataFormatsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            previousKeyboard_ = Keyboard::GetState();
            Context().Check(Bytes("CNA") == std::vector<std::uint8_t>({67, 78, 65}),
                            "the custom format payload owns the exact authored bytes");
        }

        void Update(GameTime& gameTime) override
        {
            const KeyboardState keyboard = Keyboard::GetState();
            if (!Context().Automated() && keyboard.IsKeyDown(Keys::C) && previousKeyboard_.IsKeyUp(Keys::C))
                RunClipboardProbe();
            previousKeyboard_ = keyboard;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "C: replace the real clipboard and run the format probe",
                                     Vector2(40.0f, 40.0f), Color::White);
            float y = 75.0f;
            for (const std::string& type : offered_)
            {
                spriteBatch_->DrawString(*font_, type, Vector2(60.0f, y), Color::LightGreen);
                y += 28.0f;
            }
            spriteBatch_->DrawString(*font_, "As text: " + text_, Vector2(40.0f, y + 20.0f), Color::LightGray);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void RunClipboardProbe()
        {
            offered_.clear();

            const std::optional<std::string> previous =
                Clipboard::HasTextEXT() ? std::optional(Clipboard::GetTextEXT()) : std::nullopt;

            // One format: raw bytes, readable only by a program that asks for this MIME type.
            const std::vector<std::uint8_t> tiles = {3, 1, 4, 1, 5, 9, 2, 6};
            Context().Check(Clipboard::SetDataEXT(TileFormat, tiles), "SetDataEXT accepts a custom MIME type");
            Context().Check(Clipboard::HasDataEXT(TileFormat) && Clipboard::GetDataEXT(TileFormat) == tiles,
                            "GetDataEXT returns the same bytes");
            Context().Check(!Clipboard::HasTextEXT(), "custom data alone is not text: HasTextEXT is false");

            // Several formats of the same item: a pasting program picks the one it understands.
            Context().Check(Clipboard::SetDataEXT({{TileFormat, tiles}, {"text/plain", Bytes("3 1 4 1 5 9 2 6")}}),
                            "SetDataEXT offers the item in two formats");
            for (const std::string& type : Clipboard::GetMimeTypesEXT())
            {
                Context().Print("offered: {}", type);
                offered_.push_back(type);
            }
            Context().Check(Clipboard::HasDataEXT(TileFormat) && Clipboard::HasTextEXT(),
                            "both formats are offered: the tiles and plain text");
            text_ = Clipboard::GetTextEXT();
            Context().Check(text_ == "3 1 4 1 5 9 2 6", "a text editor would paste the text version");

            Clipboard::SetTextEXT(previous.value_or(""));
        }

        KeyboardState previousKeyboard_;
        std::vector<std::string> offered_;
        std::string text_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(DataFormatsDemo, {
    .id = "input.clipboard.data_formats",
    .title = "Clipboard data formats (CNAEXT)",
    .description = "After a fresh C press in an attended run, copies a custom MIME payload, reads it back and offers "
                   "the item as custom data plus text/plain. It restores prior text when possible and explicitly "
                   "documents that the public API cannot snapshot arbitrary prior formats.",
    .apis = {"CNA::Input::Clipboard::SetDataEXT", "CNA::Input::Clipboard::GetDataEXT",
             "CNA::Input::Clipboard::HasDataEXT", "CNA::Input::Clipboard::GetMimeTypesEXT",
             "CNA::Input::Clipboard::HasTextEXT", "CNA::Input::Clipboard::GetTextEXT"},
    .tags = {"clipboard", "copy", "paste", "mime", "data", "cnaext", "explicit-consent"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = {"Unattended runs never write the host clipboard. C explicitly consents to replacement; CNA can "
                     "restore prior text but cannot snapshot/restore arbitrary foreign MIME formats."},
})
