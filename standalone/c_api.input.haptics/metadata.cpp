// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.haptics",
    .title = "Input in C: haptic devices",
    .description = "A C program compares haptic direction/effect/capability values, enumerates host devices without "
                   "opening them, then exercises absent-id open paths plus every safe closed-device operation.",
    .apis = {"cna_haptic_direction_init", "cna_haptic_direction_equals",
             "cna_haptic_effect_init", "cna_haptic_effect_equals",
             "cna_haptic_capabilities_init", "cna_haptic_capabilities_equals",
             "cna_haptics_get_count", "cna_haptics_open", "cna_haptics_open_from_joystick",
             "cna_haptics_get_is_joystick_haptic",
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
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API. Automated "
                    "checks enumerate real devices without opening them and mutate only guaranteed-closed absent-id "
                    "handles. Real opening/upload/actuation is explicitly gated in the C++ haptics demos.",
})
