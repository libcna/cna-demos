// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.skinning_data_player",
    .title = "SkinningData values and AnimationPlayer transforms in C",
    .description = "A renderer-free C workflow owns skeleton and clip data, preserves rig-root "
                   "metadata and plays local, world and skin transforms with clamp and loop modes.",
    .apis = {"cna_skinning_data_create", "cna_skinning_data_destroy",
             "cna_skinning_data_get_type_name_byte_count", "cna_skinning_data_copy_type_name",
             "cna_skinning_data_get_bone_count", "cna_skinning_data_copy_skeleton_hierarchy",
             "cna_skinning_data_copy_bind_pose", "cna_skinning_data_copy_inverse_bind_pose",
             "cna_skinning_data_copy_skeleton_root_prefix", "cna_skinning_data_get_clip_count",
             "cna_skinning_data_get_clip_name_byte_count_at",
             "cna_skinning_data_copy_clip_name_at", "cna_skinning_data_get_clip_info",
             "cna_skinning_data_copy_clip_track", "cna_animation_player_create",
             "cna_animation_player_destroy", "cna_animation_player_start_clip",
             "cna_animation_player_update", "cna_animation_player_get_current_position",
             "cna_animation_player_get_current_clip_info",
             "cna_animation_player_get_current_clip_name_byte_count",
             "cna_animation_player_copy_current_clip_name",
             "cna_animation_player_copy_bone_transforms",
             "cna_animation_player_copy_world_transforms",
             "cna_animation_player_copy_skin_transforms",
             "cna_skinning_data_get_clip_target_space_ext",
             "cna_skinning_data_set_clip_target_space_ext",
             "cna_skinning_data_get_skeleton_root_node_index_ext",
             "cna_skinning_data_set_skeleton_root_node_index_ext",
             "cna_skinning_data_get_skeleton_root_name_byte_count_ext",
             "cna_skinning_data_copy_skeleton_root_name_ext",
             "cna_skinning_data_set_skeleton_root_name_ext"},
    .tags = {"c", "binding", "graphics", "model", "skinning", "animation", "playback"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses synthetic skeleton and clip data with CPU transform evaluation. No "
                    "Model, ContentManager, graphics device, renderer, draw call, screenshot or "
                    "visual-quality claim is involved.",
})
