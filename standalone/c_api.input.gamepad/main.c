/* SPDX-License-Identifier: MS-PL */
/*
 * Gamepads from C: CNA_GamePadState is a plain struct whose single button mask carries the
 * physical buttons, the D-pad and the virtual stick and trigger buttons XNA derives. The program
 * builds a snapshot from values and queries it, applies the dead-zone rules to raw values, then
 * polls player one every update and draws the sticks, triggers and face buttons (grey while no
 * controller is connected). Back on the controller quits.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct PadState {
    CnaDemo* demo;
    CNA_Handle white;        /* owned 1x1 texture */
    CNA_Handle sprite_batch; /* owned */
    CNA_GamePadState pad;
    int frames;
    int polls_failed;
    int agreement_checked;
} PadState;

static void blank_state(CNA_GamePadState* state)
{
    memset(state, 0, sizeof(*state));
    state->struct_size = (uint32_t)sizeof(*state);
    state->struct_version = UINT32_C(1);
}

static int has(CNA_GamePadButtonFlags mask, CNA_GamePadButtonFlags bits)
{
    return (mask & bits) == bits;
}

static int down(const CNA_GamePadState* state, CNA_GamePadButtonFlags buttons)
{
    CNA_Bool is_down = CNA_FALSE;
    return cna_gamepad_state_is_button_down(state, buttons, &is_down) == CNA_RESULT_SUCCESS && is_down == CNA_TRUE;
}

static float next_float_above(float value)
{
    uint32_t bits = 0U;
    memcpy(&bits, &value, sizeof(bits));
    ++bits;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static int capabilities_are_disconnected(const CNA_GamePadCapabilities* capabilities)
{
    return capabilities->gamepad_type == CNA_GAMEPAD_TYPE_UNKNOWN && capabilities->is_connected == CNA_FALSE &&
           capabilities->has_a_button == CNA_FALSE && capabilities->has_b_button == CNA_FALSE &&
           capabilities->has_x_button == CNA_FALSE && capabilities->has_y_button == CNA_FALSE &&
           capabilities->has_back_button == CNA_FALSE && capabilities->has_start_button == CNA_FALSE &&
           capabilities->has_big_button == CNA_FALSE && capabilities->has_dpad_up_button == CNA_FALSE &&
           capabilities->has_dpad_down_button == CNA_FALSE && capabilities->has_dpad_left_button == CNA_FALSE &&
           capabilities->has_dpad_right_button == CNA_FALSE &&
           capabilities->has_left_shoulder_button == CNA_FALSE &&
           capabilities->has_right_shoulder_button == CNA_FALSE &&
           capabilities->has_left_stick_button == CNA_FALSE &&
           capabilities->has_right_stick_button == CNA_FALSE &&
           capabilities->has_left_x_thumb_stick == CNA_FALSE &&
           capabilities->has_left_y_thumb_stick == CNA_FALSE &&
           capabilities->has_right_x_thumb_stick == CNA_FALSE &&
           capabilities->has_right_y_thumb_stick == CNA_FALSE &&
           capabilities->has_left_trigger == CNA_FALSE && capabilities->has_right_trigger == CNA_FALSE &&
           capabilities->has_left_vibration_motor == CNA_FALSE &&
           capabilities->has_right_vibration_motor == CNA_FALSE &&
           capabilities->has_voice_support == CNA_FALSE && capabilities->has_light_bar_ext == CNA_FALSE &&
           capabilities->has_trigger_vibration_motors_ext == CNA_FALSE &&
           capabilities->has_misc1_ext == CNA_FALSE && capabilities->has_paddle1_ext == CNA_FALSE &&
           capabilities->has_paddle2_ext == CNA_FALSE && capabilities->has_paddle3_ext == CNA_FALSE &&
           capabilities->has_paddle4_ext == CNA_FALSE && capabilities->has_touchpad_ext == CNA_FALSE &&
           capabilities->has_gyro_ext == CNA_FALSE && capabilities->has_accelerometer_ext == CNA_FALSE &&
           capabilities->reserved[0] == UINT8_C(0);
}

static void button_values(CnaDemo* demo)
{
    static const CNA_GamePadButtonFlags array[3] = {
        CNA_GAMEPAD_BUTTON_A, CNA_GAMEPAD_BUTTON_B, CNA_GAMEPAD_BUTTON_A};
    const CNA_GamePadButtonFlags expected = CNA_GAMEPAD_BUTTON_A | CNA_GAMEPAD_BUTTON_B;
    CNA_GamePadButtonFlags empty = UINT32_MAX;
    CNA_GamePadButtonFlags from_mask = 0U;
    CNA_GamePadButtonFlags from_array = 0U;
    CNA_Bool pressed = CNA_FALSE;
    CNA_Bool equal = CNA_FALSE;
    CNA_Bool different = CNA_FALSE;
    int32_t hash = 0;

    cna_demo_check(
        demo,
        cna_gamepad_buttons_init(&empty) == CNA_RESULT_SUCCESS && empty == CNA_GAMEPAD_BUTTON_NONE &&
            cna_gamepad_buttons_init_from_mask(expected, &from_mask) == CNA_RESULT_SUCCESS &&
            from_mask == expected &&
            cna_gamepad_buttons_init_from_button_array(array, 3U, &from_array) == CNA_RESULT_SUCCESS &&
            from_array == expected,
        "button values build from empty, a mask or an array whose duplicate contributes once");
    cna_demo_check(
        demo,
        cna_gamepad_buttons_is_pressed(from_array, CNA_GAMEPAD_BUTTON_A, &pressed) == CNA_RESULT_SUCCESS &&
            pressed == CNA_TRUE &&
            cna_gamepad_buttons_equals(from_mask, from_array, &equal) == CNA_RESULT_SUCCESS && equal == CNA_TRUE &&
            cna_gamepad_buttons_not_equals(empty, from_array, &different) == CNA_RESULT_SUCCESS &&
            different == CNA_TRUE,
        "button queries, equality and inequality observe the combined mask");
    cna_demo_check(
        demo,
        cna_gamepad_buttons_get_hash_code(from_array, &hash) == CNA_RESULT_SUCCESS && hash == (int32_t)expected,
        "the canonical button hash is the mask reinterpreted as a signed value");
}

static void dpad_values(CnaDemo* demo)
{
    static const CNA_GamePadButtonFlags mixed[3] = {
        CNA_GAMEPAD_BUTTON_A, CNA_GAMEPAD_BUTTON_DPAD_LEFT, CNA_GAMEPAD_BUTTON_START};
    CNA_GamePadButtonFlags empty = UINT32_MAX;
    CNA_GamePadButtonFlags up_left = 0U;
    CNA_GamePadButtonFlags array_left = 0U;
    CNA_Bool pressed = CNA_FALSE;
    CNA_Bool equal = CNA_FALSE;
    CNA_Bool different = CNA_FALSE;
    int32_t hash = 0;

    cna_demo_check(
        demo,
        cna_gamepad_dpad_init(&empty) == CNA_RESULT_SUCCESS && empty == CNA_GAMEPAD_BUTTON_NONE &&
            cna_gamepad_dpad_init_from_states(CNA_TRUE, CNA_FALSE, CNA_TRUE, CNA_FALSE, &up_left) ==
                CNA_RESULT_SUCCESS &&
            up_left == (CNA_GAMEPAD_BUTTON_DPAD_UP | CNA_GAMEPAD_BUTTON_DPAD_LEFT) &&
            cna_gamepad_dpad_init_from_button_array(mixed, 3U, &array_left) == CNA_RESULT_SUCCESS &&
            array_left == CNA_GAMEPAD_BUTTON_DPAD_LEFT,
        "D-pad values build from states or an array while ignoring non-directional identities");
    cna_demo_check(
        demo,
        cna_gamepad_dpad_is_pressed(up_left, CNA_GAMEPAD_BUTTON_DPAD_LEFT, &pressed) == CNA_RESULT_SUCCESS &&
            pressed == CNA_TRUE &&
            cna_gamepad_dpad_equals(up_left, up_left | CNA_GAMEPAD_BUTTON_A, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE &&
            cna_gamepad_dpad_not_equals(up_left, array_left, &different) == CNA_RESULT_SUCCESS &&
            different == CNA_TRUE,
        "D-pad queries and comparisons use only the four directional bits");
    cna_demo_check(
        demo,
        cna_gamepad_dpad_get_hash_code(up_left, &hash) == CNA_RESULT_SUCCESS && hash == 10,
        "the D-pad hash uses its canonical Down/Left/Right/Up weights rather than raw button bits");
}

static void analog_values(CnaDemo* demo)
{
    const CNA_Vector2 left = {0.25f, -0.5f};
    const CNA_Vector2 right = {-0.75f, 0.125f};
    const CNA_Vector2 outside = {2.0f, -2.0f};
    CNA_GamePadThumbSticks empty_sticks;
    CNA_GamePadThumbSticks sticks;
    CNA_GamePadThumbSticks clamped;
    CNA_GamePadThumbSticks other_sticks;
    CNA_GamePadTriggers empty_triggers;
    CNA_GamePadTriggers triggers;
    CNA_GamePadTriggers other_triggers;
    CNA_Bool equal = CNA_FALSE;
    CNA_Bool different = CNA_FALSE;
    int32_t hash = 0;
    int32_t other_hash = 1;

    cna_demo_check(
        demo,
        cna_gamepad_thumb_sticks_init(&empty_sticks) == CNA_RESULT_SUCCESS &&
            empty_sticks.left.x == 0.0f && empty_sticks.left.y == 0.0f &&
            empty_sticks.right.x == 0.0f && empty_sticks.right.y == 0.0f &&
            cna_gamepad_thumb_sticks_init_from_positions(&left, &right, &sticks) == CNA_RESULT_SUCCESS &&
            sticks.left.x == left.x && sticks.left.y == left.y &&
            sticks.right.x == right.x && sticks.right.y == right.y &&
            cna_gamepad_thumb_sticks_init_from_positions(&outside, &outside, &clamped) == CNA_RESULT_SUCCESS &&
            clamped.left.x == 1.0f && clamped.left.y == -1.0f &&
            clamped.right.x == 1.0f && clamped.right.y == -1.0f,
        "thumbstick construction preserves in-range positions and square-clamps each outside component");

    other_sticks = sticks;
    cna_demo_check(
        demo,
        cna_gamepad_thumb_sticks_equals(&sticks, &other_sticks, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE &&
            cna_gamepad_thumb_sticks_get_hash_code(&sticks, &hash) == CNA_RESULT_SUCCESS &&
            cna_gamepad_thumb_sticks_get_hash_code(&other_sticks, &other_hash) == CNA_RESULT_SUCCESS &&
            hash == other_hash,
        "equal thumbstick pairs compare equal and produce the same canonical hash");
    other_sticks.left.x = 0.5f;
    cna_demo_check(
        demo,
        cna_gamepad_thumb_sticks_not_equals(&sticks, &other_sticks, &different) == CNA_RESULT_SUCCESS &&
            different == CNA_TRUE,
        "thumbstick inequality observes one exactly changed component");

    cna_demo_check(
        demo,
        cna_gamepad_triggers_init(&empty_triggers) == CNA_RESULT_SUCCESS &&
            empty_triggers.left == 0.0f && empty_triggers.right == 0.0f &&
            cna_gamepad_triggers_init_from_positions(0.25f, 0.75f, &triggers) == CNA_RESULT_SUCCESS &&
            triggers.left == 0.25f && triggers.right == 0.75f &&
            cna_gamepad_triggers_init_from_positions(-1.0f, 2.0f, &other_triggers) == CNA_RESULT_SUCCESS &&
            other_triggers.left == 0.0f && other_triggers.right == 1.0f,
        "trigger construction starts at zero, preserves in-range positions and clamps to zero through one");

    other_triggers = triggers;
    cna_demo_check(
        demo,
        cna_gamepad_triggers_get_hash_code(&triggers, &hash) == CNA_RESULT_SUCCESS &&
            cna_gamepad_triggers_get_hash_code(&other_triggers, &other_hash) == CNA_RESULT_SUCCESS &&
            hash == other_hash,
        "exactly equal trigger pairs produce the same canonical hash");
    other_triggers.left = next_float_above(triggers.left);
    cna_demo_check(
        demo,
        other_triggers.left != triggers.left &&
            cna_gamepad_triggers_equals(&triggers, &other_triggers, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE,
        "trigger equality accepts the next representable float through its epsilon comparison");
    other_triggers.left = 0.5f;
    cna_demo_check(
        demo,
        cna_gamepad_triggers_not_equals(&triggers, &other_triggers, &different) == CNA_RESULT_SUCCESS &&
            different == CNA_TRUE,
        "trigger inequality detects a difference larger than the comparison epsilon");
}

static void state_values(CnaDemo* demo)
{
    static const CNA_GamePadButtonFlags held[2] = {CNA_GAMEPAD_BUTTON_A, CNA_GAMEPAD_BUTTON_DPAD_UP};
    const CNA_Vector2 left = {1.0f, 0.0f};
    const CNA_Vector2 right = {0.0f, -1.0f};
    CNA_GamePadState rest;
    CNA_GamePadState state;
    CNA_GamePadState component_state;
    CNA_GamePadThumbSticks sticks;
    CNA_GamePadTriggers triggers;
    CNA_GamePadButtonFlags buttons = 0U;
    CNA_GamePadButtonFlags dpad = 0U;
    CNA_Bool equal = CNA_FALSE;
    CNA_Bool different = CNA_FALSE;
    CNA_Bool is_up = CNA_FALSE;
    int32_t hash = 0;
    int32_t component_hash = 1;
    const CNA_GamePadButtonFlags derived = CNA_GAMEPAD_BUTTON_LEFT_THUMBSTICK_RIGHT |
                                           CNA_GAMEPAD_BUTTON_RIGHT_THUMBSTICK_DOWN | CNA_GAMEPAD_BUTTON_LEFT_TRIGGER;

    blank_state(&rest);
    if (cna_demo_check_result(demo, cna_gamepad_state_init(&rest), "cna_gamepad_state_init")) {
        cna_demo_check(demo, rest.is_connected == CNA_FALSE && rest.pressed_buttons == CNA_GAMEPAD_BUTTON_NONE,
                       "an initialised snapshot is disconnected with no buttons");
    }

    /* Left stick pushed right, right stick pulled down, left trigger half in, A and D-pad up. */
    blank_state(&state);
    if (!cna_demo_check_result(demo, cna_gamepad_state_init_from_values(&left, &right, 0.5f, 0.0f, held, 2U, &state),
                               "cna_gamepad_state_init_from_values")) {
        return;
    }
    printf("  built snapshot: connected %d, pressed_buttons 0x%08X\n", (int)state.is_connected,
           (unsigned)state.pressed_buttons);
    cna_demo_check(demo, state.is_connected == CNA_TRUE, "a snapshot built from values reports connected");
    cna_demo_check(demo, has(state.pressed_buttons, CNA_GAMEPAD_BUTTON_A | CNA_GAMEPAD_BUTTON_DPAD_UP),
                   "the one mask holds the physical A and the D-pad bit");
    cna_demo_check(demo, has(state.pressed_buttons, derived),
                   "and the derived bits: LeftThumbstickRight, RightThumbstickDown, LeftTrigger");
    cna_demo_check(demo, !has(state.pressed_buttons, CNA_GAMEPAD_BUTTON_RIGHT_TRIGGER),
                   "the right trigger at 0 is not past CNA_GAMEPAD_TRIGGER_THRESHOLD");
    if (cna_demo_check_result(demo, cna_gamepad_state_get_dpad(&state, &dpad), "cna_gamepad_state_get_dpad")) {
        cna_demo_check(demo, dpad == CNA_GAMEPAD_BUTTON_DPAD_UP, "the D-pad view is the mask's four D-pad bits");
    }
    cna_demo_check(demo, down(&state, CNA_GAMEPAD_BUTTON_A | CNA_GAMEPAD_BUTTON_LEFT_TRIGGER),
                   "is_button_down with several bits: true when all are down");
    cna_demo_check(demo, !down(&state, CNA_GAMEPAD_BUTTON_A | CNA_GAMEPAD_BUTTON_B), "false when one of them is up");
    cna_demo_check(demo,
                   cna_gamepad_state_is_button_up(&state, CNA_GAMEPAD_BUTTON_A | CNA_GAMEPAD_BUTTON_B, &is_up) ==
                           CNA_RESULT_SUCCESS &&
                       is_up == CNA_TRUE,
                   "is_button_up: true when at least one is up");

    blank_state(&component_state);
    cna_demo_check(
        demo,
        cna_gamepad_thumb_sticks_init_from_positions(&left, &right, &sticks) == CNA_RESULT_SUCCESS &&
            cna_gamepad_triggers_init_from_positions(0.5f, 0.0f, &triggers) == CNA_RESULT_SUCCESS &&
            cna_gamepad_state_init_from_components(&sticks, &triggers, CNA_GAMEPAD_BUTTON_A,
                                                   CNA_GAMEPAD_BUTTON_DPAD_UP, &component_state) ==
                CNA_RESULT_SUCCESS &&
            cna_gamepad_state_equals(&state, &component_state, &equal) == CNA_RESULT_SUCCESS && equal == CNA_TRUE,
        "the same controls built from component values produce an equal connected snapshot");
    cna_demo_check(
        demo,
        cna_gamepad_state_get_buttons(&state, &buttons) == CNA_RESULT_SUCCESS &&
            buttons == state.pressed_buttons &&
            cna_gamepad_state_get_thumb_sticks(&state, &sticks) == CNA_RESULT_SUCCESS &&
            sticks.left.x == 1.0f && sticks.left.y == 0.0f &&
            sticks.right.x == 0.0f && sticks.right.y == -1.0f &&
            cna_gamepad_state_get_triggers(&state, &triggers) == CNA_RESULT_SUCCESS &&
            triggers.left == 0.5f && triggers.right == 0.0f,
        "state component getters recover the complete button, thumbstick and trigger values");

    component_state = state;
    cna_demo_check(
        demo,
        cna_gamepad_state_set_packet_number_ext(&component_state, 7) == CNA_RESULT_SUCCESS &&
            component_state.packet_number == 7 &&
            cna_gamepad_state_not_equals(&state, &component_state, &different) == CNA_RESULT_SUCCESS &&
            different == CNA_TRUE,
        "the packet-number extension mutates the snapshot and participates in inequality");

    component_state = state;
    component_state.analog.left_trigger = 0.75f;
    cna_demo_check(
        demo,
        cna_gamepad_state_not_equals(&state, &component_state, &different) == CNA_RESULT_SUCCESS &&
            different == CNA_TRUE &&
            cna_gamepad_state_get_hash_code(&state, &hash) == CNA_RESULT_SUCCESS &&
            cna_gamepad_state_get_hash_code(&component_state, &component_hash) == CNA_RESULT_SUCCESS &&
            hash == component_hash,
        "the canonical state hash ignores an analog difference that state equality observes");
}

static void dead_zones(CnaDemo* demo)
{
    CNA_GamePadAnalogState raw;
    CNA_GamePadAnalogState none;
    CNA_GamePadAnalogState axes;
    CNA_GamePadAnalogState circular;
    float inside = 1.0f;
    float endpoint = 0.0f;
    float outside = 0.0f;
    const float expected = 0.01f / (1.0f - CNA_GAMEPAD_LEFT_DEAD_ZONE);

    /* A small diagonal push on the left stick: each axis alone is inside the dead zone
       (CNA_GAMEPAD_LEFT_DEAD_ZONE, about 0.24), their length is not. */
    memset(&raw, 0, sizeof(raw));
    raw.left_thumb_stick.x = 0.2f;
    raw.left_thumb_stick.y = 0.2f;
    if (!cna_demo_check_result(demo, cna_gamepad_apply_dead_zone(CNA_GAMEPAD_DEAD_ZONE_NONE, &raw, &none),
                               "cna_gamepad_apply_dead_zone(NONE)") ||
        !cna_demo_check_result(demo, cna_gamepad_apply_dead_zone(CNA_GAMEPAD_DEAD_ZONE_INDEPENDENT_AXES, &raw, &axes),
                               "cna_gamepad_apply_dead_zone(INDEPENDENT_AXES)") ||
        !cna_demo_check_result(demo, cna_gamepad_apply_dead_zone(CNA_GAMEPAD_DEAD_ZONE_CIRCULAR, &raw, &circular),
                               "cna_gamepad_apply_dead_zone(CIRCULAR)")) {
        return;
    }
    printf("  raw (0.2, 0.2): NONE (%.3f, %.3f), INDEPENDENT_AXES (%.3f, %.3f), CIRCULAR (%.3f, %.3f)\n",
           none.left_thumb_stick.x, none.left_thumb_stick.y, axes.left_thumb_stick.x, axes.left_thumb_stick.y,
           circular.left_thumb_stick.x, circular.left_thumb_stick.y);
    cna_demo_check(demo, none.left_thumb_stick.x == 0.2f && none.left_thumb_stick.y == 0.2f,
                   "NONE passes the raw values through");
    cna_demo_check(demo, axes.left_thumb_stick.x == 0.0f && axes.left_thumb_stick.y == 0.0f,
                   "INDEPENDENT_AXES (what cna_gamepad_get_state uses) zeroes each small axis");
    cna_demo_check(demo,
                   circular.left_thumb_stick.x > 0.0f && circular.left_thumb_stick.x == circular.left_thumb_stick.y,
                   "CIRCULAR measures the length and keeps the diagonal");
    cna_demo_check(
        demo,
        cna_gamepad_exclude_axis_dead_zone(CNA_GAMEPAD_LEFT_DEAD_ZONE * 0.9f,
                                           CNA_GAMEPAD_LEFT_DEAD_ZONE, &inside) == CNA_RESULT_SUCCESS &&
            inside == 0.0f &&
            cna_gamepad_exclude_axis_dead_zone(1.0f, CNA_GAMEPAD_LEFT_DEAD_ZONE, &endpoint) ==
                CNA_RESULT_SUCCESS && endpoint == 1.0f &&
            cna_gamepad_exclude_axis_dead_zone(CNA_GAMEPAD_LEFT_DEAD_ZONE + 0.01f,
                                               CNA_GAMEPAD_LEFT_DEAD_ZONE, &outside) == CNA_RESULT_SUCCESS &&
            outside > expected - 0.000001f && outside < expected + 0.000001f,
        "scalar dead-zone exclusion zeros an interior value and rescales the remaining range to one");
}

static int same_vector3(const CNA_Vector3* value, float x, float y, float z)
{
    return value->x == x && value->y == y && value->z == z;
}

static int same_touchpad_finger(const CNA_GamePadTouchpadFinger* value)
{
    return value->is_down == UINT8_C(9) && value->reserved[0] == UINT8_C(8) &&
           value->reserved[1] == UINT8_C(7) && value->reserved[2] == UINT8_C(6) &&
           value->x == 5.0f && value->y == 4.0f && value->pressure == 3.0f;
}

static void disconnected_device_contract(CNA_Handle game, CnaDemo* demo)
{
    typedef CNA_Result (*text_size_route)(CNA_Handle, CNA_PlayerIndex, uint64_t*);
    typedef CNA_Result (*text_copy_route)(CNA_Handle, CNA_PlayerIndex, char*, uint64_t, uint64_t*);
    static const text_size_route text_sizes[4] = {
        cna_gamepad_get_guid_size_ext,
        cna_gamepad_get_name_size_ext,
        cna_gamepad_get_path_size_ext,
        cna_gamepad_get_serial_size_ext,
    };
    static const text_copy_route text_copies[4] = {
        cna_gamepad_copy_guid_ext,
        cna_gamepad_copy_name_ext,
        cna_gamepad_copy_path_ext,
        cna_gamepad_copy_serial_ext,
    };
    CNA_PlayerIndex player = CNA_PLAYER_INDEX_ONE;
    CNA_GamePadState snapshot;
    CNA_Bool applied = UINT8_C(9);
    CNA_Bool available = UINT8_C(9);
    CNA_Vector3 reading = {1.0f, 2.0f, 3.0f};
    CNA_PowerState power = UINT32_C(99);
    CNA_GamePadButtonLabel label = UINT32_C(99);
    CNA_GamePadConnectionState connection = UINT32_C(99);
    CNA_GamePadTouchpadFinger finger = {UINT8_C(9), {UINT8_C(8), UINT8_C(7), UINT8_C(6)},
                                        5.0f, 4.0f, 3.0f};
    int32_t player_number = 99;
    int32_t percent = 99;
    int32_t touchpads = 99;
    int32_t fingers = 99;
    uint16_t firmware = UINT16_C(99);
    uint64_t steam_handle = UINT64_C(99);
    int found = 0;
    int text_ok = 1;
    size_t index;

    for (player = CNA_PLAYER_INDEX_ONE; player <= CNA_PLAYER_INDEX_FOUR; ++player) {
        blank_state(&snapshot);
        if (cna_gamepad_get_state(game, player, &snapshot) == CNA_RESULT_SUCCESS &&
            snapshot.is_connected == CNA_FALSE) {
            found = 1;
            break;
        }
    }
    if (!cna_demo_check(demo, found, "one player slot is disconnected for the empty-device contract")) {
        return;
    }

    cna_demo_check(
        demo,
        cna_gamepad_set_vibration(game, player, 0.5f, 0.75f, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            ((applied = UINT8_C(9)),
             cna_gamepad_set_trigger_vibration_ext(game, player, 0.25f, 1.0f, &applied) == CNA_RESULT_SUCCESS) &&
            applied == CNA_FALSE &&
            cna_gamepad_set_light_bar_ext(game, player, (CNA_Color){10U, 20U, 30U, 255U}) ==
                CNA_RESULT_SUCCESS,
        "a disconnected slot refuses both vibration kinds while its light-bar request is a successful no-op");

    cna_demo_check(
        demo,
        cna_gamepad_get_gyro_ext(game, player, &reading, &available) == CNA_RESULT_SUCCESS &&
            available == CNA_FALSE && same_vector3(&reading, 1.0f, 2.0f, 3.0f) &&
            ((available = UINT8_C(9)),
             cna_gamepad_get_accelerometer_ext(game, player, &reading, &available) == CNA_RESULT_SUCCESS) &&
            available == CNA_FALSE && same_vector3(&reading, 1.0f, 2.0f, 3.0f),
        "disconnected motion sensors report unavailable without changing either vector output");

    cna_demo_check(
        demo,
        cna_gamepad_get_player_index_ext(game, player, &player_number) == CNA_RESULT_SUCCESS &&
            player_number == -1 &&
            cna_gamepad_set_player_index_ext(game, player, 2, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE,
        "a disconnected controller has player index -1 and refuses an attempted index assignment");

    cna_demo_check(
        demo,
        cna_gamepad_get_power_info_ext(game, player, &power, &percent) == CNA_RESULT_SUCCESS &&
            power == CNA_POWER_STATE_ERROR && percent == -1 &&
            cna_gamepad_get_button_label_ext(game, player, CNA_GAMEPAD_BUTTON_A, &label) == CNA_RESULT_SUCCESS &&
            label == CNA_GAMEPAD_BUTTON_LABEL_UNKNOWN &&
            cna_gamepad_get_connection_state_ext(game, player, &connection) == CNA_RESULT_SUCCESS &&
            connection == CNA_GAMEPAD_CONNECTION_STATE_UNKNOWN,
        "disconnected power, face-button label and connection report Error/-1, Unknown and Unknown");

    for (index = 0U; index < 4U; ++index) {
        uint64_t size = UINT64_C(99);
        uint64_t copied = UINT64_C(99);
        text_ok = text_ok && text_sizes[index](game, player, &size) == CNA_RESULT_SUCCESS && size == 0U &&
                  text_copies[index](game, player, NULL, 0U, &copied) == CNA_RESULT_SUCCESS && copied == 0U;
    }
    cna_demo_check(
        demo, text_ok,
        "GUID, name, path and serial all use zero-byte size/copy answers for a disconnected slot");

    cna_demo_check(
        demo,
        cna_gamepad_get_firmware_version_ext(game, player, &firmware) == CNA_RESULT_SUCCESS && firmware == 0U &&
            cna_gamepad_get_steam_handle_ext(game, player, &steam_handle) == CNA_RESULT_SUCCESS &&
            steam_handle == 0U,
        "disconnected firmware and Steam identities are both zero");

    cna_demo_check(
        demo,
        cna_gamepad_get_touchpad_count_ext(game, player, &touchpads) == CNA_RESULT_SUCCESS && touchpads == 0 &&
            cna_gamepad_get_touchpad_finger_count_ext(game, player, 0, &fingers) == CNA_RESULT_SUCCESS &&
            fingers == 0 &&
            cna_gamepad_get_touchpad_finger_ext(game, player, 0, 0, &finger, &available) == CNA_RESULT_SUCCESS &&
            available == CNA_FALSE && same_touchpad_finger(&finger),
        "a disconnected controller has no touchpads and preserves the unavailable finger output");
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    PadState* state = (PadState*)context;
    static const CNA_Color white = {255, 255, 255, 255};
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo info;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    disconnected_device_contract(game, state->demo);
    result = cna_game_get_graphics_device(game, &graphics_device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.width = 1U;
    info.height = 1U;
    info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(graphics_device, &info, &state->white);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_texture2d_set_data_rgba8(state->white, &white, 1U);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_sprite_batch_create(graphics_device, &state->sprite_batch);
    }
    return result;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    PadState* state = (PadState*)context;
    (void)game_time;
    (void)out_error;
    /* The live snapshot of player one, with the canonical IndependentAxes dead zone. */
    blank_state(&state->pad);
    if (cna_gamepad_get_state(game, CNA_PLAYER_INDEX_ONE, &state->pad) != CNA_RESULT_SUCCESS) {
        ++state->polls_failed;
    } else if (!state->agreement_checked) {
        CNA_GamePadCapabilities capabilities;
        state->agreement_checked = 1;
        memset(&capabilities, 0x5a, sizeof(capabilities));
        cna_demo_check(
            state->demo,
            cna_gamepad_capabilities_init(&capabilities) == CNA_RESULT_SUCCESS &&
                capabilities.struct_size == (uint32_t)sizeof(capabilities) &&
                capabilities.struct_version == UINT32_C(1) && capabilities_are_disconnected(&capabilities),
            "the default capability value is versioned, disconnected and has no reported features");
        if (cna_demo_check_result(state->demo, cna_gamepad_get_capabilities(game, CNA_PLAYER_INDEX_ONE, &capabilities),
                                  "cna_gamepad_get_capabilities")) {
            printf("  player one: %s\n", state->pad.is_connected ? "connected" : "no controller");
            cna_demo_check(state->demo, capabilities.is_connected == state->pad.is_connected,
                           "the snapshot and the capabilities agree on whether player one is connected");
        }
    }
    if ((state->demo->frames > 0 && state->frames >= state->demo->frames) ||
        down(&state->pad, CNA_GAMEPAD_BUTTON_BACK)) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_SpriteCommand rect(CNA_Handle texture, int32_t x, int32_t y, int32_t width, int32_t height,
                              CNA_Color color)
{
    CNA_SpriteCommand c;
    memset(&c, 0, sizeof(c));
    c.struct_size = (uint32_t)sizeof(c);
    c.struct_version = UINT32_C(1);
    c.texture = texture;
    c.destination.x = x;
    c.destination.y = y;
    c.destination.width = width;
    c.destination.height = height;
    c.source.width = 1;
    c.source.height = 1;
    c.color = color;
    c.effects = CNA_SPRITE_EFFECT_NONE;
    return c;
}

static CNA_Result draw(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    static const CNA_Color background = {24, 26, 36, 255};
    static const CNA_Color panel = {52, 56, 72, 255};
    static const CNA_Color idle = {90, 94, 110, 255};
    static const CNA_GamePadButtonFlags faces[4] = {CNA_GAMEPAD_BUTTON_A, CNA_GAMEPAD_BUTTON_B, CNA_GAMEPAD_BUTTON_X,
                                                    CNA_GAMEPAD_BUTTON_Y};
    static const CNA_Color face_colors[4] = {
        {90, 200, 90, 255}, {220, 70, 60, 255}, {70, 120, 230, 255}, {230, 200, 60, 255}};
    PadState* state = (PadState*)context;
    const CNA_GamePadAnalogState* analog = &state->pad.analog;
    const CNA_Color dot = state->pad.is_connected ? (CNA_Color){240, 240, 240, 255} : idle;
    CNA_SpriteCommand commands[16];
    CNA_SpriteBatchBeginInfo begin_info;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    uint32_t n = 0U;
    int i;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->frames;
    result = cna_game_clear(game, background);

    /* Two stick panels with a dot at the stick's position (y up), two trigger bars above them. */
    commands[n++] = rect(state->white, 120, 150, 200, 200, panel);
    commands[n++] = rect(state->white, 480, 150, 200, 200, panel);
    commands[n++] = rect(state->white, 212 + (int32_t)(analog->left_thumb_stick.x * 90.0f),
                         242 - (int32_t)(analog->left_thumb_stick.y * 90.0f), 16, 16, dot);
    commands[n++] = rect(state->white, 572 + (int32_t)(analog->right_thumb_stick.x * 90.0f),
                         242 - (int32_t)(analog->right_thumb_stick.y * 90.0f), 16, 16, dot);
    commands[n++] = rect(state->white, 120, 100, 200, 20, panel);
    commands[n++] = rect(state->white, 480, 100, 200, 20, panel);
    commands[n++] = rect(state->white, 120, 100, (int32_t)(analog->left_trigger * 200.0f), 20, dot);
    commands[n++] = rect(state->white, 480, 100, (int32_t)(analog->right_trigger * 200.0f), 20, dot);
    /* A, B, X, Y: lit while down. */
    for (i = 0; i < 4; ++i) {
        commands[n++] = rect(state->white, 340 + 32 * i, 380, 24, 24,
                             has(state->pad.pressed_buttons, faces[i]) ? face_colors[i] : idle);
    }

    if (result == CNA_RESULT_SUCCESS) {
        memset(&begin_info, 0, sizeof(begin_info));
        begin_info.struct_size = (uint32_t)sizeof(begin_info);
        begin_info.struct_version = UINT32_C(1);
        begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
        result = cna_sprite_batch_begin(state->sprite_batch, &begin_info);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_sprite_batch_submit_many(state->sprite_batch, commands, n);
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_sprite_batch_end(state->sprite_batch);
        } else {
            (void)cna_sprite_batch_end(state->sprite_batch); /* end the batch even after a failure */
        }
    }
    if (result == CNA_RESULT_SUCCESS && state->demo->frames > 0 && state->frames == state->demo->frames &&
        cna_game_get_graphics_device(game, &graphics_device) == CNA_RESULT_SUCCESS) {
        cna_demo_capture(state->demo, graphics_device);
    }
    return result;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    PadState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.input.gamepad", argc, argv, 10);
    button_values(&demo);
    dpad_values(&demo);
    analog_values(&demo);
    state_values(&demo);
    dead_zones(&demo);

    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.white = CNA_INVALID_HANDLE;
    state.sprite_batch = CNA_INVALID_HANDLE;
    blank_state(&state.pad);
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.draw = draw;
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
    printf("move the sticks, pull the triggers, press A/B/X/Y; Back quits\n");
    cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager), "HiDef profile (back-buffer read-back)");
    cna_demo_check_result(&demo, cna_game_run(game), "cna_game_run");
    cna_demo_check(&demo, state.polls_failed == 0, "player one was polled every update");
    /* Owned resources go before the game. */
    if (state.sprite_batch != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_sprite_batch_destroy(state.sprite_batch), "cna_sprite_batch_destroy");
    }
    if (state.white != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_texture2d_destroy(state.white), "cna_texture2d_destroy");
    }
    cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager), "cna_graphics_device_manager_destroy");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
