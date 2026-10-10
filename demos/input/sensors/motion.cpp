// SPDX-License-Identifier: MS-PL
// CNAEXT Sensors: the device's own accelerometer and gyroscope (phones, tablets, handhelds).
#include <memory>
#include <optional>
#include <string>

#include "CNA/Input/Sensors.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using CNA::Input::Sensors;

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

    class MotionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
            Context().Print("{} sensor(s)", Sensors::GetSensorsEXT().size());
        }

        void Update(GameTime& gameTime) override
        {
            // Each read reports false when the device has no such sensor. Acceleration is in m/s^2
            // and includes gravity; angular velocity is in radians per second.
            hasAccel_ = Sensors::GetAccelerometerEXT(accel_);
            hasGyro_ = Sensors::GetGyroscopeEXT(gyro_);
            // Roll a ball with the tilt.
            if (hasAccel_)
            {
                ball_.X = MathHelper::Clamp(ball_.X - accel_.X * 0.5f, 310.0f, 490.0f);
                ball_.Y = MathHelper::Clamp(ball_.Y + accel_.Y * 0.5f, 150.0f, 330.0f);
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, hasAccel_ ? "Accelerometer: " + Format(accel_) : std::string("Accelerometer: none"),
                                     Vector2(30.0f, 30.0f), hasAccel_ ? Color::White : Color::Gray);
            spriteBatch_->DrawString(*font_, hasGyro_ ? "Gyroscope: " + Format(gyro_) : std::string("Gyroscope: none"),
                                     Vector2(30.0f, 60.0f), hasGyro_ ? Color::White : Color::Gray);
            spriteBatch_->Draw(*pixel_, Rectangle(300, 140, 200, 200), Color::DimGray);
            spriteBatch_->Draw(*pixel_, Rectangle(static_cast<int>(ball_.X) - 10, static_cast<int>(ball_.Y) - 10, 20, 20),
                               Color::Orange);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        Vector3 accel_;
        Vector3 gyro_;
        bool hasAccel_ = false;
        bool hasGyro_ = false;
        Vector2 ball_{400.0f, 240.0f};
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(MotionDemo, {
    .id = "input.sensors.motion",
    .title = "Device motion sensors (CNAEXT)",
    .description = "Reads the device's accelerometer and gyroscope with Sensors::GetAccelerometerEXT and "
                   "GetGyroscopeEXT and rolls a ball with the tilt; on a desktop both report that there is no sensor.",
    .apis = {"CNA::Input::Sensors::GetAccelerometerEXT", "CNA::Input::Sensors::GetGyroscopeEXT",
             "CNA::Input::Sensors::GetSensorsEXT"},
    .tags = {"sensors", "accelerometer", "gyroscope", "tilt", "phone", "tablet", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .assets = {"fonts/pixel_font.cnb"},
})
