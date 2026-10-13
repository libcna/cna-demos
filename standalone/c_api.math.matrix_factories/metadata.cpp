// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.matrix_factories",
    .title = "Matrix factories in C",
    .description = "A renderer-independent C program constructs billboard, rotation, view, projection, "
                   "scale, planar, translation and world matrices and checks their row-vector semantics.",
    .apis = {"cna_matrix_create_billboard", "cna_matrix_create_constrained_billboard",
             "cna_matrix_create_from_axis_angle", "cna_matrix_create_from_quaternion",
             "cna_matrix_create_from_yaw_pitch_roll", "cna_matrix_create_look_at",
             "cna_matrix_create_orthographic", "cna_matrix_create_orthographic_off_center",
             "cna_matrix_create_perspective", "cna_matrix_create_perspective_field_of_view",
             "cna_matrix_create_perspective_off_center", "cna_matrix_create_rotation_x",
             "cna_matrix_create_rotation_y", "cna_matrix_create_rotation_z",
             "cna_matrix_create_scale_scalar", "cna_matrix_create_scale_xyz",
             "cna_matrix_create_scale_vector3", "cna_matrix_create_shadow",
             "cna_matrix_create_translation_xyz", "cna_matrix_create_translation_vector3",
             "cna_matrix_create_reflection", "cna_matrix_create_world"},
    .tags = {"c", "binding", "math", "matrix", "factory", "projection", "billboard"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent matrix-construction workflow. It creates no game or "
                    "graphics device and does not claim clipping or rendered geometry.",
})
