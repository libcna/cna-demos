// SPDX-License-Identifier: MS-PL
// Manual WP7/XNA sensor workflow against actual platform hardware (no synthetic backend).
#include <atomic>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Devices/Sensors/Accelerometer.hpp"
#include "Microsoft/Devices/Sensors/CalibrationEventArgs.hpp"
#include "Microsoft/Devices/Sensors/Compass.hpp"
#include "Microsoft/Devices/Sensors/Gyroscope.hpp"
#include "Microsoft/Devices/Sensors/Motion.hpp"
#include "Microsoft/Devices/Sensors/SensorState.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;
using namespace Microsoft::Devices::Sensors;

namespace
{
    template<class Sensor>
    void StopAndDispose(std::optional<Sensor>& sensor)
    {
        if (!sensor)
            return;
        try { sensor->Stop(); } catch (...) {}
        try { sensor->Dispose(); } catch (...) {}
        sensor.reset();
    }

    class LiveSensorReadingsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            support_ = {
                std::string("Accelerometer: ") + (Accelerometer::getIsSupportedProperty() ? "supported" : "absent"),
                std::string("Gyroscope: ") + (Gyroscope::getIsSupportedProperty() ? "supported" : "absent"),
                std::string("Compass: ") + (Compass::getIsSupportedProperty() ? "supported" : "absent"),
                std::string("Motion: ") + (Motion::getIsSupportedProperty() ? "supported" : "absent"),
            };
            spaceWasDown_ = Keyboard::GetState().IsKeyDown(Keys::Space);
        }

        void UnloadContent() override { CloseSensors(); }

        void Update(GameTime& gameTime) override
        {
            const bool spaceDown = Keyboard::GetState().IsKeyDown(Keys::Space);
            if (!Context().Automated() && spaceDown && !spaceWasDown_)
            {
                if (opened_)
                    CloseSensors();
                else
                    OpenSensors();
            }
            spaceWasDown_ = spaceDown;
            RefreshReadings();
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Space: open/close real WP7/XNA device sensors",
                                     Vector2(20.0f, 18.0f), Color::White);
            float y = 55.0f;
            for (const std::string& line : support_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(20.0f, y), Color::LightGray);
                y += 25.0f;
            }
            y += 12.0f;
            spriteBatch_->DrawString(*font_, status_, Vector2(20.0f, y), Color::Gold);
            y += 36.0f;
            for (const std::string& line : readings_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(20.0f, y), Color::LightGreen);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        template<class Sensor>
        void OpenIfSupported(bool supported, std::optional<Sensor>& sensor, const char* name)
        {
            if (!supported)
                return;
            try
            {
                sensor.emplace();
                sensor->Start();
            }
            catch (const std::exception& exception)
            {
                status_ += std::string(" ") + name + " failed: " + exception.what();
                StopAndDispose(sensor);
            }
        }

        void OpenSensors()
        {
            status_ = "Opened supported hardware.";
            OpenIfSupported(Accelerometer::getIsSupportedProperty(), accelerometer_, "Accelerometer");
            OpenIfSupported(Gyroscope::getIsSupportedProperty(), gyroscope_, "Gyroscope");
            if (Compass::getIsSupportedProperty())
            {
                try
                {
                    compass_.emplace();
                    calibrateCount_->store(0);
                    const std::weak_ptr<std::atomic<int>> count = calibrateCount_;
                    calibrateToken_ = compass_->Calibrate.Add(
                        [count](System::Object*, const CalibrationEventArgs&) {
                            if (const auto live = count.lock())
                                live->fetch_add(1);
                        });
                    compass_->Start();
                }
                catch (const std::exception& exception)
                {
                    status_ += std::string(" Compass failed: ") + exception.what();
                    if (compass_)
                        compass_->Calibrate.Remove(calibrateToken_);
                    StopAndDispose(compass_);
                }
            }
            OpenIfSupported(Motion::getIsSupportedProperty(), motion_, "Motion");
            opened_ = accelerometer_.has_value() || gyroscope_.has_value() ||
                compass_.has_value() || motion_.has_value();
            if (!opened_)
                status_ = "No supported hardware sensor could be opened.";
        }

        void CloseSensors()
        {
            StopAndDispose(accelerometer_);
            StopAndDispose(gyroscope_);
            if (compass_)
                compass_->Calibrate.Remove(calibrateToken_);
            StopAndDispose(compass_);
            StopAndDispose(motion_);
            opened_ = false;
            readings_.clear();
            status_ = "All sensor subscriptions and devices are closed.";
        }

        void RefreshReadings()
        {
            readings_.clear();
            if (accelerometer_ && accelerometer_->getIsDataValidProperty())
            {
                const Vector3 value = accelerometer_->getCurrentValueProperty().getAccelerationProperty();
                readings_.push_back("Acceleration g: " + value.ToString());
            }
            if (gyroscope_ && gyroscope_->getIsDataValidProperty())
            {
                const Vector3 value = gyroscope_->getCurrentValueProperty().getRotationRateProperty();
                readings_.push_back("Rotation rad/s: " + value.ToString());
            }
            if (compass_ && compass_->getIsDataValidProperty())
            {
                const auto& value = compass_->getCurrentValueProperty();
                readings_.push_back("Compass magnetic/true/accuracy: " +
                    std::to_string(value.getMagneticHeadingProperty()) + " / " +
                    std::to_string(value.getTrueHeadingProperty()) + " / " +
                    std::to_string(value.getHeadingAccuracyProperty()) +
                    "  calibrate events " + std::to_string(calibrateCount_->load()));
            }
            if (motion_ && motion_->getIsDataValidProperty())
            {
                const auto& value = motion_->getCurrentValueProperty();
                readings_.push_back("Motion acceleration: " + value.getDeviceAccelerationProperty().ToString());
                readings_.push_back("Motion rotation: " + value.getDeviceRotationRateProperty().ToString());
                readings_.push_back("Motion gravity: " + value.getGravityProperty().ToString());
            }
        }

        bool opened_ = false;
        bool spaceWasDown_ = false;
        std::size_t calibrateToken_ = 0;
        std::shared_ptr<std::atomic<int>> calibrateCount_ = std::make_shared<std::atomic<int>>(0);
        std::string status_ = "No sensor opens until Space is pressed.";
        std::vector<std::string> support_;
        std::vector<std::string> readings_;
        std::optional<Accelerometer> accelerometer_;
        std::optional<Gyroscope> gyroscope_;
        std::optional<Compass> compass_;
        std::optional<Motion> motion_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(LiveSensorReadingsDemo, {
    .id = "devices.sensors.live_readings",
    .title = "Live hardware sensor readings",
    .description = "After Space is pressed, opens the real Accelerometer, Gyroscope, Compass and fused Motion sensors "
                   "supported by the device, displays their live values and Compass calibration requests, then stops, "
                   "unsubscribes and disposes every device.",
    .apis = {"Accelerometer::IsSupported", "Accelerometer::Start", "Accelerometer::CurrentValue",
             "Gyroscope::IsSupported", "Gyroscope::Start", "Gyroscope::CurrentValue",
             "Compass::IsSupported", "Compass::Start", "Compass::CurrentValue", "Compass::Calibrate",
             "Motion::IsSupported", "Motion::Start", "Motion::CurrentValue", "SensorBase::Stop",
             "SensorBase::Dispose", "SensorBase::IsDataValid"},
    .tags = {"devices", "sensors", "accelerometer", "gyroscope", "compass", "motion", "android", "manual"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Manual: requires real supported sensor hardware and may require OS permission; automation never opens it.",
})
