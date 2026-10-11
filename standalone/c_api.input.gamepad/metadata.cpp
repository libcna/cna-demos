// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.gamepad",
    .title = "Input in C: gamepad state",
    .description = "A C program builds a CNA_GamePadState with cna_gamepad_state_init_from_values and shows that its "
                   "one button mask holds the physical buttons, the D-pad and the derived stick and trigger bits, "
                   "compares the three dead-zone modes of cna_gamepad_apply_dead_zone, then polls player one "
                   "every update and draws the sticks, triggers and face buttons.",
    .apis = {"cna_gamepad_get_state", "cna_gamepad_get_capabilities", "cna_gamepad_state_init",
             "cna_gamepad_state_init_from_values", "cna_gamepad_state_get_dpad", "cna_gamepad_state_is_button_down",
             "cna_gamepad_state_is_button_up", "cna_gamepad_apply_dead_zone", "CNA_GamePadState"},
    .tags = {"c", "binding", "input", "gamepad", "controller", "dead-zone"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::GamePad,
    .frames = 10,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). Without a controller the live part shows player one disconnected.",
})
