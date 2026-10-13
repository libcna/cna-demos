// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.net.available_sessions",
    .title = "Discovered network session values in C",
    .description = "A renderer-free C program creates discovered-session descriptions, inspects exact copied "
                   "metadata and exercises value equality, collection snapshots and disposal.",
    .apis = {"cna_available_network_session_create_ext",
             "cna_available_network_session_get_current_gamer_count",
             "cna_available_network_session_get_host_gamertag_size",
             "cna_available_network_session_copy_host_gamertag",
             "cna_available_network_session_get_open_private_gamer_slots",
             "cna_available_network_session_get_open_public_gamer_slots",
             "cna_available_network_session_get_quality_of_service",
             "cna_available_network_session_copy_session_properties",
             "cna_available_network_session_equals", "cna_available_network_session_not_equals",
             "cna_available_network_session_get_connect_address_size_ext",
             "cna_available_network_session_copy_connect_address_ext",
             "cna_available_network_session_get_connect_port_ext",
             "cna_available_network_session_get_session_type_ext",
             "cna_available_network_session_destroy",
             "cna_available_network_session_collection_create_ext",
             "cna_available_network_session_collection_get_count",
             "cna_available_network_session_collection_copy_session",
             "cna_available_network_session_collection_get_is_disposed",
             "cna_available_network_session_collection_dispose",
             "cna_available_network_session_collection_destroy",
             "CNA_AvailableNetworkSessionCreateInfo"},
    .tags = {"c", "binding", "net", "discovery", "session", "collection", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Synthetic discovered-session values only; performs no discovery, join, socket or renderer "
                    "work and does not claim that an advertised address is reachable.",
})
