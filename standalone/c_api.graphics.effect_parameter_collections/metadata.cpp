// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.effect_parameter_collections",
    .title = "Effect parameter collections in C",
    .description = "A deterministic C program grows a mutable effect-parameter collection, proves that index, "
                   "name and semantic lookups share stable state, then builds independent element, structure-member "
                   "and annotation collections whose children outlive their parent views.",
    .apis = {"cna_effect_parameter_get_elements", "cna_effect_parameter_get_structure_members",
             "cna_effect_parameter_get_annotations", "cna_effect_parameter_collection_create",
             "cna_effect_parameter_collection_destroy", "cna_effect_parameter_collection_get_count",
             "cna_effect_parameter_collection_add_create", "cna_effect_parameter_collection_get_at",
             "cna_effect_parameter_collection_find_name", "cna_effect_parameter_collection_find_semantic"},
    .tags = {"c", "binding", "graphics", "effect", "parameter", "collection", "reflection", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program using renderer-independent mutable effect-reflection values. "
                    "CNA-DEMOS-ISSUE-068 records that the C header calls semantic matching exact although the "
                    "underlying documented behavior ignores case. No game, graphics device, shader execution, "
                    "drawing or visual inspection. Built only with CNA_BUILD_C_API=ON.",
})
