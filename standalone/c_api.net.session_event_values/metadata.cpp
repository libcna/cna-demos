// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.net.session_event_values",
    .title = "Network session event values in C",
    .description = "A renderer-free C program initializes every versioned network-session event payload, validates "
                   "gamer identity, end reasons, canonical booleans and reserved bytes.",
    .apis = {"cna_game_ended_event_info_init", "cna_game_started_event_info_init",
             "cna_gamer_joined_event_info_init", "cna_gamer_left_event_info_init",
             "cna_host_changed_event_info_init", "cna_network_session_ended_event_info_init",
             "cna_write_leaderboards_event_info_init", "cna_network_gamer_create",
             "cna_network_gamer_destroy", "CNA_GameEndedEventInfo", "CNA_GameStartedEventInfo",
             "CNA_GamerJoinedEventInfo", "CNA_GamerLeftEventInfo", "CNA_HostChangedEventInfo",
             "CNA_NetworkSessionEndedEventInfo", "CNA_WriteLeaderboardsEventInfo"},
    .tags = {"c", "binding", "net", "session", "event", "value", "validation"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Pure event-payload initialization; creates no game, session, socket or renderer and does not "
                    "claim that any live session event was raised or dispatched.",
})
