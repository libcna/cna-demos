// SPDX-License-Identifier: MS-PL
// CNAEXT Joysticks: any game controller as raw axes, buttons, hats and balls -- wheels, flight
// sticks and pads that GamePad's Xbox layout does not describe.
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CNA/Input/JoystickCapabilities.hpp"
#include "CNA/Input/JoystickInfo.hpp"
#include "CNA/Input/JoystickState.hpp"
#include "CNA/Input/Joysticks.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace CNA::Input;

namespace
{
    class RawStateDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            JoystickInfoEXT described{17, "Example flight stick", JoystickTypeEXT::FlightStick};
            Context().Check(described == JoystickInfoEXT{17, "Example flight stick", JoystickTypeEXT::FlightStick} &&
                                described != JoystickInfoEXT{18, described.name, described.type},
                            "joystick descriptors compare id, name and physical type");

            JoystickCapabilitiesEXT authoredCaps;
            authoredCaps.isConnected = true;
            authoredCaps.axisCount = 6;
            authoredCaps.buttonCount = 12;
            authoredCaps.hatCount = 1;
            authoredCaps.ballCount = 2;
            authoredCaps.type = JoystickTypeEXT::FlightStick;
            authoredCaps.name = "Example flight stick";
            authoredCaps.guid = "00112233445566778899aabbccddeeff";
            authoredCaps.powerState = PowerStateEXT::Charging;
            authoredCaps.powerPercent = 73;
            JoystickCapabilitiesEXT copiedCaps = authoredCaps;
            copiedCaps.powerPercent--;
            Context().Check(copiedCaps != authoredCaps && copiedCaps.name == authoredCaps.name,
                            "capability snapshots compare every field independently");

            JoystickStateEXT authoredState;
            authoredState.axes = {-32768, 0, 32767};
            authoredState.buttons = {true, false, true};
            authoredState.hats = {JoystickHatPositionEXT::RightDown};
            authoredState.balls = {Point(3, -4)};
            JoystickStateEXT copiedState = authoredState;
            Context().Check(copiedState == authoredState, "raw state snapshots own equal axis, button, hat and ball data");
            copiedState.axes.front()++;
            Context().Check(copiedState != authoredState && authoredState.axes.front() == -32768,
                            "raw state copies own their variable-length data");

            const auto missingId = std::numeric_limits<std::uint32_t>::max();
            Context().Check(Joysticks::GetCapabilitiesEXT(missingId) == JoystickCapabilitiesEXT{} &&
                                Joysticks::GetStateEXT(missingId) == JoystickStateEXT{},
                            "an unknown joystick id has disconnected capabilities and an empty state");
        }

        void Update(GameTime& gameTime) override
        {
            // Joysticks are addressed by an id that stays the same while the device is plugged in.
            const std::vector<JoystickInfoEXT> joysticks = Joysticks::GetJoysticksEXT();
            count_ = joysticks.size();
            if (!joysticks.empty())
            {
                info_ = joysticks.front();
                caps_ = Joysticks::GetCapabilitiesEXT(info_.id);
                state_ = Joysticks::GetStateEXT(info_.id);
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, std::to_string(count_) + " joystick(s) connected", Vector2(20.0f, 15.0f),
                                     Color::White);
            if (count_ > 0)
            {
                spriteBatch_->DrawString(*font_,
                                         "#" + std::to_string(info_.id) + " " + info_.name + " (" +
                                             std::string(CnaDemos::Name(caps_.type)) + ")",
                                         Vector2(20.0f, 40.0f), Color::LightGreen);
                spriteBatch_->DrawString(*font_,
                                         std::to_string(caps_.axisCount) + " axes, " + std::to_string(caps_.buttonCount) +
                                             " buttons, " + std::to_string(caps_.hatCount) + " hats, " +
                                             std::to_string(caps_.ballCount) + " balls",
                                         Vector2(20.0f, 65.0f), Color::LightGray);
                // Axes are raw signed 16-bit values: -32768..32767, centre near 0, no dead zone.
                for (std::size_t i = 0; i < state_.axes.size() && i < 8; ++i)
                {
                    const int y = 100 + static_cast<int>(i) * 28;
                    spriteBatch_->Draw(*pixel_, Rectangle(120, y, 400, 20), Color::DimGray);
                    const int x = 120 + static_cast<int>((static_cast<int>(state_.axes[i]) + 32768) * 400 / 65536);
                    spriteBatch_->Draw(*pixel_, Rectangle(x - 3, y, 6, 20), Color::Orange);
                    spriteBatch_->DrawString(*font_, "axis " + std::to_string(i), Vector2(20.0f, static_cast<float>(y)),
                                             Color::LightGray);
                }
                for (std::size_t i = 0; i < state_.buttons.size() && i < 32; ++i)
                {
                    const Rectangle box(560 + static_cast<int>(i % 8) * 28, 100 + static_cast<int>(i / 8) * 28, 22, 22);
                    spriteBatch_->Draw(*pixel_, box, state_.buttons[i] ? Color::LimeGreen : Color::DimGray);
                }
                for (std::size_t i = 0; i < state_.hats.size() && i < 4; ++i)
                    spriteBatch_->DrawString(*font_, "hat " + std::to_string(i) + ": " +
                                                         std::string(CnaDemos::Name(state_.hats[i])),
                                             Vector2(560.0f, 230.0f + 25.0f * static_cast<float>(i)), Color::LightGray);
            }
            else
            {
                spriteBatch_->DrawString(*font_, "Connect a joystick, wheel or gamepad", Vector2(20.0f, 40.0f),
                                         Color::Gray);
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::size_t count_ = 0;
        JoystickInfoEXT info_;
        JoystickCapabilitiesEXT caps_;
        JoystickStateEXT state_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(RawStateDemo, {
    .id = "input.joystick.raw_state",
    .title = "Raw joystick state (CNAEXT)",
    .description = "Lists controllers with Joysticks::GetJoysticksEXT and shows the first one's capabilities and raw "
                   "state -- 16-bit axes, buttons, hats and trackballs -- while also checking the documented "
                   "disconnected result without requiring hardware.",
    .apis = {"CNA::Input::Joysticks::GetJoysticksEXT", "CNA::Input::Joysticks::GetCapabilitiesEXT",
             "CNA::Input::Joysticks::GetStateEXT", "CNA::Input::JoystickInfoEXT",
             "CNA::Input::JoystickCapabilitiesEXT", "CNA::Input::JoystickStateEXT",
             "CNA::Input::JoystickHatPositionEXT", "CNA::Input::JoystickTypeEXT"},
    .tags = {"joystick", "wheel", "flight-stick", "controller", "raw", "cnaext"},
    .automation = CnaDemos::Automation::Automated,
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb"},
})
