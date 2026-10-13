/* SPDX-License-Identifier: MS-PL */
/*
 * Host input devices from C: compare fixed descriptor values, enumerate platform devices, read
 * optional machine motion sensors, exercise the public mouse/keyboard hot-plug events without
 * hardware and read the host power snapshot.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef CNA_Result (*CountRoute)(CNA_Handle, uint32_t*);
typedef CNA_Result (*InfoRoute)(CNA_Handle, uint32_t, CNA_InputDeviceInfo*);
typedef CNA_Result (*NameSizeRoute)(CNA_Handle, uint32_t, uint64_t*);
typedef CNA_Result (*CopyNameRoute)(CNA_Handle, uint32_t, char*, uint64_t, uint64_t*);

typedef struct DeviceDemoState {
    CnaDemo* demo;
    int validated;
} DeviceDemoState;

typedef struct HotplugState {
    uint32_t mouse_connected_id;
    uint32_t mouse_disconnected_id;
    uint32_t keyboard_connected_id;
    uint32_t keyboard_disconnected_id;
    int mouse_connected_calls;
    int mouse_disconnected_calls;
    int keyboard_connected_calls;
    int keyboard_disconnected_calls;
} HotplugState;

static CNA_StringView text_of(const char* value)
{
    CNA_StringView view;
    view.data = value;
    view.byte_length = (uint64_t)strlen(value);
    return view;
}

static int same_vector3(const CNA_Vector3* value, float x, float y, float z)
{
    return value->x == x && value->y == y && value->z == z;
}

static void descriptor_values(CnaDemo* demo)
{
    CNA_SensorInfo sensor;
    CNA_SensorInfo other_sensor;
    CNA_InputDeviceInfo device;
    CNA_InputDeviceInfo other_device;
    CNA_Bool equal = UINT8_C(9);

    memset(&sensor, 0x5a, sizeof(sensor));
    cna_demo_check(
        demo,
        cna_sensor_info_init(&sensor) == CNA_RESULT_SUCCESS &&
            sensor.struct_size == (uint32_t)sizeof(sensor) && sensor.struct_version == UINT32_C(1) &&
            sensor.id == UINT32_C(0) && sensor.type == CNA_SENSOR_TYPE_UNKNOWN,
        "the default sensor descriptor is versioned with zero id and Unknown type");
    other_sensor = sensor;
    cna_demo_check(
        demo,
        cna_sensor_info_equals(&sensor, text_of("gyro"), &other_sensor, text_of("gyro"), &equal) ==
                CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE &&
            ((other_sensor.id = UINT32_C(7)),
             cna_sensor_info_equals(&sensor, text_of("gyro"), &other_sensor, text_of("gyro"), &equal) ==
                 CNA_RESULT_SUCCESS) &&
            equal == CNA_FALSE &&
            ((other_sensor = sensor), (other_sensor.type = CNA_SENSOR_TYPE_GYROSCOPE),
             cna_sensor_info_equals(&sensor, text_of("gyro"), &other_sensor, text_of("gyro"), &equal) ==
                 CNA_RESULT_SUCCESS) &&
            equal == CNA_FALSE &&
            cna_sensor_info_equals(&sensor, text_of("gyro"), &sensor, text_of("accelerometer"), &equal) ==
                CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "sensor equality observes the identifier, kind and UTF-8 name");

    memset(&device, 0x5a, sizeof(device));
    cna_demo_check(
        demo,
        cna_input_device_info_init(&device) == CNA_RESULT_SUCCESS &&
            device.struct_size == (uint32_t)sizeof(device) && device.struct_version == UINT32_C(1) &&
            device.id == UINT64_C(0),
        "the default input-device descriptor is versioned with a zero 64-bit id");
    other_device = device;
    other_device.id = UINT64_C(0x1FFFFFFFF);
    cna_demo_check(
        demo,
        cna_input_device_info_equals(&device, text_of("mouse"), &device, text_of("mouse"), &equal) ==
                CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE &&
            cna_input_device_info_equals(&device, text_of("mouse"), &other_device, text_of("mouse"), &equal) ==
                CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE &&
            cna_input_device_info_equals(&device, text_of("mouse"), &device, text_of("keyboard"), &equal) ==
                CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "input-device equality observes a full-width identifier and the UTF-8 name");
}

static void enumerate_devices(CNA_Handle game, CnaDemo* demo, const char* kind,
                              CountRoute count_route, InfoRoute info_route,
                              NameSizeRoute size_route, CopyNameRoute copy_route)
{
    CNA_InputDeviceInfo info;
    char label[192];
    uint32_t count = UINT32_C(99);
    int entries_ok = 1;
    uint32_t index;

    snprintf(label, sizeof(label), "the host %s count is available and zero is an ordinary result", kind);
    cna_demo_check(demo, count_route(game, &count) == CNA_RESULT_SUCCESS, label);
    printf("  %s count: %u\n", kind, (unsigned)count);
    for (index = UINT32_C(0); index < count; ++index) {
        char name[256];
        uint64_t bytes = UINT64_C(99);
        memset(name, 0, sizeof(name));
        entries_ok = entries_ok && info_route(game, index, &info) == CNA_RESULT_SUCCESS &&
                     info.struct_size == (uint32_t)sizeof(info) && info.struct_version == UINT32_C(1) &&
                     size_route(game, index, &bytes) == CNA_RESULT_SUCCESS && bytes < sizeof(name) &&
                     copy_route(game, index, name, sizeof(name), &bytes) == CNA_RESULT_SUCCESS &&
                     strlen(name) == (size_t)bytes;
        if (entries_ok) {
            printf("    #%llu %s\n", (unsigned long long)info.id, name);
        }
    }
    snprintf(label, sizeof(label),
             "every reported %s has a versioned descriptor and an exact copied name", kind);
    cna_demo_check(demo, entries_ok, label);
    snprintf(label, sizeof(label),
             "the index equal to the %s count is refused by info and both name routes", kind);
    cna_demo_check(
        demo,
        info_route(game, count, &info) == CNA_RESULT_INVALID_ARGUMENT &&
            size_route(game, count, &(uint64_t){UINT64_C(99)}) == CNA_RESULT_INVALID_ARGUMENT &&
            copy_route(game, count, NULL, 0U, &(uint64_t){UINT64_C(99)}) == CNA_RESULT_INVALID_ARGUMENT,
        label);
}

static void sensor_queries(CNA_Handle game, CnaDemo* demo)
{
    CNA_SensorInfo info;
    CNA_Vector3 acceleration = {1.0f, 2.0f, 3.0f};
    CNA_Vector3 angular_velocity = {-1.0f, -2.0f, -3.0f};
    CNA_Bool acceleration_available = UINT8_C(9);
    CNA_Bool angular_velocity_available = UINT8_C(9);
    uint32_t count = UINT32_C(99);
    int entries_ok = 1;
    uint32_t index;

    cna_demo_check(demo, cna_sensors_get_count(game, &count) == CNA_RESULT_SUCCESS,
                   "the host sensor count is available and zero is an ordinary result");
    printf("  sensors: %u\n", (unsigned)count);
    for (index = UINT32_C(0); index < count; ++index) {
        char name[256];
        uint64_t bytes = UINT64_C(99);
        memset(name, 0, sizeof(name));
        entries_ok = entries_ok && cna_sensors_get_info_at(game, index, &info) == CNA_RESULT_SUCCESS &&
                     info.struct_size == (uint32_t)sizeof(info) && info.struct_version == UINT32_C(1) &&
                     info.type <= CNA_SENSOR_TYPE_MAXIMUM &&
                     cna_sensors_get_name_size_at(game, index, &bytes) == CNA_RESULT_SUCCESS &&
                     bytes < sizeof(name) &&
                     cna_sensors_copy_name_at(game, index, name, sizeof(name), &bytes) == CNA_RESULT_SUCCESS &&
                     strlen(name) == (size_t)bytes;
        if (entries_ok) {
            printf("    #%u %s, type %u\n", (unsigned)info.id, name, (unsigned)info.type);
        }
    }
    cna_demo_check(demo, entries_ok,
                   "every reported sensor has a versioned descriptor, known kind and exact copied name");
    cna_demo_check(
        demo,
        cna_sensors_get_info_at(game, count, &info) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_sensors_get_name_size_at(game, count, &(uint64_t){UINT64_C(99)}) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_sensors_copy_name_at(game, count, NULL, 0U, &(uint64_t){UINT64_C(99)}) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "the index equal to the sensor count is refused by info and both name routes");
    cna_demo_check(
        demo,
        cna_sensors_get_accelerometer(game, &acceleration, &acceleration_available) == CNA_RESULT_SUCCESS &&
            ((acceleration_available == CNA_FALSE && same_vector3(&acceleration, 1.0f, 2.0f, 3.0f)) ||
             (acceleration_available == CNA_TRUE && isfinite(acceleration.x) && isfinite(acceleration.y) &&
              isfinite(acceleration.z))) &&
            cna_sensors_get_gyroscope(game, &angular_velocity, &angular_velocity_available) == CNA_RESULT_SUCCESS &&
            ((angular_velocity_available == CNA_FALSE &&
              same_vector3(&angular_velocity, -1.0f, -2.0f, -3.0f)) ||
             (angular_velocity_available == CNA_TRUE && isfinite(angular_velocity.x) &&
              isfinite(angular_velocity.y) && isfinite(angular_velocity.z))),
        "host motion reads return finite values or preserve their outputs when unavailable");
}

static void on_mouse_connected(uint32_t id, void* context)
{
    HotplugState* state = (HotplugState*)context;
    state->mouse_connected_id = id;
    ++state->mouse_connected_calls;
}

static void on_mouse_disconnected(uint32_t id, void* context)
{
    HotplugState* state = (HotplugState*)context;
    state->mouse_disconnected_id = id;
    ++state->mouse_disconnected_calls;
}

static void on_keyboard_connected(uint32_t id, void* context)
{
    HotplugState* state = (HotplugState*)context;
    state->keyboard_connected_id = id;
    ++state->keyboard_connected_calls;
}

static void on_keyboard_disconnected(uint32_t id, void* context)
{
    HotplugState* state = (HotplugState*)context;
    state->keyboard_disconnected_id = id;
    ++state->keyboard_disconnected_calls;
}

static void hotplug_events(CNA_Handle game, CnaDemo* demo)
{
    HotplugState state;
    CNA_Handle mouse_connected = CNA_INVALID_HANDLE;
    CNA_Handle mouse_disconnected = CNA_INVALID_HANDLE;
    CNA_Handle keyboard_connected = CNA_INVALID_HANDLE;
    CNA_Handle keyboard_disconnected = CNA_INVALID_HANDLE;

    memset(&state, 0, sizeof(state));
    cna_demo_check_result(demo, cna_input_devices_reset_for_tests_ext(game),
                          "reset the process-wide host-device events");
    cna_demo_check(
        demo,
        cna_input_devices_subscribe_mouse_connected_ext(on_mouse_connected, &state, &mouse_connected) ==
                CNA_RESULT_SUCCESS &&
            cna_input_devices_subscribe_mouse_disconnected_ext(on_mouse_disconnected, &state,
                                                                &mouse_disconnected) == CNA_RESULT_SUCCESS &&
            cna_input_devices_subscribe_keyboard_connected_ext(on_keyboard_connected, &state,
                                                                &keyboard_connected) == CNA_RESULT_SUCCESS &&
            cna_input_devices_subscribe_keyboard_disconnected_ext(on_keyboard_disconnected, &state,
                                                                   &keyboard_disconnected) == CNA_RESULT_SUCCESS &&
            mouse_connected != CNA_INVALID_HANDLE && mouse_disconnected != CNA_INVALID_HANDLE &&
            keyboard_connected != CNA_INVALID_HANDLE && keyboard_disconnected != CNA_INVALID_HANDLE,
        "all four mouse and keyboard hot-plug subscriptions return owned handles");
    cna_demo_check(
        demo,
        cna_input_devices_raise_mouse_connected_ext(game, UINT32_C(21)) == CNA_RESULT_SUCCESS &&
            cna_input_devices_raise_mouse_disconnected_ext(game, UINT32_C(22)) == CNA_RESULT_SUCCESS &&
            cna_input_devices_raise_keyboard_connected_ext(game, UINT32_C(23)) == CNA_RESULT_SUCCESS &&
            cna_input_devices_raise_keyboard_disconnected_ext(game, UINT32_C(24)) == CNA_RESULT_SUCCESS,
        "all four public hot-plug events can be raised without physical hardware");
    cna_demo_check(
        demo,
        state.mouse_connected_calls == 1 && state.mouse_connected_id == UINT32_C(21) &&
            state.mouse_disconnected_calls == 1 && state.mouse_disconnected_id == UINT32_C(22) &&
            state.keyboard_connected_calls == 1 && state.keyboard_connected_id == UINT32_C(23) &&
            state.keyboard_disconnected_calls == 1 && state.keyboard_disconnected_id == UINT32_C(24),
        "each synchronous event reaches only its handler with the raised id");
    cna_demo_check(
        demo,
        cna_input_devices_unsubscribe_ext(mouse_connected) == CNA_RESULT_SUCCESS &&
            cna_input_devices_raise_mouse_connected_ext(game, UINT32_C(25)) == CNA_RESULT_SUCCESS &&
            state.mouse_connected_calls == 1 &&
            cna_input_devices_raise_keyboard_connected_ext(game, UINT32_C(26)) == CNA_RESULT_SUCCESS &&
            state.keyboard_connected_calls == 2 && state.keyboard_connected_id == UINT32_C(26),
        "unsubscribing one registration leaves the other event handlers attached");
    cna_demo_check(
        demo,
        cna_input_devices_reset_for_tests_ext(game) == CNA_RESULT_SUCCESS &&
            cna_input_devices_raise_mouse_disconnected_ext(game, UINT32_C(27)) == CNA_RESULT_SUCCESS &&
            cna_input_devices_raise_keyboard_disconnected_ext(game, UINT32_C(28)) == CNA_RESULT_SUCCESS &&
            state.mouse_disconnected_calls == 1 && state.keyboard_disconnected_calls == 1,
        "reset detaches every remaining host-device handler before another raise returns");
    cna_demo_check(
        demo,
        cna_input_devices_unsubscribe_ext(mouse_disconnected) == CNA_RESULT_SUCCESS &&
            cna_input_devices_unsubscribe_ext(keyboard_connected) == CNA_RESULT_SUCCESS &&
            cna_input_devices_unsubscribe_ext(keyboard_disconnected) == CNA_RESULT_SUCCESS,
        "registrations release safely after reset has already cleared their events");
}

static void power_snapshot(CNA_Handle game, CnaDemo* demo)
{
    CNA_PowerState state = UINT32_C(99);
    int32_t seconds_left = INT32_C(12345);
    int32_t percent = INT32_C(12345);

    cna_demo_check(
        demo,
        cna_power_get_info(game, &state, &seconds_left, &percent) == CNA_RESULT_SUCCESS &&
            state <= CNA_POWER_STATE_CHARGED && seconds_left >= -1 && percent >= -1 && percent <= 100 &&
            (state != CNA_POWER_STATE_NO_BATTERY || (seconds_left == -1 && percent == -1)),
        "one power snapshot writes a known state and charge/time values or -1 for unknown");
    printf("  power: state %u, percent %d, seconds %d\n", (unsigned)state, (int)percent,
           (int)seconds_left);
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    DeviceDemoState* state = (DeviceDemoState*)context;
    (void)game_time;
    (void)out_error;
    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    enumerate_devices(game, state->demo, "mouse",
                      cna_input_devices_get_mouse_count, cna_input_devices_get_mouse_info_at,
                      cna_input_devices_get_mouse_name_size_at, cna_input_devices_copy_mouse_name_at);
    enumerate_devices(game, state->demo, "keyboard",
                      cna_input_devices_get_keyboard_count, cna_input_devices_get_keyboard_info_at,
                      cna_input_devices_get_keyboard_name_size_at, cna_input_devices_copy_keyboard_name_at);
    enumerate_devices(game, state->demo, "touch device",
                      cna_input_devices_get_touch_device_count, cna_input_devices_get_touch_device_info_at,
                      cna_input_devices_get_touch_device_name_size_at,
                      cna_input_devices_copy_touch_device_name_at);
    sensor_queries(game, state->demo);
    hotplug_events(game, state->demo);
    power_snapshot(game, state->demo);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    DeviceDemoState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.input.devices", argc, argv, 0);
    descriptor_values(&demo);
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
    cna_demo_check_result(&demo, cna_game_run_one_frame(game), "run one frame for host-device queries");
    cna_demo_check(&demo, state.validated == 1, "the update callback completed the host-device audit once");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
