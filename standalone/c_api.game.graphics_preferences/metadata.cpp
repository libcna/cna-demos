// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.graphics_preferences",
    .title = "Graphics device manager preferences in C",
    .description = "A C game owns a graphics device manager, validates its device-information value and "
                   "round-trips every preference before any reconfiguration is applied.",
    .apis = {"cna_graphics_device_information_init", "cna_graphics_device_information_clone",
             "cna_graphics_device_manager_create", "cna_graphics_device_manager_get_graphics_profile",
             "cna_graphics_device_manager_set_graphics_profile",
             "cna_graphics_device_manager_get_graphics_device",
             "cna_graphics_device_manager_get_is_full_screen",
             "cna_graphics_device_manager_set_is_full_screen",
             "cna_graphics_device_manager_get_prefer_multi_sampling",
             "cna_graphics_device_manager_set_prefer_multi_sampling",
             "cna_graphics_device_manager_get_preferred_back_buffer_format",
             "cna_graphics_device_manager_set_preferred_back_buffer_format",
             "cna_graphics_device_manager_get_preferred_back_buffer_width",
             "cna_graphics_device_manager_set_preferred_back_buffer_width",
             "cna_graphics_device_manager_get_preferred_back_buffer_height",
             "cna_graphics_device_manager_set_preferred_back_buffer_height",
             "cna_graphics_device_manager_get_preferred_depth_stencil_format",
             "cna_graphics_device_manager_set_preferred_depth_stencil_format",
             "cna_graphics_device_manager_get_synchronize_with_vertical_retrace",
             "cna_graphics_device_manager_set_synchronize_with_vertical_retrace",
             "cna_graphics_device_manager_get_supported_orientations",
             "cna_graphics_device_manager_set_supported_orientations",
             "cna_graphics_device_manager_get_preferred_presentation_mode_ext",
             "cna_graphics_device_manager_set_preferred_presentation_mode_ext",
             "CNA_GraphicsDeviceInformation", "CNA_PresentationMode"},
    .tags = {"c", "binding", "game", "graphics", "manager", "preferences", "value"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone game workflow requiring a video display. Preferences are inspected before "
                    "application; adapter negotiation and visual output are outside this demo.",
})
