// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.curve_key_collection",
    .title = "CurveKeyCollection ownership in C",
    .description = "A renderer-independent C program builds a sorted key collection, replaces and "
                   "copies entries, then demonstrates clone, removal and handle lifetime behavior.",
    .apis = {"cna_curve_key_collection_create", "cna_curve_key_collection_destroy",
             "cna_curve_key_collection_get_count", "cna_curve_key_collection_get_is_read_only",
             "cna_curve_key_collection_get", "cna_curve_key_collection_set",
             "cna_curve_key_collection_add", "cna_curve_key_collection_clear",
             "cna_curve_key_collection_clone", "cna_curve_key_collection_contains",
             "cna_curve_key_collection_copy_to", "cna_curve_key_collection_index_of",
             "cna_curve_key_collection_remove", "cna_curve_key_collection_remove_at",
             "CNA_CurveKeyCollectionHandle", "CNA_CurveKey"},
    .tags = {"c", "binding", "math", "curve", "collection", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent owned collection workflow. Curve-owned borrowed "
                    "views and evaluation are covered separately.",
})
