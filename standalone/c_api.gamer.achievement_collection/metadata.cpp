// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.achievement_collection",
    .title = "Achievement values and collections in C",
    .description = "A renderer-free C program reads exact achievement value fields and text, compares values, "
                   "then indexes, copies and mutates an owned achievement collection.",
    .apis = {"cna_achievement_create_ext", "cna_achievement_destroy",
             "cna_achievement_get_info", "cna_achievement_get_key_size",
             "cna_achievement_copy_key", "cna_achievement_get_name_size",
             "cna_achievement_copy_name", "cna_achievement_get_description_size",
             "cna_achievement_copy_description", "cna_achievement_get_how_to_earn_size",
             "cna_achievement_copy_how_to_earn", "cna_achievement_get_picture_size",
             "cna_achievement_copy_picture", "cna_achievement_equals",
             "cna_achievement_collection_create_ext", "cna_achievement_collection_destroy",
             "cna_achievement_collection_get_count", "cna_achievement_collection_get_is_disposed",
             "cna_achievement_collection_get_is_read_only", "cna_achievement_collection_get_at",
             "cna_achievement_collection_get_by_key", "cna_achievement_collection_index_of",
             "cna_achievement_collection_contains", "cna_achievement_collection_add",
             "cna_achievement_collection_insert", "cna_achievement_collection_remove_at",
             "cna_achievement_collection_remove", "cna_achievement_collection_clear",
             "cna_achievement_collection_copy_to"},
    .tags = {"c", "binding", "gamer", "achievement", "collection", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Synthetic achievement fixtures only; opens no game, renderer or gamer service. Factory "
                    "values have no configured picture or how-to-earn catalog text. The mutable collection's "
                    "contradictory read-only flag retains CNA-DEMOS-ISSUE-077; unknown-key error translation "
                    "is recorded as CNA-DEMOS-ISSUE-078.",
    .knownIssue = "CNA-DEMOS-ISSUE-077",
})
