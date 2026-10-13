// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.matrix_values",
    .title = "Matrix values in C",
    .description = "A renderer-independent C program inspects Matrix storage and directional properties, "
                   "decomposes affine values, performs component and composition operations, and checks exact "
                   "formatting, transpose, inverse and quaternion rotation.",
    .apis = {"cna_matrix_init", "cna_matrix_init_values", "cna_matrix_get_identity",
             "cna_matrix_get_backward", "cna_matrix_set_backward", "cna_matrix_get_down",
             "cna_matrix_set_down", "cna_matrix_get_forward", "cna_matrix_set_forward",
             "cna_matrix_get_left", "cna_matrix_set_left", "cna_matrix_get_right",
             "cna_matrix_set_right", "cna_matrix_get_translation",
             "cna_matrix_set_translation", "cna_matrix_get_up", "cna_matrix_set_up",
             "cna_matrix_decompose", "cna_matrix_determinant", "cna_matrix_equals",
             "cna_matrix_not_equals", "cna_matrix_get_hash_code",
             "cna_matrix_get_string_size", "cna_matrix_copy_string", "cna_matrix_add",
             "cna_matrix_divide", "cna_matrix_divide_scalar", "cna_matrix_invert",
             "cna_matrix_lerp", "cna_matrix_multiply", "cna_matrix_multiply_scalar",
             "cna_matrix_negate", "cna_matrix_subtract", "cna_matrix_transpose",
             "cna_matrix_transform", "CNA_Matrix"},
    .tags = {"c", "binding", "math", "matrix", "decompose", "inverse", "value"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent C value workflow. Matrix factory families are covered "
                    "separately; this demo creates no game or graphics device.",
})
