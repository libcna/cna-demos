// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.friend_collection",
    .title = "Friend gamer collections in C",
    .description = "A renderer-free C program creates two friend snapshots, reads their distinct status and "
                   "free-text presence, then indexes, copies, enumerates and mutates a retained collection.",
    .apis = {"cna_friend_gamer_create_ext", "cna_friend_gamer_get_info",
             "cna_friend_gamer_get_presence_size", "cna_friend_gamer_copy_presence",
             "cna_friend_collection_create_ext", "cna_friend_collection_get_is_disposed",
             "cna_gamer_collection_get_count", "cna_gamer_collection_get_at",
             "cna_gamer_collection_index_of", "cna_gamer_collection_contains",
             "cna_gamer_collection_copy_to", "cna_gamer_collection_add",
             "cna_gamer_collection_remove", "cna_gamer_collection_clear",
             "cna_gamer_collection_create_enumerator", "cna_gamer_enumerator_move_next",
             "cna_gamer_enumerator_get_current", "cna_gamer_enumerator_reset",
             "cna_gamer_enumerator_destroy", "cna_gamer_collection_destroy",
             "cna_gamer_destroy"},
    .tags = {"c", "binding", "gamer", "friend", "collection", "enumerator", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Synthetic friend fixtures only; opens no game, renderer or network service. Status and "
                    "presence values come from the explicit factory and do not claim a live account friend graph.",
})
