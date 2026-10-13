// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.effect_annotations",
    .title = "Effect annotations in C",
    .description = "A deterministic C program creates immutable effect annotations from copied UTF-8 and raw "
                   "values, reads every typed view, and demonstrates ordered collection copies, exact-name lookup "
                   "and independent owned lifetimes.",
    .apis = {"cna_effect_annotation_create", "cna_effect_annotation_destroy",
             "cna_effect_annotation_get_info", "cna_effect_annotation_get_name_byte_count",
             "cna_effect_annotation_copy_name", "cna_effect_annotation_get_semantic_byte_count",
             "cna_effect_annotation_copy_semantic", "cna_effect_annotation_get_value_boolean",
             "cna_effect_annotation_get_value_int32", "cna_effect_annotation_get_value_single",
             "cna_effect_annotation_get_value_string_byte_count",
             "cna_effect_annotation_copy_value_string", "cna_effect_annotation_get_value_vector2",
             "cna_effect_annotation_get_value_vector3", "cna_effect_annotation_get_value_vector4",
             "cna_effect_annotation_get_value_matrix", "cna_effect_annotation_collection_create",
             "cna_effect_annotation_collection_destroy", "cna_effect_annotation_collection_get_count",
             "cna_effect_annotation_collection_add", "cna_effect_annotation_collection_get_at",
             "cna_effect_annotation_collection_find"},
    .tags = {"c", "binding", "graphics", "effect", "annotation", "reflection", "collection", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program using renderer-independent owned effect-reflection values; no game, "
                    "graphics device, drawing or visual inspection. Built only with CNA_BUILD_C_API=ON.",
})
