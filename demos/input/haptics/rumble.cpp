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
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // A haptic device is opened by id (from GetHapticsEXT), from a joystick, or from the
            // mouse. Where there is none, the returned device simply is not open.
            if (!Haptics::IsMouseHapticEXT())
                Context().Check(!Haptics::OpenFromMouseEXT().IsOpenEXT(),
                                "without a force-feedback mouse OpenFromMouseEXT gives a closed device");
            const std::vector<HapticInfoEXT> haptics = Haptics::GetHapticsEXT();
            if (!haptics.empty())
                device_.emplace(Haptics::OpenEXT(haptics.front().id));

            if (device_ && device_->IsOpenEXT())
            {
                const HapticCapabilitiesEXT caps = device_->GetCapabilitiesEXT();
                status_ = "Opened \"" + device_->GetNameEXT() + "\", " + std::to_string(caps.axisCount) + " axes, " +
                          (caps.rumbleSupported ? "rumble supported" : "no simple rumble");
                // Simple rumble needs one initialisation, then any number of PlayRumbleEXT calls.
                rumbleReady_ = caps.rumbleSupported && device_->InitRumbleEXT();
            }
            else
            {
                status_ = "No force-feedback device found";
            }
            Context().Print("{}", status_);
        }

        void UnloadContent() override
        {
            if (rumbleReady_)
                device_->StopRumbleEXT();
            if (device_)
                device_->Dispose();
        }

        void Update(GameTime& gameTime) override
        {
            // Space: a 300 ms rumble at 75 % strength.
            const KeyboardState keyboard = Keyboard::GetState();
            if (rumbleReady_ && keyboard.IsKeyDown(Keys::Space) && previous_.IsKeyUp(Keys::Space))
                device_->PlayRumbleEXT(0.75f, 300);
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
        // Optional rather than a default-constructed closed device: CNA-DEMOS-ISSUE-009.
        std::optional<HapticDevice> device_;
        bool rumbleReady_ = false;
        std::string status_;
        KeyboardState previous_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(RumbleDemo, {
    .id = "input.haptics.rumble",
    .title = "Force-feedback rumble (CNAEXT)",
    .description = "Opens the first device from Haptics::GetHapticsEXT, reads its capabilities and plays a simple "
                   "rumble with InitRumbleEXT/PlayRumbleEXT on Space; a missing device opens closed instead of "
                   "throwing.",
    .apis = {"CNA::Input::Haptics::GetHapticsEXT", "CNA::Input::Haptics::OpenEXT",
             "CNA::Input::Haptics::OpenFromMouseEXT", "CNA::Input::Haptics::IsMouseHapticEXT",
             "CNA::Input::HapticDevice::IsOpenEXT", "CNA::Input::HapticDevice::GetNameEXT",
             "CNA::Input::HapticDevice::GetCapabilitiesEXT", "CNA::Input::HapticDevice::InitRumbleEXT",
             "CNA::Input::HapticDevice::PlayRumbleEXT", "CNA::Input::HapticDevice::StopRumbleEXT",
             "CNA::Input::HapticDevice::Dispose"},
    .tags = {"haptics", "force-feedback", "rumble", "joystick", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad,
    .assets = {"fonts/pixel_font.cnb"},
})
