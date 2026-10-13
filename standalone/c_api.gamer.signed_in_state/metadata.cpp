// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.signed_in_state",
    .title = "Signed-in gamer state in C",
    .description = "A renderer-free C program inspects guest/service/player identity, mutates party and presence "
                   "state, reads privileges and walks the process signed-in collection by position and player.",
    .apis = {"cna_invite_accepted_event_info_init", "cna_gamer_presence_init",
             "cna_signed_in_gamer_get_is_guest", "cna_signed_in_gamer_get_is_signed_in_to_live",
             "cna_signed_in_gamer_get_party_size", "cna_signed_in_gamer_set_party_size",
             "cna_signed_in_gamer_get_player_index", "cna_signed_in_gamer_get_presence",
             "cna_signed_in_gamer_set_presence", "cna_signed_in_gamer_set_presence_mode_string_ext",
             "cna_signed_in_gamer_get_privileges", "cna_signed_in_gamer_is_friend",
             "cna_signed_in_gamer_get_friends", "cna_gamer_collection_get_count",
             "cna_gamer_collection_destroy", "cna_gamer_get_signed_in_gamer_at",
             "cna_gamer_signed_in_index_of", "cna_gamer_signed_in_contains",
             "cna_gamer_get_signed_in_gamer_at_player_index", "cna_signed_in_gamer_create_ext",
             "cna_gamer_set_signed_in_gamers_ext", "cna_gamer_get_signed_in_gamer_count",
             "cna_signed_in_gamer_destroy"},
    .tags = {"c", "binding", "gamer", "signed-in", "presence", "privileges", "collection"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Explicit single-process fixtures only; opens no game, renderer, microphone or network "
                    "service. Friendship and friend-list results are the truthful offline empty state; the "
                    "free-text presence extension has no service publication claim.",
})
