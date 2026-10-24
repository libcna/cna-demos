// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.skinned_models",
    .title = "SkinnedModel skeleton, clips, moves and part ownership in C",
    .description = "A C workflow copies a skeleton and clip, evaluates skin transforms, moves "
                   "models and replaces a tinted owned render part with real graphics resources.",
    .apis = {"cna_skinned_model_ext_create_default", "cna_skinned_model_ext_create",
             "cna_skinned_model_ext_destroy", "cna_skinned_model_ext_create_move",
             "cna_skinned_model_ext_move_assign", "cna_skinned_model_ext_set_skeleton",
             "cna_skinned_model_ext_get_bone_count",
             "cna_skinned_model_ext_copy_parent_bone_indices",
             "cna_skinned_model_ext_copy_bind_pose_local",
             "cna_skinned_model_ext_copy_inverse_bind_pose_global",
             "cna_skinned_model_ext_set_clip", "cna_skinned_model_ext_remove_clip",
             "cna_skinned_model_ext_get_clip_count",
             "cna_skinned_model_ext_get_clip_name_byte_count_at",
             "cna_skinned_model_ext_copy_clip_name_at",
             "cna_skinned_model_ext_get_clip_info", "cna_skinned_model_ext_copy_clip_track",
             "cna_skinned_model_ext_compute_bone_transforms",
             "cna_skinned_model_ext_add_part", "cna_skinned_model_ext_attach_parts",
             "cna_skinned_model_ext_remove_part", "cna_skinned_model_ext_get_part_count",
             "cna_skinned_model_ext_get_part_name_byte_count_at",
             "cna_skinned_model_ext_copy_part_name_at", "cna_skinned_model_ext_get_part_at",
             "cna_skinned_model_ext_get_part_diffuse_color_at",
             "cna_skinned_model_ext_set_part_diffuse_color_at",
             "cna_skinned_model_ext_get_owned_resource_counts"},
    .tags = {"c", "binding", "graphics", "model", "skinning", "animation", "ownership"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses synthetic skeleton, clip and empty graphics buffers. It validates CPU "
                    "transforms and resource ownership without a shader, texture sampling, draw "
                    "call, screenshot or visual-quality claim.",
})
