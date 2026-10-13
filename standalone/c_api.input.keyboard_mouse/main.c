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

typedef struct ClickRecord {
    int count;
    int32_t last_button;
} ClickRecord;

static void mouse_clicked(int32_t button, void* context)
{
    ClickRecord* record = (ClickRecord*)context;
    ++record->count;
    record->last_button = button;
}

static int key_down(const CNA_KeyboardState* keyboard, CNA_Key key)
{
    CNA_Bool down = CNA_FALSE;
    return cna_keyboard_state_is_key_down(keyboard, key, &down) == CNA_RESULT_SUCCESS && down == CNA_TRUE;
}

static void demonstrate_keyboard_values(CnaDemo* demo, const CNA_KeyboardState* snapshot)
{
    static const CNA_Key reordered_keys[2] = {CNA_KEY_SPACE, CNA_KEY_LEFT};
    static const CNA_Key a_key[1] = {CNA_KEY_A};
    static const CNA_Key num_pad1_key[1] = {CNA_KEY_NUM_PAD1};
    CNA_KeyboardState empty;
    CNA_KeyboardState reordered;
    CNA_KeyboardState a_state;
    CNA_KeyboardState num_pad1_state;
    CNA_KeyState left_state = CNA_KEY_STATE_UP;
    CNA_KeyState up_state = CNA_KEY_STATE_DOWN;
    CNA_Bool equal = CNA_FALSE;
    CNA_Bool different = CNA_FALSE;
    int32_t a_hash = 0;
    int32_t num_pad1_hash = 1;

    memset(&empty, 0x5a, sizeof(empty));
    cna_demo_check(
        demo,
        cna_keyboard_state_init(&empty) == CNA_RESULT_SUCCESS &&
            empty.struct_size == (uint32_t)sizeof(empty) && empty.struct_version == UINT32_C(1) &&
            empty.pressed_key_words[0] == 0U && empty.pressed_key_words[1] == 0U &&
            empty.pressed_key_words[2] == 0U && empty.pressed_key_words[3] == 0U,
        "the default keyboard value has no pressed keys and initializes its versioned layout");

    cna_demo_check(
        demo,
        cna_keyboard_state_get_key_state(snapshot, CNA_KEY_LEFT, &left_state) == CNA_RESULT_SUCCESS &&
            left_state == CNA_KEY_STATE_DOWN &&
            cna_keyboard_state_get_key_state(snapshot, CNA_KEY_UP, &up_state) == CNA_RESULT_SUCCESS &&
            up_state == CNA_KEY_STATE_UP,
        "the keyboard indexer reports Down for a held key and Up for a released key");

    memset(&reordered, 0, sizeof(reordered));
    cna_demo_check(
        demo,
        cna_keyboard_state_init_from_keys(reordered_keys, 2U, &reordered) == CNA_RESULT_SUCCESS &&
            cna_keyboard_state_equals(snapshot, &reordered, &equal) == CNA_RESULT_SUCCESS && equal == CNA_TRUE &&
            cna_keyboard_state_not_equals(snapshot, &empty, &different) == CNA_RESULT_SUCCESS &&
            different == CNA_TRUE,
        "keyboard equality ignores construction order while inequality detects a different pressed set");

    memset(&a_state, 0, sizeof(a_state));
    memset(&num_pad1_state, 0, sizeof(num_pad1_state));
    cna_demo_check(
        demo,
        cna_keyboard_state_init_from_keys(a_key, 1U, &a_state) == CNA_RESULT_SUCCESS &&
            cna_keyboard_state_init_from_keys(num_pad1_key, 1U, &num_pad1_state) == CNA_RESULT_SUCCESS &&
            cna_keyboard_state_not_equals(&a_state, &num_pad1_state, &different) == CNA_RESULT_SUCCESS &&
            different == CNA_TRUE &&
            cna_keyboard_state_get_hash_code(&a_state, &a_hash) == CNA_RESULT_SUCCESS &&
            cna_keyboard_state_get_hash_code(&num_pad1_state, &num_pad1_hash) == CNA_RESULT_SUCCESS &&
            a_hash == num_pad1_hash,
        "the canonical eight-word XOR hash permits the documented 32-slot A/NumPad1 collision");
}

static void demonstrate_keyboard_platform(CnaDemo* demo, CNA_Handle game)
{
    CNA_KeyboardState live;
    CNA_KeyboardState player_one;
    CNA_KeyboardState player_four;
    CNA_Bool equal_one = CNA_FALSE;
    CNA_Bool equal_four = CNA_FALSE;
    CNA_KeyModifiers modifiers = UINT32_MAX;
    CNA_Key mapped_key = CNA_KEY_NONE;
    CNA_Key round_trip = CNA_KEY_NONE;
    uint64_t bytes = 0U;
    uint64_t copied = 0U;
    char name[32];

    (void)cna_keyboard_state_init(&live);
    (void)cna_keyboard_state_init(&player_one);
    (void)cna_keyboard_state_init(&player_four);
    cna_demo_check(
        demo,
        cna_keyboard_get_state(game, &live) == CNA_RESULT_SUCCESS &&
            cna_keyboard_get_state_for_player(game, CNA_PLAYER_INDEX_ONE, &player_one) == CNA_RESULT_SUCCESS &&
            cna_keyboard_get_state_for_player(game, CNA_PLAYER_INDEX_FOUR, &player_four) == CNA_RESULT_SUCCESS &&
            cna_keyboard_state_equals(&live, &player_one, &equal_one) == CNA_RESULT_SUCCESS &&
            equal_one == CNA_TRUE &&
            cna_keyboard_state_equals(&live, &player_four, &equal_four) == CNA_RESULT_SUCCESS &&
            equal_four == CNA_TRUE,
        "all player slots observe the same single live keyboard snapshot");

    cna_demo_check(
        demo,
        cna_keyboard_get_mod_state_ext(game, &modifiers) == CNA_RESULT_SUCCESS &&
            (modifiers & ~CNA_KEY_MODIFIER_ALL) == 0U,
        "live keyboard modifiers contain only the public combinable modifier bits");
    printf("keyboard modifiers: 0x%02x\n", (unsigned)modifiers);

    memset(name, 0, sizeof(name));
    cna_demo_check(
        demo,
        cna_keyboard_get_scancode_name_size_ext(game, CNA_KEY_A, &bytes) == CNA_RESULT_SUCCESS && bytes > 0U &&
            bytes < (uint64_t)sizeof(name) &&
            cna_keyboard_copy_scancode_name_ext(game, CNA_KEY_A, name, (uint64_t)sizeof(name), &copied) ==
                CNA_RESULT_SUCCESS && copied == bytes &&
            cna_keyboard_get_scancode_from_name_ext(game, (CNA_StringView){name, copied}, &round_trip) ==
                CNA_RESULT_SUCCESS && round_trip == CNA_KEY_A,
        "a physical A scancode name copies without a terminator and round-trips to the same scancode");

    bytes = 0U;
    copied = 0U;
    round_trip = CNA_KEY_NONE;
    memset(name, 0, sizeof(name));
    cna_demo_check(
        demo,
        cna_keyboard_get_key_from_scancode_ext(game, CNA_KEY_A, &mapped_key) == CNA_RESULT_SUCCESS &&
            mapped_key != CNA_KEY_NONE &&
            cna_keyboard_get_key_name_size_ext(game, CNA_KEY_A, &bytes) == CNA_RESULT_SUCCESS && bytes > 0U &&
            bytes < (uint64_t)sizeof(name) &&
            cna_keyboard_copy_key_name_ext(game, CNA_KEY_A, name, (uint64_t)sizeof(name), &copied) ==
                CNA_RESULT_SUCCESS && copied == bytes &&
            cna_keyboard_get_key_from_name_ext(game, (CNA_StringView){name, copied}, &round_trip) ==
                CNA_RESULT_SUCCESS && round_trip == mapped_key,
        "the active layout maps physical A to the key recovered from its copied key name");
}

static void demonstrate_mouse_values(CnaDemo* demo)
{
    static const char expected_text[] = "[MouseState X=12, Y=34, Buttons=Left Right, Wheel=120]";
    CNA_MouseState empty;
    CNA_MouseState ordinary;
    CNA_MouseState extended;
    CNA_Bool equal = CNA_TRUE;
    CNA_Bool different = CNA_FALSE;
    int32_t ordinary_hash = 0;
    int32_t extended_hash = 1;
    uint64_t bytes = 0U;
    char text[96];

    memset(&empty, 0x5a, sizeof(empty));
    cna_demo_check(
        demo,
        cna_mouse_state_init(&empty) == CNA_RESULT_SUCCESS && empty.x == 0 && empty.y == 0 &&
            empty.scroll_wheel == 0 && empty.horizontal_scroll_wheel == 0 && empty.pressed_buttons == 0U,
        "the default mouse value is at the origin with no buttons or wheel movement");

    memset(&ordinary, 0, sizeof(ordinary));
    cna_demo_check(
        demo,
        cna_mouse_state_init_from_values(
            12, 34, 120, CNA_MOUSE_BUTTON_LEFT | CNA_MOUSE_BUTTON_RIGHT, &ordinary) == CNA_RESULT_SUCCESS &&
            ordinary.x == 12 && ordinary.y == 34 && ordinary.scroll_wheel == 120 &&
            ordinary.horizontal_scroll_wheel == 0 &&
            ordinary.pressed_buttons == (CNA_MOUSE_BUTTON_LEFT | CNA_MOUSE_BUTTON_RIGHT),
        "the ordinary mouse constructor maps position, wheel and button bits and leaves the horizontal wheel zero");

    memset(&extended, 0, sizeof(extended));
    cna_demo_check(
        demo,
        cna_mouse_state_init_from_values_ext(
            12, 34, 120, -240, CNA_MOUSE_BUTTON_MIDDLE, &extended) == CNA_RESULT_SUCCESS &&
            extended.x == 12 && extended.y == 34 && extended.scroll_wheel == 120 &&
            extended.horizontal_scroll_wheel == -240 && extended.pressed_buttons == CNA_MOUSE_BUTTON_MIDDLE,
        "the extended mouse constructor also preserves the horizontal wheel");

    cna_demo_check(
        demo,
        cna_mouse_state_equals(&ordinary, &extended, &equal) == CNA_RESULT_SUCCESS && equal == CNA_FALSE &&
            cna_mouse_state_not_equals(&ordinary, &extended, &different) == CNA_RESULT_SUCCESS &&
            different == CNA_TRUE,
        "mouse equality includes buttons and the horizontal wheel");
    cna_demo_check(
        demo,
        cna_mouse_state_get_hash_code(&ordinary, &ordinary_hash) == CNA_RESULT_SUCCESS &&
            cna_mouse_state_get_hash_code(&extended, &extended_hash) == CNA_RESULT_SUCCESS &&
            ordinary_hash == extended_hash,
        "the canonical mouse hash uses position and the vertical wheel only");

    memset(text, 0, sizeof(text));
    cna_demo_check(
        demo,
        cna_mouse_state_get_string_size(&ordinary, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == (uint64_t)strlen(expected_text) &&
            cna_mouse_state_copy_string(&ordinary, text, (uint64_t)sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == (uint64_t)strlen(expected_text) && strcmp(text, expected_text) == 0,
        "mouse text reports position, named buttons and the vertical wheel");
}

static void demonstrate_mouse_platform(CnaDemo* demo, CNA_Handle game)
{
    CNA_MouseState original;
    CNA_MouseState moved;
    CNA_MouseEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    ClickRecord clicks = {0, -1};
    uint64_t window = 0U;
    uint64_t round_trip = 0U;
    CNA_Bool relative_before = CNA_FALSE;
    CNA_Bool relative_changed = CNA_FALSE;
    CNA_Bool relative_restored = CNA_FALSE;
    CNA_Bool capture_enabled = CNA_FALSE;
    CNA_Bool capture_disabled = CNA_FALSE;
    CNA_Bool warp_applied = CNA_FALSE;
    CNA_Bool restore_applied = CNA_FALSE;
    CNA_Result relative_change_result;
    CNA_Result relative_restore_result = CNA_RESULT_SUCCESS;
    CNA_Result capture_enable_result;
    CNA_Result capture_disable_result;
    CNA_Result original_result;
    CNA_Result position_result = CNA_RESULT_INVALID_STATE;
    CNA_Result moved_result = CNA_RESULT_INVALID_STATE;
    CNA_Result position_restore_result = CNA_RESULT_INVALID_STATE;
    CNA_Result global_result;
    CNA_Result warp_result;
    CNA_Result warped_result;
    CNA_Result warp_restore_result = CNA_RESULT_INVALID_STATE;
    int32_t global_x = 0;
    int32_t global_y = 0;
    int32_t warped_x = 0;
    int32_t warped_y = 0;

    memset(&original, 0, sizeof(original));
    original.struct_size = (uint32_t)sizeof(original);
    original.struct_version = UINT32_C(1);
    memset(&moved, 0, sizeof(moved));
    moved.struct_size = (uint32_t)sizeof(moved);
    moved.struct_version = UINT32_C(1);

    cna_demo_check(
        demo,
        cna_mouse_get_window_handle(game, &window) == CNA_RESULT_SUCCESS &&
            cna_mouse_set_window_handle(game, window) == CNA_RESULT_SUCCESS &&
            cna_mouse_get_window_handle(game, &round_trip) == CNA_RESULT_SUCCESS && round_trip == window,
        "the mouse window token round-trips, including a renderer's explicit zero token");

    original_result = cna_mouse_get_state(game, &original);
    if (original_result == CNA_RESULT_SUCCESS) {
        position_result = cna_mouse_set_position(game, 123, 77);
        moved_result = cna_mouse_get_state(game, &moved);
        position_restore_result = cna_mouse_set_position(game, original.x, original.y);
    }
    cna_demo_check(
        demo,
        original_result == CNA_RESULT_SUCCESS && position_result == CNA_RESULT_SUCCESS &&
            moved_result == CNA_RESULT_SUCCESS && moved.x == 123 && moved.y == 77 &&
            position_restore_result == CNA_RESULT_SUCCESS,
        "SetPosition is visible in the next live mouse snapshot before the original position is restored");

    if (cna_mouse_get_is_relative_mouse_mode_ext(game, &relative_before) == CNA_RESULT_SUCCESS) {
        relative_change_result = cna_mouse_set_is_relative_mouse_mode_ext(
            game, relative_before == CNA_TRUE ? CNA_FALSE : CNA_TRUE);
        (void)cna_mouse_get_is_relative_mouse_mode_ext(game, &relative_changed);
        if (relative_change_result == CNA_RESULT_SUCCESS) {
            relative_restore_result = cna_mouse_set_is_relative_mouse_mode_ext(game, relative_before);
        }
        (void)cna_mouse_get_is_relative_mouse_mode_ext(game, &relative_restored);
    } else {
        relative_change_result = CNA_RESULT_INVALID_STATE;
        relative_restore_result = CNA_RESULT_INVALID_STATE;
    }
    printf("relative mode: window=%llu before=%u change_result=%u changed=%u restore_result=%u restored=%u\n",
           (unsigned long long)window, (unsigned)relative_before, (unsigned)relative_change_result,
           (unsigned)relative_changed, (unsigned)relative_restore_result, (unsigned)relative_restored);
    cna_demo_check(
        demo,
        ((relative_change_result == CNA_RESULT_SUCCESS && relative_restore_result == CNA_RESULT_SUCCESS &&
             relative_restored == relative_before &&
             (relative_changed != relative_before || (window == 0U && relative_changed == relative_before))) ||
            (relative_change_result == CNA_RESULT_PLATFORM && window == 0U &&
             relative_changed == relative_before && relative_restored == relative_before)),
        "relative mode toggles and restores, or stays unchanged without a renderer window");

    capture_enable_result = cna_mouse_set_capture_ext(game, CNA_TRUE, &capture_enabled);
    capture_disable_result = cna_mouse_set_capture_ext(game, CNA_FALSE, &capture_disabled);
    printf("capture: enable_result=%u applied=%u disable_result=%u applied=%u\n",
           (unsigned)capture_enable_result, (unsigned)capture_enabled,
           (unsigned)capture_disable_result, (unsigned)capture_disabled);
    cna_demo_check(
        demo,
        capture_enable_result == CNA_RESULT_SUCCESS && capture_disable_result == CNA_RESULT_SUCCESS &&
            (capture_enabled == CNA_TRUE || capture_enabled == CNA_FALSE) &&
            (capture_disabled == CNA_TRUE || capture_disabled == CNA_FALSE),
        "capture and release report whether the active renderer applied each request");

    global_result = cna_mouse_get_global_position_ext(game, &global_x, &global_y);
    warp_result = cna_mouse_warp_global_ext(game, 17, 19, &warp_applied);
    warped_result = cna_mouse_get_global_position_ext(game, &warped_x, &warped_y);
    if (global_result == CNA_RESULT_SUCCESS && warp_applied == CNA_TRUE) {
        warp_restore_result = cna_mouse_warp_global_ext(game, global_x, global_y, &restore_applied);
    }
    cna_demo_check(
        demo,
        global_result == CNA_RESULT_SUCCESS && warp_result == CNA_RESULT_SUCCESS &&
            warped_result == CNA_RESULT_SUCCESS && warp_applied == CNA_TRUE &&
            warped_x == 17 && warped_y == 19 && warp_restore_result == CNA_RESULT_SUCCESS &&
            restore_applied == CNA_TRUE,
        "a global warp is observable and the original desktop position is restored");

    cna_demo_check(
        demo,
        cna_mouse_subscribe_clicked_ext(mouse_clicked, &clicks, &registration) == CNA_RESULT_SUCCESS &&
            registration != CNA_INVALID_HANDLE &&
            cna_mouse_raise_clicked_ext(game, 3) == CNA_RESULT_SUCCESS && clicks.count == 1 &&
            clicks.last_button == 3,
        "the clicked callback runs synchronously with its native button number");
    cna_demo_check(
        demo,
        cna_mouse_unsubscribe_clicked_ext(registration) == CNA_RESULT_SUCCESS &&
            cna_mouse_raise_clicked_ext(game, 4) == CNA_RESULT_SUCCESS && clicks.count == 1,
        "unsubscribing stops later clicked notifications");

    registration = CNA_INVALID_HANDLE;
    clicks.count = 0;
    cna_demo_check(
        demo,
        cna_mouse_subscribe_clicked_ext(mouse_clicked, &clicks, &registration) == CNA_RESULT_SUCCESS &&
            cna_mouse_reset_for_tests_ext(game) == CNA_RESULT_SUCCESS &&
            cna_mouse_set_window_handle(game, window) == CNA_RESULT_SUCCESS &&
            cna_mouse_raise_clicked_ext(game, 5) == CNA_RESULT_SUCCESS && clicks.count == 0 &&
            cna_mouse_unsubscribe_clicked_ext(registration) == CNA_RESULT_SUCCESS,
        "mouse reset drops registrations and the released registration remains safely owned");
}

static void demonstrate_mouse_cursors(InputState* state, CNA_Handle game)
{
    CNA_MouseCursorHandle owned_default = CNA_INVALID_HANDLE;
    CNA_MouseCursorHandle stock_arrow = CNA_INVALID_HANDLE;
    CNA_MouseCursorHandle custom = CNA_INVALID_HANDLE;
    CNA_Result create_result;
    CNA_Result set_result = CNA_RESULT_INVALID_STATE;
    CNA_Result first_dispose_result = CNA_RESULT_INVALID_STATE;
    CNA_Result second_dispose_result = CNA_RESULT_INVALID_STATE;
    CNA_Result destroy_result = CNA_RESULT_INVALID_STATE;
    CNA_Result stock_result;
    CNA_Result stock_dispose_result = CNA_RESULT_INVALID_STATE;
    CNA_Result stock_set_result = CNA_RESULT_INVALID_STATE;
    CNA_Result custom_result;
    CNA_Result custom_set_result = CNA_RESULT_INVALID_STATE;
    CNA_Result arrow_restore_result = CNA_RESULT_INVALID_STATE;
    CNA_Result custom_first_dispose_result = CNA_RESULT_INVALID_STATE;
    CNA_Result custom_second_dispose_result = CNA_RESULT_INVALID_STATE;
    CNA_Result custom_destroy_result = CNA_RESULT_INVALID_STATE;
    CNA_Result stock_destroy_result = CNA_RESULT_INVALID_STATE;

    create_result = cna_mouse_cursor_create_ext(&owned_default);
    if (create_result == CNA_RESULT_SUCCESS) {
        set_result = cna_mouse_set_cursor_ext(game, owned_default);
        first_dispose_result = cna_mouse_cursor_dispose(owned_default);
        second_dispose_result = cna_mouse_cursor_dispose(owned_default);
        destroy_result = cna_mouse_cursor_destroy(owned_default);
    }
    cna_demo_check(
        state->demo,
        create_result == CNA_RESULT_SUCCESS && set_result == CNA_RESULT_SUCCESS &&
            first_dispose_result == CNA_RESULT_SUCCESS && second_dispose_result == CNA_RESULT_SUCCESS &&
            destroy_result == CNA_RESULT_SUCCESS,
        "an owned default cursor can be installed, disposed idempotently and destroyed");

    stock_result = cna_mouse_cursor_get_stock_ext(game, CNA_MOUSE_CURSOR_STOCK_ARROW, &stock_arrow);
    if (stock_result == CNA_RESULT_SUCCESS) {
        stock_dispose_result = cna_mouse_cursor_dispose(stock_arrow);
        stock_set_result = cna_mouse_set_cursor_ext(game, stock_arrow);
    }
    cna_demo_check(
        state->demo,
        stock_result == CNA_RESULT_SUCCESS && stock_dispose_result == CNA_RESULT_SUCCESS &&
            stock_set_result == CNA_RESULT_SUCCESS,
        "disposing a borrowed stock Arrow is a no-op and it remains installable");

    custom_result = cna_mouse_cursor_create_from_texture2d(game, state->white, 0, 0, &custom);
    if (custom_result == CNA_RESULT_SUCCESS) {
        custom_set_result = cna_mouse_set_cursor_ext(game, custom);
        if (stock_arrow != CNA_INVALID_HANDLE) {
            arrow_restore_result = cna_mouse_set_cursor_ext(game, stock_arrow);
        }
        custom_first_dispose_result = cna_mouse_cursor_dispose(custom);
        custom_second_dispose_result = cna_mouse_cursor_dispose(custom);
        custom_destroy_result = cna_mouse_cursor_destroy(custom);
    }
    if (stock_arrow != CNA_INVALID_HANDLE) {
        stock_destroy_result = cna_mouse_cursor_destroy(stock_arrow);
    }
    cna_demo_check(
        state->demo,
        custom_result == CNA_RESULT_SUCCESS && custom_set_result == CNA_RESULT_SUCCESS &&
            arrow_restore_result == CNA_RESULT_SUCCESS && custom_first_dispose_result == CNA_RESULT_SUCCESS &&
            custom_second_dispose_result == CNA_RESULT_SUCCESS && custom_destroy_result == CNA_RESULT_SUCCESS &&
            stock_destroy_result == CNA_RESULT_SUCCESS,
        "a texture cursor owns copied pixels and releases after the stock Arrow is restored");
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
        demonstrate_keyboard_platform(state->demo, game);
        demonstrate_mouse_platform(state->demo, game);
        demonstrate_mouse_cursors(state, game);
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

    demonstrate_keyboard_values(&demo, &snapshot);
    demonstrate_mouse_values(&demo);

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
