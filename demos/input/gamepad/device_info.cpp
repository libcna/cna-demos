// SPDX-License-Identifier: MS-PL
// CNAEXT gamepad device information: name, GUID, connection, battery and the face-button labels.
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CNA/Input/GamePadButtonLabel.hpp"
#include "CNA/Input/GamePadConnectionState.hpp"
#include "CNA/Input/PowerState.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Buttons.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    // Everything CNA can tell about the controller of one player, as lines of text.
    std::vector<std::string> Describe(PlayerIndex player)
    {
        const int number = static_cast<int>(player) + 1;
        if (!GamePad::GetState(player).getIsConnectedProperty())
            return {"Player " + std::to_string(number) + ": not connected"};

        int percent = -1;
        const CNA::Input::PowerStateEXT power = GamePad::GetPowerInfoEXT(player, percent);
        // On a PlayStation controller Buttons::A is the cross: prompts should say so.
        std::string labels;
        for (const Buttons button : {Buttons::A, Buttons::B, Buttons::X, Buttons::Y})
            labels += std::string(labels.empty() ? "" : ", ") + std::string(CnaDemos::Name(button)) + " is " +
                      std::string(CnaDemos::Name(GamePad::GetButtonLabelEXT(player, button)));
        return {
            "Player " + std::to_string(number) + ": " + GamePad::GetNameEXT(player),
            "  GUID " + GamePad::GetGUIDEXT(player),
            "  " + std::string(CnaDemos::Name(GamePad::GetConnectionStateEXT(player))) + ", battery " +
                std::string(CnaDemos::Name(power)) + (percent >= 0 ? " " + std::to_string(percent) + "%" : ""),
            "  serial \"" + GamePad::GetSerialEXT(player) + "\", firmware " +
                std::to_string(GamePad::GetFirmwareVersionEXT(player)) + ", LED index " +
                std::to_string(GamePad::GetPlayerIndexEXT(player)),
            "  path " + GamePad::GetPathEXT(player) + ", Steam handle " +
                std::to_string(GamePad::GetSteamHandleEXT(player)),
            "  " + labels,
        };
    }

    class DeviceInfoDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pWasDown_ = Keyboard::GetState().IsKeyDown(Keys::P);
        }

        void Update(GameTime& gameTime) override
        {
            // Writing a controller's hardware LED is deliberately opt-in. Save the value the
            // device reported before the first change so leaving the demo can restore it.
            const bool pDown = Keyboard::GetState().IsKeyDown(Keys::P);
            if (!Context().Automated() && pDown && !pWasDown_)
            {
                bool found = false;
                for (int i = 0; i < 4 && !found; ++i)
                {
                    const auto player = static_cast<PlayerIndex>(i);
                    if (!GamePad::GetState(player).getIsConnectedProperty())
                        continue;
                    found = true;
                    if (!changed_[static_cast<std::size_t>(i)])
                        originalLed_[static_cast<std::size_t>(i)] = GamePad::GetPlayerIndexEXT(player);
                    const int current = GamePad::GetPlayerIndexEXT(player);
                    const int next = current < 0 ? 0 : (current + 1) % 4;
                    const bool accepted = GamePad::SetPlayerIndexEXT(player, next);
                    changed_[static_cast<std::size_t>(i)] =
                        changed_[static_cast<std::size_t>(i)] || accepted;
                    Context().Print("player {} SetPlayerIndexEXT({}, LED {})", i + 1, next,
                                    accepted ? "accepted" : "refused");
                }
                if (!found)
                    Context().Print("SetPlayerIndexEXT: no controller is connected");
            }
            pWasDown_ = pDown;

            lines_.clear();
            lines_.push_back("Press P to cycle the first connected controller's player LED; it is restored on exit.");
            for (int i = 0; i < 4; ++i)
            {
                for (const std::string& line : Describe(static_cast<PlayerIndex>(i)))
                    lines_.push_back(line);
            }
            if (!printed_)
            {
                for (const std::string& line : lines_)
                    Context().Print("{}", line);
                printed_ = true;
            }
            Game::Update(gameTime);
        }

        void UnloadContent() override
        {
            for (int i = 0; i < 4; ++i)
            {
                if (changed_[static_cast<std::size_t>(i)])
                    GamePad::SetPlayerIndexEXT(static_cast<PlayerIndex>(i), originalLed_[static_cast<std::size_t>(i)]);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            float y = 20.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(20.0f, y), Color::White);
                y += 24.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::vector<std::string> lines_;
        bool printed_ = false;
        bool pWasDown_ = false;
        std::array<int, 4> originalLed_ = {-1, -1, -1, -1};
        std::array<bool, 4> changed_ = {false, false, false, false};
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(DeviceInfoDemo, {
    .id = "input.gamepad.device_info",
    .title = "Gamepad device information (CNAEXT)",
    .description = "Lists what CNA reports about each connected controller: GetNameEXT, GetGUIDEXT, "
                   "GetConnectionStateEXT, GetPowerInfoEXT, serial, firmware, LED player index and "
                   "GetButtonLabelEXT for the face buttons. Press P to cycle the first connected controller's "
                   "player-number LED with SetPlayerIndexEXT; the original value is restored on exit.",
    .apis = {"GamePad::GetNameEXT", "GamePad::GetGUIDEXT", "GamePad::GetConnectionStateEXT",
             "GamePad::GetPowerInfoEXT", "GamePad::GetSerialEXT", "GamePad::GetFirmwareVersionEXT",
             "GamePad::GetPlayerIndexEXT", "GamePad::SetPlayerIndexEXT", "GamePad::GetPathEXT",
             "GamePad::GetSteamHandleEXT",
             "GamePad::GetButtonLabelEXT", "CNA::Input::GamePadButtonLabelEXT",
             "CNA::Input::GamePadConnectionStateEXT", "CNA::Input::PowerStateEXT"},
    .tags = {"gamepad", "controller", "battery", "player-led", "playstation", "button-prompts", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Writing the LED needs a fresh P press in an attended run and a connected controller that "
                    "implements the SDL player-index command; unattended runs only exercise read-only enumeration.",
})
