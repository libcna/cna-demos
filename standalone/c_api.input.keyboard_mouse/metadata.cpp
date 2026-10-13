// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.keyboard_mouse",
    .title = "Input in C: keyboard and mouse state",
    .description = "A C program polls CNA_KeyboardState and CNA_MouseState every update: the arrow keys move a "
                   "square, a dot follows the mouse and turns red while the left button is down, Escape quits; "
                   "a snapshot built from known keys shows complementary up/down queries and count/copy behavior.",
    .apis = {"cna_keyboard_get_state", "cna_keyboard_state_is_key_down", "cna_keyboard_state_is_key_up",
             "cna_keyboard_state_init_from_keys", "cna_keyboard_state_get_pressed_key_count",
             "cna_keyboard_state_copy_pressed_keys", "cna_mouse_get_state", "CNA_KeyboardState", "CNA_MouseState"},
    .tags = {"c", "binding", "input", "keyboard", "mouse"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Keyboard | CnaDemos::Need::Mouse,
    .frames = 30,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
