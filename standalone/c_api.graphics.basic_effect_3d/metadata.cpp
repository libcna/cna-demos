// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.basic_effect_3d",
    .title = "3D in C: buffers and a BasicEffect",
    .description = "A C program puts a cube into a vertex buffer (the stock VertexPositionNormalTexture declaration) "
                   "and an index buffer, and draws it spinning with a BasicEffect lit by the default three-light "
                   "rig; matrices go through the IEffectMatrices routes. The back buffer shows the lit faces.",
    .apis = {"cna_vertex_type_copy_elements", "cna_vertex_declaration_create", "cna_vertex_buffer_create",
             "cna_vertex_buffer_set_data_raw", "cna_index_buffer_create", "cna_index_buffer_set_data",
             "cna_basic_effect_create", "cna_basic_effect_set_diffuse_color", "cna_effect_lights_enable_default",
             "cna_effect_matrices_set_world", "cna_effect_matrices_set_view", "cna_effect_matrices_set_projection",
             "cna_matrix_create_look_at", "cna_matrix_create_perspective_field_of_view", "cna_matrix_multiply",
             "cna_depth_stencil_state_init", "cna_graphics_device_set_depth_stencil_state",
             "cna_graphics_device_set_vertex_buffer", "cna_graphics_device_set_index_buffer", "cna_effect_apply",
             "cna_graphics_device_draw_indexed_primitives", "cna_effect_destroy", "cna_index_buffer_destroy",
             "cna_vertex_buffer_destroy", "cna_vertex_declaration_destroy"},
    .tags = {"c", "binding", "3d", "basic-effect", "vertex-buffer"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 10,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
