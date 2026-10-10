// SPDX-License-Identifier: MS-PL
// CNAEXT haptic effects: describe a force (shape, strength, timing), upload it, run it.
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
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            const std::vector<HapticInfoEXT> haptics = Haptics::GetHapticsEXT();
            if (!haptics.empty())
                device_.emplace(Haptics::OpenEXT(haptics.front().id));
            if (!device_ || !device_->IsOpenEXT())
            {
                lines_.push_back("No force-feedback device found");
                return;
            }
            const HapticCapabilitiesEXT caps = device_->GetCapabilitiesEXT();
            lines_.push_back("\"" + device_->GetNameEXT() + "\": up to " + std::to_string(caps.maxEffects) +
                             " effects stored, " + std::to_string(caps.maxEffectsPlaying) + " playing at once");
            // Effects are uploaded once and then run by id, as often as needed.
            const char* names[] = {"1: sine", "2: constant", "3: ramp"};
            const HapticEffectEXT effects[] = {Sine(), Constant(), Ramp()};
            for (int i = 0; i < 3; ++i)
            {
                ids_[i] = device_->IsEffectSupportedEXT(effects[i]) ? device_->CreateEffectEXT(effects[i]) : -1;
                lines_.push_back(std::string(names[i]) + (ids_[i] >= 0 ? " -- ready" : " -- not supported"));
            }
            // Optional device controls, present only when the feature flags say so.
            if ((caps.features & HapticFeatureEXT::Gain) != HapticFeatureEXT::None)
                device_->SetGainEXT(80);
            hasStatus_ = (caps.features & HapticFeatureEXT::Status) != HapticFeatureEXT::None;
        }

        void UnloadContent() override
        {
            if (!device_)
                return;
            if (device_->IsOpenEXT())
            {
                device_->StopAllEffectsEXT();
                for (const int id : ids_)
                {
                    if (id >= 0)
                        device_->DestroyEffectEXT(id);
                }
            }
            device_->Dispose();
        }

        void Update(GameTime& gameTime) override
        {
            const KeyboardState keyboard = Keyboard::GetState();
            const Keys keys[] = {Keys::D1, Keys::D2, Keys::D3};
            for (int i = 0; i < 3; ++i)
            {
                if (ids_[i] >= 0 && keyboard.IsKeyDown(keys[i]) && previous_.IsKeyUp(keys[i]))
                    device_->RunEffectEXT(ids_[i], 1);
            }
            previous_ = keyboard;
            playing_ = false;
            for (const int id : ids_)
                playing_ = playing_ || (hasStatus_ && id >= 0 && device_->GetEffectStatusEXT(id));
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
        // Optional rather than a default-constructed closed device: CNA-DEMOS-ISSUE-009.
        std::optional<HapticDevice> device_;
        int ids_[3] = {-1, -1, -1};
        bool hasStatus_ = false;
        bool playing_ = false;
        std::vector<std::string> lines_;
        KeyboardState previous_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(EffectsDemo, {
    .id = "input.haptics.effects",
    .title = "Force-feedback effects (CNAEXT)",
    .description = "Describes sine, constant and ramp forces as HapticEffectEXT values, uploads the supported ones "
                   "with CreateEffectEXT and runs them with keys 1-3; gain and effect status are used when the "
                   "device's feature flags allow.",
    .apis = {"CNA::Input::HapticEffectEXT", "CNA::Input::HapticEffectTypeEXT", "CNA::Input::HapticFeatureEXT",
             "CNA::Input::HapticDevice::IsEffectSupportedEXT", "CNA::Input::HapticDevice::CreateEffectEXT",
             "CNA::Input::HapticDevice::RunEffectEXT", "CNA::Input::HapticDevice::GetEffectStatusEXT",
             "CNA::Input::HapticDevice::StopAllEffectsEXT", "CNA::Input::HapticDevice::DestroyEffectEXT",
             "CNA::Input::HapticDevice::SetGainEXT", "CNA::Input::HapticDevice::GetCapabilitiesEXT"},
    .tags = {"haptics", "force-feedback", "wheel", "joystick", "effects", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad,
    .assets = {"fonts/pixel_font.cnb"},
})
