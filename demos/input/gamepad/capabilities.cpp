// SPDX-License-Identifier: MS-PL
// GamePad::GetCapabilities: which controllers are connected, what kind they are, what they have.
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadCapabilities.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadType.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    // A short summary of the controls a controller reports.
    std::string Controls(const GamePadCapabilities& caps)
    {
        std::string text;
        const auto add = [&text](bool has, const char* name) {
            if (has)
                text += text.empty() ? name : std::string(" ") + name;
        };
        add(caps.getHasAButtonProperty() && caps.getHasBButtonProperty() && caps.getHasXButtonProperty() &&
                caps.getHasYButtonProperty(),
            "ABXY");
        add(caps.getHasDPadUpButtonProperty() && caps.getHasDPadDownButtonProperty() &&
                caps.getHasDPadLeftButtonProperty() && caps.getHasDPadRightButtonProperty(),
            "D-pad");
        add(caps.getHasLeftXThumbStickProperty() && caps.getHasLeftYThumbStickProperty(), "left-stick");
        add(caps.getHasRightXThumbStickProperty() && caps.getHasRightYThumbStickProperty(), "right-stick");
        add(caps.getHasLeftTriggerProperty() && caps.getHasRightTriggerProperty(), "triggers");
        add(caps.getHasLeftShoulderButtonProperty() && caps.getHasRightShoulderButtonProperty(), "shoulders");
        add(caps.getHasStartButtonProperty(), "Start");
        add(caps.getHasBackButtonProperty(), "Back");
        add(caps.getHasBigButtonProperty(), "BigButton");
        add(caps.getHasLeftVibrationMotorProperty() || caps.getHasRightVibrationMotorProperty(), "vibration");
        add(caps.getHasVoiceSupportProperty(), "voice");
        return text;
    }

    bool HasAnything(const GamePadCapabilities& caps)
    {
        return caps.getHasAButtonProperty() || caps.getHasBButtonProperty() || caps.getHasXButtonProperty() ||
               caps.getHasYButtonProperty() || caps.getHasBackButtonProperty() || caps.getHasStartButtonProperty() ||
               caps.getHasBigButtonProperty() || caps.getHasDPadUpButtonProperty() ||
               caps.getHasLeftXThumbStickProperty() || caps.getHasRightXThumbStickProperty() ||
               caps.getHasLeftTriggerProperty() || caps.getHasRightTriggerProperty() ||
               caps.getHasLeftVibrationMotorProperty() || caps.getHasRightVibrationMotorProperty();
    }

    class CapabilitiesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Update(GameTime& gameTime) override
        {
            // Capabilities can change while the game runs: check again when a controller connects.
            for (int i = 0; i < 4; ++i)
            {
                const auto player = static_cast<PlayerIndex>(i);
                const GamePadCapabilities caps = GamePad::GetCapabilities(player);
                lines_[static_cast<std::size_t>(i)] =
                    "Player " + std::to_string(i + 1) + ": " +
                    (caps.getIsConnectedProperty() ? std::string(CnaDemos::Name(caps.getGamePadTypeProperty())) + ", " +
                                                         Controls(caps)
                                                   : std::string("not connected"));
                if (!checked_)
                {
                    Context().Print("{}", lines_[static_cast<std::size_t>(i)]);
                    Context().Check(caps.getIsConnectedProperty() ==
                                        GamePad::GetState(player).getIsConnectedProperty(),
                                    "player " + std::to_string(i + 1) +
                                        ": GetCapabilities and GetState agree about the connection");
                    if (!caps.getIsConnectedProperty())
                        Context().Check(!HasAnything(caps),
                                        "player " + std::to_string(i + 1) + ": a missing controller reports no controls");
                }
            }
            checked_ = true;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "GamePad::GetCapabilities", Vector2(30.0f, 30.0f), Color::White);
            for (std::size_t i = 0; i < lines_.size(); ++i)
                spriteBatch_->DrawString(*font_, lines_[i], Vector2(30.0f, 80.0f + 40.0f * static_cast<float>(i)),
                                         Color::LightGray);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::array<std::string, 4> lines_;
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(CapabilitiesDemo, {
    .id = "input.gamepad.capabilities",
    .title = "Gamepad capabilities",
    .description = "Lists GamePad::GetCapabilities for the four players -- connected or not, the GamePadType and "
                   "which buttons, sticks, triggers and motors the controller has.",
    .apis = {"GamePad::GetCapabilities", "GamePadCapabilities::IsConnected", "GamePadCapabilities::GamePadType",
             "GamePadCapabilities::HasAButton", "GamePadCapabilities::HasBButton", "GamePadCapabilities::HasXButton",
             "GamePadCapabilities::HasYButton", "GamePadCapabilities::HasBackButton",
             "GamePadCapabilities::HasStartButton", "GamePadCapabilities::HasBigButton",
             "GamePadCapabilities::HasDPadUpButton", "GamePadCapabilities::HasDPadDownButton",
             "GamePadCapabilities::HasDPadLeftButton", "GamePadCapabilities::HasDPadRightButton",
             "GamePadCapabilities::HasLeftShoulderButton", "GamePadCapabilities::HasRightShoulderButton",
             "GamePadCapabilities::HasLeftXThumbStick", "GamePadCapabilities::HasLeftYThumbStick",
             "GamePadCapabilities::HasRightXThumbStick", "GamePadCapabilities::HasRightYThumbStick",
             "GamePadCapabilities::HasLeftTrigger", "GamePadCapabilities::HasRightTrigger",
             "GamePadCapabilities::HasLeftVibrationMotor", "GamePadCapabilities::HasRightVibrationMotor",
             "GamePadCapabilities::HasVoiceSupport", "GamePadType"},
    .tags = {"gamepad", "controller", "capabilities", "connected"},
    .needs = CnaDemos::Need::GamePad,
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb"},
})
