// SPDX-License-Identifier: MS-PL
// Microsoft.Devices.Sensors: asking which motion sensors this device has before using them.
#include <exception>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Devices/DeviceType.hpp"
#include "Microsoft/Devices/Environment.hpp"
#include "Microsoft/Devices/Sensors/Accelerometer.hpp"
#include "Microsoft/Devices/Sensors/AccelerometerFailedException.hpp"
#include "Microsoft/Devices/Sensors/Compass.hpp"
#include "Microsoft/Devices/Sensors/Gyroscope.hpp"
#include "Microsoft/Devices/Sensors/Motion.hpp"
#include "Microsoft/Devices/Sensors/SensorState.hpp"

using namespace Microsoft::Devices;
using namespace Microsoft::Devices::Sensors;

namespace
{
    const char* StateName(SensorState state)
    {
        switch (state)
        {
            case SensorState::NotSupported: return "NotSupported";
            case SensorState::Ready: return "Ready";
            case SensorState::Initializing: return "Initializing";
            case SensorState::NoData: return "NoData";
            case SensorState::NoPermissions: return "NoPermissions";
            case SensorState::Disabled: return "Disabled";
        }
        return "?";
    }

    void Availability(CnaDemos::DemoContext& ctx)
    {
        // A phone game checks IsSupported and offers another control scheme when a sensor is
        // missing; a desktop usually has none of them.
        // CNA reports Device on mobile platforms and Emulator elsewhere (a desktop plays the part
        // of the phone emulator).
        ctx.Print("Environment::DeviceType: {}", Environment::getDeviceTypeProperty() == DeviceType::Emulator ? "Emulator"
                                                                                                         : "Device");
        ctx.Print("Accelerometer::IsSupported {}", Accelerometer::getIsSupportedProperty());
        ctx.Print("Compass::IsSupported       {}", Compass::getIsSupportedProperty());
        ctx.Print("Gyroscope::IsSupported     {}", Gyroscope::getIsSupportedProperty());
        ctx.Print("Motion::IsSupported        {}", Motion::getIsSupportedProperty());

        Accelerometer accelerometer;
        const bool supported = Accelerometer::getIsSupportedProperty();
        ctx.Print("a new Accelerometer: State {}", StateName(accelerometer.getStateProperty()));
        ctx.Check((accelerometer.getStateProperty() == SensorState::NotSupported) == !supported,
                  "a new Accelerometer's State is NotSupported exactly when IsSupported is false");
        ctx.Check(!accelerometer.getIsDataValidProperty(), "before Start there is no valid reading");

        if (!supported)
        {
            // Without the sensor, Start reports the failure the WP7 way.
            bool failed = false;
            try
            {
                accelerometer.Start();
                accelerometer.Stop();
            }
            catch (const AccelerometerFailedException& failure)
            {
                failed = true;
                ctx.Print("Start: AccelerometerFailedException: {}", failure.what());
            }
            ctx.Check(failed, "starting a missing accelerometer throws AccelerometerFailedException");
        }
        // Sensors hold platform resources: dispose them when done.
        accelerometer.Dispose();
    }
}

CNA_DEMOS_CONSOLE(Availability, {
    .id = "devices.sensors.availability",
    .title = "Which sensors are there",
    .description = "Asks Accelerometer, Compass, Gyroscope and Motion whether this device supports them and reads "
                   "Environment::DeviceType; a new Accelerometer reports NotSupported without the sensor, and Start "
                   "then throws AccelerometerFailedException.",
    .apis = {"Accelerometer::IsSupported", "Compass::IsSupported", "Gyroscope::IsSupported", "Motion::IsSupported",
             "Accelerometer::Accelerometer", "SensorBase::State", "SensorBase::IsDataValid", "Accelerometer::Start",
             "AccelerometerFailedException", "SensorBase::Dispose", "Environment::DeviceType", "SensorState"},
    .tags = {"devices", "sensors", "accelerometer", "phone", "capabilities"},
})
