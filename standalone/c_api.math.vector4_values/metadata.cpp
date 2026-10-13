// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.vector4_values",
    .title = "Vector4 values in C",
    .description = "A renderer-independent C program covers Vector4 construction, arithmetic and "
                   "interpolation, then contrasts homogeneous Vector2, Vector3 and Vector4 matrix and "
                   "quaternion transforms with bounded arrays.",
    .apis = {"cna_vector4_init", "cna_vector4_init_xyzw", "cna_vector4_init_vector2_zw",
             "cna_vector4_init_vector3_w", "cna_vector4_init_scalar", "cna_vector4_get_zero",
             "cna_vector4_get_one", "cna_vector4_get_unit_x", "cna_vector4_get_unit_y",
             "cna_vector4_get_unit_z", "cna_vector4_get_unit_w", "cna_vector4_equals",
             "cna_vector4_not_equals", "cna_vector4_get_hash_code", "cna_vector4_length",
             "cna_vector4_length_squared", "cna_vector4_normalize_in_place",
             "cna_vector4_get_string_size", "cna_vector4_copy_string", "cna_vector4_add",
             "cna_vector4_barycentric", "cna_vector4_catmull_rom", "cna_vector4_clamp",
             "cna_vector4_distance", "cna_vector4_distance_squared", "cna_vector4_divide",
             "cna_vector4_divide_scalar", "cna_vector4_dot", "cna_vector4_hermite",
             "cna_vector4_lerp", "cna_vector4_max", "cna_vector4_min",
             "cna_vector4_multiply", "cna_vector4_multiply_scalar", "cna_vector4_negate",
             "cna_vector4_normalize", "cna_vector4_smooth_step", "cna_vector4_subtract",
             "cna_vector4_transform_vector2_matrix", "cna_vector4_transform_vector3_matrix",
             "cna_vector4_transform_matrix", "cna_vector4_transform_matrix_array",
             "cna_vector4_transform_vector2_quaternion",
             "cna_vector4_transform_vector3_quaternion", "cna_vector4_transform_quaternion",
             "cna_vector4_transform_quaternion_array", "CNA_Vector4"},
    .tags = {"c", "binding", "math", "vector4", "homogeneous", "transform", "array"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent C value workflow. It creates no game or graphics "
                    "device and makes no projection, clipping or SIMD-performance claim.",
})
