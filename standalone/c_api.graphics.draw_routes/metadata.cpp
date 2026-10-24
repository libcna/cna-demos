// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.draw_routes",
    .title = "Graphics-device draw routes in C",
    .description = "A C game draws the same vertex-colored geometry from a bound buffer, caller arrays, indices and "
                   "two repeated instances; exact pixel probes verify explicit current-effect, blend, depth-test and "
                   "depth-write behavior.",
    .apis = {"cna_primitive_type_get_vertex_count", "cna_graphics_device_draw_primitives",
             "cna_graphics_device_draw_instanced_primitives", "cna_graphics_device_draw_user_primitives",
             "cna_graphics_device_draw_user_indexed_primitives", "cna_graphics_device_set_current_effect",
             "cna_graphics_device_set_depth_test_enabled", "cna_graphics_device_set_blend_enabled",
             "cna_graphics_device_set_depth_write_enabled", "cna_graphics_device_get_backbuffer_data_window",
             "cna_graphics_device_set_vertex_buffer", "cna_graphics_device_set_index_buffer",
             "cna_basic_effect_create", "cna_basic_effect_set_vertex_color_enabled", "cna_effect_apply",
             "cna_matrix_get_identity"},
    .tags = {"c", "binding", "graphics", "3d", "draw", "vertices", "indices", "instancing", "depth", "blend"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C program. Draw results are checked through one-pixel RGBA8 readback under an "
                    "explicit HiDef Color/Depth24Stencil8 back buffer; no screenshot or visual inspection is claimed. "
                    "The blend-enable toggle installs non-premultiplied alpha blending on every renderer "
                    "(CNA-DEMOS-ISSUE-066). Built only with CNA_BUILD_C_API=ON.",
})
