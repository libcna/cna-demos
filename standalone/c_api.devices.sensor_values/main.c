/* SPDX-License-Identifier: MS-PL */
/*
 * Sensor values from C: construct every reading family and inspect the canonical value semantics.
 * These are pure POD operations, so no game or physical sensor is needed.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_DateTimeOffset stamp(int64_t ticks, int64_t offset_ticks)
{
    CNA_DateTimeOffset value;
    value.ticks = ticks;
    value.offset_ticks = offset_ticks;
    return value;
}

static CNA_Vector3 vector3(float x, float y, float z)
{
    CNA_Vector3 value;
    value.x = x;
    value.y = y;
    value.z = z;
    return value;
}

static CNA_Quaternion identity_quaternion(void)
{
    CNA_Quaternion value;
    memset(&value, 0, sizeof(value));
    value.w = 1.0F;
    return value;
}

static CNA_Matrix identity_matrix(void)
{
    CNA_Matrix value;
    memset(&value, 0, sizeof(value));
    value.m11 = 1.0F;
    value.m22 = 1.0F;
    value.m33 = 1.0F;
    value.m44 = 1.0F;
    return value;
}

static void accelerometer_value(CnaDemo* demo)
{
    const CNA_DateTimeOffset when = stamp(INT64_C(637000000000000000), INT64_C(36000000000));
    CNA_AccelerometerReading reading;
    CNA_AccelerometerReading other;
    CNA_Bool equal = UINT8_C(9);
    uint64_t hash = UINT64_C(0);
    uint64_t other_hash = UINT64_C(1);
    uint64_t bytes = UINT64_C(0);
    char text[256];

    memset(&reading, 9, sizeof(reading));
    cna_demo_check(
        demo,
        cna_accelerometer_reading_init(&reading) == CNA_RESULT_SUCCESS &&
            reading.struct_size == (uint32_t)sizeof(reading) && reading.struct_version == UINT32_C(1) &&
            reading.timestamp.ticks == INT64_C(0) && reading.acceleration.x == 0.0F &&
            reading.acceleration.y == 0.0F && reading.acceleration.z == 0.0F,
        "an accelerometer reading defaults to a versioned zero measurement");
    cna_demo_check(
        demo,
        cna_accelerometer_reading_init_from_values(when, vector3(1.0F, 2.0F, 3.0F), &reading) ==
                CNA_RESULT_SUCCESS &&
            reading.timestamp.ticks == when.ticks && reading.timestamp.offset_ticks == when.offset_ticks &&
            reading.acceleration.y == 2.0F,
        "the accelerometer constructor preserves its timestamp and g vector");

    other = reading;
    cna_demo_check(
        demo,
        cna_accelerometer_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE &&
            cna_accelerometer_reading_get_hash_code(&reading, &hash) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_reading_get_hash_code(&other, &other_hash) == CNA_RESULT_SUCCESS &&
            hash == other_hash,
        "equal accelerometer readings have equal hashes");
    other.acceleration.z = 4.0F;
    cna_demo_check(
        demo,
        cna_accelerometer_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "accelerometer equality observes the acceleration vector");
    other = reading;
    ++other.timestamp.ticks;
    cna_demo_check(
        demo,
        cna_accelerometer_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "accelerometer equality also observes the timestamp omitted from its text");

    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_accelerometer_reading_get_string_size(&reading, &bytes) == CNA_RESULT_SUCCESS &&
            bytes < (uint64_t)sizeof(text) &&
            cna_accelerometer_reading_copy_string(&reading, text, sizeof(text), &bytes) ==
                CNA_RESULT_SUCCESS &&
            strncmp(text, "Acceleration:", 13U) == 0,
        "accelerometer text presents the acceleration");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_accelerometer_reading_get_type_name_size(&bytes) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_reading_copy_type_name(text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            strcmp(text, "Microsoft.Devices.Sensors.AccelerometerReading") == 0,
        "the accelerometer reading reports its exact canonical type name");
}

static void gyroscope_value(CnaDemo* demo)
{
    CNA_GyroscopeReading reading;
    CNA_GyroscopeReading other;
    CNA_Bool equal = UINT8_C(9);
    uint64_t hash = UINT64_C(0);
    uint64_t other_hash = UINT64_C(1);
    uint64_t bytes = UINT64_C(0);
    char text[256];

    memset(&reading, 9, sizeof(reading));
    cna_demo_check(
        demo,
        cna_gyroscope_reading_init(&reading) == CNA_RESULT_SUCCESS &&
            reading.struct_size == (uint32_t)sizeof(reading) && reading.rotation_rate.x == 0.0F &&
            cna_gyroscope_reading_init_from_values(
                vector3(0.5F, 1.5F, 2.5F), stamp(INT64_C(42), INT64_C(0)), &reading) ==
                CNA_RESULT_SUCCESS &&
            reading.rotation_rate.z == 2.5F && reading.timestamp.ticks == INT64_C(42),
        "a gyroscope reading preserves angular velocity and timestamp");
    other = reading;
    cna_demo_check(
        demo,
        cna_gyroscope_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE && cna_gyroscope_reading_get_hash_code(&reading, &hash) == CNA_RESULT_SUCCESS &&
            cna_gyroscope_reading_get_hash_code(&other, &other_hash) == CNA_RESULT_SUCCESS &&
            hash == other_hash,
        "equal gyroscope readings compare and hash equally");
    other.rotation_rate.x = -0.5F;
    cna_demo_check(
        demo,
        cna_gyroscope_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "gyroscope equality observes angular velocity");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_gyroscope_reading_get_string_size(&reading, &bytes) == CNA_RESULT_SUCCESS &&
            cna_gyroscope_reading_copy_string(&reading, text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            strncmp(text, "RotationRate:", 13U) == 0,
        "gyroscope text presents the rotation rate");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_gyroscope_reading_get_type_name_size(&bytes) == CNA_RESULT_SUCCESS &&
            cna_gyroscope_reading_copy_type_name(text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            strcmp(text, "Microsoft.Devices.Sensors.GyroscopeReading") == 0,
        "the gyroscope reading reports its exact canonical type name");
}

static void attitude_value(CnaDemo* demo, CNA_AttitudeReading* out_attitude)
{
    CNA_AttitudeReading reading;
    CNA_AttitudeReading other;
    CNA_Bool equal = UINT8_C(9);
    uint64_t hash = UINT64_C(0);
    uint64_t other_hash = UINT64_C(1);
    uint64_t bytes = UINT64_C(0);
    char text[256];
    unsigned char guard[8];

    memset(&reading, 9, sizeof(reading));
    cna_demo_check(
        demo,
        cna_attitude_reading_init(&reading) == CNA_RESULT_SUCCESS && reading.pitch == 0.0F &&
            reading.roll == 0.0F && reading.yaw == 0.0F &&
            cna_attitude_reading_init_from_values(
                0.25F, 0.5F, 0.75F, identity_quaternion(), identity_matrix(),
                stamp(INT64_C(7), INT64_C(0)), &reading) == CNA_RESULT_SUCCESS &&
            reading.pitch == 0.25F && reading.roll == 0.5F && reading.yaw == 0.75F &&
            reading.quaternion.w == 1.0F && reading.rotation_matrix.m33 == 1.0F,
        "an attitude reading preserves Euler angles, quaternion, matrix and time");
    other = reading;
    cna_demo_check(
        demo,
        cna_attitude_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE && cna_attitude_reading_get_hash_code(&reading, &hash) == CNA_RESULT_SUCCESS &&
            cna_attitude_reading_get_hash_code(&other, &other_hash) == CNA_RESULT_SUCCESS &&
            hash == other_hash,
        "equal attitude readings compare and hash equally");
    other.rotation_matrix.m12 = 0.5F;
    cna_demo_check(
        demo,
        cna_attitude_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "attitude equality observes the full rotation matrix");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_attitude_reading_get_string_size(&reading, &bytes) == CNA_RESULT_SUCCESS &&
            cna_attitude_reading_copy_string(&reading, text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            strncmp(text, "Pitch:", 6U) == 0,
        "attitude text starts with the Euler pitch");
    memset(guard, 0x7f, sizeof(guard));
    cna_demo_check(
        demo,
        cna_attitude_reading_copy_string(&reading, (char*)guard, UINT64_C(2), &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            guard[0] == 0x7fU,
        "a short attitude-text buffer is refused before any write");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_attitude_reading_get_type_name_size(&bytes) == CNA_RESULT_SUCCESS &&
            cna_attitude_reading_copy_type_name(text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            strcmp(text, "Microsoft.Devices.Sensors.AttitudeReading") == 0,
        "the attitude reading reports its exact canonical type name");
    *out_attitude = reading;
}

static void compass_value(CnaDemo* demo)
{
    CNA_CompassReading reading;
    CNA_CompassReading other;
    CNA_Bool equal = UINT8_C(9);
    uint64_t hash = UINT64_C(0);
    uint64_t other_hash = UINT64_C(1);
    uint64_t bytes = UINT64_C(0);
    char text[256];

    memset(&reading, 9, sizeof(reading));
    cna_demo_check(
        demo,
        cna_compass_reading_init(&reading) == CNA_RESULT_SUCCESS && reading.heading_accuracy == 0.0 &&
            reading.magnetic_heading == 0.0 && reading.true_heading == 0.0 &&
            cna_compass_reading_init_from_values(
                1.5, 90.0, vector3(4.0F, 5.0F, 6.0F), stamp(INT64_C(11), INT64_C(0)), 92.5,
                &reading) == CNA_RESULT_SUCCESS &&
            reading.heading_accuracy == 1.5 && reading.magnetic_heading == 90.0 &&
            reading.true_heading == 92.5 && reading.magnetometer_reading.z == 6.0F,
        "a compass reading preserves accuracy, both headings, field vector and time");
    other = reading;
    cna_demo_check(
        demo,
        cna_compass_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE && cna_compass_reading_get_hash_code(&reading, &hash) == CNA_RESULT_SUCCESS &&
            cna_compass_reading_get_hash_code(&other, &other_hash) == CNA_RESULT_SUCCESS &&
            hash == other_hash,
        "equal compass readings compare and hash equally");
    other.true_heading = 93.0;
    cna_demo_check(
        demo,
        cna_compass_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "compass equality observes true north independently");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_compass_reading_get_string_size(&reading, &bytes) == CNA_RESULT_SUCCESS &&
            cna_compass_reading_copy_string(&reading, text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            strncmp(text, "MagneticHeading:", 16U) == 0,
        "compass text starts with its magnetic heading");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_compass_reading_get_type_name_size(&bytes) == CNA_RESULT_SUCCESS &&
            cna_compass_reading_copy_type_name(text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            strcmp(text, "Microsoft.Devices.Sensors.CompassReading") == 0,
        "the compass reading reports its exact canonical type name");
}

static void motion_value(CnaDemo* demo, const CNA_AttitudeReading* attitude)
{
    CNA_MotionReading reading;
    CNA_MotionReading other;
    CNA_Bool equal = UINT8_C(9);
    uint64_t hash = UINT64_C(0);
    uint64_t other_hash = UINT64_C(1);
    uint64_t bytes = UINT64_C(0);
    char text[256];

    memset(&reading, 9, sizeof(reading));
    cna_demo_check(
        demo,
        cna_motion_reading_init(&reading) == CNA_RESULT_SUCCESS &&
            reading.attitude.struct_size == (uint32_t)sizeof(CNA_AttitudeReading) &&
            reading.gravity.y == 0.0F &&
            cna_motion_reading_init_from_values(
                attitude, vector3(1.0F, 0.0F, 0.0F), vector3(0.0F, 1.0F, 0.0F),
                vector3(0.0F, 0.0F, -1.0F), stamp(INT64_C(21), INT64_C(0)), &reading) ==
                CNA_RESULT_SUCCESS &&
            reading.attitude.yaw == 0.75F && reading.device_acceleration.x == 1.0F &&
            reading.device_rotation_rate.y == 1.0F && reading.gravity.z == -1.0F,
        "a motion reading preserves nested attitude, acceleration, angular rate and gravity");
    other = reading;
    cna_demo_check(
        demo,
        cna_motion_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE && cna_motion_reading_get_hash_code(&reading, &hash) == CNA_RESULT_SUCCESS &&
            cna_motion_reading_get_hash_code(&other, &other_hash) == CNA_RESULT_SUCCESS &&
            hash == other_hash,
        "equal fused-motion readings compare and hash equally");
    other.attitude.pitch = 2.0F;
    cna_demo_check(
        demo,
        cna_motion_reading_equals(&reading, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "motion equality observes fields inside the nested attitude");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_motion_reading_get_string_size(&reading, &bytes) == CNA_RESULT_SUCCESS &&
            cna_motion_reading_copy_string(&reading, text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            strncmp(text, "DeviceAcceleration:", 19U) == 0 && strstr(text, " Gravity:") != NULL &&
            strstr(text, "Pitch:") == NULL,
        "motion text presents acceleration and gravity while omitting its attitude");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_motion_reading_get_type_name_size(&bytes) == CNA_RESULT_SUCCESS &&
            cna_motion_reading_copy_type_name(text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            strcmp(text, "Microsoft.Devices.Sensors.MotionReading") == 0,
        "the motion reading reports its exact canonical type name");
}

static void legacy_event_values(CnaDemo* demo)
{
    const CNA_DateTimeOffset when = stamp(INT64_C(637000000000000000), INT64_C(36000000000));
    CNA_AccelerometerReadingEventInfo info;
    CNA_AccelerometerReadingEventInfo other;
    CNA_Bool equal = UINT8_C(9);
    uint64_t hash = UINT64_C(0);
    uint64_t other_hash = UINT64_C(1);
    uint64_t bytes = UINT64_C(0);
    char text[256];

    memset(&info, 9, sizeof(info));
    cna_demo_check(
        demo,
        cna_accelerometer_reading_event_info_init(&info) == CNA_RESULT_SUCCESS &&
            info.struct_size == (uint32_t)sizeof(info) && info.struct_version == UINT32_C(1) &&
            info.x == 0.0 && info.y == 0.0 && info.z == 0.0 && info.timestamp.ticks == INT64_C(0) &&
            cna_accelerometer_reading_event_info_init_from_values(1.0, 2.0, 3.0, when, &info) ==
                CNA_RESULT_SUCCESS &&
            info.x == 1.0 && info.y == 2.0 && info.z == 3.0 && info.timestamp.ticks == when.ticks,
        "the legacy accelerometer event value keeps three doubles and a timestamp");
    other = info;
    cna_demo_check(
        demo,
        cna_accelerometer_reading_event_info_equals(&info, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE &&
            cna_accelerometer_reading_event_info_get_hash_code(&info, &hash) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_reading_event_info_get_hash_code(&other, &other_hash) == CNA_RESULT_SUCCESS &&
            hash == other_hash,
        "equal legacy event values compare and hash equally");
    ++other.timestamp.ticks;
    cna_demo_check(
        demo,
        cna_accelerometer_reading_event_info_equals(&info, &other, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "legacy event equality observes the timestamp omitted from its text");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_accelerometer_reading_event_info_get_string_size(&info, &bytes) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_reading_event_info_copy_string(&info, text, sizeof(text), &bytes) ==
                CNA_RESULT_SUCCESS &&
            strcmp(text, "{X:1 Y:2 Z:3}") == 0,
        "the legacy event has the canonical three-component text");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_accelerometer_reading_event_info_get_type_name_size(&bytes) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_reading_event_info_copy_type_name(text, sizeof(text), &bytes) ==
                CNA_RESULT_SUCCESS &&
            strcmp(text, "Microsoft.Devices.Sensors.AccelerometerReadingEventArgs") == 0,
        "the legacy event value reports its exact canonical type name");
    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_calibration_event_info_get_type_name_size(&bytes) == CNA_RESULT_SUCCESS &&
            cna_calibration_event_info_copy_type_name(text, sizeof(text), &bytes) ==
                CNA_RESULT_SUCCESS &&
            strcmp(text, "Microsoft.Devices.Sensors.CalibrationEventArgs") == 0,
        "the empty calibration event is represented by its exact type name");
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_AttitudeReading attitude;

    cna_demo_init(&demo, "c_api.devices.sensor_values", argc, argv, 0);
    accelerometer_value(&demo);
    gyroscope_value(&demo);
    attitude_value(&demo, &attitude);
    compass_value(&demo);
    motion_value(&demo, &attitude);
    legacy_event_values(&demo);
    return cna_demo_finish(&demo);
}
