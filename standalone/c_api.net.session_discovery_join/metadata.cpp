// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.net.session_discovery_join",
    .title = "Network session discovery and join in C",
    .description = "A renderer-free C program compares implicit, explicit and asynchronous discovery, joins a "
                   "synthetic Local description, and records the service-backed online and invitation refusals.",
    .apis = {"cna_network_session_find", "cna_network_session_find_with_local_gamers",
             "cna_network_session_find_async", "cna_network_session_find_with_local_gamers_async",
             "cna_network_session_join", "cna_network_session_join_async",
             "cna_network_session_join_invited",
             "cna_network_session_join_invited_with_local_gamers",
             "cna_network_session_join_invited_async",
             "cna_network_session_join_invited_with_local_gamers_async",
             "cna_available_network_session_create_ext",
             "cna_available_network_session_collection_get_count",
             "cna_available_network_session_collection_destroy",
             "cna_available_network_session_destroy", "cna_network_session_get_session_type",
             "cna_network_session_get_is_host", "cna_network_session_get_instance_count_ext",
             "cna_network_session_get_active_action_count_ext", "cna_network_session_destroy",
             "cna_signed_in_gamer_create_ext", "cna_gamer_set_signed_in_gamers_ext",
             "cna_gamer_get_signed_in_gamer_count", "cna_signed_in_gamer_destroy"},
    .tags = {"c", "binding", "net", "session", "discovery", "join", "async", "service"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Single-process synthetic/Local behavior only; opens no socket or renderer. Online search "
                    "and invitation paths truthfully demonstrate the no-service refusal; no authenticated "
                    "directory, accepted invitation, address reachability or remote peer is claimed.",
})
