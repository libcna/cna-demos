// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.net.local_gamer_packets",
    .title = "Local network gamer packets in C",
    .description = "A renderer-free C program validates LocalNetworkGamer identity, voice/party policy, injected "
                   "packet queues, all receive overloads and every byte/PacketWriter send form.",
    .apis = {"cna_local_network_gamer_create_ext",
             "cna_local_network_gamer_get_is_data_available",
             "cna_local_network_gamer_get_signed_in_gamer",
             "cna_local_network_gamer_enable_send_voice",
             "cna_local_network_gamer_send_party_invites",
             "cna_local_network_gamer_receive_data", "cna_local_network_gamer_receive_data_at",
             "cna_local_network_gamer_receive_data_into_packet_reader",
             "cna_local_network_gamer_send_data", "cna_local_network_gamer_send_data_range",
             "cna_local_network_gamer_send_data_to",
             "cna_local_network_gamer_send_data_range_to",
             "cna_local_network_gamer_send_packet_writer",
             "cna_local_network_gamer_send_packet_writer_to",
             "cna_local_network_gamer_clear_packet_queue_ext",
             "cna_local_network_gamer_enqueue_packet_ext",
             "cna_network_session_create_with_local_gamers", "cna_network_session_get_gamer",
             "cna_network_session_add_remote_gamer_ext", "cna_network_session_destroy",
             "cna_network_gamer_create", "cna_network_gamer_destroy",
             "cna_packet_reader_create", "cna_packet_reader_copy_data_ext",
             "cna_packet_reader_destroy", "cna_packet_writer_create",
             "cna_packet_writer_write_single", "cna_packet_writer_destroy",
             "cna_signed_in_gamer_create_ext", "cna_signed_in_gamer_get_gamertag_size",
             "cna_signed_in_gamer_copy_gamertag", "cna_signed_in_gamer_destroy"},
    .tags = {"c", "binding", "net", "local-gamer", "packet", "send", "receive"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Single-process Local session and explicit in-memory packet injection; opens no socket or "
                    "renderer and does not claim wire delivery, voice audio or party-service invitations. One "
                    "offset destination-preservation check retains CNA-DEMOS-ISSUE-075.",
    .knownIssue = "CNA-DEMOS-ISSUE-075",
})
