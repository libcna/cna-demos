// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.model_scene_metadata",
    .title = "Model cameras, skins, bounds and infinite projection in C",
    .description = "A C workflow authors imported-camera values, maps a retained skeleton to a "
                   "real Model mesh and derives its bone-placed aggregate bounding sphere.",
    .apis = {"cna_model_get_camera_count_ext", "cna_model_get_camera_ext",
             "cna_model_get_camera_name_byte_count_ext", "cna_model_copy_camera_name_ext",
             "cna_model_clear_cameras_ext", "cna_model_add_camera_ext",
             "cna_model_get_skin_count_ext", "cna_model_get_skin_ext",
             "cna_model_create_skin_skeleton_handle_ext",
             "cna_model_get_skin_name_byte_count_ext", "cna_model_copy_skin_name_ext",
             "cna_model_get_skin_mesh_index_ext", "cna_model_clear_skins_ext",
             "cna_model_add_skin_ext", "cna_model_get_bounding_sphere_ext",
             "cna_matrix_create_infinite_perspective_field_of_view_ext"},
    .tags = {"c", "binding", "graphics", "model", "camera", "skinning", "bounds"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses a synthetic camera, one-bone skeleton and empty ModelMesh solely for "
                    "scene metadata and bounds. No material, buffer, texture, draw call, "
                    "screenshot or visual-quality claim is involved.",
})
