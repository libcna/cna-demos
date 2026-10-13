/* SPDX-License-Identifier: MS-PL */
/*
 * Input from C: CNA_KeyboardState and CNA_MouseState are plain structs (no handles) captured with
 * cna_keyboard_get_state / cna_mouse_get_state and queried with cna_keyboard_state_* routes.
 * Arrow keys move a square, a dot follows the mouse (red while the left button is down), Escape quits.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct InputState {
    CnaDemo* demo;
    CNA_Handle white;        /* owned 1x1 texture */
    CNA_Handle sprite_batch; /* owned */
    int square_x;
    int square_y;
    CNA_MouseState mouse;
    int frames;
    int polls_failed;
} InputState;

static int key_down(const CNA_KeyboardState* keyboard, CNA_Key key)
{
    CNA_Bool down = CNA_FALSE;
    return cna_keyboard_state_is_key_down(keyboard, key, &down) == CNA_RESULT_SUCCESS && down == CNA_TRUE;
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    InputState* state = (InputState*)context;
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
    InputState* state = (InputState*)context;
    CNA_KeyboardState keyboard;
    (void)game_time;
    (void)out_error;
    /* A snapshot taken now: each call reads the devices again. */
    memset(&keyboard, 0, sizeof(keyboard));
    keyboard.struct_size = (uint32_t)sizeof(keyboard);
    keyboard.struct_version = UINT32_C(1);
    memset(&state->mouse, 0, sizeof(state->mouse));
    state->mouse.struct_size = (uint32_t)sizeof(state->mouse);
    state->mouse.struct_version = UINT32_C(1);
    if (cna_keyboard_get_state(game, &keyboard) != CNA_RESULT_SUCCESS ||
        cna_mouse_get_state(game, &state->mouse) != CNA_RESULT_SUCCESS) {
        ++state->polls_failed;
        return CNA_RESULT_SUCCESS;
    }
    if (key_down(&keyboard, CNA_KEY_ESCAPE)) {
        return cna_game_request_exit(game);
    }
    state->square_x += (key_down(&keyboard, CNA_KEY_RIGHT) - key_down(&keyboard, CNA_KEY_LEFT)) * 4;
    state->square_y += (key_down(&keyboard, CNA_KEY_DOWN) - key_down(&keyboard, CNA_KEY_UP)) * 4;
    if (state->demo->frames > 0 && state->frames >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_SpriteCommand box(CNA_Handle texture, int x, int y, int size, CNA_Color color)
{
    CNA_SpriteCommand c;
    memset(&c, 0, sizeof(c));
    c.struct_size = (uint32_t)sizeof(c);
    c.struct_version = UINT32_C(1);
    c.texture = texture;
    c.destination.x = x - size / 2;
    c.destination.y = y - size / 2;
    c.destination.width = size;
    c.destination.height = size;
    c.source.width = 1;
    c.source.height = 1;
    c.color = color;
    return c;
}

static CNA_Result draw(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    static const CNA_Color background = {28, 32, 44, 255};
    static const CNA_Color square_color = {90, 170, 255, 255};
    static const CNA_Color up_color = {240, 240, 240, 255};
    static const CNA_Color down_color = {255, 70, 70, 255};
    InputState* state = (InputState*)context;
    CNA_SpriteBatchBeginInfo begin_info;
    CNA_SpriteCommand commands[2];
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->frames;
    result = cna_game_clear(game, background);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    commands[0] = box(state->white, 400 + state->square_x, 240 + state->square_y, 60, square_color);
    commands[1] = box(state->white, state->mouse.x, state->mouse.y, 12,
                      (state->mouse.pressed_buttons & CNA_MOUSE_BUTTON_LEFT) != 0U ? down_color : up_color);
    memset(&begin_info, 0, sizeof(begin_info));
    begin_info.struct_size = (uint32_t)sizeof(begin_info);
    begin_info.struct_version = UINT32_C(1);
    begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
    result = cna_sprite_batch_begin(state->sprite_batch, &begin_info);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_sprite_batch_submit_many(state->sprite_batch, commands, 2U);
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_sprite_batch_end(state->sprite_batch);
        } else {
            (void)cna_sprite_batch_end(state->sprite_batch);
        }
    }
    if (result == CNA_RESULT_SUCCESS && state->frames == state->demo->frames) {
        CNA_Handle graphics_device = CNA_INVALID_HANDLE;
        if (cna_game_get_graphics_device(game, &graphics_device) == CNA_RESULT_SUCCESS) {
            cna_demo_capture(state->demo, graphics_device);
        }
    }
    return result;
}

int main(int argc, char** argv)
{
    static const CNA_Key held[2] = {CNA_KEY_LEFT, CNA_KEY_SPACE};
    CnaDemo demo;
    InputState state;
    CNA_KeyboardState snapshot;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_Key copied_keys[2] = {UINT32_MAX, UINT32_MAX};
    CNA_Bool is_up = CNA_FALSE;
    uint32_t copied_count = 0U;
    uint32_t pressed = 0U;

    cna_demo_init(&demo, "c_api.input.keyboard_mouse", argc, argv, 30);

    /* A keyboard state is a value: one can be built from a list of keys and queried like a live one. */
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.struct_size = (uint32_t)sizeof(snapshot);
    snapshot.struct_version = UINT32_C(1);
    if (cna_demo_check_result(&demo, cna_keyboard_state_init_from_keys(held, 2U, &snapshot),
                              "cna_keyboard_state_init_from_keys(Left, Space)") &&
        cna_demo_check_result(&demo, cna_keyboard_state_get_pressed_key_count(&snapshot, &pressed),
                              "cna_keyboard_state_get_pressed_key_count")) {
        cna_demo_check(&demo, pressed == 2U, "two keys are pressed in the snapshot");
        cna_demo_check(&demo, key_down(&snapshot, CNA_KEY_LEFT) && key_down(&snapshot, CNA_KEY_SPACE) &&
                                  !key_down(&snapshot, CNA_KEY_UP),
                       "is_key_down: Left and Space down, Up not");
        cna_demo_check(&demo,
                       cna_keyboard_state_is_key_up(&snapshot, CNA_KEY_LEFT, &is_up) == CNA_RESULT_SUCCESS &&
                           is_up == CNA_FALSE &&
                           cna_keyboard_state_is_key_up(&snapshot, CNA_KEY_UP, &is_up) == CNA_RESULT_SUCCESS &&
                           is_up == CNA_TRUE,
                       "is_key_up complements the down state for held and released keys");
        cna_demo_check(&demo,
                       cna_keyboard_state_copy_pressed_keys(&snapshot, copied_keys, 1U, &copied_count) ==
                               CNA_RESULT_BUFFER_TOO_SMALL &&
                           copied_count == 2U && copied_keys[0] == UINT32_MAX && copied_keys[1] == UINT32_MAX,
                       "a short pressed-key buffer reports the count without a partial copy");
        cna_demo_check(&demo,
                       cna_keyboard_state_copy_pressed_keys(&snapshot, copied_keys, 2U, &copied_count) ==
                               CNA_RESULT_SUCCESS &&
                           copied_count == 2U && copied_keys[0] == CNA_KEY_SPACE && copied_keys[1] == CNA_KEY_LEFT,
                       "pressed keys copy in ascending numeric order");
    }

    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.white = CNA_INVALID_HANDLE;
    state.sprite_batch = CNA_INVALID_HANDLE;
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
    printf("arrow keys move the square, the dot follows the mouse, Escape quits\n");
    cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager), "HiDef profile (back-buffer read-back)");
    cna_demo_check_result(&demo, cna_game_run(game), "cna_game_run");
    cna_demo_check(&demo, state.polls_failed == 0, "the keyboard and the mouse were polled every update");
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
