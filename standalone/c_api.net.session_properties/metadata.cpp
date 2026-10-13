// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.net.session_properties",
    .title = "Network session values in C",
    .description = "A renderer-free C program initializes network quality, reads the thread join diagnostic and "
                   "exercises all fixed session-property list, copy, enumeration and ownership behavior.",
    .apis = {"cna_quality_of_service_init", "cna_quality_of_service_init_measured",
             "cna_net_get_last_join_error", "cna_network_session_properties_create",
             "cna_network_session_properties_get_count",
             "cna_network_session_properties_get_is_read_only",
             "cna_network_session_properties_get_item",
             "cna_network_session_properties_set_item",
             "cna_network_session_properties_index_of",
             "cna_network_session_properties_insert",
             "cna_network_session_properties_remove_at", "cna_network_session_properties_add",
             "cna_network_session_properties_remove", "cna_network_session_properties_contains",
             "cna_network_session_properties_clear", "cna_network_session_properties_copy_to",
             "cna_network_session_properties_create_enumerator",
             "cna_network_session_property_enumerator_move_next",
             "cna_network_session_property_enumerator_get_current",
             "cna_network_session_property_enumerator_reset",
             "cna_network_session_property_enumerator_destroy",
             "cna_network_session_properties_destroy", "CNA_QualityOfService",
             "CNA_OptionalInt32"},
    .tags = {"c", "binding", "net", "session", "properties", "enumerator", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Pure value and in-memory collection behavior; creates no game, network session, socket or "
                    "renderer and does not claim measured host/network throughput or a real join failure.",
})
