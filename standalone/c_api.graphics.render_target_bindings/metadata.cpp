// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.render_target_bindings",
    .title = "Render-target bindings in C",
    .description = "A C game creates matching 2D and cube render targets, inspects their applied state, binds "
                   "both as an MRT array with atomic read-back, replaces it with one cube face, restores the "
                   "backbuffer and demonstrates the silent ContentLost subscription lifecycle.",
    .apis = {"cna_render_target_usage_preserves_contents", "cna_render_target_cube_create",
             "cna_render_target_get_info", "cna_graphics_device_set_render_target_cube",
             "cna_graphics_device_set_render_targets", "cna_graphics_device_get_render_target_count",
             "cna_graphics_device_copy_render_targets", "cna_render_target_subscribe_content_lost",
             "cna_render_target_unsubscribe_content_lost", "CNA_RenderTargetInfo",
             "CNA_RenderTargetBinding"},
    .tags = {"c", "binding", "graphics", "render-target", "cube", "mrt", "events", "buffers"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C program; current Linux renderers do not lose devices, so ContentLost "
                    "registration is exercised but notification delivery remains renderer-specific. No pixels "
                    "or visual result are claimed. Built only with CNA_BUILD_C_API=ON.",
})
