// SPDX-License-Identifier: MS-PL
// CNAEXT Haptics: open a force-feedback device and play simple rumble.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CNA/Input/HapticCapabilities.hpp"
#include "CNA/Input/HapticDevice.hpp"
#include "CNA/Input/HapticInfo.hpp"
#include "CNA/Input/Haptics.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;
using namespace CNA::Input;

namespace
{
    class RumbleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            Context().Check(!device_.IsOpenEXT(), "a default-constructed haptic member starts closed");
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            mouseHaptic_ = Haptics::IsMouseHapticEXT();
            Context().Check(mouseHaptic_ == Haptics::IsMouseHapticEXT(),
                            "mouse haptic capability inspection is stable and does not open the device");
            haptics_ = Haptics::GetHapticsEXT();
            status_ = haptics_.empty() ? "No force-feedback device found"
                                       : "O: explicitly open and initialize \"" + haptics_.front().name + "\"";
            previous_ = Keyboard::GetState();
            Context().Print("{}", status_);
        }

        void UnloadContent() override
        {
            if (playedRumble_)
                device_.StopRumbleEXT();
            device_.Dispose();
        }

        void Update(GameTime& gameTime) override
        {
            const KeyboardState keyboard = Keyboard::GetState();
            if (!Context().Automated() && !device_.IsOpenEXT() && !haptics_.empty() &&
                keyboard.IsKeyDown(Keys::O) && previous_.IsKeyUp(Keys::O))
            {
                device_ = Haptics::OpenEXT(haptics_.front().id);
                if (device_.IsOpenEXT())
                {
                    const HapticCapabilitiesEXT caps = device_.GetCapabilitiesEXT();
                    rumbleReady_ = caps.rumbleSupported && device_.InitRumbleEXT();
                    status_ = "Opened \"" + device_.GetNameEXT() + "\", " + std::to_string(caps.axisCount) +
                              " axes, " + (rumbleReady_ ? "Space: rumble" : "no simple rumble");
                }
                else
                    status_ = "Selected device disconnected before it could be opened";
            }
            if (!Context().Automated() && rumbleReady_ && keyboard.IsKeyDown(Keys::Space) &&
                previous_.IsKeyUp(Keys::Space))
                playedRumble_ = device_.PlayRumbleEXT(0.75f, 300) || playedRumble_;
            previous_ = keyboard;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, status_, Vector2(30.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, rumbleReady_ ? "Space: rumble for 300 ms" : "(nothing to rumble)",
                                     Vector2(30.0f, 60.0f), Color::LightGray);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        HapticDevice device_;
        bool rumbleReady_ = false;
        bool playedRumble_ = false;
        bool mouseHaptic_ = false;
        std::vector<HapticInfoEXT> haptics_;
        std::string status_;
        KeyboardState previous_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(RumbleDemo, {
    .id = "input.haptics.rumble",
    .title = "Force-feedback rumble (CNAEXT)",
    .description = "Enumerates haptic devices and mouse capability without opening them. In an attended run, O opens "
                   "and initializes the first device and a later Space press requests 300 ms of simple rumble; "
                   "owned output and the handle are released on shutdown.",
    .apis = {"CNA::Input::Haptics::GetHapticsEXT", "CNA::Input::Haptics::OpenEXT",
             "CNA::Input::Haptics::IsMouseHapticEXT",
             "CNA::Input::HapticDevice::IsOpenEXT", "CNA::Input::HapticDevice::GetNameEXT",
             "CNA::Input::HapticDevice::GetCapabilitiesEXT", "CNA::Input::HapticDevice::InitRumbleEXT",
             "CNA::Input::HapticDevice::PlayRumbleEXT", "CNA::Input::HapticDevice::StopRumbleEXT",
             "CNA::Input::HapticDevice::Dispose"},
    .tags = {"haptics", "force-feedback", "rumble", "joystick", "cnaext", "explicit-consent"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = {"Unattended runs only enumerate. Device opening/initialization and output require separate fresh "
                     "O and Space presses in an attended run."},
})
