// SPDX-License-Identifier: MS-PL
// CNAEXT controller motion sensors: GamePad::GetGyroEXT and GetAccelerometerEXT.
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
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    std::string Format(const Vector3& v)
    {
        const auto f = [](float value) {
            const std::string text = std::to_string(value);
            return text.substr(0, text.find('.') + 3);
        };
        return f(v.X) + ", " + f(v.Y) + ", " + f(v.Z);
    }

    class MotionSensorsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
            if (!GamePad::GetState(PlayerIndex::One).getIsConnectedProperty())
            {
                Vector3 unused;
                Context().Check(!GamePad::GetGyroEXT(PlayerIndex::One, unused) &&
                                    !GamePad::GetAccelerometerEXT(PlayerIndex::One, unused),
                                "without a controller the sensor reads report false");
            }
        }

        void Update(GameTime& gameTime) override
        {
            // Both return false when the controller has no such sensor. The gyroscope gives
            // angular velocity, the accelerometer the acceleration including gravity -- which
            // makes it a tilt sensor while the controller is held still.
            const GamePadCapabilities caps = GamePad::GetCapabilities(PlayerIndex::One);
            hasGyro_ = caps.getHasGyroEXTProperty() && GamePad::GetGyroEXT(PlayerIndex::One, gyro_);
            hasAccel_ = caps.getHasAccelerometerEXTProperty() && GamePad::GetAccelerometerEXT(PlayerIndex::One, accel_);
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, hasGyro_ ? "Gyroscope: " + Format(gyro_) : std::string("Gyroscope: none"),
                                     Vector2(30.0f, 30.0f), hasGyro_ ? Color::White : Color::Gray);
            spriteBatch_->DrawString(*font_,
                                     hasAccel_ ? "Accelerometer: " + Format(accel_) : std::string("Accelerometer: none"),
                                     Vector2(30.0f, 60.0f), hasAccel_ ? Color::White : Color::Gray);
            // Tilt: where gravity pulls, seen from above.
            spriteBatch_->Draw(*pixel_, Rectangle(300, 140, 200, 200), Color::DimGray);
            const Vector2 tilt = hasAccel_ ? Vector2(accel_.X, accel_.Z) / 9.81f : Vector2::Zero;
            const int x = 400 + static_cast<int>(MathHelper::Clamp(tilt.X, -1.0f, 1.0f) * 90.0f);
            const int y = 240 + static_cast<int>(MathHelper::Clamp(tilt.Y, -1.0f, 1.0f) * 90.0f);
            spriteBatch_->Draw(*pixel_, Rectangle(x - 10, y - 10, 20, 20), Color::Orange);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        Vector3 gyro_;
        Vector3 accel_;
        bool hasGyro_ = false;
        bool hasAccel_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(MotionSensorsDemo, {
    .id = "input.gamepad.motion_sensors",
    .title = "Controller motion sensors (CNAEXT)",
    .description = "Reads controller one's gyroscope and accelerometer with GamePad::GetGyroEXT and "
                   "GetAccelerometerEXT (when HasGyroEXT/HasAccelerometerEXT say they exist) and shows the tilt.",
    .apis = {"GamePad::GetGyroEXT", "GamePad::GetAccelerometerEXT", "GamePadCapabilities::HasGyroEXT",
             "GamePadCapabilities::HasAccelerometerEXT"},
    .tags = {"gamepad", "controller", "gyroscope", "accelerometer", "motion", "tilt", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad,
    .assets = {"fonts/pixel_font.cnb"},
})
