// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.vector2_values",
    .title = "Vector2 values in C",
    .description = "A renderer-independent C program covers Vector2 construction, identity, arithmetic, "
                   "interpolation, reflection and point/normal transforms, including bounded array windows.",
    .apis = {"cna_vector2_init", "cna_vector2_init_xy", "cna_vector2_init_scalar",
             "cna_vector2_get_zero", "cna_vector2_get_one", "cna_vector2_get_unit_x",
             "cna_vector2_get_unit_y", "cna_vector2_equals", "cna_vector2_not_equals",
             "cna_vector2_get_hash_code", "cna_vector2_length", "cna_vector2_length_squared",
             "cna_vector2_normalize_in_place", "cna_vector2_get_string_size",
             "cna_vector2_copy_string", "cna_vector2_add", "cna_vector2_barycentric",
             "cna_vector2_catmull_rom", "cna_vector2_clamp", "cna_vector2_distance",
             "cna_vector2_distance_squared", "cna_vector2_divide",
             "cna_vector2_divide_scalar", "cna_vector2_dot", "cna_vector2_hermite",
             "cna_vector2_lerp", "cna_vector2_max", "cna_vector2_min",
             "cna_vector2_multiply", "cna_vector2_multiply_scalar", "cna_vector2_negate",
             "cna_vector2_normalize", "cna_vector2_reflect", "cna_vector2_smooth_step",
             "cna_vector2_subtract", "cna_vector2_transform_matrix",
             "cna_vector2_transform_matrix_array", "cna_vector2_transform_quaternion",
             "cna_vector2_transform_quaternion_array", "cna_vector2_transform_normal",
             "cna_vector2_transform_normal_array", "CNA_Vector2"},
    .tags = {"c", "binding", "math", "vector2", "interpolation", "transform", "array"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent C value workflow. It creates no game or graphics "
                    "device and makes no geometry rendering or SIMD-performance claim.",
})
