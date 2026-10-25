// SPDX-License-Identifier: MS-PL
// GamePad::SetVibration: the low- and high-frequency rumble motors, driven by the triggers.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadState.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class VibrationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
            previousKeyboard_ = Keyboard::GetState();
            const GamePadState pad = GamePad::GetState(PlayerIndex::One);
            Context().Check(pad.getIsConnectedProperty() == GamePad::GetState(PlayerIndex::One).getIsConnectedProperty(),
                            "read-only connection state is stable across the capability probe");
        }

        void UnloadContent() override
        {
            StopOwnedVibration();
        }

        void Update(GameTime& gameTime) override
        {
            const GamePadState pad = GamePad::GetState(PlayerIndex::One);
            left_ = pad.getTriggersProperty().getLeftProperty();
            right_ = pad.getTriggersProperty().getRightProperty();
            const KeyboardState keyboard = Keyboard::GetState();
            if (!Context().Automated() && keyboard.IsKeyDown(Keys::Space) &&
                previousKeyboard_.IsKeyUp(Keys::Space))
            {
                StopOwnedVibration();
                // A person explicitly requests one bounded pulse. Trigger positions select the
                // motor strengths, with a visible fallback pulse when both are at rest.
                const float low = left_ == 0.0f && right_ == 0.0f ? 0.35f : left_;
                const float high = left_ == 0.0f && right_ == 0.0f ? 0.35f : right_;
                accepted_ = GamePad::SetVibration(PlayerIndex::One, low, high);
                ownsVibration_ = accepted_;
                stopAtSeconds_ = gameTime.getTotalGameTimeProperty().getTotalSecondsProperty() + 0.25;
            }
            previousKeyboard_ = keyboard;
            if (ownsVibration_ && gameTime.getTotalGameTimeProperty().getTotalSecondsProperty() >= stopAtSeconds_)
                StopOwnedVibration();
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Pull the triggers, then press Space for one 250 ms pulse",
                                     Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_,
                                     accepted_ ? "SetVibration: accepted" : "SetVibration: false (no controller or motors)",
                                     Vector2(40.0f, 60.0f), accepted_ ? Color::LightGreen : Color::Gray);
            Meter(150, "left motor (low frequency)", left_);
            Meter(260, "right motor (high frequency)", right_);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void StopOwnedVibration()
        {
            if (!ownsVibration_)
                return;
            GamePad::SetVibration(PlayerIndex::One, 0.0f, 0.0f);
            ownsVibration_ = false;
        }

        void Meter(int y, const std::string& label, float value)
        {
            spriteBatch_->DrawString(*font_, label, Vector2(40.0f, static_cast<float>(y)), Color::LightGray);
            spriteBatch_->Draw(*pixel_, Rectangle(40, y + 30, 700, 30), Color::DimGray);
            spriteBatch_->Draw(*pixel_, Rectangle(40, y + 30, static_cast<int>(700.0f * value), 30), Color::Orange);
        }

        float left_ = 0.0f;
        float right_ = 0.0f;
        bool accepted_ = false;
        bool ownsVibration_ = false;
        double stopAtSeconds_ = 0.0;
        KeyboardState previousKeyboard_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(VibrationDemo, {
    .id = "input.gamepad.vibration",
    .title = "Gamepad vibration",
    .description = "Reads controller and trigger state without side effects. In an attended run, a fresh Space press "
                   "requests one bounded SetVibration pulse using the trigger positions; accepted output is stopped "
                   "on timeout or shutdown.",
    .apis = {"GamePad::SetVibration", "GamePad::GetState", "GamePadTriggers::Left", "GamePadTriggers::Right"},
    .tags = {"gamepad", "controller", "rumble", "force-feedback", "haptics", "explicit-consent"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = {"Unattended and smoke runs perform read-only polling and never call SetVibration."},
})
