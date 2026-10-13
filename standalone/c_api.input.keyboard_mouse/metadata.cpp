// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.keyboard_mouse",
    .title = "Input in C: keyboard and mouse state",
    .description = "A C program exercises keyboard values, live player slots, modifiers and key-name mapping, then "
                   "polls keyboard/mouse state, exercises mouse position, relative, capture, warp and clicked-event "
                   "behavior, and demonstrates cursor lifetimes; arrow keys move a square and Escape quits.",
    .apis = {"cna_keyboard_get_state", "cna_keyboard_state_is_key_down", "cna_keyboard_state_is_key_up",
             "cna_keyboard_state_init_from_keys", "cna_keyboard_state_get_pressed_key_count",
             "cna_keyboard_state_copy_pressed_keys", "cna_keyboard_state_init",
             "cna_keyboard_state_get_key_state", "cna_keyboard_state_equals", "cna_keyboard_state_not_equals",
             "cna_keyboard_state_get_hash_code", "cna_keyboard_get_state_for_player",
             "cna_keyboard_get_key_from_scancode_ext", "cna_keyboard_get_mod_state_ext",
             "cna_keyboard_get_scancode_name_size_ext", "cna_keyboard_copy_scancode_name_ext",
             "cna_keyboard_get_scancode_from_name_ext", "cna_keyboard_get_key_name_size_ext",
             "cna_keyboard_copy_key_name_ext", "cna_keyboard_get_key_from_name_ext",
             "cna_mouse_get_state", "cna_mouse_state_init",
             "cna_mouse_state_init_from_values", "cna_mouse_state_init_from_values_ext", "cna_mouse_state_equals",
             "cna_mouse_state_not_equals", "cna_mouse_state_get_hash_code", "cna_mouse_state_get_string_size",
             "cna_mouse_state_copy_string", "cna_mouse_get_window_handle", "cna_mouse_set_window_handle",
             "cna_mouse_set_position", "cna_mouse_get_is_relative_mouse_mode_ext",
             "cna_mouse_set_is_relative_mouse_mode_ext", "cna_mouse_set_capture_ext",
             "cna_mouse_get_global_position_ext", "cna_mouse_warp_global_ext", "cna_mouse_subscribe_clicked_ext",
             "cna_mouse_unsubscribe_clicked_ext", "cna_mouse_raise_clicked_ext", "cna_mouse_reset_for_tests_ext",
             "cna_mouse_cursor_create_ext", "cna_mouse_cursor_get_stock_ext",
             "cna_mouse_cursor_create_from_texture2d", "cna_mouse_cursor_dispose", "cna_mouse_cursor_destroy",
             "cna_mouse_set_cursor_ext", "CNA_KeyboardState", "CNA_MouseState", "CNA_MouseCursorHandle"},
    .tags = {"c", "binding", "input", "keyboard", "mouse"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Keyboard | CnaDemos::Need::Mouse | CnaDemos::Need::Window,
    .frames = 30,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
