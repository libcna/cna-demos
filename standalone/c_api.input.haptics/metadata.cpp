// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.haptics",
    .title = "Input in C: haptic devices",
    .description = "A C program compares haptic direction, effect and capability values, enumerates host devices, "
                   "then exercises absent-id, mouse and joystick open paths plus every safe closed-device effect, "
                   "rumble, global-control and ownership operation.",
    .apis = {"cna_haptic_direction_init", "cna_haptic_direction_equals",
             "cna_haptic_effect_init", "cna_haptic_effect_equals",
             "cna_haptic_capabilities_init", "cna_haptic_capabilities_equals",
             "cna_haptics_get_count", "cna_haptics_open", "cna_haptics_open_from_joystick",
             "cna_haptics_open_from_mouse", "cna_haptics_get_is_joystick_haptic",
             "cna_haptics_get_is_mouse_haptic", "cna_haptic_device_get_is_open",
             "cna_haptic_device_get_name_size", "cna_haptic_device_copy_name",
             "cna_haptic_device_get_capabilities", "cna_haptic_device_get_is_effect_supported",
             "cna_haptic_device_init_rumble", "cna_haptic_device_play_rumble",
             "cna_haptic_device_stop_rumble", "cna_haptic_device_create_effect",
             "cna_haptic_device_update_effect", "cna_haptic_device_run_effect",
             "cna_haptic_device_stop_effect", "cna_haptic_device_destroy_effect",
             "cna_haptic_device_get_effect_status", "cna_haptic_device_stop_all_effects",
             "cna_haptic_device_set_gain", "cna_haptic_device_set_autocenter",
             "cna_haptic_device_pause", "cna_haptic_device_resume",
             "cna_haptic_device_dispose", "cna_haptic_device_destroy"},
    .tags = {"c", "binding", "input", "haptics", "force-feedback", "rumble", "effects", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). Automated checks use closed devices; populated enumeration, successful "
                    "effect upload/actuation and physical force feedback require suitable hardware and manual review.",
})
