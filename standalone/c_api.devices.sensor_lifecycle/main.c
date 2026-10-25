/* SPDX-License-Identifier: MS-PL */
/*
 * Device sensors from C: report real availability, then drive the public synthetic/test backends
 * so acquisition, readings and event lifetimes stay deterministic without physical hardware.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct AccelerometerEvents {
    int sequence;
    int current_order;
    int legacy_order;
    int current_calls;
    int legacy_calls;
    CNA_AccelerometerReading current;
    CNA_AccelerometerReadingEventInfo legacy;
} AccelerometerEvents;

typedef struct GyroscopeEvents {
    int calls;
    CNA_GyroscopeReading last;
} GyroscopeEvents;

typedef struct CompassEvents {
    int readings;
    int calibrations;
    CNA_CompassReading last;
} CompassEvents;

typedef struct MotionEvents {
    int readings;
    int calibrations;
    CNA_MotionReading last;
} MotionEvents;

typedef struct SensorDemoState {
    CnaDemo* demo;
    int validated;
} SensorDemoState;

static int near_float(float left, float right)
{
    return fabsf(left - right) < 0.001F;
}

static CNA_Vector3 vector3(float x, float y, float z)
{
    CNA_Vector3 value;
    value.x = x;
    value.y = y;
    value.z = z;
    return value;
}

static void on_accelerometer_current(const CNA_AccelerometerReading* reading, void* context)
{
    AccelerometerEvents* events = (AccelerometerEvents*)context;
    events->current = *reading;
    events->current_order = ++events->sequence;
    ++events->current_calls;
}

static void on_accelerometer_legacy(const CNA_AccelerometerReadingEventInfo* info, void* context)
{
    AccelerometerEvents* events = (AccelerometerEvents*)context;
    events->legacy = *info;
    events->legacy_order = ++events->sequence;
    ++events->legacy_calls;
}

static void on_gyroscope_current(const CNA_GyroscopeReading* reading, void* context)
{
    GyroscopeEvents* events = (GyroscopeEvents*)context;
    events->last = *reading;
    ++events->calls;
}

static void on_compass_current(const CNA_CompassReading* reading, void* context)
{
    CompassEvents* events = (CompassEvents*)context;
    events->last = *reading;
    ++events->readings;
}

static void on_compass_calibration(void* context)
{
    ++((CompassEvents*)context)->calibrations;
}

static void on_motion_current(const CNA_MotionReading* reading, void* context)
{
    MotionEvents* events = (MotionEvents*)context;
    events->last = *reading;
    ++events->readings;
}

static void on_motion_calibration(void* context)
{
    ++((MotionEvents*)context)->calibrations;
}

static void accelerometer_contract(CNA_Handle game, CnaDemo* demo)
{
    CNA_AccelerometerHandle sensor = CNA_INVALID_HANDLE;
    CNA_SensorEventRegistrationHandle current_registration = CNA_INVALID_HANDLE;
    CNA_SensorEventRegistrationHandle legacy_registration = CNA_INVALID_HANDLE;
    CNA_AccelerometerReading current;
    CNA_SensorState sensor_state = UINT32_C(99);
    CNA_Bool supported = UINT8_C(9);
    CNA_Bool valid = UINT8_C(9);
    int64_t ticks = INT64_C(-1);
    AccelerometerEvents events;

    memset(&events, 0, sizeof(events));
    if (!cna_demo_check(
            demo,
            cna_accelerometer_get_is_supported(game, &supported) == CNA_RESULT_SUCCESS &&
                (supported == CNA_FALSE || supported == CNA_TRUE) &&
                cna_accelerometer_create(game, &sensor) == CNA_RESULT_SUCCESS &&
                sensor != CNA_INVALID_HANDLE &&
                cna_accelerometer_get_state(sensor, &sensor_state) == CNA_RESULT_SUCCESS &&
                sensor_state <= CNA_SENSOR_STATE_MAXIMUM &&
                cna_accelerometer_get_is_data_valid(sensor, &valid) == CNA_RESULT_SUCCESS &&
                valid == CNA_FALSE,
            "accelerometer availability, owned creation, state and empty-data flag are coherent")) {
        if (sensor != CNA_INVALID_HANDLE) {
            cna_accelerometer_destroy(sensor);
        }
        return;
    }
    printf("  accelerometer supported: %u\n", (unsigned)supported);

    cna_demo_check(demo, supported == CNA_FALSE || supported == CNA_TRUE,
                   "accelerometer host support is reported without starting physical acquisition");

    cna_demo_check(
        demo,
        cna_accelerometer_set_time_between_updates_ticks(sensor, INT64_C(200000)) ==
                CNA_RESULT_SUCCESS &&
            cna_accelerometer_get_time_between_updates_ticks(sensor, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(200000),
        "the accelerometer update interval round-trips in 100-nanosecond ticks");
    cna_demo_check(
        demo,
        cna_accelerometer_set_supported_for_tests_ext(sensor, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_set_started_for_tests_ext(sensor, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_subscribe_current_value_changed(
                sensor, on_accelerometer_current, &events, &current_registration) ==
                CNA_RESULT_SUCCESS &&
            cna_accelerometer_subscribe_reading_changed(
                sensor, on_accelerometer_legacy, &events, &legacy_registration) ==
                CNA_RESULT_SUCCESS,
        "synthetic accelerometer acquisition has both owned event registrations");
    cna_demo_check(
        demo,
        cna_accelerometer_inject_synthetic_update_ext(sensor, 0.0F, 0.0F, 9.80665F) ==
                CNA_RESULT_SUCCESS &&
            events.current_calls == 1 && events.legacy_calls == 1 &&
            events.current_order == 1 && events.legacy_order == 2 &&
            near_float(events.current.acceleration.z, 1.0F) &&
            events.legacy.z > 0.999 && events.legacy.z < 1.001 &&
            events.current.timestamp.ticks != INT64_C(0) && events.legacy.timestamp.ticks != INT64_C(0) &&
            cna_accelerometer_get_current_value(sensor, &current) == CNA_RESULT_SUCCESS &&
            near_float(current.acceleration.z, 1.0F) &&
            cna_accelerometer_get_is_data_valid(sensor, &valid) == CNA_RESULT_SUCCESS &&
            valid == CNA_TRUE,
        "one platform-unit injection publishes current data then the legacy g event");
    cna_demo_check(
        demo,
        cna_sensor_unsubscribe_ext(legacy_registration) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_inject_synthetic_update_ext(sensor, 9.80665F, 0.0F, 0.0F) ==
                CNA_RESULT_SUCCESS &&
            events.current_calls == 2 && events.legacy_calls == 1 &&
            near_float(events.current.acceleration.x, 1.0F),
        "unsubscribing the legacy event leaves current-value delivery attached");
    cna_demo_check(
        demo,
        cna_accelerometer_stop(sensor) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_get_state(sensor, &sensor_state) == CNA_RESULT_SUCCESS &&
            sensor_state == CNA_SENSOR_STATE_DISABLED &&
            cna_accelerometer_inject_synthetic_update_ext(sensor, 0.0F, 9.80665F, 0.0F) ==
                CNA_RESULT_SUCCESS &&
            events.current_calls == 2 &&
            cna_sensor_unsubscribe_ext(current_registration) == CNA_RESULT_SUCCESS,
        "Stop disables synthetic accelerometer delivery and the remaining registration releases");
    cna_demo_check(
        demo,
        cna_accelerometer_dispose(sensor) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_dispose(sensor) == CNA_RESULT_INVALID_STATE,
        "accelerometer disposal succeeds once and refuses a second call");
    cna_demo_check(
        demo,
        cna_accelerometer_get_current_value(sensor, &current) == CNA_RESULT_INVALID_STATE,
        "a disposed accelerometer refuses CurrentValue (CNA-DEMOS-ISSUE-062)");
    cna_demo_check(
        demo,
        cna_accelerometer_destroy(sensor) == CNA_RESULT_SUCCESS &&
            cna_accelerometer_destroy(sensor) == CNA_RESULT_INVALID_HANDLE,
        "accelerometer handle destruction releases exactly once");
}

static void gyroscope_contract(CNA_Handle game, CnaDemo* demo)
{
    CNA_GyroscopeHandle sensor = CNA_INVALID_HANDLE;
    CNA_SensorEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    CNA_GyroscopeReading current;
    CNA_SensorState sensor_state = UINT32_C(99);
    CNA_Bool supported = UINT8_C(9);
    CNA_Bool valid = UINT8_C(9);
    int64_t ticks = INT64_C(-1);
    GyroscopeEvents events;

    memset(&events, 0, sizeof(events));
    if (!cna_demo_check(
            demo,
            cna_gyroscope_get_is_supported(game, &supported) == CNA_RESULT_SUCCESS &&
                (supported == CNA_FALSE || supported == CNA_TRUE) &&
                cna_gyroscope_create(game, &sensor) == CNA_RESULT_SUCCESS &&
                cna_gyroscope_get_state(sensor, &sensor_state) == CNA_RESULT_SUCCESS &&
                sensor_state <= CNA_SENSOR_STATE_MAXIMUM &&
                cna_gyroscope_get_is_data_valid(sensor, &valid) == CNA_RESULT_SUCCESS &&
                valid == CNA_FALSE,
            "gyroscope availability, owned creation, state and empty-data flag are coherent")) {
        if (sensor != CNA_INVALID_HANDLE) {
            cna_gyroscope_destroy(sensor);
        }
        return;
    }
    printf("  gyroscope supported: %u\n", (unsigned)supported);
    cna_demo_check(demo, supported == CNA_FALSE || supported == CNA_TRUE,
                   "gyroscope host support is reported without starting physical acquisition");
    cna_demo_check(
        demo,
        cna_gyroscope_set_time_between_updates_ticks(sensor, INT64_C(300000)) ==
                CNA_RESULT_SUCCESS &&
            cna_gyroscope_get_time_between_updates_ticks(sensor, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(300000),
        "the gyroscope update interval round-trips in 100-nanosecond ticks");
    cna_demo_check(
        demo,
        cna_gyroscope_set_supported_for_tests_ext(sensor, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_gyroscope_set_started_for_tests_ext(sensor, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_gyroscope_subscribe_current_value_changed(
                sensor, on_gyroscope_current, &events, &registration) == CNA_RESULT_SUCCESS &&
            cna_gyroscope_inject_synthetic_update_ext(sensor, 0.125F, 0.25F, 0.5F) ==
                CNA_RESULT_SUCCESS &&
            events.calls == 1 && events.last.rotation_rate.x == 0.125F &&
            events.last.rotation_rate.z == 0.5F &&
            cna_gyroscope_get_current_value(sensor, &current) == CNA_RESULT_SUCCESS &&
            current.rotation_rate.y == 0.25F &&
            cna_gyroscope_get_is_data_valid(sensor, &valid) == CNA_RESULT_SUCCESS &&
            valid == CNA_TRUE,
        "a synthetic angular-rate update becomes current data and reaches its handler");
    cna_demo_check(
        demo,
        cna_gyroscope_stop(sensor) == CNA_RESULT_SUCCESS &&
            cna_gyroscope_inject_synthetic_update_ext(sensor, 1.0F, 2.0F, 3.0F) ==
                CNA_RESULT_SUCCESS &&
            events.calls == 1 && cna_sensor_unsubscribe_ext(registration) == CNA_RESULT_SUCCESS,
        "Stop disables synthetic gyroscope delivery and the registration releases");
    cna_demo_check(
        demo,
        cna_gyroscope_dispose(sensor) == CNA_RESULT_SUCCESS &&
            cna_gyroscope_dispose(sensor) == CNA_RESULT_INVALID_STATE,
        "gyroscope disposal succeeds once and refuses a second call");
    cna_demo_check(
        demo,
        cna_gyroscope_get_current_value(sensor, &current) == CNA_RESULT_INVALID_STATE,
        "a disposed gyroscope refuses CurrentValue (CNA-DEMOS-ISSUE-062)");
    cna_demo_check(
        demo,
        cna_gyroscope_destroy(sensor) == CNA_RESULT_SUCCESS &&
            cna_gyroscope_destroy(sensor) == CNA_RESULT_INVALID_HANDLE,
        "gyroscope handle destruction releases exactly once");
}

static void compass_contract(CNA_Handle game, CnaDemo* demo)
{
    const CNA_DateTimeOffset when = {INT64_C(637000000000000000), INT64_C(0)};
    CNA_CompassHandle sensor = CNA_INVALID_HANDLE;
    CNA_SensorEventRegistrationHandle readings = CNA_INVALID_HANDLE;
    CNA_SensorEventRegistrationHandle calibrations = CNA_INVALID_HANDLE;
    CNA_CompassReading reading;
    CNA_CompassReading current;
    CNA_SensorState sensor_state = UINT32_C(99);
    CNA_Bool supported = UINT8_C(9);
    CNA_Bool valid = UINT8_C(9);
    int64_t ticks = INT64_C(-1);
    CompassEvents events;

    memset(&events, 0, sizeof(events));
    if (!cna_demo_check(
            demo,
            cna_compass_get_is_supported(game, &supported) == CNA_RESULT_SUCCESS &&
                (supported == CNA_FALSE || supported == CNA_TRUE) &&
                cna_compass_create(game, &sensor) == CNA_RESULT_SUCCESS &&
                cna_compass_get_state(sensor, &sensor_state) == CNA_RESULT_SUCCESS &&
                sensor_state <= CNA_SENSOR_STATE_MAXIMUM &&
                cna_compass_get_is_data_valid(sensor, &valid) == CNA_RESULT_SUCCESS &&
                valid == CNA_FALSE,
            "compass availability, owned creation, state and empty-data flag are coherent")) {
        if (sensor != CNA_INVALID_HANDLE) {
            cna_compass_destroy(sensor);
        }
        return;
    }
    printf("  compass supported: %u\n", (unsigned)supported);
    cna_demo_check(demo, supported == CNA_FALSE || supported == CNA_TRUE,
                   "compass host support is reported without starting physical acquisition");

    cna_demo_check(
        demo,
        cna_compass_set_test_backend_ext(sensor, CNA_TRUE, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_compass_subscribe_current_value_changed(
                sensor, on_compass_current, &events, &readings) == CNA_RESULT_SUCCESS &&
            cna_compass_subscribe_calibrate(
                sensor, on_compass_calibration, &events, &calibrations) == CNA_RESULT_SUCCESS &&
            cna_compass_set_time_between_updates_ticks(sensor, INT64_C(400000)) ==
                CNA_RESULT_SUCCESS &&
            cna_compass_get_time_between_updates_ticks(sensor, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(400000) && cna_compass_start(sensor) == CNA_RESULT_SUCCESS &&
            cna_compass_get_state(sensor, &sensor_state) == CNA_RESULT_SUCCESS &&
            sensor_state == CNA_SENSOR_STATE_READY,
        "the compass test backend starts with reading/calibration handlers and interval");
    cna_demo_check(
        demo,
        cna_compass_reading_init_from_values(
            2.5, 45.0, vector3(1.0F, 2.0F, 3.0F), when, 46.5, &reading) ==
                CNA_RESULT_SUCCESS &&
            cna_compass_inject_synthetic_update_ext(sensor, &reading) == CNA_RESULT_SUCCESS &&
            events.readings == 1 && events.last.magnetic_heading == 45.0 &&
            events.last.true_heading == 46.5 && events.last.magnetometer_reading.z == 3.0F &&
            cna_compass_get_current_value(sensor, &current) == CNA_RESULT_SUCCESS &&
            current.heading_accuracy == 2.5 &&
            cna_compass_get_is_data_valid(sensor, &valid) == CNA_RESULT_SUCCESS && valid == CNA_TRUE,
        "an authored compass reading becomes valid current data and reaches its handler");
    cna_demo_check(
        demo,
        cna_compass_inject_calibration_request_ext(sensor) == CNA_RESULT_SUCCESS &&
            events.calibrations == 1 &&
            cna_sensor_unsubscribe_ext(calibrations) == CNA_RESULT_SUCCESS &&
            cna_compass_inject_calibration_request_ext(sensor) == CNA_RESULT_SUCCESS &&
            events.calibrations == 1,
        "compass calibration is context-only and selective unsubscribe stops it");
    cna_demo_check(
        demo,
        cna_compass_set_test_backend_ext(sensor, CNA_FALSE, CNA_FALSE) ==
                CNA_RESULT_INVALID_STATE &&
            cna_compass_stop(sensor) == CNA_RESULT_SUCCESS &&
            cna_compass_get_state(sensor, &sensor_state) == CNA_RESULT_SUCCESS &&
            sensor_state == CNA_SENSOR_STATE_DISABLED &&
            cna_compass_inject_synthetic_update_ext(sensor, &reading) == CNA_RESULT_INVALID_STATE &&
            events.readings == 1 && cna_sensor_unsubscribe_ext(readings) == CNA_RESULT_SUCCESS &&
            cna_compass_set_test_backend_ext(sensor, CNA_FALSE, CNA_FALSE) == CNA_RESULT_SUCCESS,
        "a running compass keeps its backend; Stop ends delivery before backend removal");
    cna_demo_check(
        demo,
        cna_compass_dispose(sensor) == CNA_RESULT_SUCCESS &&
            cna_compass_dispose(sensor) == CNA_RESULT_INVALID_STATE,
        "compass disposal succeeds once and refuses a second call");
    cna_demo_check(
        demo,
        cna_compass_get_current_value(sensor, &current) == CNA_RESULT_INVALID_STATE,
        "a disposed compass refuses CurrentValue (CNA-DEMOS-ISSUE-062)");
    cna_demo_check(
        demo,
        cna_compass_destroy(sensor) == CNA_RESULT_SUCCESS &&
            cna_compass_destroy(sensor) == CNA_RESULT_INVALID_HANDLE,
        "compass handle destruction releases exactly once");
}

static void motion_contract(CNA_Handle game, CnaDemo* demo)
{
    const CNA_DateTimeOffset when = {INT64_C(637000000000000000), INT64_C(0)};
    CNA_MotionHandle sensor = CNA_INVALID_HANDLE;
    CNA_SensorEventRegistrationHandle readings = CNA_INVALID_HANDLE;
    CNA_SensorEventRegistrationHandle calibrations = CNA_INVALID_HANDLE;
    CNA_AttitudeReading attitude;
    CNA_MotionReading reading;
    CNA_MotionReading current;
    CNA_Quaternion orientation;
    CNA_Matrix rotation;
    CNA_SensorState sensor_state = UINT32_C(99);
    CNA_Bool supported = UINT8_C(9);
    CNA_Bool valid = UINT8_C(9);
    CNA_Bool north = UINT8_C(9);
    int64_t ticks = INT64_C(-1);
    MotionEvents events;

    memset(&events, 0, sizeof(events));
    memset(&orientation, 0, sizeof(orientation));
    orientation.w = 1.0F;
    memset(&rotation, 0, sizeof(rotation));
    rotation.m11 = 1.0F;
    rotation.m22 = 1.0F;
    rotation.m33 = 1.0F;
    rotation.m44 = 1.0F;
    if (!cna_demo_check(
            demo,
            cna_motion_get_is_supported(game, &supported) == CNA_RESULT_SUCCESS &&
                (supported == CNA_FALSE || supported == CNA_TRUE) &&
                cna_motion_create(game, &sensor) == CNA_RESULT_SUCCESS &&
                cna_motion_get_state(sensor, &sensor_state) == CNA_RESULT_SUCCESS &&
                sensor_state <= CNA_SENSOR_STATE_MAXIMUM &&
                cna_motion_get_is_data_valid(sensor, &valid) == CNA_RESULT_SUCCESS &&
                valid == CNA_FALSE &&
                cna_motion_get_is_attitude_north_referenced_ext(sensor, &north) ==
                    CNA_RESULT_SUCCESS &&
                north == CNA_TRUE,
            "motion availability, owned creation, state and vacuous north reference are coherent")) {
        if (sensor != CNA_INVALID_HANDLE) {
            cna_motion_destroy(sensor);
        }
        return;
    }
    printf("  fused motion supported: %u\n", (unsigned)supported);
    cna_demo_check(demo, supported == CNA_FALSE || supported == CNA_TRUE,
                   "fused-motion host support is reported without starting physical acquisition");
    cna_demo_check(
        demo,
        cna_motion_set_test_backend_ext(sensor, CNA_TRUE, CNA_TRUE, CNA_FALSE) ==
                CNA_RESULT_SUCCESS &&
            cna_motion_subscribe_current_value_changed(
                sensor, on_motion_current, &events, &readings) == CNA_RESULT_SUCCESS &&
            cna_motion_subscribe_calibrate(
                sensor, on_motion_calibration, &events, &calibrations) == CNA_RESULT_SUCCESS &&
            cna_motion_set_time_between_updates_ticks(sensor, INT64_C(500000)) ==
                CNA_RESULT_SUCCESS &&
            cna_motion_get_time_between_updates_ticks(sensor, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(500000) && cna_motion_start(sensor) == CNA_RESULT_SUCCESS &&
            cna_motion_get_state(sensor, &sensor_state) == CNA_RESULT_SUCCESS &&
            sensor_state == CNA_SENSOR_STATE_READY &&
            cna_motion_get_is_attitude_north_referenced_ext(sensor, &north) ==
                CNA_RESULT_SUCCESS &&
            north == CNA_FALSE,
        "the drift-prone motion backend starts with handlers, interval and false north reference");
    cna_demo_check(
        demo,
        cna_attitude_reading_init_from_values(
            0.0F, 0.25F, 0.5F, orientation, rotation, when, &attitude) == CNA_RESULT_SUCCESS &&
            cna_motion_reading_init_from_values(
                &attitude, vector3(0.25F, 0.5F, 0.75F), vector3(1.0F, 1.25F, 1.5F),
                vector3(0.0F, -1.0F, 0.0F), when, &reading) == CNA_RESULT_SUCCESS &&
            cna_motion_inject_synthetic_update_ext(sensor, &reading) == CNA_RESULT_SUCCESS &&
            events.readings == 1 && events.last.attitude.roll == 0.25F &&
            events.last.device_acceleration.z == 0.75F && events.last.gravity.y == -1.0F &&
            cna_motion_get_current_value(sensor, &current) == CNA_RESULT_SUCCESS &&
            current.device_rotation_rate.x == 1.0F &&
            cna_motion_get_is_data_valid(sensor, &valid) == CNA_RESULT_SUCCESS && valid == CNA_TRUE,
        "an authored fused-motion reading becomes valid current data and reaches its handler");
    cna_demo_check(
        demo,
        cna_motion_inject_calibration_request_ext(sensor) == CNA_RESULT_SUCCESS &&
            events.calibrations == 1 && cna_sensor_unsubscribe_ext(calibrations) == CNA_RESULT_SUCCESS &&
            cna_motion_inject_calibration_request_ext(sensor) == CNA_RESULT_SUCCESS &&
            events.calibrations == 1,
        "motion calibration is context-only and selective unsubscribe stops it");
    cna_demo_check(
        demo,
        cna_motion_set_test_backend_ext(sensor, CNA_FALSE, CNA_FALSE, CNA_FALSE) ==
                CNA_RESULT_INVALID_STATE &&
            cna_motion_stop(sensor) == CNA_RESULT_SUCCESS &&
            cna_motion_inject_synthetic_update_ext(sensor, &reading) == CNA_RESULT_INVALID_STATE &&
            events.readings == 1 && cna_sensor_unsubscribe_ext(readings) == CNA_RESULT_SUCCESS &&
            cna_motion_set_test_backend_ext(sensor, CNA_FALSE, CNA_FALSE, CNA_FALSE) ==
                CNA_RESULT_SUCCESS,
        "a running motion sensor keeps its backend; Stop ends delivery before backend removal");
    cna_demo_check(
        demo,
        cna_motion_dispose(sensor) == CNA_RESULT_SUCCESS &&
            cna_motion_dispose(sensor) == CNA_RESULT_INVALID_STATE &&
            cna_motion_get_is_attitude_north_referenced_ext(sensor, &north) ==
                CNA_RESULT_INVALID_STATE,
        "motion disposal is observable to both disposal and extension queries");
    cna_demo_check(
        demo,
        cna_motion_get_current_value(sensor, &current) == CNA_RESULT_INVALID_STATE,
        "a disposed motion sensor refuses CurrentValue (CNA-DEMOS-ISSUE-062)");
    cna_demo_check(
        demo,
        cna_motion_destroy(sensor) == CNA_RESULT_SUCCESS &&
            cna_motion_destroy(sensor) == CNA_RESULT_INVALID_HANDLE,
        "motion handle destruction releases exactly once");
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    SensorDemoState* state = (SensorDemoState*)context;
    (void)game_time;
    (void)out_error;
    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    accelerometer_contract(game, state->demo);
    gyroscope_contract(game, state->demo);
    compass_contract(game, state->demo);
    motion_contract(game, state->demo);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    SensorDemoState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.devices.sensor_lifecycle", argc, argv, 0);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                          "run one frame for owner-thread sensor access");
    cna_demo_check(&demo, state.validated == 1,
                   "the update callback completed the four sensor lifecycles once");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
