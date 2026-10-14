// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.model_bones",
    .title = "ModelBone graphs and collections in C",
    .description = "A renderer-free C program builds a retained bone hierarchy, mutates local "
                   "transforms and exercises live collection lookup and ownership.",
    .apis = {"cna_model_bone_create_default", "cna_model_bone_create",
             "cna_model_bone_destroy", "cna_model_bone_get_name_byte_count",
             "cna_model_bone_copy_name", "cna_model_bone_get_index",
             "cna_model_bone_get_transform", "cna_model_bone_set_transform",
             "cna_model_bone_get_parent", "cna_model_bone_get_children",
             "cna_model_bone_add_child", "cna_model_bone_collection_create",
             "cna_model_bone_collection_destroy", "cna_model_bone_collection_get_count",
             "cna_model_bone_collection_get_at", "cna_model_bone_collection_find",
             "cna_model_bone_collection_contains"},
    .tags = {"c", "binding", "graphics", "model", "bone", "hierarchy", "collection"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses standalone ModelBone values only. No Model, mesh, ContentManager, "
                    "graphics device, renderer, animation player or draw call is involved.",
})
