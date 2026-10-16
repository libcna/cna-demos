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
#include "Microsoft/Devices/Sensors/SensorFailedException.hpp"
#include "Microsoft/Devices/Sensors/SensorState.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/TimeSpan.hpp"

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
        Compass compass;
        Gyroscope gyroscope;
        Motion motion;
        const bool accelerometerSupported = Accelerometer::getIsSupportedProperty();
        const bool compassSupported = Compass::getIsSupportedProperty();
        const bool gyroscopeSupported = Gyroscope::getIsSupportedProperty();
        const bool motionSupported = Motion::getIsSupportedProperty();
        ctx.Print("a new Accelerometer: State {}", StateName(accelerometer.getStateProperty()));
        ctx.Check((accelerometer.getStateProperty() == SensorState::NotSupported) == !accelerometerSupported &&
                      (compass.getStateProperty() == SensorState::NotSupported) == !compassSupported &&
                      (gyroscope.getStateProperty() == SensorState::NotSupported) == !gyroscopeSupported &&
                      (motion.getStateProperty() == SensorState::NotSupported) == !motionSupported,
                  "each new sensor reports NotSupported exactly when its static support probe is false");
        ctx.Check(!accelerometer.getIsDataValidProperty() && !compass.getIsDataValidProperty() &&
                      !gyroscope.getIsDataValidProperty() && !motion.getIsDataValidProperty(),
                  "before Start none of the four sensors has a valid reading");

        const System::TimeSpan initialInterval = accelerometer.getTimeBetweenUpdatesProperty();
        accelerometer.setTimeBetweenUpdatesProperty(initialInterval);
        accelerometer.setTimeBetweenUpdatesProperty(System::TimeSpan::FromMilliseconds(10.0));
        ctx.Check(initialInterval == System::TimeSpan::FromMilliseconds(2.0) &&
                      accelerometer.getTimeBetweenUpdatesProperty() == System::TimeSpan::FromMilliseconds(10.0),
                  "TimeBetweenUpdates starts at 2 ms and stores a new value");

        int missingStartFailures = 0;
        bool missingErrorIdsDefault = true;
        if (!accelerometerSupported)
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
                ++missingStartFailures;
                missingErrorIdsDefault = missingErrorIdsDefault && failure.getErrorIdProperty() == 0;
                ctx.Print("Start: AccelerometerFailedException: {}", failure.what());
            }
            ctx.Check(failed, "starting a missing accelerometer throws AccelerometerFailedException");
        }

        const auto startMissing = [&missingStartFailures, &missingErrorIdsDefault](auto& sensor) {
            try
            {
                sensor.Start();
                sensor.Stop();
            }
            catch (const SensorFailedException& failure)
            {
                ++missingStartFailures;
                missingErrorIdsDefault = missingErrorIdsDefault && failure.getErrorIdProperty() == 0;
            }
        };
        if (!compassSupported) startMissing(compass);
        if (!gyroscopeSupported) startMissing(gyroscope);
        if (!motionSupported) startMissing(motion);
        ctx.Check(missingStartFailures ==
                      static_cast<int>(!accelerometerSupported) + static_cast<int>(!compassSupported) +
                          static_cast<int>(!gyroscopeSupported) + static_cast<int>(!motionSupported),
                  "every missing sensor refuses Start with its documented failed exception");
        ctx.Check(missingErrorIdsDefault,
                  "unsupported-sensor failures carry the documented default platform error ID");

        int unsupportedCurrentValues = 0;
        const auto readMissing = [&unsupportedCurrentValues](const auto& sensor) {
            try
            {
                static_cast<void>(sensor.getCurrentValueProperty());
            }
            catch (const System::InvalidOperationException&)
            {
                ++unsupportedCurrentValues;
            }
        };
        if (!accelerometerSupported) readMissing(accelerometer);
        if (!compassSupported) readMissing(compass);
        if (!gyroscopeSupported) readMissing(gyroscope);
        if (!motionSupported) readMissing(motion);
        ctx.Check(unsupportedCurrentValues ==
                      static_cast<int>(!accelerometerSupported) + static_cast<int>(!compassSupported) +
                          static_cast<int>(!gyroscopeSupported) + static_cast<int>(!motionSupported),
                  "CurrentValue refuses every unsupported sensor before returning a placeholder reading");

        // Sensors hold platform resources: dispose them when done. Repeating the operation on all
        // four verifies the shared SensorBase lifecycle with one grouped check.
        accelerometer.Dispose();
        compass.Dispose();
        gyroscope.Dispose();
        motion.Dispose();
        int repeatedDisposeFailures = 0;
        const auto repeatDispose = [&repeatedDisposeFailures](auto& sensor) {
            try
            {
                sensor.Dispose();
            }
            catch (const System::ObjectDisposedException&)
            {
                ++repeatedDisposeFailures;
            }
        };
        repeatDispose(accelerometer);
        repeatDispose(compass);
        repeatDispose(gyroscope);
        repeatDispose(motion);
        ctx.Check(repeatedDisposeFailures == 4,
                  "repeated Dispose throws ObjectDisposedException for every sensor instance");
    }
}

CNA_DEMOS_CONSOLE(Availability, {
    .id = "devices.sensors.availability",
    .title = "Which sensors are there",
    .description = "Asks Accelerometer, Compass, Gyroscope and Motion whether this device supports them and reads "
                   "Environment::DeviceType; a new Accelerometer reports NotSupported without the sensor, and Start "
                   "then throws the sensor-specific failed exception. It also shows the shared update interval and "
                   "disposal contracts.",
    .apis = {"Accelerometer::IsSupported", "Compass::IsSupported", "Gyroscope::IsSupported", "Motion::IsSupported",
             "Accelerometer::Accelerometer", "Compass::Compass", "Gyroscope::Gyroscope", "Motion::Motion",
             "SensorBase::State", "SensorBase::CurrentValue", "SensorBase::IsDataValid",
             "SensorBase::TimeBetweenUpdates",
             "Accelerometer::Start", "Compass::Start", "Gyroscope::Start", "Motion::Start",
             "AccelerometerFailedException", "SensorFailedException", "SensorFailedException::ErrorId",
             "SensorBase::Dispose",
             "Environment::DeviceType", "SensorState::NotSupported"},
    .tags = {"devices", "sensors", "accelerometer", "phone", "capabilities"},
})
