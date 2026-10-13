// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.signed_in_achievements",
    .title = "Signed-in gamer achievements in C",
    .description = "A renderer-free C program reads the offline title catalog, awards entries through synchronous "
                   "and callback adapters, and proves idempotent persistent earned state with fresh objects.",
    .apis = {"cna_signed_in_gamer_get_achievements",
             "cna_signed_in_gamer_begin_get_achievements",
             "cna_signed_in_gamer_award_achievement",
             "cna_signed_in_gamer_begin_award_achievement",
             "cna_signed_in_gamer_create_ext", "cna_signed_in_gamer_destroy",
             "cna_achievement_collection_get_count", "cna_achievement_collection_get_by_key",
             "cna_achievement_collection_destroy", "cna_achievement_get_info",
             "cna_achievement_get_name_size", "cna_achievement_copy_name",
             "cna_achievement_get_how_to_earn_size", "cna_achievement_copy_how_to_earn",
             "cna_achievement_destroy", "cna_storage_set_app_name_ext"},
    .tags = {"c", "binding", "gamer", "achievement", "catalog", "async", "persistence"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses the deployed three-entry offline achievement catalog and a dedicated persistent local "
                    "application store. It opens no game, renderer or online gamer service and does not repeat "
                    "the existing EarnedOnline assertion from CNA-DEMOS-ISSUE-017.",
})
