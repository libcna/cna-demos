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

            // SetVibration reports whether the request reached a controller.
            for (int i = 0; i < 4; ++i)
            {
                const auto player = static_cast<PlayerIndex>(i);
                if (!GamePad::GetState(player).getIsConnectedProperty())
                    Context().Check(!GamePad::SetVibration(player, 0.0f, 0.0f),
                                    "player " + std::to_string(i + 1) + " is not connected: SetVibration returns false");
            }
        }

        void UnloadContent() override
        {
            // Motors keep running until told otherwise -- even after the game stops asking.
            GamePad::SetVibration(PlayerIndex::One, 0.0f, 0.0f);
        }

        void Update(GameTime& gameTime) override
        {
            // Left motor: heavy, low-frequency rumble. Right motor: light, high-frequency buzz.
            const GamePadState pad = GamePad::GetState(PlayerIndex::One);
            left_ = pad.getTriggersProperty().getLeftProperty();
            right_ = pad.getTriggersProperty().getRightProperty();
            accepted_ = GamePad::SetVibration(PlayerIndex::One, left_, right_);
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Pull the triggers of controller one", Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_,
                                     accepted_ ? "SetVibration: accepted" : "SetVibration: false (no controller or motors)",
                                     Vector2(40.0f, 60.0f), accepted_ ? Color::LightGreen : Color::Gray);
            Meter(150, "left motor (low frequency)", left_);
            Meter(260, "right motor (high frequency)", right_);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void Meter(int y, const std::string& label, float value)
        {
            spriteBatch_->DrawString(*font_, label, Vector2(40.0f, static_cast<float>(y)), Color::LightGray);
            spriteBatch_->Draw(*pixel_, Rectangle(40, y + 30, 700, 30), Color::DimGray);
            spriteBatch_->Draw(*pixel_, Rectangle(40, y + 30, static_cast<int>(700.0f * value), 30), Color::Orange);
        }

        float left_ = 0.0f;
        float right_ = 0.0f;
        bool accepted_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(VibrationDemo, {
    .id = "input.gamepad.vibration",
    .title = "Gamepad vibration",
    .description = "Drives the two rumble motors of controller one with GamePad::SetVibration from the trigger "
                   "positions, shows whether the call was accepted, and stops the motors on unload.",
    .apis = {"GamePad::SetVibration", "GamePad::GetState", "GamePadTriggers::Left", "GamePadTriggers::Right"},
    .tags = {"gamepad", "controller", "rumble", "force-feedback", "haptics"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad,
    .assets = {"fonts/pixel_font.cnb"},
})
