// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.morph_targets",
    .title = "Morph-target data, tracks, blending and part upload in C",
    .description = "A C workflow owns morph source data, evaluates weight tracks, blends CPU "
                   "vertices and uploads an attached result through ModelMeshPart.",
    .apis = {"cna_morph_target_data_ext_create", "cna_morph_target_data_ext_destroy",
             "cna_morph_target_data_ext_get_type_name_byte_count",
             "cna_morph_target_data_ext_copy_type_name", "cna_morph_target_data_ext_get_stride",
             "cna_morph_target_data_ext_get_base_vertex_byte_count",
             "cna_morph_target_data_ext_copy_base_vertex_bytes",
             "cna_morph_target_data_ext_get_target_count",
             "cna_morph_target_data_ext_copy_position_deltas",
             "cna_morph_target_data_ext_copy_normal_deltas",
             "cna_morph_target_data_ext_set_weights", "cna_morph_target_data_ext_copy_weights",
             "cna_morph_target_data_ext_set_weight_track",
             "cna_morph_target_data_ext_get_weight_track_info",
             "cna_morph_target_data_ext_copy_weight_keyframe",
             "cna_morph_target_data_ext_blend", "cna_morph_weight_track_ext_evaluate",
             "cna_model_mesh_part_set_morph_target_data_ext",
             "cna_model_mesh_part_get_morph_target_data_ext",
             "cna_model_mesh_part_set_morph_weights_ext",
             "cna_morph_target_data_ext_copy_tangent_deltas",
             "cna_morph_target_data_ext_get_recompute_flat_normals_ext",
             "cna_morph_target_data_ext_set_recompute_flat_normals_ext",
             "cna_morph_target_data_ext_copy_triangle_indices_ext",
             "cna_morph_target_data_ext_set_triangle_indices_ext",
             "cna_morph_target_data_ext_set_tangent_deltas"},
    .tags = {"c", "binding", "graphics", "model", "morph", "animation", "buffer"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses one synthetic 32-byte vertex and direct VertexBuffer readback. No mesh, "
                    "Model, ContentManager, animation player, draw call or visual result is involved.",
})
