// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.device_presentation",
    .title = "Graphics device presentation in C",
    .description = "A C game borrows its callback-scoped graphics device, reads complete applied presentation "
                   "parameters, changes and verifies the presentation interval, restores the exact original "
                   "value, reads the device display mode and demonstrates the native-window limitation.",
    .apis = {"cna_graphics_device_get_presentation_parameters",
             "cna_graphics_device_set_presentation_parameters",
             "cna_graphics_device_get_display_mode", "cna_graphics_device_get_native_window_handle",
             "CNA_PresentationParameters", "CNA_DisplayMode"},
    .tags = {"c", "binding", "graphics", "device", "display", "presentation", "limitations"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C program; the presentation interval is restored during the same callback. "
                    "Native window handles are intentionally unavailable through a graphics device, and no "
                    "rendering or visual result is claimed. Built only with CNA_BUILD_C_API=ON.",
})
