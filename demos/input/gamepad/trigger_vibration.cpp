// SPDX-License-Identifier: MS-PL
// CNAEXT GamePad::SetTriggerVibrationEXT: the motors inside the triggers (Xbox One "impulse triggers").
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
#include "Microsoft/Xna/Framework/Input/GamePadCapabilities.hpp"
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
    class TriggerVibrationDemo final : public CnaDemos::DemoGame
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
        }

        void UnloadContent() override { StopOwnedVibration(); }

        void Update(GameTime& gameTime) override
        {
            const GamePadState pad = GamePad::GetState(PlayerIndex::One);
            left_ = pad.getTriggersProperty().getLeftProperty();
            right_ = pad.getTriggersProperty().getRightProperty();
            supported_ = GamePad::GetCapabilities(PlayerIndex::One).getHasTriggerVibrationMotorsEXTProperty();
            const KeyboardState keyboard = Keyboard::GetState();
            if (!Context().Automated() && keyboard.IsKeyDown(Keys::Space) &&
                previousKeyboard_.IsKeyUp(Keys::Space))
            {
                StopOwnedVibration();
                const float left = left_ == 0.0f && right_ == 0.0f ? 0.35f : left_;
                const float right = left_ == 0.0f && right_ == 0.0f ? 0.35f : right_;
                accepted_ = GamePad::SetTriggerVibrationEXT(PlayerIndex::One, left, right);
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
            spriteBatch_->DrawString(*font_,
                                     supported_ ? "Pull the triggers, then press Space for one bounded pulse"
                                                : "No trigger motors on controller one",
                                     Vector2(30.0f, 30.0f), supported_ ? Color::White : Color::Gray);
            spriteBatch_->DrawString(*font_, accepted_ ? "SetTriggerVibrationEXT: accepted" : "SetTriggerVibrationEXT: false",
                                     Vector2(30.0f, 60.0f), Color::LightGray);
            spriteBatch_->Draw(*pixel_, Rectangle(200, 120, 60, 250), Color::DimGray);
            spriteBatch_->Draw(*pixel_, Rectangle(540, 120, 60, 250), Color::DimGray);
            const int leftHeight = static_cast<int>(250.0f * left_);
            const int rightHeight = static_cast<int>(250.0f * right_);
            spriteBatch_->Draw(*pixel_, Rectangle(200, 370 - leftHeight, 60, leftHeight), Color::Orange);
            spriteBatch_->Draw(*pixel_, Rectangle(540, 370 - rightHeight, 60, rightHeight), Color::Orange);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void StopOwnedVibration()
        {
            if (!ownsVibration_)
                return;
            GamePad::SetTriggerVibrationEXT(PlayerIndex::One, 0.0f, 0.0f);
            ownsVibration_ = false;
        }

        float left_ = 0.0f;
        float right_ = 0.0f;
        bool supported_ = false;
        bool accepted_ = false;
        bool ownsVibration_ = false;
        double stopAtSeconds_ = 0.0;
        KeyboardState previousKeyboard_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(TriggerVibrationDemo, {
    .id = "input.gamepad.trigger_vibration",
    .title = "Trigger vibration (CNAEXT)",
    .description = "Checks HasTriggerVibrationMotorsEXT and reads triggers without side effects. A fresh Space press "
                   "in an attended run requests one bounded trigger-motor pulse; accepted output is stopped on "
                   "timeout or shutdown.",
    .apis = {"GamePad::SetTriggerVibrationEXT", "GamePadCapabilities::HasTriggerVibrationMotorsEXT"},
    .tags = {"gamepad", "controller", "rumble", "trigger", "impulse-triggers", "haptics", "cnaext",
             "explicit-consent"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = {"Unattended and smoke runs perform read-only polling and never call SetTriggerVibrationEXT."},
})
