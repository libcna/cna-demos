// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.net.session_construction",
    .title = "Network session construction in C",
    .description = "A renderer-free C program compares Local session constructor overloads, publishes the "
                   "process signed-in collection, adds another local gamer and observes all three canonical "
                   "asynchronous completion adapters.",
    .apis = {"cna_network_session_create", "cna_network_session_create_with_properties",
             "cna_network_session_add_local_gamer", "cna_network_session_create_async",
             "cna_network_session_create_with_properties_async",
             "cna_network_session_create_with_local_gamers_async",
             "cna_signed_in_gamer_create_ext", "cna_gamer_set_signed_in_gamers_ext",
             "cna_gamer_get_signed_in_gamer_count", "cna_network_session_properties_create",
             "cna_network_session_properties_set_item", "cna_network_session_copy_session_properties",
             "cna_network_session_properties_get_item", "cna_network_session_properties_destroy",
             "cna_network_session_get_max_gamers", "cna_network_session_get_private_gamer_slots",
             "cna_network_session_get_gamer_count", "cna_network_session_get_instance_count_ext",
             "cna_network_session_get_active_action_count_ext", "cna_network_session_destroy",
             "cna_signed_in_gamer_destroy"},
    .tags = {"c", "binding", "net", "session", "constructor", "async", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Single-process Local sessions only; opens no socket or renderer. The asynchronous routes "
                    "drive CNA's canonical Begin/End adapters to completion inside each call; no concurrent, "
                    "remote-service or cross-machine behavior is claimed.",
})
