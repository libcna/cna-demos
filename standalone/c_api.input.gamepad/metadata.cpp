// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.gamepad",
    .title = "Input in C: gamepad state",
    .description = "A C program exercises gamepad button, D-pad, stick, trigger, capability and composed-state values, "
                   "checks the live API's coherent disconnected-device contract, compares scalar and full dead-zone "
                   "transforms, then polls player one every update and draws the sticks, triggers and face buttons.",
    .apis = {"cna_gamepad_get_state", "cna_gamepad_get_capabilities", "cna_gamepad_capabilities_init",
             "cna_gamepad_buttons_init", "cna_gamepad_buttons_init_from_mask",
             "cna_gamepad_buttons_init_from_button_array", "cna_gamepad_buttons_is_pressed",
             "cna_gamepad_buttons_equals", "cna_gamepad_buttons_not_equals", "cna_gamepad_buttons_get_hash_code",
             "cna_gamepad_dpad_init", "cna_gamepad_dpad_init_from_states",
             "cna_gamepad_dpad_init_from_button_array", "cna_gamepad_dpad_is_pressed",
             "cna_gamepad_dpad_equals", "cna_gamepad_dpad_not_equals", "cna_gamepad_dpad_get_hash_code",
             "cna_gamepad_thumb_sticks_init", "cna_gamepad_thumb_sticks_init_from_positions",
             "cna_gamepad_thumb_sticks_equals", "cna_gamepad_thumb_sticks_not_equals",
             "cna_gamepad_thumb_sticks_get_hash_code", "cna_gamepad_triggers_init",
             "cna_gamepad_triggers_init_from_positions", "cna_gamepad_triggers_equals",
             "cna_gamepad_triggers_not_equals", "cna_gamepad_triggers_get_hash_code",
             "cna_gamepad_state_init", "cna_gamepad_state_init_from_components",
             "cna_gamepad_state_init_from_values", "cna_gamepad_state_get_buttons",
             "cna_gamepad_state_get_dpad", "cna_gamepad_state_get_thumb_sticks",
             "cna_gamepad_state_get_triggers", "cna_gamepad_state_set_packet_number_ext",
             "cna_gamepad_state_equals", "cna_gamepad_state_not_equals", "cna_gamepad_state_get_hash_code",
             "cna_gamepad_exclude_axis_dead_zone", "cna_gamepad_state_is_button_down",
             "cna_gamepad_state_is_button_up", "cna_gamepad_apply_dead_zone", "cna_gamepad_set_vibration",
             "cna_gamepad_set_light_bar_ext", "cna_gamepad_set_trigger_vibration_ext", "cna_gamepad_get_gyro_ext",
             "cna_gamepad_get_accelerometer_ext", "cna_gamepad_get_player_index_ext",
             "cna_gamepad_set_player_index_ext", "cna_gamepad_get_power_info_ext",
             "cna_gamepad_get_button_label_ext", "cna_gamepad_get_guid_size_ext", "cna_gamepad_copy_guid_ext",
             "cna_gamepad_get_name_size_ext", "cna_gamepad_copy_name_ext", "cna_gamepad_get_path_size_ext",
             "cna_gamepad_copy_path_ext", "cna_gamepad_get_serial_size_ext", "cna_gamepad_copy_serial_ext",
             "cna_gamepad_get_firmware_version_ext", "cna_gamepad_get_steam_handle_ext",
             "cna_gamepad_get_connection_state_ext", "cna_gamepad_get_touchpad_count_ext",
             "cna_gamepad_get_touchpad_finger_count_ext", "cna_gamepad_get_touchpad_finger_ext",
             "CNA_GamePadState"},
    .tags = {"c", "binding", "input", "gamepad", "controller", "dead-zone"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::GamePad,
    .frames = 10,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). The automated contract uses one disconnected player slot. Physical "
                    "controller actuation, sensor data, identity values and touchpad input require manual testing.",
})
