// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.graphics_manager_lifecycle",
    .title = "Graphics device manager lifecycle in C",
    .description = "Manager events expose and mutate prepared settings while explicit apply, recreate, "
                   "full-screen, draw and disposal operations retain their public ownership contracts.",
    .apis = {"cna_graphics_device_manager_create", "cna_graphics_device_manager_apply_changes",
             "cna_graphics_device_manager_create_device", "cna_graphics_device_manager_begin_draw",
             "cna_graphics_device_manager_end_draw", "cna_graphics_device_manager_dispose",
             "cna_graphics_device_manager_subscribe",
             "cna_graphics_device_manager_subscribe_preparing_device_settings",
             "cna_graphics_device_manager_subscribe_preparing_device_settings_ext",
             "cna_graphics_device_manager_destroy", "CNA_GraphicsDeviceManagerEvent",
             "CNA_PreparingDeviceSettingsCallback", "CNA_PreparingDeviceSettingsMutatorEXT"},
    .tags = {"c", "binding", "game", "graphics", "manager", "events", "lifecycle", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone game workflow requiring a video display. Platform refusal is an explicit "
                    "outcome for reconfiguration; full-screen mode switching remains manual.",
})
