// SPDX-License-Identifier: MS-PL
// CNAEXT Accelerometer keyboard emulation: tilt-controlled games playable (and testable) on a desktop.
#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Devices/Sensors/Accelerometer.hpp"
#include "Microsoft/Devices/Sensors/AccelerometerReading.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using Microsoft::Devices::Sensors::Accelerometer;

namespace
{
    class KeyboardAccelerometerDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            ball_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white[] = {Color::White};
            ball_->SetData(white, 0, 1);

            // Arrow keys tilt an emulated device lying flat on the table. The game's own code is
            // the ordinary sensor API, so the same build plays on a phone with a real sensor.
            Accelerometer::setKeyboardEmulationEnabledEXT(true);
            Context().Check(Accelerometer::getIsSupportedProperty(), "with emulation the accelerometer is supported");
            accelerometer_.emplace();
            accelerometer_->Start();
        }

        void UnloadContent() override
        {
            // Process-wide: stop the sensor first, then switch the emulation off again.
            accelerometer_->Stop();
            accelerometer_->Dispose();
            Accelerometer::setKeyboardEmulationEnabledEXT(false);
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            if (!accelerometer_->getIsDataValidProperty())
            {
                return;
            }
            // Acceleration is in g: lying flat at rest it is (0, 0, -1), gravity pulling "down"
            // through the screen. Tilting moves X and Y; the ball rolls that way.
            acceleration_ = accelerometer_->getCurrentValueProperty().getAccelerationProperty();
            if (!checked_)
            {
                checked_ = true;
                Context().Print("first reading: ({:.2f}, {:.2f}, {:.2f})", acceleration_.X, acceleration_.Y, acceleration_.Z);
                if (Context().Automated())
                {
                    Context().Check(std::abs(acceleration_.X) < 0.01f && std::abs(acceleration_.Y) < 0.01f &&
                                        std::abs(acceleration_.Z + 1.0f) < 0.01f,
                                    "untouched, the device lies flat: (0, 0, -1)");
                }
            }
            const float dt = static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
            velocity_ += Vector2(acceleration_.X, -acceleration_.Y) * 600.0f * dt;
            velocity_ *= 0.98f;
            position_ += velocity_ * dt;
            position_.X = std::clamp(position_.X, 20.0f, 760.0f);
            position_.Y = std::clamp(position_.Y, 120.0f, 440.0f);
        }

        void EndRun() override
        {
            if (Context().Automated())
            {
                Context().Check(checked_, "readings arrived (IsDataValid) during the run");
            }
            DemoGame::EndRun();
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 50, 40));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Accelerometer (keyboard emulation): arrow keys tilt", Vector2(20.0f, 16.0f),
                                     Color::White);
            const std::string reading = "Acceleration (" + std::to_string(acceleration_.X).substr(0, 5) + ", " +
                std::to_string(acceleration_.Y).substr(0, 5) + ", " + std::to_string(acceleration_.Z).substr(0, 5) + ") g";
            spriteBatch_->DrawString(*font_, reading, Vector2(20.0f, 50.0f), Color::LightGreen);
            spriteBatch_->Draw(*ball_, Rectangle(static_cast<int>(position_.X) - 12, static_cast<int>(position_.Y) - 12, 24, 24),
                               Color::Gold);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::optional<Accelerometer> accelerometer_;
        Vector3 acceleration_;
        Vector2 position_ = Vector2(400.0f, 280.0f);
        Vector2 velocity_;
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> ball_;
    };
}

CNA_DEMOS_GAME(KeyboardAccelerometerDemo, {
    .id = "devices.sensors.keyboard_accelerometer",
    .title = "Tilt with the arrow keys (CNAEXT)",
    .description = "Turns on Accelerometer::setKeyboardEmulationEnabledEXT, starts an Accelerometer and rolls a ball "
                   "by its CurrentValue: flat at rest it reads (0, 0, -1) g, and the arrow keys tilt the emulated "
                   "device; emulation is switched off on unload.",
    .apis = {"Accelerometer::KeyboardEmulationEnabledEXT", "Accelerometer::IsSupported", "Accelerometer::Start",
             "Accelerometer::Stop", "SensorBase::CurrentValue", "SensorBase::IsDataValid",
             "AccelerometerReading::Acceleration"},
    .tags = {"devices", "sensors", "accelerometer", "tilt", "keyboard", "testing", "cnaext"},
    .needs = CnaDemos::Need::Keyboard,
    .frames = 40,
    .assets = {"fonts/pixel_font.cnb"},
})
