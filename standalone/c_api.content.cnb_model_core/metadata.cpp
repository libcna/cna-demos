// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_model_core",
    .title = "CNB model core data in C",
    .description = "A renderer-free C program builds and reads a neutral model graph's flags, bones, part bytes, "
                   "material values, eight texture names and seven importer-state slots.",
    .apis = {"cna_cnb_model_create", "cna_cnb_model_destroy", "cna_cnb_model_get_info",
             "cna_cnb_model_set_flags", "cna_cnb_model_add_bone", "cna_cnb_model_get_bone",
             "cna_cnb_model_get_bone_name_size", "cna_cnb_model_copy_bone_name",
             "cna_cnb_model_add_part", "cna_cnb_model_get_part", "cna_cnb_model_set_part",
             "cna_cnb_model_get_part_name_size", "cna_cnb_model_copy_part_name",
             "cna_cnb_model_get_part_external_effect_size",
             "cna_cnb_model_copy_part_external_effect",
             "cna_cnb_model_set_part_vertex_bytes", "cna_cnb_model_copy_part_vertex_bytes",
             "cna_cnb_model_set_part_index_bytes", "cna_cnb_model_copy_part_index_bytes",
             "cna_cnb_model_get_material", "cna_cnb_model_set_material",
             "cna_cnb_model_get_material_texture_size",
             "cna_cnb_model_copy_material_texture", "cna_cnb_model_set_material_texture",
             "cna_cnb_model_get_material_texture_coordinate_set",
             "cna_cnb_model_set_material_texture_coordinate_set",
             "cna_cnb_model_get_material_texture_transform",
             "cna_cnb_model_set_material_texture_transform",
             "cna_cnb_model_get_material_sampler", "cna_cnb_model_set_material_sampler"},
    .tags = {"c", "binding", "content", "cnb", "model", "material", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Pure neutral model data: no document codec, ContentManager, graphics device, renderer or "
                    "draw call is used. Morph, mesh, skeleton, animation and light data remain separate.",
})
