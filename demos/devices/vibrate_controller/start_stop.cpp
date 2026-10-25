// SPDX-License-Identifier: MS-PL
// Microsoft.Devices.VibrateController: capability checks plus explicitly requested output.
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Devices/VibrateController.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/TimeSpan.hpp"

using Microsoft::Devices::VibrateController;
using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class StartStopDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            vibrate_ = VibrateController::getDefaultProperty();
            Context().Check(vibrate_ != nullptr && vibrate_ == VibrateController::getDefaultProperty(),
                            "Default is the one shared controller");
            supported_ = vibrate_->getIsSupportedProperty();
            deviceName_ = vibrate_->getDeviceNameProperty();
            Context().Check(supported_ == vibrate_->getIsSupportedProperty() &&
                                deviceName_ == vibrate_->getDeviceNameProperty() &&
                                (!supported_ || !deviceName_.empty()),
                            "support and diagnostic device name form a stable capability snapshot");

            bool overlongRefused = false;
            bool negativeRefused = false;
            bool dualOverlongRefused = false;
            try { vibrate_->Start(System::TimeSpan::FromSeconds(6.0)); }
            catch (const System::ArgumentOutOfRangeException&) { overlongRefused = true; }
            try { vibrate_->Start(System::TimeSpan::FromMilliseconds(-1.0)); }
            catch (const System::ArgumentOutOfRangeException&) { negativeRefused = true; }
            try { vibrate_->StartLeftRight(0.5f, 0.5f, System::TimeSpan::FromSeconds(6.0)); }
            catch (const System::ArgumentOutOfRangeException&) { dualOverlongRefused = true; }
            Context().Check(overlongRefused && negativeRefused && dualOverlongRefused,
                            "invalid durations are rejected before device access");

            previous_ = Keyboard::GetState();
            Context().Print("VibrateController support: {}; device: {}; Space=full, I=intensity, D=dual motor",
                            supported_, deviceName_.empty() ? "none" : deviceName_);
        }

        void UnloadContent() override { StopOwnedOutput(); }

        void Update(GameTime& gameTime) override
        {
            const KeyboardState keyboard = Keyboard::GetState();
            if (!Context().Automated())
            {
                if (Fresh(keyboard, Keys::Space))
                    StartFull(gameTime, System::TimeSpan::FromMilliseconds(150.0));
                else if (Fresh(keyboard, Keys::I))
                    StartIntensity(gameTime, System::TimeSpan::FromMilliseconds(300.0), 0.3f);
                else if (Fresh(keyboard, Keys::D))
                    StartDual(gameTime, System::TimeSpan::FromMilliseconds(300.0));
            }
            previous_ = keyboard;

            if (ownsOutput_ && gameTime.getTotalGameTimeProperty().getTotalSecondsProperty() >= stopAtSeconds_)
                StopOwnedOutput();
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(ownsOutput_ ? Color::OrangeRed : Color::DarkSlateGray);
            Game::Draw(gameTime);
        }

    private:
        [[nodiscard]] bool Fresh(const KeyboardState& keyboard, Keys key) const
        {
            return keyboard.IsKeyDown(key) && previous_.IsKeyUp(key);
        }

        void BeginOwned(const GameTime& gameTime, System::TimeSpan duration)
        {
            ownsOutput_ = true;
            stopAtSeconds_ = gameTime.getTotalGameTimeProperty().getTotalSecondsProperty() +
                duration.getTotalSecondsProperty();
        }

        void StartFull(const GameTime& gameTime, System::TimeSpan duration)
        {
            StopOwnedOutput();
            vibrate_->Start(duration);
            BeginOwned(gameTime, duration);
        }

        void StartIntensity(const GameTime& gameTime, System::TimeSpan duration, float intensity)
        {
            StopOwnedOutput();
            vibrate_->Start(duration, intensity);
            BeginOwned(gameTime, duration);
        }

        void StartDual(const GameTime& gameTime, System::TimeSpan duration)
        {
            StopOwnedOutput();
            vibrate_->StartLeftRight(0.8f, 0.25f, duration);
            BeginOwned(gameTime, duration);
        }

        void StopOwnedOutput()
        {
            if (!ownsOutput_)
                return;
            vibrate_->Stop();
            ownsOutput_ = false;
        }

        VibrateController* vibrate_ = nullptr;
        KeyboardState previous_;
        bool supported_ = false;
        bool ownsOutput_ = false;
        double stopAtSeconds_ = 0.0;
        std::string deviceName_;
    };
}

CNA_DEMOS_GAME(StartStopDemo, {
    .id = "devices.vibrate_controller.start_stop",
    .title = "Vibration",
    .description = "Inspects VibrateController capability and duration validation automatically. Fresh Space, I or D "
                   "presses in an attended run request full, intensity or dual-motor vibration; only output owned by "
                   "this demo is stopped on timeout or shutdown.",
    .apis = {"VibrateController::Default", "VibrateController::IsSupported", "VibrateController::DeviceName",
             "VibrateController::Start", "VibrateController::StartLeftRight", "VibrateController::Stop"},
    .tags = {"devices", "vibration", "haptics", "phone", "feedback", "explicit-consent"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard,
    .restrictions = {"Unattended runs never issue a valid Start call; physical output requires a fresh key press."},
})
