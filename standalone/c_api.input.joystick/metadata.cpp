// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.joystick",
    .title = "Input in C: raw joysticks",
    .description = "A C program compares raw-joystick descriptors, enumerates host devices, verifies disconnected "
                   "capability and owned state snapshots, and exercises synchronous hot-plug subscriptions, raises, "
                   "selective unsubscribe and reset.",
    .apis = {"cna_joystick_info_init", "cna_joystick_info_equals",
             "cna_joystick_capabilities_init", "cna_joystick_capabilities_equals",
             "cna_joysticks_get_count", "cna_joysticks_get_capabilities",
             "cna_joysticks_get_capabilities_name_size", "cna_joysticks_copy_capabilities_name",
             "cna_joysticks_get_capabilities_guid_size", "cna_joysticks_copy_capabilities_guid",
             "cna_joysticks_capture_state", "cna_joystick_state_get_axis_count",
             "cna_joystick_state_copy_axes", "cna_joystick_state_get_button_count",
             "cna_joystick_state_copy_buttons", "cna_joystick_state_get_hat_count",
             "cna_joystick_state_copy_hats", "cna_joystick_state_get_ball_count",
             "cna_joystick_state_copy_balls", "cna_joystick_state_equals",
             "cna_joystick_state_destroy", "cna_joysticks_subscribe_connected_ext",
             "cna_joysticks_subscribe_disconnected_ext", "cna_joysticks_unsubscribe_ext",
             "cna_joysticks_raise_connected_ext", "cna_joysticks_raise_disconnected_ext",
             "cna_joysticks_reset_for_tests_ext"},
    .tags = {"c", "binding", "input", "joystick", "raw", "hot-plug", "ownership", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). The automated contract uses disconnected snapshots and public event "
                    "raises; populated device metadata and physical axes/buttons/hats/balls require hardware.",
})
