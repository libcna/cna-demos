// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.model_animations",
    .title = "ModelAnimationsEXT collections and direct Model posing in C",
    .description = "A C workflow owns sorted scene clips, applies one to selected Model bones and "
                   "uploads a SkinningData bind palette to a real SkinnedEffect.",
    .apis = {"cna_model_animations_ext_create", "cna_model_animations_ext_destroy",
             "cna_model_animations_ext_get_type_name_byte_count",
             "cna_model_animations_ext_copy_type_name",
             "cna_model_animations_ext_get_clip_count",
             "cna_model_animations_ext_get_clip_name_byte_count_at",
             "cna_model_animations_ext_copy_clip_name_at",
             "cna_model_animations_ext_get_clip_info_at",
             "cna_model_animations_ext_set_clip_target_space_at",
             "cna_model_apply_clip_to_bones_ext",
             "cna_model_apply_bind_pose_bone_transforms_ext"},
    .tags = {"c", "binding", "graphics", "model", "skinning", "animation", "effect"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses synthetic clips and skeletons plus one empty ModelMeshPart solely to "
                    "inspect Model and SkinnedEffect transform state. No buffers, texture, draw "
                    "call, screenshot or visual-quality claim is involved.",
})
