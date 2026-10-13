// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.leaderboard_entries",
    .title = "Leaderboard entry values in C",
    .description = "A renderer-free C program creates gamer-backed and anonymous leaderboard entries, observes "
                   "rating hooks and equality, and mutates an entry-owned property dictionary.",
    .apis = {"cna_leaderboard_entry_create_ext", "cna_leaderboard_entry_destroy",
             "cna_leaderboard_entry_get_info", "cna_leaderboard_entry_set_rating",
             "cna_leaderboard_entry_get_gamer", "cna_leaderboard_entry_get_columns",
             "cna_leaderboard_entry_set_rating_changed_hook_ext", "cna_leaderboard_entry_equals",
             "cna_property_dictionary_get_count", "cna_property_dictionary_set_int32",
             "cna_property_dictionary_get_int32", "cna_property_dictionary_destroy",
             "cna_gamer_get_gamertag_size", "cna_gamer_copy_gamertag", "cna_gamer_destroy",
             "cna_signed_in_gamer_create_ext", "cna_signed_in_gamer_destroy"},
    .tags = {"c", "binding", "gamer", "leaderboard", "entry", "columns", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Synthetic in-memory entries only; opens no game, renderer or service and does not write a "
                    "leaderboard. The absent-gamer output follows current deterministic initialization while "
                    "recording CNA-DEMOS-ISSUE-079's header mismatch.",
})
