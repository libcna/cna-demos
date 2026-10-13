// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.effect_parameter_values",
    .title = "Effect parameter values in C",
    .description = "A deterministic C program creates a mutable effect parameter, round-trips every tagged scalar "
                   "and array value family, checks transposed matrices and atomic array/text copies, then proves "
                   "typed texture retention and release with a CPU-only texture.",
    .apis = {"cna_effect_parameter_create", "cna_effect_parameter_destroy",
             "cna_effect_parameter_get_info", "cna_effect_parameter_get_name_byte_count",
             "cna_effect_parameter_copy_name", "cna_effect_parameter_get_semantic_byte_count",
             "cna_effect_parameter_copy_semantic", "cna_effect_parameter_get_value",
             "cna_effect_parameter_get_values", "cna_effect_parameter_set_value",
             "cna_effect_parameter_set_values", "cna_effect_parameter_get_value_string_byte_count",
             "cna_effect_parameter_copy_value_string", "cna_effect_parameter_set_value_string",
             "cna_effect_parameter_get_value_texture", "cna_effect_parameter_set_value_texture",
             "cna_texture2d_create_cpu_only_rgba8", "cna_texture2d_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "parameter", "values", "arrays", "texture", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program using an ownerless mutable parameter and a CPU-only Texture2D; no game, "
                    "graphics device, shader execution, drawing or visual inspection. Built only with "
                    "CNA_BUILD_C_API=ON.",
})
