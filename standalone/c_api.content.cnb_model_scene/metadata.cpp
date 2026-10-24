// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_model_scene",
    .title = "CNB model scene and codec in C",
    .description = "A renderer-free C program round-trips a model's mesh, skeleton, animation, light and camera "
                   "through a complete CNB document with exact nested ownership and removal behavior.",
    .apis = {"cna_cnb_model_add_mesh", "cna_cnb_model_get_mesh",
             "cna_cnb_model_get_mesh_name_size", "cna_cnb_model_copy_mesh_name",
             "cna_cnb_model_copy_mesh_part_indices", "cna_cnb_model_set_skeleton",
             "cna_cnb_model_clear_skeleton", "cna_cnb_model_get_skeleton",
             "cna_cnb_model_copy_skeleton_hierarchy", "cna_cnb_model_copy_skeleton_matrices",
             "cna_cnb_model_add_animation", "cna_cnb_model_get_animation_name_size",
             "cna_cnb_model_copy_animation_name", "cna_cnb_model_get_animation",
             "cna_cnb_model_get_animation_track", "cna_cnb_model_copy_animation_keyframes",
             "cna_cnb_model_add_light", "cna_cnb_model_get_light",
             "cna_cnb_model_add_camera", "cna_cnb_model_get_camera_count",
             "cna_cnb_model_get_camera", "cna_cnb_model_get_camera_name_size",
             "cna_cnb_model_copy_camera_name",
             "cna_cnb_encode_model", "cna_cnb_decode_model"},
    .tags = {"c", "binding", "content", "cnb", "model", "skeleton", "animation"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses a synthetic in-memory model and CNB document. No CNJ file, ContentManager, "
                    "graphics device, renderer, animation player or draw call is used.",
})
