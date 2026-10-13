// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.vector3_values",
    .title = "Vector3 values in C",
    .description = "A renderer-independent C program covers Vector3 construction and directions, arithmetic, "
                   "cross products, interpolation, reflection and point/normal transforms with bounded arrays.",
    .apis = {"cna_vector3_init", "cna_vector3_init_xyz", "cna_vector3_init_scalar",
             "cna_vector3_init_vector2_z", "cna_vector3_get_zero", "cna_vector3_get_one",
             "cna_vector3_get_unit_x", "cna_vector3_get_unit_y", "cna_vector3_get_unit_z",
             "cna_vector3_get_up", "cna_vector3_get_down", "cna_vector3_get_right",
             "cna_vector3_get_left", "cna_vector3_get_forward", "cna_vector3_get_backward",
             "cna_vector3_equals", "cna_vector3_not_equals", "cna_vector3_get_hash_code",
             "cna_vector3_length", "cna_vector3_length_squared",
             "cna_vector3_normalize_in_place", "cna_vector3_get_string_size",
             "cna_vector3_copy_string", "cna_vector3_add", "cna_vector3_barycentric",
             "cna_vector3_catmull_rom", "cna_vector3_clamp", "cna_vector3_cross",
             "cna_vector3_distance", "cna_vector3_distance_squared", "cna_vector3_divide",
             "cna_vector3_divide_scalar", "cna_vector3_dot", "cna_vector3_hermite",
             "cna_vector3_lerp", "cna_vector3_max", "cna_vector3_min",
             "cna_vector3_multiply", "cna_vector3_multiply_scalar", "cna_vector3_negate",
             "cna_vector3_normalize", "cna_vector3_reflect", "cna_vector3_smooth_step",
             "cna_vector3_subtract", "cna_vector3_transform_matrix",
             "cna_vector3_transform_matrix_array", "cna_vector3_transform_quaternion",
             "cna_vector3_transform_quaternion_array", "cna_vector3_transform_normal",
             "cna_vector3_transform_normal_array", "CNA_Vector3"},
    .tags = {"c", "binding", "math", "vector3", "cross", "transform", "array"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent C value workflow. It creates no game or graphics "
                    "device and makes no geometry rendering or SIMD-performance claim.",
})
