// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.device_clears",
    .title = "Graphics-device buffer clears in C",
    .description = "A C game clears an actual color/depth/stencil back buffer through all three graphics-device "
                   "clear routes, reads exact color pixels after each operation and proves invalid or empty masks "
                   "leave the established color buffer unchanged.",
    .apis = {"cna_graphics_device_clear_rgba", "cna_graphics_device_clear_color_depth",
             "cna_graphics_device_clear_options", "cna_graphics_device_get_backbuffer_data_rgba8",
             "CNA_ClearOptions"},
    .tags = {"c", "binding", "graphics", "device", "clear", "backbuffer", "depth", "stencil", "readback"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C program. Pixel assertions use back-buffer readback under both automated renderer "
                    "selections; no screenshot or visual result is claimed. Built only with CNA_BUILD_C_API=ON.",
})
