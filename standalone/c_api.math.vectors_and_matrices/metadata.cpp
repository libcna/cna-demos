// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.vectors_and_matrices",
    .title = "Math in C: vectors and matrices",
    .description = "A C program uses the C ABI's value types and math routes without a game: builds a "
                   "scale-rotate-translate world matrix in XNA's row-vector order, transforms a point with it and "
                   "back with its inverse, transforms directions and bounded array ranges, checks refusal before writes, and prints as "
                   "Vector3.ToString does.",
    .apis = {"cna_vector3_init_xyz", "cna_vector3_cross", "cna_vector3_normalize", "cna_vector3_length",
             "cna_vector3_transform_matrix", "cna_vector3_transform_normal", "cna_vector3_transform_matrix_array", "cna_vector3_copy_string", "cna_matrix_create_scale_scalar",
             "cna_matrix_create_rotation_y", "cna_matrix_create_translation_xyz", "cna_matrix_multiply",
             "cna_matrix_invert", "CNA_Vector3", "CNA_Matrix"},
    .tags = {"c", "binding", "math", "matrix", "vector"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program; built only when CNA has its C API (CNA_BUILD_C_API=ON).",
})
