// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.model_mesh_parts",
    .title = "ModelMeshPart values and collections in C",
    .description = "A renderer-free C program exercises standalone mesh-part scalars, tags, "
                   "topologies, seven sampler slots and retained collection views.",
    .apis = {"cna_model_mesh_part_create_default", "cna_model_mesh_part_create",
             "cna_model_mesh_part_destroy", "cna_model_mesh_part_get_num_vertices",
             "cna_model_mesh_part_set_num_vertices", "cna_model_mesh_part_get_primitive_count",
             "cna_model_mesh_part_set_primitive_count", "cna_model_mesh_part_get_start_index",
             "cna_model_mesh_part_set_start_index", "cna_model_mesh_part_get_vertex_offset",
             "cna_model_mesh_part_set_vertex_offset", "cna_model_mesh_part_get_tag",
             "cna_model_mesh_part_set_tag", "cna_model_mesh_part_collection_create",
             "cna_model_mesh_part_collection_destroy",
             "cna_model_mesh_part_collection_get_count",
             "cna_model_mesh_part_collection_get_at",
             "cna_model_mesh_part_get_primitive_type_ext",
             "cna_model_mesh_part_set_primitive_type_ext",
             "cna_model_mesh_part_get_sampler_state_ext",
             "cna_model_mesh_part_set_sampler_state_ext"},
    .tags = {"c", "binding", "graphics", "model", "mesh-part", "sampler", "collection"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses standalone ModelMeshPart values without attached effects or buffers. "
                    "No Model, mesh, graphics device, renderer or draw call is involved.",
})
