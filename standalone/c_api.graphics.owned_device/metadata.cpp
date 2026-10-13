// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.owned_device",
    .title = "Owned graphics-device lifecycle in C",
    .description = "A C program creates two graphics devices outside any Game, reconstructs and resets one, "
                   "verifies ordered lifecycle callbacks and changed presentation state, then destroys each owned "
                   "device independently.",
    .apis = {"cna_graphics_device_create", "cna_graphics_device_destroy",
             "cna_graphics_device_present", "cna_graphics_device_reset",
             "cna_graphics_device_reset_with_parameters",
             "cna_graphics_device_recreate_renderer_for_multi_sample_count_ext",
             "cna_graphics_device_subscribe_event", "cna_graphics_device_unsubscribe",
             "cna_graphics_device_get_presentation_parameters"},
    .tags = {"c", "binding", "graphics", "device", "ownership", "lifecycle", "reset", "events", "present"},
    .needs = CnaDemos::Need::CApi,
    .frames = 0,
    .restrictions = "Standalone C program with two caller-owned native graphics devices and no Game. Renderer "
                    "recreation occurs only while the target device tracks no GPU resource. Presentation success "
                    "and synchronous lifecycle state are checked; no displayed pixels, screenshot or visual "
                    "inspection is claimed. Built only with CNA_BUILD_C_API=ON.",
})
