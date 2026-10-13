// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.leaderboard_readers",
    .title = "Offline leaderboard readers in C",
    .description = "A renderer-free C program creates canonical leaderboard identities, completes every local "
                   "read shape, inspects empty pages and exercises synchronous and callback paging contracts.",
    .apis = {"cna_leaderboard_identity_init", "cna_leaderboard_reader_read",
             "cna_leaderboard_reader_read_from_pivot", "cna_leaderboard_reader_read_from_gamers",
             "cna_leaderboard_reader_begin_read", "cna_leaderboard_reader_begin_read_from_pivot",
             "cna_leaderboard_reader_begin_read_from_gamers", "cna_leaderboard_reader_destroy",
             "cna_leaderboard_reader_get_info", "cna_leaderboard_reader_get_identity",
             "cna_leaderboard_reader_get_entry_at", "cna_leaderboard_reader_page_down",
             "cna_leaderboard_reader_page_up", "cna_leaderboard_reader_begin_page_down",
             "cna_leaderboard_reader_begin_page_up", "cna_storage_set_app_name_ext",
             "cna_gamer_set_signed_in_gamers_ext", "cna_signed_in_gamer_create_ext",
             "cna_signed_in_gamer_destroy"},
    .tags = {"c", "binding", "gamer", "leaderboard", "reader", "paging", "async", "offline"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses a dedicated empty offline store; opens no game, renderer or service. Positive entry "
                    "lookup and page-down need persisted rows from a writer/service, which public C cannot "
                    "construct, so their deterministic empty-page refusals are demonstrated here.",
})
