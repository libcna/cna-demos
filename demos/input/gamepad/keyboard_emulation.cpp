// SPDX-License-Identifier: MS-PL
// CNAEXT gamepad keyboard emulation: the keyboard drives player one's GamePadState.
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Buttons.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadCapabilities.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadState.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadType.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    constexpr std::array<Buttons, 14> Watched = {
        Buttons::A,           Buttons::B,          Buttons::X,           Buttons::Y,
        Buttons::LeftShoulder, Buttons::RightShoulder, Buttons::LeftStick, Buttons::RightStick,
        Buttons::Start,       Buttons::Back,       Buttons::DPadUp,      Buttons::DPadDown,
        Buttons::DPadLeft,    Buttons::DPadRight};

    class KeyboardEmulationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // With emulation on, player one is connected even without a controller: game code
            // written for a gamepad can be played (and tested) from the keyboard. A physical
            // controller in slot one keeps working; the two are merged.
            physicalPad_ = GamePad::GetCapabilities(PlayerIndex::One).getIsConnectedProperty();
            GamePad::setKeyboardEmulationEnabledEXT(true);
            const GamePadCapabilities caps = GamePad::GetCapabilities(PlayerIndex::One);
            Context().Check(GamePad::getKeyboardEmulationEnabledEXT(), "keyboard emulation reads back as enabled");
            Context().Check(caps.getIsConnectedProperty() && GamePad::GetState(PlayerIndex::One).getIsConnectedProperty(),
                            "player one is connected");
            Context().Check(caps.getGamePadTypeProperty() == GamePadType::GamePad && caps.getHasAButtonProperty() &&
                                caps.getHasLeftXThumbStickProperty() && caps.getHasRightTriggerProperty(),
                            "it reports a full gamepad: buttons, sticks and triggers");
        }

        void UnloadContent() override
        {
            // A process-wide switch: turn it off again.
            GamePad::setKeyboardEmulationEnabledEXT(false);
            if (!physicalPad_)
                Context().Check(!GamePad::GetCapabilities(PlayerIndex::One).getIsConnectedProperty(),
                                "with emulation off and no controller, player one is disconnected again");
        }

        void Update(GameTime& gameTime) override
        {
            pad_ = GamePad::GetState(PlayerIndex::One);
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            const std::array<std::string_view, 6> legend = {
                "W A S D: left stick       arrows: right stick",
                "K L J I: A B X Y          Q E: shoulders",
                "Z C: triggers             T F G H: D-pad",
                "Enter: Start   Escape: Back (also quits)",
                "Left/Right Shift: stick buttons",
                ""};
            float y = 20.0f;
            for (const std::string_view line : legend)
            {
                spriteBatch_->DrawString(*font_, std::string(line), Vector2(30.0f, y), Color::White);
                y += 26.0f;
            }

            const Vector2 left = pad_.getThumbSticksProperty().getLeftProperty();
            const Vector2 right = pad_.getThumbSticksProperty().getRightProperty();
            spriteBatch_->DrawString(*font_, "Left stick  " + Format(left), Vector2(30.0f, 190.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, "Right stick " + Format(right), Vector2(30.0f, 220.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_,
                                     "Triggers    " + std::to_string(pad_.getTriggersProperty().getLeftProperty()).substr(0, 4) +
                                         ", " + std::to_string(pad_.getTriggersProperty().getRightProperty()).substr(0, 4),
                                     Vector2(30.0f, 250.0f), Color::LightGray);
            std::string down = "Buttons down:";
            for (const Buttons button : Watched)
            {
                if (pad_.IsButtonDown(button))
                    down += " " + std::string(CnaDemos::Name(button));
            }
            spriteBatch_->DrawString(*font_, down, Vector2(30.0f, 290.0f), Color::LightGreen);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        static std::string Format(Vector2 value)
        {
            return std::to_string(value.X).substr(0, 5) + ", " + std::to_string(value.Y).substr(0, 5);
        }

        bool physicalPad_ = false;
        GamePadState pad_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(KeyboardEmulationDemo, {
    .id = "input.gamepad.keyboard_emulation",
    .title = "Gamepad from the keyboard (CNAEXT)",
    .description = "Turns on GamePad::setKeyboardEmulationEnabledEXT: player one becomes a connected gamepad whose "
                   "sticks, triggers and buttons are driven by keys, shown live; emulation is switched off on unload.",
    .apis = {"GamePad::KeyboardEmulationEnabledEXT", "GamePad::GetCapabilities", "GamePad::GetState",
             "GamePadState::IsButtonDown"},
    .tags = {"gamepad", "controller", "keyboard", "emulation", "testing", "cnaext"},
    .needs = CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
})
