// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.net.session_events",
    .title = "Network session events in C",
    .description = "A renderer-free C program subscribes to every session event family and observes local roster "
                   "and state events raised by remote-gamer mutation and injected session events.",
    .apis = {"cna_network_session_send_network_event_ext",
             "cna_network_session_add_remote_gamer_ext", "cna_network_session_remove_gamer_ext",
             "cna_network_session_subscribe_game_started",
             "cna_network_session_subscribe_game_ended",
             "cna_network_session_subscribe_gamer_joined",
             "cna_network_session_subscribe_gamer_left",
             "cna_network_session_subscribe_host_changed",
             "cna_network_session_subscribe_session_ended",
             "cna_network_session_subscribe_write_arbitrated_leaderboard",
             "cna_network_session_subscribe_write_unarbitrated_leaderboard",
             "cna_network_session_subscribe_write_true_skill",
             "cna_network_session_subscribe_invite_accepted",
             "cna_network_session_unsubscribe", "cna_network_session_create_with_local_gamers",
             "cna_network_session_get_gamer", "cna_network_session_update",
             "cna_network_session_start_game", "cna_network_session_end_game",
             "cna_network_session_destroy", "cna_network_gamer_create",
             "cna_network_gamer_destroy", "cna_signed_in_gamer_create_ext",
             "cna_signed_in_gamer_destroy", "CNA_NetworkEventInfo"},
    .tags = {"c", "binding", "net", "session", "event", "subscription", "lifetime"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Single-process Local session with injected events; opens no socket or renderer. Leaderboard "
                    "and invite routes validate registration lifetime only because this workflow cannot raise them.",
})
