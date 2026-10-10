// SPDX-License-Identifier: MS-PL
// CNAEXT device lists: the mice, keyboards, touch screens, joysticks, haptic devices and sensors.
#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "CNA/Input/HapticInfo.hpp"
#include "CNA/Input/Haptics.hpp"
#include "CNA/Input/InputDeviceInfo.hpp"
#include "CNA/Input/InputDevices.hpp"
#include "CNA/Input/JoystickInfo.hpp"
#include "CNA/Input/Joysticks.hpp"
#include "CNA/Input/Sensors.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace CNA::Input;

namespace
{
    class DeviceListDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // XNA has one mouse and one keyboard; the platform may know several (a laptop's
            // touchpad and a USB mouse, or virtual devices). Their states are still merged.
            List("Mice", InputDevices::GetMiceEXT());
            List("Keyboards", InputDevices::GetKeyboardsEXT());
            List("Touch devices", InputDevices::GetTouchDevicesEXT());

            const std::vector<JoystickInfoEXT> joysticks = Joysticks::GetJoysticksEXT();
            Add("Joysticks: " + std::to_string(joysticks.size()));
            for (const JoystickInfoEXT& joystick : joysticks)
                Add("  #" + std::to_string(joystick.id) + " " + joystick.name + " (" +
                    std::string(CnaDemos::Name(joystick.type)) + ")");

            const std::vector<HapticInfoEXT> haptics = Haptics::GetHapticsEXT();
            Add("Haptic devices: " + std::to_string(haptics.size()) +
                (Haptics::IsMouseHapticEXT() ? ", the mouse has force feedback" : ""));
            for (const HapticInfoEXT& haptic : haptics)
                Add("  #" + std::to_string(haptic.id) + " " + haptic.name);

            const std::vector<SensorInfoEXT> sensors = Sensors::GetSensorsEXT();
            Add("Sensors: " + std::to_string(sensors.size()));
            for (const SensorInfoEXT& sensor : sensors)
                Add("  #" + std::to_string(sensor.id) + " " + sensor.name + " (" +
                    std::string(CnaDemos::Name(sensor.type)) + ")");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            float y = 15.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(20.0f, y), Color::White);
                y += 22.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void List(const std::string& kind, const std::vector<InputDeviceInfoEXT>& devices)
        {
            Add(kind + ": " + std::to_string(devices.size()));
            std::set<std::uint64_t> ids;
            for (const InputDeviceInfoEXT& device : devices)
            {
                Add("  #" + std::to_string(device.id) + " " + device.name);
                ids.insert(device.id);
            }
            Context().Check(ids.size() == devices.size(), kind + ": every device has its own id");
        }

        void Add(const std::string& line)
        {
            Context().Print("{}", line);
            lines_.push_back(line);
        }

        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(DeviceListDemo, {
    .id = "input.devices.list",
    .title = "Input device lists (CNAEXT)",
    .description = "Lists the input devices the platform reports: InputDevices::GetMiceEXT, GetKeyboardsEXT and "
                   "GetTouchDevicesEXT, Joysticks::GetJoysticksEXT, Haptics::GetHapticsEXT and "
                   "Sensors::GetSensorsEXT, each device with its id and name.",
    .apis = {"CNA::Input::InputDevices::GetMiceEXT", "CNA::Input::InputDevices::GetKeyboardsEXT",
             "CNA::Input::InputDevices::GetTouchDevicesEXT", "CNA::Input::InputDeviceInfoEXT",
             "CNA::Input::Joysticks::GetJoysticksEXT", "CNA::Input::Haptics::GetHapticsEXT",
             "CNA::Input::Haptics::IsMouseHapticEXT", "CNA::Input::Sensors::GetSensorsEXT",
             "CNA::Input::SensorInfoEXT"},
    .tags = {"devices", "mouse", "keyboard", "joystick", "haptics", "sensors", "cnaext"},
    .needs = CnaDemos::Need::Window,
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb"},
})
