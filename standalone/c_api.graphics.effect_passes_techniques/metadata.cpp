// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.effect_passes_techniques",
    .title = "Effect passes and techniques in C",
    .description = "A deterministic C program demonstrates ownerless effect passes and techniques, reflected "
                   "indices, default P0 structure, annotation views, exact collection lookup, stable identities "
                   "and descendant lifetimes after their parent collections are released.",
    .apis = {"cna_effect_pass_create", "cna_effect_pass_create_indexed_ext",
             "cna_effect_pass_get_index_ext", "cna_effect_pass_destroy",
             "cna_effect_pass_get_name_byte_count", "cna_effect_pass_copy_name",
             "cna_effect_pass_get_annotations", "cna_effect_pass_apply",
             "cna_effect_pass_collection_create", "cna_effect_pass_collection_destroy",
             "cna_effect_pass_collection_get_count", "cna_effect_pass_collection_add_create",
             "cna_effect_pass_collection_get_at", "cna_effect_pass_collection_find",
             "cna_effect_technique_create_default", "cna_effect_technique_create_named",
             "cna_effect_technique_destroy", "cna_effect_technique_get_name_byte_count",
             "cna_effect_technique_copy_name", "cna_effect_technique_get_identity",
             "cna_effect_technique_get_index_ext", "cna_effect_technique_create_reflected_ext",
             "cna_effect_technique_get_passes", "cna_effect_technique_get_annotations",
             "cna_effect_technique_collection_create", "cna_effect_technique_collection_destroy",
             "cna_effect_technique_collection_get_count", "cna_effect_technique_collection_add_default",
             "cna_effect_technique_collection_add_named", "cna_effect_technique_collection_get_at",
             "cna_effect_technique_collection_find"},
    .tags = {"c", "binding", "graphics", "effect", "pass", "technique", "collection", "reflection", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program using ownerless reflection values. Apply is the documented successful "
                    "ownerless no-op; no effect-owned pass, game, graphics device, shader execution, drawing or "
                    "visual inspection. Built only with CNA_BUILD_C_API=ON.",
})
