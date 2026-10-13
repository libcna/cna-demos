/* SPDX-License-Identifier: MS-PL */
/* Local NetworkSession state, policy, roster and lifecycle through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_OptionalInt32 present(int32_t value)
{
    CNA_OptionalInt32 result = {CNA_TRUE, {0U, 0U, 0U}, 0};
    result.value = value;
    return result;
}

int main(int argc, char** argv)
{
    static const char Player[] = "Local State Player";
    static const char TypeName[] = "Microsoft.Xna.Framework.Net.NetworkSession";
    CnaDemo demo;
    CNA_SignedInGamerHandle signed_in = CNA_INVALID_HANDLE;
    CNA_NetworkSessionPropertiesHandle source_properties = CNA_INVALID_HANDLE;
    CNA_NetworkSessionPropertiesHandle copied_properties = CNA_INVALID_HANDLE;
    CNA_NetworkSessionHandle session = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle local = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle host = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle found = CNA_INVALID_HANDLE;
    CNA_Handle local_inputs[1];
    CNA_OptionalInt32 property;
    CNA_Bool flag = CNA_FALSE;
    CNA_NetworkSessionState state = UINT32_C(99);
    CNA_NetworkSessionType type = UINT32_C(99);
    int32_t number = -1;
    int64_t ticks = INT64_C(-1);
    float loss = -1.0F;
    uint64_t owned = UINT64_C(99);
    uint64_t bytes = UINT64_C(0);
    uint8_t gamer_id = UINT8_C(0);
    char text[64];
    char before[64];

    cna_demo_init(&demo, "c_api.net.local_session_state", argc, argv, 1);
    cna_demo_check(
        &demo,
        cna_network_session_get_instance_count_ext(&number) == CNA_RESULT_SUCCESS &&
            number == 0 &&
            cna_network_session_get_active_action_count_ext(&number) == CNA_RESULT_SUCCESS &&
            number == 0,
        "the isolated process begins without sessions or pending creation actions");
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                (CNA_StringView){Player, (uint64_t)(sizeof(Player) - 1U)},
                CNA_FALSE, CNA_FALSE, CNA_PLAYER_INDEX_ONE, &signed_in),
            "create the explicit signed-in gamer") ||
        !cna_demo_check_result(
            &demo, cna_network_session_properties_create(&source_properties),
            "create the source session-property list")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_session_properties_set_item(source_properties, 0, present(7)) ==
            CNA_RESULT_SUCCESS,
        "author the first property before session construction");
    local_inputs[0] = signed_in;
    if (!cna_demo_check_result(
            &demo,
            cna_network_session_create_with_local_gamers(
                CNA_NETWORK_SESSION_TYPE_LOCAL, local_inputs, UINT64_C(1), 8, 2,
                source_properties, &session),
            "create a Local session from the explicit gamer and copied properties")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_session_properties_destroy(source_properties) == CNA_RESULT_SUCCESS &&
            cna_network_session_get_instance_count_ext(&number) == CNA_RESULT_SUCCESS &&
            number == 1 &&
            cna_network_session_get_active_action_count_ext(&number) == CNA_RESULT_SUCCESS &&
            number == 0,
        "the session owns copied state and is the process's only completed instance");
    source_properties = CNA_INVALID_HANDLE;

    cna_demo_check(
        &demo,
        cna_network_session_get_is_disposed(session, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_network_session_get_session_state(session, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_NETWORK_SESSION_STATE_LOBBY &&
            cna_network_session_get_session_type(session, &type) == CNA_RESULT_SUCCESS &&
            type == CNA_NETWORK_SESSION_TYPE_LOCAL,
        "a new Local session is live in its lobby");
    cna_demo_check(
        &demo,
        cna_network_session_get_is_host(session, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_network_session_get_is_everyone_ready(session, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_network_session_get_bytes_per_second_received(session, &number) ==
                CNA_RESULT_SUCCESS &&
            number == 0 &&
            cna_network_session_get_bytes_per_second_sent(session, &number) ==
                CNA_RESULT_SUCCESS &&
            number == 0,
        "the local machine hosts one initially unready gamer with zero measured traffic");
    cna_demo_check(
        &demo,
        cna_network_session_get_gamer_count(
            session, CNA_NETWORK_SESSION_ROSTER_ALL, &number) == CNA_RESULT_SUCCESS &&
            number == 1 &&
            cna_network_session_get_gamer_count(
                session, CNA_NETWORK_SESSION_ROSTER_LOCAL, &number) == CNA_RESULT_SUCCESS &&
            number == 1 &&
            cna_network_session_get_gamer_count(
                session, CNA_NETWORK_SESSION_ROSTER_REMOTE, &number) == CNA_RESULT_SUCCESS &&
            number == 0 &&
            cna_network_session_get_gamer_count(
                session, CNA_NETWORK_SESSION_ROSTER_PREVIOUS, &number) == CNA_RESULT_SUCCESS &&
            number == 0,
        "all four roster views distinguish the one local participant");
    cna_demo_check(
        &demo,
        cna_network_session_get_gamer_count(session, UINT32_C(4), &number) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_network_session_get_gamer(
                session, CNA_NETWORK_SESSION_ROSTER_ALL, -1, &local) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            local == CNA_INVALID_HANDLE,
        "unknown rosters and invalid indices are refused without publishing a gamer view");
    if (!cna_demo_check_result(
            &demo,
            cna_network_session_get_gamer(
                session, CNA_NETWORK_SESSION_ROSTER_LOCAL, 0, &local),
            "borrow the local gamer from the session roster") ||
        !cna_demo_check_result(
            &demo, cna_network_session_get_host(session, &host),
            "borrow the session host")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_gamer_get_is_local(local, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_network_gamer_get_is_host(local, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_network_gamer_get_is_host(host, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "the roster entry and Host view expose the same local authority roles");
    cna_demo_check(
        &demo,
        cna_network_gamer_get_id(local, &gamer_id) == CNA_RESULT_SUCCESS &&
            cna_network_session_find_gamer_by_id(session, gamer_id, &found) ==
                CNA_RESULT_SUCCESS &&
            found != CNA_INVALID_HANDLE,
        "FindGamerById returns a live view for the roster's assigned identifier");
    cna_demo_check(
        &demo,
        cna_network_gamer_set_is_ready(local, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_network_session_get_is_everyone_ready(session, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_network_session_reset_ready(session) == CNA_RESULT_SUCCESS &&
            cna_network_session_get_is_everyone_ready(session, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "local ready publication and host reset change the aggregate ready state");
    cna_demo_check(
        &demo,
        cna_network_session_get_owned_gamer_count_ext(session, &owned) ==
                CNA_RESULT_SUCCESS &&
            owned == UINT64_C(1) &&
            cna_network_session_destroy(session) == CNA_RESULT_INVALID_STATE,
        "one session-owned gamer is retained and borrowed views prevent premature destruction");

    cna_demo_check(
        &demo,
        cna_network_session_get_allow_host_migration(session, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_network_session_set_allow_host_migration(session, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_get_allow_host_migration(session, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_network_session_get_allow_join_in_progress(session, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_network_session_set_allow_join_in_progress(session, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_get_allow_join_in_progress(session, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "host migration and join-in-progress policies mutate independently");
    cna_demo_check(
        &demo,
        cna_network_session_set_allow_host_migration(session, (CNA_Bool)2) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_network_session_set_allow_join_in_progress(session, (CNA_Bool)2) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "both policy setters reject noncanonical booleans");
    cna_demo_check(
        &demo,
        cna_network_session_get_max_gamers(session, &number) == CNA_RESULT_SUCCESS &&
            number == 8 &&
            cna_network_session_set_max_gamers(session, 6) == CNA_RESULT_SUCCESS &&
            cna_network_session_get_max_gamers(session, &number) == CNA_RESULT_SUCCESS &&
            number == 6 &&
            cna_network_session_get_private_gamer_slots(session, &number) ==
                CNA_RESULT_SUCCESS &&
            number == 2 &&
            cna_network_session_set_private_gamer_slots(session, 1) == CNA_RESULT_SUCCESS &&
            cna_network_session_get_private_gamer_slots(session, &number) ==
                CNA_RESULT_SUCCESS &&
            number == 1,
        "host slot policies retain valid changed limits");
    cna_demo_check(
        &demo,
        cna_network_session_set_max_gamers(session, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_network_session_set_private_gamer_slots(session, 6) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_network_session_get_max_gamers(session, &number) == CNA_RESULT_SUCCESS &&
            number == 6,
        "invalid slot limits are refused without changing the accepted maximum");

    if (!cna_demo_check_result(
            &demo,
            cna_network_session_copy_session_properties(session, &copied_properties),
            "copy the session's current property values")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_session_properties_get_item(copied_properties, 0, &property) ==
                CNA_RESULT_SUCCESS &&
            property.has_value == CNA_TRUE && property.value == 7 &&
            cna_network_session_properties_set_item(copied_properties, 0, present(23)) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_replace_session_properties(session, copied_properties) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_properties_destroy(copied_properties) == CNA_RESULT_SUCCESS,
        "copy-mutate-replace changes session properties without exposing an internal reference");
    copied_properties = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_network_session_copy_session_properties(session, &copied_properties) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_properties_get_item(copied_properties, 0, &property) ==
                CNA_RESULT_SUCCESS &&
            property.has_value == CNA_TRUE && property.value == 23,
        "a fresh property copy observes the replaced value");

    cna_demo_check(
        &demo,
        cna_network_session_get_simulated_latency_ticks(session, &ticks) ==
                CNA_RESULT_SUCCESS &&
            ticks == INT64_C(0) &&
            cna_network_session_set_simulated_latency_ticks(session, INT64_C(123456)) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_get_simulated_latency_ticks(session, &ticks) ==
                CNA_RESULT_SUCCESS &&
            ticks == INT64_C(123456) &&
            cna_network_session_get_simulated_packet_loss(session, &loss) ==
                CNA_RESULT_SUCCESS &&
            loss == 0.0F &&
            cna_network_session_set_simulated_packet_loss(session, 0.25F) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_get_simulated_packet_loss(session, &loss) ==
                CNA_RESULT_SUCCESS &&
            loss == 0.25F,
        "simulated latency and packet loss retain exact configured values");
    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(before));
    cna_demo_check(
        &demo,
        cna_network_session_get_type_name_size(session, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(TypeName) - 1U &&
            cna_network_session_copy_type_name(session, text, bytes - 1U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == sizeof(TypeName) - 1U && memcmp(text, before, sizeof(text)) == 0 &&
            cna_network_session_copy_type_name(session, text, sizeof(text), &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == sizeof(TypeName) - 1U && memcmp(text, TypeName, (size_t)bytes) == 0,
        "the fixed NetworkSession type name is reported and copied exactly and atomically");

    cna_demo_check(
        &demo,
        cna_network_session_start_game(session) == CNA_RESULT_SUCCESS &&
            cna_network_session_get_session_state(session, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_NETWORK_SESSION_STATE_LOBBY &&
            cna_network_session_update(session) == CNA_RESULT_SUCCESS &&
            cna_network_session_get_session_state(session, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_NETWORK_SESSION_STATE_PLAYING,
        "StartGame queues a transition that Update applies from lobby to playing");
    cna_demo_check(
        &demo,
        cna_network_session_end_game(session) == CNA_RESULT_SUCCESS &&
            cna_network_session_update(session) == CNA_RESULT_SUCCESS &&
            cna_network_session_get_session_state(session, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_NETWORK_SESSION_STATE_LOBBY,
        "EndGame and Update return the session to its lobby");

cleanup:
    if (copied_properties != CNA_INVALID_HANDLE) {
        (void)cna_network_session_properties_destroy(copied_properties);
    }
    if (found != CNA_INVALID_HANDLE) {
        (void)cna_network_gamer_destroy(found);
    }
    if (host != CNA_INVALID_HANDLE) {
        (void)cna_network_gamer_destroy(host);
    }
    if (local != CNA_INVALID_HANDLE) {
        (void)cna_network_gamer_destroy(local);
    }
    if (session != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo,
            cna_network_session_dispose(session) == CNA_RESULT_SUCCESS &&
                cna_network_session_dispose(session) == CNA_RESULT_SUCCESS &&
                cna_network_session_get_is_disposed(session, &flag) == CNA_RESULT_SUCCESS &&
                flag == CNA_TRUE &&
                cna_network_session_update(session) == CNA_RESULT_INVALID_STATE,
            "session disposal is idempotent and prevents further updates");
        cna_demo_check(
            &demo,
            cna_network_session_destroy(session) == CNA_RESULT_SUCCESS &&
                cna_network_session_destroy(session) == CNA_RESULT_INVALID_HANDLE &&
                cna_network_session_get_instance_count_ext(&number) == CNA_RESULT_SUCCESS &&
                number == 0,
            "the disposed session releases exactly once and leaves no process instance");
    }
    if (source_properties != CNA_INVALID_HANDLE) {
        (void)cna_network_session_properties_destroy(source_properties);
    }
    if (signed_in != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo, cna_signed_in_gamer_destroy(signed_in) == CNA_RESULT_SUCCESS,
            "release the signed-in gamer after its session");
    }
    return cna_demo_finish(&demo);
}
