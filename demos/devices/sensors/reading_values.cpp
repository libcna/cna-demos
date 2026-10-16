// SPDX-License-Identifier: MS-PL
// Microsoft.Devices.Sensors: the value objects carried by sensor events.
#include <string>
#include <utility>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Devices/Sensors/AccelerometerReading.hpp"
#include "Microsoft/Devices/Sensors/AccelerometerReadingEventArgs.hpp"
#include "Microsoft/Devices/Sensors/AttitudeReading.hpp"
#include "Microsoft/Devices/Sensors/CalibrationEventArgs.hpp"
#include "Microsoft/Devices/Sensors/CompassReading.hpp"
#include "Microsoft/Devices/Sensors/GyroscopeReading.hpp"
#include "Microsoft/Devices/Sensors/ISensorReading.hpp"
#include "Microsoft/Devices/Sensors/MotionReading.hpp"
#include "Microsoft/Devices/Sensors/SensorReadingEventArgs.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/DateTime.hpp"
#include "System/DateTimeOffset.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Devices::Sensors;
using namespace Microsoft::Xna::Framework;

namespace
{
    void ReadingValues(CnaDemos::DemoContext& ctx)
    {
        const System::DateTimeOffset timestamp(System::DateTime(1'000'000LL), System::TimeSpan::Zero);
        const Vector3 acceleration(0.25f, -0.5f, 1.0f);
        const Vector3 rotationRate(1.0f, 2.0f, 3.0f);
        const Vector3 magnetometer(12.0f, 23.0f, 34.0f);
        const Quaternion orientation = Quaternion::Identity;
        const Matrix rotation = Matrix::getIdentityProperty();

        const AccelerometerReading accelerometer(timestamp, acceleration);
        const GyroscopeReading gyroscope(rotationRate, timestamp);
        const AttitudeReading attitude(0.1f, 0.2f, 0.3f, orientation, rotation, timestamp);
        const CompassReading compass(2.5, 87.0, magnetometer, timestamp, 91.0);
        const MotionReading motion(attitude, acceleration, rotationRate, Vector3(0.0f, -1.0f, 0.0f), timestamp);

        ctx.Check(accelerometer.getAccelerationProperty() == acceleration &&
                      accelerometer.getTimestampProperty() == timestamp,
                  "AccelerometerReading stores acceleration and timestamp");
        ctx.Check(gyroscope.getRotationRateProperty() == rotationRate &&
                      gyroscope.getTimestampProperty() == timestamp,
                  "GyroscopeReading stores angular velocity and timestamp");
        ctx.Check(attitude.getPitchProperty() == 0.1f && attitude.getRollProperty() == 0.2f &&
                      attitude.getYawProperty() == 0.3f &&
                      attitude.getQuaternionProperty() == orientation &&
                      attitude.getRotationMatrixProperty() == rotation &&
                      attitude.getTimestampProperty() == timestamp,
                  "AttitudeReading stores Euler, quaternion, matrix and timestamp forms");
        ctx.Check(compass.getHeadingAccuracyProperty() == 2.5 &&
                      compass.getMagneticHeadingProperty() == 87.0 &&
                      compass.getMagnetometerReadingProperty() == magnetometer &&
                      compass.getTrueHeadingProperty() == 91.0 &&
                      compass.getTimestampProperty() == timestamp,
                  "CompassReading stores accuracy, both headings, field vector and timestamp");
        ctx.Check(motion.getAttitudeProperty() == attitude &&
                      motion.getDeviceAccelerationProperty() == acceleration &&
                      motion.getDeviceRotationRateProperty() == rotationRate &&
                      motion.getGravityProperty() == Vector3(0.0f, -1.0f, 0.0f) &&
                      motion.getTimestampProperty() == timestamp,
                  "MotionReading combines attitude, acceleration, rotation, gravity and timestamp");

        const ISensorReading& reading = motion;
        ctx.Check(reading.getTimestampProperty() == timestamp,
                  "ISensorReading exposes the timestamp polymorphically");

        const AccelerometerReading accelerometerCopy(timestamp, acceleration);
        const GyroscopeReading gyroscopeCopy(rotationRate, timestamp);
        const AttitudeReading attitudeCopy(0.1f, 0.2f, 0.3f, orientation, rotation, timestamp);
        const CompassReading compassCopy(2.5, 87.0, magnetometer, timestamp, 91.0);
        const MotionReading motionCopy(attitudeCopy, acceleration, rotationRate,
                                       Vector3(0.0f, -1.0f, 0.0f), timestamp);
        ctx.Check(accelerometer == accelerometerCopy && gyroscope == gyroscopeCopy &&
                      attitude == attitudeCopy && compass == compassCopy && motion == motionCopy &&
                      accelerometer.GetHashCode() == accelerometerCopy.GetHashCode() &&
                      gyroscope.GetHashCode() == gyroscopeCopy.GetHashCode() &&
                      attitude.GetHashCode() == attitudeCopy.GetHashCode() &&
                      compass.GetHashCode() == compassCopy.GetHashCode() &&
                      motion.GetHashCode() == motionCopy.GetHashCode(),
                  "equal reading values compare equal and produce equal hashes");
        ctx.Check(accelerometer != AccelerometerReading(timestamp, Vector3::Zero) &&
                      gyroscope != GyroscopeReading(Vector3::Zero, timestamp) &&
                      attitude != AttitudeReading(0.0f, 0.2f, 0.3f, orientation, rotation, timestamp) &&
                      compass != CompassReading(9.0, 87.0, magnetometer, timestamp, 91.0) &&
                      motion != MotionReading(attitude, Vector3::Zero, rotationRate,
                                              Vector3(0.0f, -1.0f, 0.0f), timestamp),
                  "each reading's inequality observes its authored values");

        ctx.Check(accelerometer.ToString().contains("Acceleration:") &&
                      gyroscope.ToString().contains("RotationRate:") &&
                      attitude.ToString().contains("Pitch:0.1") &&
                      compass.ToString().contains("MagneticHeading:87") &&
                      motion.ToString().contains("DeviceAcceleration:"),
                  "reading strings identify their useful values");
        ctx.Check(accelerometer.GetTypeName() == "Microsoft.Devices.Sensors.AccelerometerReading" &&
                      gyroscope.GetTypeName() == "Microsoft.Devices.Sensors.GyroscopeReading" &&
                      attitude.GetTypeName() == "Microsoft.Devices.Sensors.AttitudeReading" &&
                      compass.GetTypeName() == "Microsoft.Devices.Sensors.CompassReading" &&
                      motion.GetTypeName() == "Microsoft.Devices.Sensors.MotionReading",
                  "reading type names use their Microsoft.Devices.Sensors identities");

        const AccelerometerReadingEventArgs legacy(0.25, -0.5, 1.0, timestamp);
        const AccelerometerReadingEventArgs legacyCopy(0.25, -0.5, 1.0, timestamp);
        ctx.Check(legacy.getXProperty() == 0.25 && legacy.getYProperty() == -0.5 &&
                      legacy.getZProperty() == 1.0 && legacy.getTimestampProperty() == timestamp &&
                      legacy == legacyCopy && legacy.GetHashCode() == legacyCopy.GetHashCode() &&
                      legacy.ToString().contains("X:0.25") &&
                      legacy.GetTypeName() == "Microsoft.Devices.Sensors.AccelerometerReadingEventArgs",
                  "the legacy event payload preserves axes, timestamp and value semantics");

        SensorReadingEventArgs<AccelerometerReading> current;
        current.setSensorReadingProperty(accelerometer);
        SensorReadingEventArgs<GyroscopeReading> moved(GyroscopeReading(rotationRate, timestamp));
        moved.setSensorReadingProperty(GyroscopeReading(Vector3(4.0f, 5.0f, 6.0f), timestamp));
        ctx.Check(current.getSensorReadingProperty() == accelerometer &&
                      moved.getSensorReadingProperty().getRotationRateProperty() == Vector3(4.0f, 5.0f, 6.0f),
                  "SensorReadingEventArgs accepts copied and moved readings");

        const CalibrationEventArgs calibration;
        ctx.Check(calibration.GetTypeName() == "Microsoft.Devices.Sensors.CalibrationEventArgs",
                  "CalibrationEventArgs is the empty calibration-request payload");

        ctx.Print("attitude: {}", attitude.ToString());
        ctx.Print("compass:  {}", compass.ToString());
        ctx.Print("motion:   {}", motion.ToString());
    }
}

CNA_DEMOS_CONSOLE(ReadingValues, {
    .id = "devices.sensors.reading_values",
    .title = "Sensor reading value objects",
    .description = "Builds accelerometer, gyroscope, attitude, compass and fused motion readings, checks every "
                   "payload property, their shared timestamp interface, value semantics, text/type identity and "
                   "the modern, legacy and calibration event payloads without requiring sensor hardware.",
    .apis = {"AccelerometerReading::AccelerometerReading", "AccelerometerReading::Acceleration",
             "AccelerometerReading::Timestamp", "GyroscopeReading::GyroscopeReading",
             "GyroscopeReading::RotationRate", "GyroscopeReading::Timestamp",
             "AttitudeReading::AttitudeReading", "AttitudeReading::Pitch", "AttitudeReading::Roll",
             "AttitudeReading::Yaw", "AttitudeReading::Quaternion", "AttitudeReading::RotationMatrix",
             "AttitudeReading::Timestamp", "CompassReading::CompassReading", "CompassReading::HeadingAccuracy",
             "CompassReading::MagneticHeading", "CompassReading::MagnetometerReading",
             "CompassReading::TrueHeading", "CompassReading::Timestamp", "MotionReading::MotionReading",
             "MotionReading::Attitude", "MotionReading::DeviceAcceleration", "MotionReading::DeviceRotationRate",
             "MotionReading::Gravity", "MotionReading::Timestamp", "ISensorReading::Timestamp",
             "AccelerometerReading::operator==", "GyroscopeReading::operator==", "AttitudeReading::operator==",
             "CompassReading::operator==", "MotionReading::operator==", "AccelerometerReading::operator!=",
             "GyroscopeReading::operator!=", "AttitudeReading::operator!=", "CompassReading::operator!=",
             "MotionReading::operator!=", "AccelerometerReading::GetHashCode", "GyroscopeReading::GetHashCode",
             "AttitudeReading::GetHashCode", "CompassReading::GetHashCode", "MotionReading::GetHashCode",
             "AccelerometerReading::ToString", "GyroscopeReading::ToString", "AttitudeReading::ToString",
             "CompassReading::ToString", "MotionReading::ToString", "AccelerometerReading::GetTypeName",
             "GyroscopeReading::GetTypeName", "AttitudeReading::GetTypeName", "CompassReading::GetTypeName",
             "MotionReading::GetTypeName", "AccelerometerReadingEventArgs::AccelerometerReadingEventArgs",
             "AccelerometerReadingEventArgs::X", "AccelerometerReadingEventArgs::Y",
             "AccelerometerReadingEventArgs::Z", "AccelerometerReadingEventArgs::Timestamp",
             "AccelerometerReadingEventArgs::operator==", "AccelerometerReadingEventArgs::GetHashCode",
             "AccelerometerReadingEventArgs::ToString", "AccelerometerReadingEventArgs::GetTypeName",
             "SensorReadingEventArgs::SensorReading", "CalibrationEventArgs::CalibrationEventArgs",
             "CalibrationEventArgs::GetTypeName"},
    .tags = {"devices", "sensors", "readings", "values", "events", "cnaext"},
})
