// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.occlusion_query",
    .title = "Occlusion-query lifecycle and visible samples in C",
    .description = "A C game checks the query state machine, measures a full-screen visible quad, interprets an "
                   "exact sample tally or a backend's 0/1 visibility flag, and proves owned-handle cleanup.",
    .apis = {"cna_occlusion_query_create", "cna_occlusion_query_begin", "cna_occlusion_query_end",
             "cna_occlusion_query_get_is_complete", "cna_occlusion_query_get_pixel_count",
             "cna_occlusion_query_get_is_pixel_count_precise_ext", "cna_occlusion_query_has_renderer",
             "cna_occlusion_query_destroy", "cna_graphics_device_draw_user_primitives",
             "cna_graphics_device_get_backbuffer_data_window", "cna_basic_effect_create",
             "cna_basic_effect_set_vertex_color_enabled", "cna_effect_apply"},
    .tags = {"c", "binding", "graphics", "3d", "occlusion", "query", "visibility"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD | CnaDemos::Need::OcclusionQuery,
    .frames = 30,
    .restrictions = "Standalone C program. The visible result is verified through query state and one-pixel RGBA8 "
                    "readback; precise backends return a sample tally while boolean-query backends return one. No "
                    "screenshot or visual inspection is claimed. Built only with CNA_BUILD_C_API=ON.",
})
