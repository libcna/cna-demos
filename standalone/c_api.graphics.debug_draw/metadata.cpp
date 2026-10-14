// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.debug_draw",
    .title = "DebugDraw geometry in C",
    .description = "A C game builds depth-tested and overlay line lists, copies their exact vertices, exercises "
                   "frame reset/clear/end semantics and validates box, clamped sphere, frustum and cross geometry "
                   "before issuing the real draw.",
    .apis = {"cna_debug_draw_create", "cna_debug_draw_destroy",
             "cna_debug_draw_begin", "cna_debug_draw_end", "cna_debug_draw_clear",
             "cna_debug_draw_add_line", "cna_debug_draw_add_box",
             "cna_debug_draw_add_sphere", "cna_debug_draw_add_bounding_sphere",
             "cna_debug_draw_add_frustum", "cna_debug_draw_add_cross",
             "cna_debug_draw_is_depth_tested", "cna_debug_draw_set_depth_tested",
             "cna_debug_draw_get_line_count", "cna_debug_draw_copy_vertices"},
    .tags = {"c", "binding", "graphics", "extension", "debug", "lines", "geometry", "depth", "overlay"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C game. Generated line geometry, list routing and successful drawing are checked; "
                    "the existing C++ DebugDraw scene owns detailed pixel/visual evidence. No screenshot or manual "
                    "visual-quality inspection is claimed. Built only with CNA_BUILD_C_API=ON and CNA_CNAEXT=ON.",
})
