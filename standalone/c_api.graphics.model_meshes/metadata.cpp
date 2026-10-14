// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.model_meshes",
    .title = "ModelMesh resources, collections and drawing in C",
    .description = "A C game retains real effect and buffer resources in a ModelMeshPart, audits "
                   "live mesh/effect collections and pixel-checks ModelMesh drawing.",
    .apis = {"cna_model_mesh_part_get_effect", "cna_model_mesh_part_set_effect",
             "cna_model_mesh_part_get_vertex_buffer", "cna_model_mesh_part_set_vertex_buffer",
             "cna_model_mesh_part_get_index_buffer", "cna_model_mesh_part_set_index_buffer",
             "cna_model_mesh_create", "cna_model_mesh_create_named", "cna_model_mesh_destroy",
             "cna_model_mesh_get_bounding_sphere", "cna_model_mesh_set_bounding_sphere",
             "cna_model_mesh_get_mesh_parts", "cna_model_mesh_get_effects",
             "cna_model_mesh_get_name_byte_count", "cna_model_mesh_copy_name",
             "cna_model_mesh_get_parent_bone", "cna_model_mesh_set_parent_bone",
             "cna_model_mesh_get_tag", "cna_model_mesh_set_tag", "cna_model_mesh_draw",
             "cna_model_mesh_collection_create", "cna_model_mesh_collection_destroy",
             "cna_model_mesh_collection_get_count", "cna_model_mesh_collection_get_at",
             "cna_model_mesh_collection_find", "cna_model_mesh_collection_contains",
             "cna_model_effect_collection_destroy", "cna_model_effect_collection_get_count",
             "cna_model_effect_collection_get_at", "cna_model_effect_collection_contains",
             "cna_model_effect_collection_add", "cna_model_effect_collection_remove"},
    .tags = {"c", "binding", "graphics", "model", "mesh", "effect", "buffer", "draw"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "CNA-DEMOS-ISSUE-081: a part effect attached before mesh construction is "
                    "absent from Effects until the public setter changes it. Uses a synthetic "
                    "triangle and one-pixel probes; no ContentManager, asset, top-level Model or "
                    "visual-quality claim is involved.",
})
