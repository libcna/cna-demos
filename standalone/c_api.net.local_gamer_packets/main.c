/* SPDX-License-Identifier: MS-PL */
/* LocalNetworkGamer identity, packet queues and send/receive overloads through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int release_sender(CNA_NetworkGamerHandle* sender)
{
    if (*sender == CNA_INVALID_HANDLE) {
        return 1;
    }
    if (cna_network_gamer_destroy(*sender) != CNA_RESULT_SUCCESS) {
        return 0;
    }
    *sender = CNA_INVALID_HANDLE;
    return 1;
}

int main(int argc, char** argv)
{
    static const char Player[] = "Packet Player";
    static const char RemoteName[] = "Packet Sender";
    static const uint8_t Payload[4] = {UINT8_C(0x11), UINT8_C(0x22), UINT8_C(0x33), UINT8_C(0x44)};
    CnaDemo demo;
    CNA_SignedInGamerHandle signed_in = CNA_INVALID_HANDLE;
    CNA_SignedInGamerHandle backing = CNA_INVALID_HANDLE;
    CNA_NetworkSessionHandle session = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle local = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle standalone_local = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle remote = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle sender = CNA_INVALID_HANDLE;
    CNA_PacketReaderHandle reader = CNA_INVALID_HANDLE;
    CNA_PacketWriterHandle writer = CNA_INVALID_HANDLE;
    CNA_Handle locals[1];
    CNA_NetworkEventInfo event;
    CNA_Bool flag = CNA_FALSE;
    uint8_t buffer[12];
    uint64_t received = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    uint64_t bytes = UINT64_C(0);
    char gamertag[32];

    cna_demo_init(&demo, "c_api.net.local_gamer_packets", argc, argv, 1);
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                (CNA_StringView){Player, (uint64_t)(sizeof(Player) - 1U)},
                CNA_FALSE, CNA_FALSE, CNA_PLAYER_INDEX_ONE, &signed_in),
            "create the packet player's signed-in identity")) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo,
            cna_local_network_gamer_create_ext(
                signed_in, CNA_INVALID_HANDLE, &standalone_local),
            "create a LocalNetworkGamer outside any roster")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_local_network_gamer_get_is_data_available(standalone_local, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_local_network_gamer_get_signed_in_gamer(standalone_local, &backing) ==
                CNA_RESULT_SUCCESS &&
            backing != CNA_INVALID_HANDLE,
        "a standalone local gamer starts with an empty queue and exposes its backing identity");
    memset(gamertag, 0xA5, sizeof(gamertag));
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_get_gamertag_size(backing, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(Player) - 1U &&
            cna_signed_in_gamer_copy_gamertag(
                backing, gamertag, sizeof(gamertag), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(Player) - 1U && memcmp(gamertag, Player, (size_t)bytes) == 0 &&
            cna_signed_in_gamer_destroy(backing) == CNA_RESULT_SUCCESS,
        "the borrowed backing gamer has the exact signed-in gamertag and independent handle");
    backing = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_network_gamer_destroy(standalone_local) == CNA_RESULT_SUCCESS,
        "release the standalone local gamer before creating a session roster");
    standalone_local = CNA_INVALID_HANDLE;

    locals[0] = signed_in;
    if (!cna_demo_check_result(
            &demo,
            cna_network_session_create_with_local_gamers(
                CNA_NETWORK_SESSION_TYPE_LOCAL, locals, UINT64_C(1), 8, 0,
                CNA_INVALID_HANDLE, &session),
            "create a Local packet session") ||
        !cna_demo_check_result(
            &demo,
            cna_network_session_get_gamer(
                session, CNA_NETWORK_SESSION_ROSTER_LOCAL, 0, &local),
            "borrow the session's LocalNetworkGamer")) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo,
            cna_network_gamer_create(
                session, (CNA_StringView){RemoteName, (uint64_t)(sizeof(RemoteName) - 1U)},
                &remote),
            "create a same-session remote gamer") ||
        !cna_demo_check_result(
            &demo, cna_network_session_add_remote_gamer_ext(session, remote),
            "retain the remote gamer in the session")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_local_network_gamer_enable_send_voice(local, remote, CNA_FALSE) ==
                CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_enable_send_voice(local, remote, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_enable_send_voice(local, CNA_INVALID_HANDLE, CNA_TRUE) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_local_network_gamer_enable_send_voice(local, remote, (CNA_Bool)2) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "voice routing accepts a same-session remote and rejects absent or invalid inputs");
    cna_demo_check(
        &demo,
        cna_local_network_gamer_send_party_invites(local) == CNA_RESULT_INVALID_STATE,
        "party invitations are unavailable for a service-free Local session");

    memset(&event, 0, sizeof(event));
    event.struct_size = (uint32_t)sizeof(event);
    event.struct_version = UINT32_C(1);
    event.type = CNA_NETWORK_EVENT_TYPE_PACKET_SEND;
    event.reliable = CNA_SEND_DATA_OPTIONS_RELIABLE;
    event.state = CNA_NETWORK_SESSION_STATE_LOBBY;
    event.reason = CNA_NETWORK_SESSION_END_REASON_DISCONNECTED;
    event.gamer = remote;
    event.sender = CNA_INVALID_HANDLE;
    event.packet = Payload;
    event.packet_byte_count = (uint64_t)sizeof(Payload);
    cna_demo_check(
        &demo,
        cna_local_network_gamer_get_is_data_available(local, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_local_network_gamer_enqueue_packet_ext(local, &event) == CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_get_is_data_available(local, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "transport-style injection changes the local incoming-queue availability");
    memset(buffer, 0, sizeof(buffer));
    cna_demo_check(
        &demo,
        cna_local_network_gamer_receive_data(
            local, buffer, sizeof(Payload), &sender, &received) == CNA_RESULT_SUCCESS &&
            received == sizeof(Payload) && memcmp(buffer, Payload, sizeof(Payload)) == 0 &&
            sender != CNA_INVALID_HANDLE && release_sender(&sender),
        "buffer receive returns the exact packet and an independently releasable sender view");
    cna_demo_check(
        &demo,
        cna_local_network_gamer_receive_data(
            local, buffer, sizeof(Payload), &sender, &received) == CNA_RESULT_SUCCESS &&
            received == UINT64_C(0) && sender == CNA_INVALID_HANDLE,
        "an empty queue returns zero bytes and no sender");

    cna_demo_check(
        &demo,
        cna_local_network_gamer_enqueue_packet_ext(local, &event) == CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_receive_data_at(
                local, buffer, sizeof(Payload), 3, &sender, &received) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_local_network_gamer_get_is_data_available(local, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "an offset that leaves too little capacity is refused without consuming the packet");
    memset(buffer, 0xA5, sizeof(buffer));
    cna_demo_check(
        &demo,
        cna_local_network_gamer_receive_data_at(
            local, buffer, sizeof(buffer), 2, &sender, &received) == CNA_RESULT_SUCCESS &&
            received == sizeof(Payload) && memcmp(buffer + 2, Payload, sizeof(Payload)) == 0 &&
            release_sender(&sender) &&
            buffer[0] == UINT8_C(0xA5) && buffer[1] == UINT8_C(0xA5) &&
            buffer[6] == UINT8_C(0xA5) && buffer[7] == UINT8_C(0xA5),
        "offset receive writes the exact packet while preserving the prefix");

    if (!cna_demo_check_result(
            &demo, cna_packet_reader_create(0, &reader),
            "create the packet reader destination")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_local_network_gamer_enqueue_packet_ext(local, &event) == CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_receive_data_into_packet_reader(
                local, reader, &sender, &received) == CNA_RESULT_SUCCESS &&
            received == sizeof(Payload) && release_sender(&sender) &&
            cna_packet_reader_copy_data_ext(reader, buffer, sizeof(buffer), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == sizeof(Payload) && memcmp(buffer, Payload, sizeof(Payload)) == 0,
        "PacketReader receive sizes and copies the exact queued payload");
    cna_demo_check(
        &demo,
        cna_packet_reader_destroy(reader) == CNA_RESULT_SUCCESS,
        "release the packet reader after its copied payload");
    reader = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_local_network_gamer_enqueue_packet_ext(local, &event) == CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_clear_packet_queue_ext(local) == CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_get_is_data_available(local, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "explicit queue clearing removes every pending packet");

    if (!cna_demo_check_result(
            &demo, cna_packet_writer_create(0, &writer),
            "create a PacketWriter send source") ||
        !cna_demo_check_result(
            &demo, cna_packet_writer_write_single(writer, 1.5F),
            "write one scalar packet")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_local_network_gamer_send_data(
            local, Payload, sizeof(Payload), CNA_SEND_DATA_OPTIONS_RELIABLE) ==
                CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_send_data_range(
                local, Payload, sizeof(Payload), 1, 2, CNA_SEND_DATA_OPTIONS_IN_ORDER) ==
                CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_send_data_to(
                local, Payload, sizeof(Payload), CNA_SEND_DATA_OPTIONS_NONE, remote) ==
                CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_send_data_range_to(
                local, Payload, sizeof(Payload), 0, 4, CNA_SEND_DATA_OPTIONS_CHAT, remote) ==
                CNA_RESULT_SUCCESS,
        "all byte-array broadcast, range and recipient send forms accept exact inputs");
    cna_demo_check(
        &demo,
        cna_local_network_gamer_send_packet_writer(
            local, writer, CNA_SEND_DATA_OPTIONS_RELIABLE_IN_ORDER) == CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_send_packet_writer_to(
                local, writer, CNA_SEND_DATA_OPTIONS_RELIABLE, remote) == CNA_RESULT_SUCCESS,
        "both PacketWriter broadcast and recipient send forms accept the authored packet");
    cna_demo_check(
        &demo,
        cna_local_network_gamer_send_data(
            local, Payload, sizeof(Payload), UINT32_C(9)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_local_network_gamer_send_data(
                local, NULL, sizeof(Payload), CNA_SEND_DATA_OPTIONS_NONE) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_local_network_gamer_send_data_range(
                local, Payload, sizeof(Payload), 3, 2, CNA_SEND_DATA_OPTIONS_NONE) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "send validation rejects unknown options, missing bytes and an invalid range");
    cna_demo_check(
        &demo,
        cna_packet_writer_destroy(writer) == CNA_RESULT_SUCCESS &&
            cna_local_network_gamer_clear_packet_queue_ext(local) == CNA_RESULT_SUCCESS,
        "release the writer and clear any Local-session loopback packets");
    writer = CNA_INVALID_HANDLE;

cleanup:
    if (writer != CNA_INVALID_HANDLE) {
        (void)cna_packet_writer_destroy(writer);
    }
    if (reader != CNA_INVALID_HANDLE) {
        (void)cna_packet_reader_destroy(reader);
    }
    (void)release_sender(&sender);
    if (remote != CNA_INVALID_HANDLE) {
        (void)cna_network_gamer_destroy(remote);
    }
    if (local != CNA_INVALID_HANDLE) {
        (void)cna_network_gamer_destroy(local);
    }
    if (session != CNA_INVALID_HANDLE) {
        (void)cna_demo_check_result(
            &demo, cna_network_session_destroy(session),
            "release the Local packet session");
    }
    if (standalone_local != CNA_INVALID_HANDLE) {
        (void)cna_network_gamer_destroy(standalone_local);
    }
    if (backing != CNA_INVALID_HANDLE) {
        (void)cna_signed_in_gamer_destroy(backing);
    }
    if (signed_in != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo, cna_signed_in_gamer_destroy(signed_in) == CNA_RESULT_SUCCESS,
            "release the packet player's signed-in identity");
    }
    return cna_demo_finish(&demo);
}
