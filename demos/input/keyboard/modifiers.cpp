// SPDX-License-Identifier: MS-PL
// CNAEXT Keyboard::GetModStateEXT: modifier keys plus the on/off state of the lock keys.
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "CNA/Input/KeyModifiers.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;
using CNA::Input::KeyModifiersEXT;

namespace
{
    struct Flag
    {
        KeyModifiersEXT flag;
        std::string_view label;
    };

    // Held modifiers (either side of the keyboard) and toggled lock states.
    constexpr std::array<Flag, 8> Flags = {{{KeyModifiersEXT::Shift, "Shift"},
                                            {KeyModifiersEXT::Ctrl, "Ctrl"},
                                            {KeyModifiersEXT::Alt, "Alt"},
                                            {KeyModifiersEXT::Gui, "Gui (Windows/Command)"},
                                            {KeyModifiersEXT::Caps, "Caps Lock on"},
                                            {KeyModifiersEXT::Num, "Num Lock on"},
                                            {KeyModifiersEXT::Scroll, "Scroll Lock on"},
                                            {KeyModifiersEXT::Mode, "Mode (AltGr)"}}};

    class ModifiersDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            KeyModifiersEXT composed = KeyModifiersEXT::Shift | KeyModifiersEXT::Ctrl;
            composed |= KeyModifiersEXT::Caps;
            Context().Check((composed & KeyModifiersEXT::Shift) == KeyModifiersEXT::Shift &&
                                (composed & KeyModifiersEXT::Caps) == KeyModifiersEXT::Caps &&
                                (composed & KeyModifiersEXT::Alt) == KeyModifiersEXT::None,
                            "modifier flags compose and mask independently");
            composed &= ~KeyModifiersEXT::Ctrl;
            Context().Check((composed & KeyModifiersEXT::Ctrl) == KeyModifiersEXT::None,
                            "modifier flags can be removed with complement and AND assignment");
        }

        void Update(GameTime& gameTime) override
        {
            // KeyboardState says whether Caps Lock is being pressed; only the modifier state says
            // whether it is on -- what decides between "a" and "A".
            modifiers_ = Keyboard::GetModStateEXT();
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Hold modifiers or toggle the lock keys", Vector2(40.0f, 30.0f),
                                     Color::White);
            for (std::size_t i = 0; i < Flags.size(); ++i)
            {
                const bool set = (modifiers_ & Flags[i].flag) != KeyModifiersEXT::None;
                const int y = 80 + static_cast<int>(i) * 44;
                spriteBatch_->Draw(*pixel_, Rectangle(40, y, 30, 30), set ? Color::LimeGreen : Color::DimGray);
                spriteBatch_->DrawString(*font_, std::string(Flags[i].label), Vector2(90.0f, static_cast<float>(y + 6)),
                                         set ? Color::White : Color::Gray);
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        KeyModifiersEXT modifiers_ = KeyModifiersEXT::None;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(ModifiersDemo, {
    .id = "input.keyboard.modifiers",
    .title = "Modifier and lock keys (CNAEXT)",
    .description = "Shows Keyboard::GetModStateEXT each frame: held Shift, Ctrl, Alt, Gui and AltGr, and whether "
                   "Caps Lock, Num Lock and Scroll Lock are on -- state a KeyboardState does not carry.",
    .apis = {"Keyboard::GetModStateEXT", "CNA::Input::KeyModifiersEXT"},
    .tags = {"keys", "shift", "caps-lock", "num-lock", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
})
