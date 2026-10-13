/* SPDX-License-Identifier: MS-PL */
/*
 * Raw joystick input from C: compare the POD descriptors, enumerate any host devices, then make
 * the disconnected capability/state and public hot-plug contracts observable without hardware.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct HotplugState {
    uint32_t connected_id;
    uint32_t disconnected_id;
    int connected_calls;
    int disconnected_calls;
} HotplugState;

typedef struct JoystickDemoState {
    CnaDemo* demo;
    int validated;
} JoystickDemoState;

static CNA_StringView text_of(const char* value)
{
    CNA_StringView view;
    view.data = value;
    view.byte_length = (uint64_t)strlen(value);
    return view;
}

static CNA_StringView empty_text(void)
{
    CNA_StringView view;
    view.data = NULL;
    view.byte_length = UINT64_C(0);
    return view;
}

static int capabilities_are_default(const CNA_JoystickCapabilities* capabilities)
{
    return capabilities->struct_size == (uint32_t)sizeof(*capabilities) &&
           capabilities->struct_version == UINT32_C(1) &&
           capabilities->axis_count == 0 && capabilities->button_count == 0 &&
           capabilities->hat_count == 0 && capabilities->ball_count == 0 &&
           capabilities->type == CNA_JOYSTICK_TYPE_UNKNOWN &&
           capabilities->power_state == CNA_POWER_STATE_UNKNOWN &&
           capabilities->power_percent == -1 && capabilities->is_connected == CNA_FALSE &&
           capabilities->reserved[0] == UINT8_C(0) &&
           capabilities->reserved[1] == UINT8_C(0) &&
           capabilities->reserved[2] == UINT8_C(0);
}

static void descriptor_values(CnaDemo* demo)
{
    CNA_JoystickInfo info;
    CNA_JoystickInfo other_info;
    CNA_JoystickCapabilities capabilities;
    CNA_JoystickCapabilities other_capabilities;
    CNA_Bool equal = UINT8_C(9);
    CNA_Result same_result;
    CNA_Result id_result;
    CNA_Result type_result;
    CNA_Result name_result;
    CNA_Result field_result;
    CNA_Result guid_result;
    CNA_Bool same_equal;
    CNA_Bool id_equal;
    CNA_Bool type_equal;
    CNA_Bool name_equal;
    CNA_Bool field_equal;
    CNA_Bool guid_equal;

    memset(&info, 9, sizeof(info));
    cna_demo_check(
        demo,
        cna_joystick_info_init(&info) == CNA_RESULT_SUCCESS &&
            info.struct_size == (uint32_t)sizeof(info) && info.struct_version == UINT32_C(1) &&
            info.id == UINT32_C(0) && info.type == CNA_JOYSTICK_TYPE_UNKNOWN,
        "the default joystick descriptor is versioned with unknown type and id zero");
    other_info = info;
    same_result = cna_joystick_info_equals(
        &info, text_of("wheel"), &other_info, text_of("wheel"), &equal);
    same_equal = equal;
    other_info.id = UINT32_C(7);
    id_result = cna_joystick_info_equals(
        &info, text_of("wheel"), &other_info, text_of("wheel"), &equal);
    id_equal = equal;
    other_info = info;
    other_info.type = CNA_JOYSTICK_TYPE_WHEEL;
    type_result = cna_joystick_info_equals(
        &info, text_of("wheel"), &other_info, text_of("wheel"), &equal);
    type_equal = equal;
    name_result = cna_joystick_info_equals(
        &info, text_of("wheel"), &info, text_of("stick"), &equal);
    name_equal = equal;
    cna_demo_check(
        demo,
        same_result == CNA_RESULT_SUCCESS && id_result == CNA_RESULT_SUCCESS &&
            type_result == CNA_RESULT_SUCCESS && name_result == CNA_RESULT_SUCCESS &&
            same_equal == CNA_TRUE && id_equal == CNA_FALSE && type_equal == CNA_FALSE &&
            name_equal == CNA_FALSE,
        "joystick descriptor equality observes id, physical type and UTF-8 name");

    memset(&capabilities, 9, sizeof(capabilities));
    cna_demo_check(
        demo,
        cna_joystick_capabilities_init(&capabilities) == CNA_RESULT_SUCCESS &&
            capabilities_are_default(&capabilities),
        "default joystick capabilities describe a disconnected device with unknown charge");
    other_capabilities = capabilities;
    same_result = cna_joystick_capabilities_equals(
        &capabilities, text_of("stick"), text_of("03000000"),
        &other_capabilities, text_of("stick"), text_of("03000000"), &equal);
    same_equal = equal;
    other_capabilities.axis_count = 2;
    field_result = cna_joystick_capabilities_equals(
        &capabilities, empty_text(), empty_text(), &other_capabilities,
        empty_text(), empty_text(), &equal);
    field_equal = equal;
    name_result = cna_joystick_capabilities_equals(
        &capabilities, text_of("stick"), text_of("03000000"),
        &capabilities, text_of("wheel"), text_of("03000000"), &equal);
    name_equal = equal;
    guid_result = cna_joystick_capabilities_equals(
        &capabilities, text_of("stick"), text_of("03000000"),
        &capabilities, text_of("stick"), text_of("03000001"), &equal);
    guid_equal = equal;
    cna_demo_check(
        demo,
        same_result == CNA_RESULT_SUCCESS && field_result == CNA_RESULT_SUCCESS &&
            name_result == CNA_RESULT_SUCCESS && guid_result == CNA_RESULT_SUCCESS &&
            same_equal == CNA_TRUE && field_equal == CNA_FALSE && name_equal == CNA_FALSE &&
            guid_equal == CNA_FALSE,
        "capability equality observes the hardware fields, name and GUID text");
}

static void enumerate_joysticks(CNA_Handle game, CnaDemo* demo)
{
    uint32_t count = UINT32_C(99);
    uint32_t index;
    int entries_ok = 1;
    CNA_JoystickInfo info;
    char name[256];
    uint64_t bytes = UINT64_C(99);

    if (!cna_demo_check(demo, cna_joysticks_get_count(game, &count) == CNA_RESULT_SUCCESS,
                        "the host joystick count is available and zero is an ordinary result")) {
        return;
    }
    printf("  joysticks: %u\n", (unsigned)count);
    for (index = UINT32_C(0); index < count; ++index) {
        CNA_JoystickCapabilities capabilities;
        CNA_JoystickStateHandle state = CNA_INVALID_HANDLE;
        memset(name, 0, sizeof(name));
        entries_ok = entries_ok &&
                     cna_joysticks_get_info_at(game, index, &info) == CNA_RESULT_SUCCESS &&
                     info.struct_size == (uint32_t)sizeof(info) &&
                     info.struct_version == UINT32_C(1) && info.type <= CNA_JOYSTICK_TYPE_MAXIMUM &&
                     cna_joysticks_get_name_size_at(game, index, &bytes) == CNA_RESULT_SUCCESS &&
                     bytes < (uint64_t)sizeof(name) &&
                     cna_joysticks_copy_name_at(game, index, name, sizeof(name), &bytes) ==
                         CNA_RESULT_SUCCESS &&
                     strlen(name) == (size_t)bytes &&
                     cna_joysticks_get_capabilities(game, info.id, &capabilities) ==
                         CNA_RESULT_SUCCESS &&
                     capabilities.is_connected == CNA_TRUE && capabilities.type == info.type &&
                     cna_joysticks_capture_state(game, info.id, &state) == CNA_RESULT_SUCCESS &&
                     state != CNA_INVALID_HANDLE &&
                     cna_joystick_state_destroy(state) == CNA_RESULT_SUCCESS;
        if (entries_ok) {
            printf("    #%u %s, type %u\n", (unsigned)info.id, name, (unsigned)info.type);
        }
    }
    cna_demo_check(demo, entries_ok,
                   "every reported joystick has a versioned descriptor, exact name and live capability/state");
    cna_demo_check(
        demo,
        cna_joysticks_get_info_at(game, count, &info) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_joysticks_get_name_size_at(game, count, &bytes) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_joysticks_copy_name_at(game, count, name, sizeof(name), &bytes) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "the index equal to the joystick count is refused by info and both name routes");
}

static void disconnected_joystick(CNA_Handle game, CnaDemo* demo)
{
    const uint32_t first_absent = UINT32_C(0xFFFFFFFF);
    const uint32_t second_absent = UINT32_C(0xFFFFFFFE);
    CNA_JoystickCapabilities capabilities;
    CNA_JoystickStateHandle first = CNA_INVALID_HANDLE;
    CNA_JoystickStateHandle second = CNA_INVALID_HANDLE;
    CNA_Bool equal = UINT8_C(9);
    uint32_t axes = UINT32_C(99);
    uint32_t buttons = UINT32_C(99);
    uint32_t hats = UINT32_C(99);
    uint32_t balls = UINT32_C(99);
    uint64_t bytes = UINT64_C(99);
    uint64_t copied = UINT64_C(99);
    unsigned char name_guard[8];
    unsigned char guid_guard[8];
    int16_t axis_guard = INT16_C(1234);
    CNA_Bool button_guard = UINT8_C(9);
    CNA_JoystickHatPosition hat_guard = UINT32_C(99);
    CNA_Point ball_guard = {123, 456};

    cna_demo_check(
        demo,
        cna_joysticks_get_capabilities(game, first_absent, &capabilities) == CNA_RESULT_SUCCESS &&
            capabilities_are_default(&capabilities),
        "an absent joystick id returns the canonical disconnected capabilities");
    memset(name_guard, 0x7f, sizeof(name_guard));
    memset(guid_guard, 0x7f, sizeof(guid_guard));
    cna_demo_check(
        demo,
        cna_joysticks_get_capabilities_name_size(game, first_absent, &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(0) &&
            cna_joysticks_copy_capabilities_name(
                game, first_absent, (char*)name_guard, sizeof(name_guard), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == UINT64_C(0) && name_guard[0] == 0x7fU &&
            cna_joysticks_get_capabilities_guid_size(game, first_absent, &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(0) &&
            cna_joysticks_copy_capabilities_guid(
                game, first_absent, (char*)guid_guard, sizeof(guid_guard), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == UINT64_C(0) && guid_guard[0] == 0x7fU,
        "an absent joystick has empty name and GUID size/copy results without writes");
    cna_demo_check(
        demo,
        cna_joysticks_capture_state(game, first_absent, &first) == CNA_RESULT_SUCCESS &&
            first != CNA_INVALID_HANDLE &&
            cna_joysticks_capture_state(game, second_absent, &second) == CNA_RESULT_SUCCESS &&
            second != CNA_INVALID_HANDLE,
        "two absent joystick ids still produce independent owned state snapshots");
    cna_demo_check(
        demo,
        cna_joystick_state_get_axis_count(first, &axes) == CNA_RESULT_SUCCESS && axes == 0U &&
            cna_joystick_state_get_button_count(first, &buttons) == CNA_RESULT_SUCCESS && buttons == 0U &&
            cna_joystick_state_get_hat_count(first, &hats) == CNA_RESULT_SUCCESS && hats == 0U &&
            cna_joystick_state_get_ball_count(first, &balls) == CNA_RESULT_SUCCESS && balls == 0U,
        "a disconnected snapshot has no axes, buttons, hats or trackballs");
    cna_demo_check(
        demo,
        cna_joystick_state_copy_axes(first, &axis_guard, UINT64_C(1), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == UINT64_C(0) && axis_guard == INT16_C(1234) &&
            cna_joystick_state_copy_buttons(first, &button_guard, UINT64_C(1), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == UINT64_C(0) && button_guard == UINT8_C(9) &&
            cna_joystick_state_copy_hats(first, &hat_guard, UINT64_C(1), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == UINT64_C(0) && hat_guard == UINT32_C(99) &&
            cna_joystick_state_copy_balls(first, &ball_guard, UINT64_C(1), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == UINT64_C(0) && ball_guard.x == 123 && ball_guard.y == 456,
        "all four empty state copies report zero and preserve their destinations");
    cna_demo_check(
        demo,
        cna_joystick_state_equals(first, second, &equal) == CNA_RESULT_SUCCESS && equal == CNA_TRUE,
        "disconnected snapshots compare equal by their four empty arrays");
    cna_demo_check(
        demo,
        cna_joystick_state_destroy(first) == CNA_RESULT_SUCCESS &&
            cna_joystick_state_destroy(second) == CNA_RESULT_SUCCESS,
        "both independently owned joystick snapshots release successfully");
}

static void on_connected(uint32_t id, void* context)
{
    HotplugState* state = (HotplugState*)context;
    state->connected_id = id;
    ++state->connected_calls;
}

static void on_disconnected(uint32_t id, void* context)
{
    HotplugState* state = (HotplugState*)context;
    state->disconnected_id = id;
    ++state->disconnected_calls;
}

static void hotplug_events(CNA_Handle game, CnaDemo* demo)
{
    HotplugState state;
    CNA_JoystickEventRegistrationHandle connected = CNA_INVALID_HANDLE;
    CNA_JoystickEventRegistrationHandle disconnected = CNA_INVALID_HANDLE;

    memset(&state, 0, sizeof(state));
    cna_demo_check(
        demo,
        cna_joysticks_subscribe_connected_ext(on_connected, &state, &connected) ==
                CNA_RESULT_SUCCESS &&
            cna_joysticks_subscribe_disconnected_ext(on_disconnected, &state, &disconnected) ==
                CNA_RESULT_SUCCESS &&
            connected != CNA_INVALID_HANDLE && disconnected != CNA_INVALID_HANDLE,
        "both process-wide joystick hot-plug subscriptions return owned handles");
    cna_demo_check(
        demo,
        cna_joysticks_raise_connected_ext(game, UINT32_C(21)) == CNA_RESULT_SUCCESS &&
            state.connected_calls == 1 && state.connected_id == UINT32_C(21) &&
            state.disconnected_calls == 0 &&
            cna_joysticks_raise_disconnected_ext(game, UINT32_C(22)) == CNA_RESULT_SUCCESS &&
            state.disconnected_calls == 1 && state.disconnected_id == UINT32_C(22) &&
            state.connected_calls == 1,
        "each synchronous hot-plug raise reaches only its handler with the exact id");
    cna_demo_check(
        demo,
        cna_joysticks_unsubscribe_ext(connected) == CNA_RESULT_SUCCESS &&
            cna_joysticks_raise_connected_ext(game, UINT32_C(23)) == CNA_RESULT_SUCCESS &&
            state.connected_calls == 1 &&
            cna_joysticks_raise_disconnected_ext(game, UINT32_C(24)) == CNA_RESULT_SUCCESS &&
            state.disconnected_calls == 2 && state.disconnected_id == UINT32_C(24),
        "unsubscribing connected events leaves the disconnected handler attached");
    cna_demo_check(
        demo,
        cna_joysticks_reset_for_tests_ext(game) == CNA_RESULT_SUCCESS &&
            cna_joysticks_raise_disconnected_ext(game, UINT32_C(25)) == CNA_RESULT_SUCCESS &&
            state.disconnected_calls == 2,
        "reset detaches the remaining joystick handler before another raise returns");
    cna_demo_check(
        demo,
        cna_joysticks_unsubscribe_ext(disconnected) == CNA_RESULT_SUCCESS,
        "the disconnected registration releases safely after reset cleared its event");
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    JoystickDemoState* state = (JoystickDemoState*)context;
    (void)game_time;
    (void)out_error;
    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    enumerate_joysticks(game, state->demo);
    disconnected_joystick(game, state->demo);
    hotplug_events(game, state->demo);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    JoystickDemoState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.input.joystick", argc, argv, 0);
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
    cna_demo_check_result(&demo, cna_game_run_one_frame(game), "run one frame for raw joystick access");
    cna_demo_check(&demo, state.validated == 1, "the update callback completed the joystick audit once");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
