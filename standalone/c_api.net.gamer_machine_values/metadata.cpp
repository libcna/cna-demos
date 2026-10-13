// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.net.gamer_machine_values",
    .title = "Network gamer and machine values in C",
    .description = "A renderer-free C program creates a detached NetworkGamer and NetworkMachine, changes the "
                   "transport-owned value fields, copies machine state and verifies invalid live-session operations.",
    .apis = {"cna_network_gamer_create", "cna_network_gamer_get_has_left_session",
             "cna_network_gamer_set_has_left_session_ext", "cna_network_gamer_get_has_voice",
             "cna_network_gamer_get_id", "cna_network_gamer_set_id_ext",
             "cna_network_gamer_get_is_guest", "cna_network_gamer_get_is_host",
             "cna_network_gamer_set_is_host_ext", "cna_network_gamer_get_is_local",
             "cna_network_gamer_get_is_muted_by_local_user",
             "cna_network_gamer_get_is_private_slot", "cna_network_gamer_get_is_ready",
             "cna_network_gamer_set_is_ready", "cna_network_gamer_get_is_talking",
             "cna_network_gamer_copy_machine", "cna_network_gamer_set_machine",
             "cna_network_gamer_get_roundtrip_ticks",
             "cna_network_gamer_set_roundtrip_ticks_ext", "cna_network_gamer_get_session",
             "cna_network_gamer_destroy", "cna_network_machine_create",
             "cna_network_machine_get_gamer_count", "cna_network_machine_get_gamer",
             "cna_network_machine_remove_from_session", "cna_network_machine_destroy"},
    .tags = {"c", "binding", "net", "gamer", "machine", "values", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Detached value behavior only; creates no game, session, socket or renderer. Populated machine "
                    "collections, local gamers, ready publication and successful removal need a live session.",
})
