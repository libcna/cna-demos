// SPDX-License-Identifier: MS-PL
// CNAEXT haptic effects: describe a force (shape, strength, timing), upload it, run it.
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CNA/Input/HapticCapabilities.hpp"
#include "CNA/Input/HapticDevice.hpp"
#include "CNA/Input/HapticEffect.hpp"
#include "CNA/Input/HapticEffectType.hpp"
#include "CNA/Input/HapticFeature.hpp"
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
    HapticEffectEXT FullyAuthoredCustomEffect()
    {
        HapticEffectEXT effect;
        effect.type = HapticEffectTypeEXT::Custom;
        effect.direction = {HapticDirectionTypeEXT::Cartesian, {100, -200, 300}};
        effect.length = HapticEffectEXT::InfiniteLengthEXT;
        effect.delay = 11;
        effect.button = 2;
        effect.interval = 13;
        effect.level = -1200;
        effect.period = 17;
        effect.magnitude = 1800;
        effect.offset = -1900;
        effect.phase = 2000;
        effect.rampStart = -2100;
        effect.rampEnd = 2200;
        effect.rightSaturation = {2300, 2400, 2500};
        effect.leftSaturation = {2600, 2700, 2800};
        effect.rightCoefficient = {-2900, 3000, -3100};
        effect.leftCoefficient = {3200, -3300, 3400};
        effect.deadband = {3500, 3600, 3700};
        effect.center = {-3800, 3900, -4000};
        effect.largeMagnitude = 4100;
        effect.smallMagnitude = 4200;
        effect.customChannels = 2;
        effect.customPeriod = 43;
        effect.customData = {4400, 4500, 4600, 4700};
        effect.attackLength = 48;
        effect.attackLevel = 4900;
        effect.fadeLength = 50;
        effect.fadeLevel = 5100;
        return effect;
    }

    // Three classic effects. Strengths are signed 16-bit, times in milliseconds.
    HapticEffectEXT Sine()
    {
        HapticEffectEXT effect;
        effect.type = HapticEffectTypeEXT::Sine;  // a periodic wave
        effect.length = 1000;
        effect.period = 100;
        effect.magnitude = 20000;
        return effect;
    }

    HapticEffectEXT Constant()
    {
        HapticEffectEXT effect;
        effect.type = HapticEffectTypeEXT::Constant;  // a steady push, faded in and out
        effect.length = 800;
        effect.level = 15000;
        effect.attackLength = 200;
        effect.fadeLength = 300;
        return effect;
    }

    HapticEffectEXT Ramp()
    {
        HapticEffectEXT effect;
        effect.type = HapticEffectTypeEXT::Ramp;  // growing from nothing to strong
        effect.length = 1000;
        effect.rampStart = 0;
        effect.rampEnd = 25000;
        return effect;
    }

    class EffectsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            Context().Check(!device_.IsOpenEXT(), "a default-constructed haptic member starts closed");
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            const HapticEffectEXT authored = FullyAuthoredCustomEffect();
            HapticEffectEXT copied = authored;
            Context().Check(copied == authored && copied.direction == authored.direction &&
                                copied.length == HapticEffectEXT::InfiniteLengthEXT,
                            "effect records preserve every authored field and direction");
            copied.customData.front()++;
            Context().Check(copied != authored && copied.direction != HapticDirectionEXT{},
                            "effect copies own custom samples and compare every field");

            HapticFeatureEXT features = HapticFeatureEXT::Constant | HapticFeatureEXT::Custom;
            features |= HapticFeatureEXT::Pause;
            Context().Check((features & HapticFeatureEXT::Custom) == HapticFeatureEXT::Custom &&
                                (features & HapticFeatureEXT::Gain) == HapticFeatureEXT::None,
                            "haptic feature flags compose and mask independently");
            features &= ~HapticFeatureEXT::Constant;
            Context().Check((features & HapticFeatureEXT::Constant) == HapticFeatureEXT::None,
                            "haptic feature flags can be removed with complement and AND assignment");

            const HapticCapabilitiesEXT unavailableCaps;
            Context().Check(unavailableCaps == HapticCapabilitiesEXT{} &&
                                unavailableCaps.maxEffects == -1 && unavailableCaps.maxEffectsPlaying == -1,
                            "default capabilities describe a closed unknown device");
            const HapticInfoEXT described{42, "Example force-feedback device"};
            Context().Check(described == HapticInfoEXT{42, "Example force-feedback device"} &&
                                described != HapticInfoEXT{43, described.name},
                            "haptic descriptors compare both id and name");

            const auto missingId = std::numeric_limits<std::uint32_t>::max();
            Context().Check(!Haptics::IsJoystickHapticEXT(missingId) &&
                                !Haptics::OpenFromJoystickEXT(missingId).IsOpenEXT(),
                            "a missing joystick is not haptic and opens as a closed device");
            HapticDevice missing = Haptics::OpenEXT(missingId);
            const HapticCapabilitiesEXT closedCaps = missing.GetCapabilitiesEXT();
            Context().Check(!missing.IsOpenEXT() && missing.GetNameEXT().empty() &&
                                closedCaps == unavailableCaps,
                            "a missing standalone id has canonical closed-device capabilities");
            Context().Check(!missing.IsEffectSupportedEXT(authored) && !missing.InitRumbleEXT() &&
                                !missing.PlayRumbleEXT(0.5f, 100) && !missing.StopRumbleEXT() &&
                                missing.CreateEffectEXT(authored) == -1,
                            "closed devices reject effect creation and simple rumble safely");
            Context().Check(!missing.UpdateEffectEXT(7, authored) && !missing.RunEffectEXT(7) &&
                                !missing.StopEffectEXT(7) && !missing.GetEffectStatusEXT(7) &&
                                !missing.StopAllEffectsEXT(),
                            "closed devices reject the uploaded-effect lifecycle safely");
            missing.DestroyEffectEXT(7);
            Context().Check(!missing.SetGainEXT(50) && !missing.SetAutocenterEXT(50) &&
                                !missing.PauseEXT() && !missing.ResumeEXT(),
                            "closed devices reject global haptic controls safely");
            missing.Dispose();
            missing.Dispose();
            Context().Check(!missing.IsOpenEXT(), "disposing a closed haptic device twice is safe");

            haptics_ = Haptics::GetHapticsEXT();
            lines_.push_back("Read-only inventory: " + std::to_string(haptics_.size()) + " haptic device(s)");
            for (const HapticInfoEXT& info : haptics_)
                lines_.push_back("  #" + std::to_string(info.id) + ": " + info.name);
            if (haptics_.empty())
            {
                lines_.push_back("No force-feedback device found");
                lines_.push_back("Automated checks use the deterministic closed-device contract above");
            }
            lines_.push_back("O: explicitly open first device and upload effects; then keys 1-3 run them");
            lines_.push_back("Gain is never changed: the public API cannot read and restore its prior value");
            previous_ = Keyboard::GetState();
        }

        void UnloadContent() override
        {
            if (device_.IsOpenEXT())
            {
                for (const int id : ids_)
                {
                    if (id >= 0)
                    {
                        device_.StopEffectEXT(id);
                        device_.DestroyEffectEXT(id);
                    }
                }
            }
            device_.Dispose();
        }

        void Update(GameTime& gameTime) override
        {
            const KeyboardState keyboard = Keyboard::GetState();
            if (!Context().Automated() && !device_.IsOpenEXT() && !haptics_.empty() &&
                keyboard.IsKeyDown(Keys::O) && previous_.IsKeyUp(Keys::O))
            {
                OpenAndUpload();
            }
            const Keys keys[] = {Keys::D1, Keys::D2, Keys::D3};
            for (int i = 0; i < 3; ++i)
            {
                if (!Context().Automated() && ids_[i] >= 0 && keyboard.IsKeyDown(keys[i]) &&
                    previous_.IsKeyUp(keys[i]))
                    device_.RunEffectEXT(ids_[i], 1);
            }
            previous_ = keyboard;
            playing_ = false;
            for (const int id : ids_)
                playing_ = playing_ || (hasStatus_ && id >= 0 && device_.GetEffectStatusEXT(id));
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(playing_ ? Color::DarkOliveGreen : Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            float y = 30.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(30.0f, y), Color::White);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void OpenAndUpload()
        {
            device_ = Haptics::OpenEXT(haptics_.front().id);
            if (!device_.IsOpenEXT())
            {
                lines_.push_back("The selected device could not be opened (disconnected or unavailable)");
                return;
            }

            const HapticCapabilitiesEXT caps = device_.GetCapabilitiesEXT();
            lines_.push_back("Opened \"" + device_.GetNameEXT() + "\": up to " +
                             std::to_string(caps.maxEffects) + " effects stored, " +
                             std::to_string(caps.maxEffectsPlaying) + " playing at once");
            const char* names[] = {"1: sine", "2: constant", "3: ramp"};
            const HapticEffectEXT effects[] = {Sine(), Constant(), Ramp()};
            for (int i = 0; i < 3; ++i)
            {
                ids_[i] = device_.IsEffectSupportedEXT(effects[i]) ? device_.CreateEffectEXT(effects[i]) : -1;
                lines_.push_back(std::string(names[i]) + (ids_[i] >= 0 ? " -- ready" : " -- not supported"));
            }
            hasStatus_ = (caps.features & HapticFeatureEXT::Status) != HapticFeatureEXT::None;
        }

        HapticDevice device_;
        int ids_[3] = {-1, -1, -1};
        bool hasStatus_ = false;
        bool playing_ = false;
        std::vector<HapticInfoEXT> haptics_;
        std::vector<std::string> lines_;
        KeyboardState previous_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(EffectsDemo, {
    .id = "input.haptics.effects",
    .title = "Force-feedback effects (CNAEXT)",
    .description = "Automates effect value semantics, read-only device enumeration and every safe closed-device "
                   "operation. A fresh O press in an attended run opens the first real device and uploads sine, "
                   "constant and ramp effects; keys 1-3 run them and shutdown stops and releases only the uploaded "
                   "ids owned by this demo, then closes its handle.",
    .apis = {"CNA::Input::HapticEffectEXT", "CNA::Input::HapticDirectionEXT",
             "CNA::Input::HapticDirectionTypeEXT", "CNA::Input::HapticEffectTypeEXT",
             "CNA::Input::HapticFeatureEXT", "CNA::Input::HapticCapabilitiesEXT", "CNA::Input::HapticInfoEXT",
             "CNA::Input::Haptics::GetHapticsEXT", "CNA::Input::Haptics::OpenEXT",
             "CNA::Input::Haptics::OpenFromJoystickEXT", "CNA::Input::Haptics::IsJoystickHapticEXT",
             "CNA::Input::HapticDevice::IsOpenEXT", "CNA::Input::HapticDevice::GetNameEXT",
             "CNA::Input::HapticDevice::GetCapabilitiesEXT", "CNA::Input::HapticDevice::IsEffectSupportedEXT",
             "CNA::Input::HapticDevice::InitRumbleEXT", "CNA::Input::HapticDevice::PlayRumbleEXT",
             "CNA::Input::HapticDevice::StopRumbleEXT", "CNA::Input::HapticDevice::CreateEffectEXT",
             "CNA::Input::HapticDevice::UpdateEffectEXT", "CNA::Input::HapticDevice::RunEffectEXT",
             "CNA::Input::HapticDevice::StopEffectEXT", "CNA::Input::HapticDevice::DestroyEffectEXT",
             "CNA::Input::HapticDevice::GetEffectStatusEXT", "CNA::Input::HapticDevice::StopAllEffectsEXT",
             "CNA::Input::HapticDevice::SetGainEXT", "CNA::Input::HapticDevice::SetAutocenterEXT",
             "CNA::Input::HapticDevice::PauseEXT", "CNA::Input::HapticDevice::ResumeEXT",
             "CNA::Input::HapticDevice::Dispose"},
    .tags = {"haptics", "force-feedback", "wheel", "joystick", "effects", "cnaext", "explicit-consent"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard,
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = {"Unattended runs enumerate only; opening devices, uploading/running effects and other physical "
                     "operations require fresh attended key presses. Gain is never changed because CNA has no getter."},
})
