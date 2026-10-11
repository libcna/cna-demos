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

static void state_values(CnaDemo* demo)
{
    static const CNA_GamePadButtonFlags held[2] = {CNA_GAMEPAD_BUTTON_A, CNA_GAMEPAD_BUTTON_DPAD_UP};
    const CNA_Vector2 left = {1.0f, 0.0f};
    const CNA_Vector2 right = {0.0f, -1.0f};
    CNA_GamePadState rest;
    CNA_GamePadState state;
    CNA_GamePadButtonFlags dpad = 0U;
    CNA_Bool is_up = CNA_FALSE;
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
}

static void dead_zones(CnaDemo* demo)
{
    CNA_GamePadAnalogState raw;
    CNA_GamePadAnalogState none;
    CNA_GamePadAnalogState axes;
    CNA_GamePadAnalogState circular;

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
        memset(&capabilities, 0, sizeof(capabilities));
        capabilities.struct_size = (uint32_t)sizeof(capabilities);
        capabilities.struct_version = UINT32_C(1);
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
