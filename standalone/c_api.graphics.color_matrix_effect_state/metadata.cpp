// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.color_matrix_effect_state",
    .title = "ColorMatrixEffect finite state in C",
    .description = "A C game round-trips a finite row-major color transform and offset, proves atomic "
                   "rejection of non-finite values, inspects the Rec. 709 grayscale preset and reset, "
                   "and follows authored state through cloning and source destruction.",
    .apis = {"cna_color_matrix_effect_create",
             "cna_color_matrix_effect_get_matrix", "cna_color_matrix_effect_set_matrix",
             "cna_color_matrix_effect_get_offset", "cna_color_matrix_effect_set_offset",
             "cna_color_matrix_effect_set_grayscale", "cna_color_matrix_effect_reset",
             "cna_effect_clone", "cna_effect_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "color-matrix", "grayscale", "finite", "clone", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C game. State and apply are validated under each selected renderer, but no "
                    "SpriteBatch draw, transformed pixel or visual result is claimed. The transform affects "
                    "sprites only on SOFTWARE. Built only with CNA_BUILD_C_API=ON.",
})
