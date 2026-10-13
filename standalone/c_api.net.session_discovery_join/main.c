/* SPDX-License-Identifier: MS-PL */
/* NetworkSession discovery, Join and invitation adapters through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* data, uint64_t byte_length)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = byte_length;
    return value;
}

static CNA_AvailableNetworkSessionCreateInfo make_available_info(CNA_NetworkSessionType type)
{
    static const char Host[] = "Discovery Host";
    static const char Address[] = "127.0.0.1";
    CNA_AvailableNetworkSessionCreateInfo info;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.current_gamer_count = 1;
    info.open_private_gamer_slots = 1;
    info.open_public_gamer_slots = 6;
    info.session_type = type;
    info.host_port = UINT16_C(27015);
    info.host_gamertag = text_view(Host, sizeof(Host) - 1U);
    info.host_address = text_view(Address, sizeof(Address) - 1U);
    info.session_properties = CNA_INVALID_HANDLE;
    return info;
}

static void on_completed(void* context)
{
    ++*(int*)context;
}

static int collection_is_empty(CNA_AvailableNetworkSessionCollectionHandle collection)
{
    int32_t count = -1;
    return cna_available_network_session_collection_get_count(collection, &count) ==
               CNA_RESULT_SUCCESS &&
        count == 0;
}

int main(int argc, char** argv)
{
    static const char Player[] = "Discovery Player";
    CnaDemo demo;
    CNA_SignedInGamerHandle signed_in = CNA_INVALID_HANDLE;
    CNA_SignedInGamerHandle local_gamers[1];
    CNA_AvailableNetworkSessionCollectionHandle collection = CNA_INVALID_HANDLE;
    CNA_AvailableNetworkSessionHandle available = CNA_INVALID_HANDLE;
    CNA_NetworkSessionHandle session = CNA_INVALID_HANDLE;
    CNA_AvailableNetworkSessionCreateInfo info;
    CNA_NetworkSessionType type = UINT32_C(99);
    CNA_Bool flag = CNA_TRUE;
    int completions = 0;
    int32_t count = -1;

    cna_demo_init(&demo, "c_api.net.session_discovery_join", argc, argv, 1);
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(Player, sizeof(Player) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_ONE, &signed_in),
            "create the discovery signed-in gamer")) {
        goto cleanup;
    }
    local_gamers[0] = signed_in;
    cna_demo_check(
        &demo,
        cna_gamer_set_signed_in_gamers_ext(local_gamers, UINT64_C(1)) ==
                CNA_RESULT_SUCCESS &&
            cna_gamer_get_signed_in_gamer_count(&count) == CNA_RESULT_SUCCESS && count == 1,
        "publish the one local gamer used by implicit discovery and join overloads");

    collection = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_network_session_find(
            CNA_NETWORK_SESSION_TYPE_LOCAL, 1, CNA_INVALID_HANDLE, &collection) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            collection == CNA_INVALID_HANDLE,
        "ordinary Local sessions are not discoverable and publish no collection");
    collection = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_network_session_find(
            CNA_NETWORK_SESSION_TYPE_PLAYER_MATCH, 1, CNA_INVALID_HANDLE, &collection) ==
                CNA_RESULT_NOT_SUPPORTED &&
            collection == CNA_INVALID_HANDLE,
        "online discovery is refused when no CNA service is configured");

    if (cna_demo_check_result(
            &demo,
            cna_network_session_find(
                CNA_NETWORK_SESSION_TYPE_LOCAL_WITH_LEADERBOARDS, 1, CNA_INVALID_HANDLE,
                &collection),
            "find LocalWithLeaderboards sessions for the published gamer")) {
        cna_demo_check(
            &demo, collection_is_empty(collection),
            "single-machine LocalWithLeaderboards discovery returns an empty collection");
        cna_demo_check_result(
            &demo, cna_available_network_session_collection_destroy(collection),
            "release the implicit discovery result");
        collection = CNA_INVALID_HANDLE;
    }

    if (cna_demo_check_result(
            &demo,
            cna_network_session_find_with_local_gamers(
                CNA_NETWORK_SESSION_TYPE_LOCAL_WITH_LEADERBOARDS, local_gamers,
                UINT64_C(1), CNA_INVALID_HANDLE, &collection),
            "find LocalWithLeaderboards sessions for an explicit gamer list")) {
        cna_demo_check(
            &demo, collection_is_empty(collection),
            "explicit-gamer discovery returns the same empty local-only result");
        cna_demo_check_result(
            &demo, cna_available_network_session_collection_destroy(collection),
            "release the explicit discovery result");
        collection = CNA_INVALID_HANDLE;
    }

    if (cna_demo_check_result(
            &demo,
            cna_network_session_find_async(
                CNA_NETWORK_SESSION_TYPE_LOCAL_WITH_LEADERBOARDS, 1, CNA_INVALID_HANDLE,
                on_completed, &completions, &collection),
            "find through the implicit asynchronous adapter")) {
        cna_demo_check(
            &demo, completions == 1 && collection_is_empty(collection),
            "implicit asynchronous discovery completes once before returning its empty result");
        cna_demo_check_result(
            &demo, cna_available_network_session_collection_destroy(collection),
            "release the implicit asynchronous result");
        collection = CNA_INVALID_HANDLE;
    }

    if (cna_demo_check_result(
            &demo,
            cna_network_session_find_with_local_gamers_async(
                CNA_NETWORK_SESSION_TYPE_LOCAL_WITH_LEADERBOARDS, local_gamers,
                UINT64_C(1), CNA_INVALID_HANDLE, on_completed, &completions, &collection),
            "find through the explicit-gamer asynchronous adapter")) {
        cna_demo_check(
            &demo, completions == 2 && collection_is_empty(collection),
            "explicit asynchronous discovery completes once before returning its empty result");
        cna_demo_check_result(
            &demo, cna_available_network_session_collection_destroy(collection),
            "release the explicit asynchronous result");
        collection = CNA_INVALID_HANDLE;
    }

    info = make_available_info(CNA_NETWORK_SESSION_TYPE_PLAYER_MATCH);
    if (cna_demo_check_result(
            &demo, cna_available_network_session_create_ext(&info, NULL, &available),
            "create a synthetic online discovered-session description")) {
        session = UINT64_C(99);
        cna_demo_check(
            &demo,
            cna_network_session_join(available, &session) == CNA_RESULT_INVALID_STATE &&
                session == CNA_INVALID_HANDLE &&
                cna_network_session_join_async(
                    available, on_completed, &completions, &session) ==
                    CNA_RESULT_INVALID_STATE &&
                session == CNA_INVALID_HANDLE && completions == 2,
            "descriptive online metadata cannot replace an authenticated live search result");
        cna_demo_check_result(
            &demo, cna_available_network_session_destroy(available),
            "release the synthetic online description");
        available = CNA_INVALID_HANDLE;
    }

    info = make_available_info(CNA_NETWORK_SESSION_TYPE_LOCAL);
    if (!cna_demo_check_result(
            &demo, cna_available_network_session_create_ext(&info, NULL, &available),
            "create a synthetic Local discovered-session description")) {
        goto cleanup;
    }
    if (cna_demo_check_result(
            &demo, cna_network_session_join(available, &session),
            "join the synthetic Local description")) {
        cna_demo_check(
            &demo,
            cna_network_session_get_session_type(session, &type) == CNA_RESULT_SUCCESS &&
                type == CNA_NETWORK_SESSION_TYPE_LOCAL &&
                cna_network_session_get_is_host(session, &flag) == CNA_RESULT_SUCCESS &&
                flag == CNA_FALSE,
            "the generic join preserves Local type and marks this process as a joining peer");
        cna_demo_check_result(
            &demo, cna_network_session_destroy(session),
            "destroy the synchronously joined Local session");
        session = CNA_INVALID_HANDLE;
    }
    if (cna_demo_check_result(
            &demo,
            cna_network_session_join_async(
                available, on_completed, &completions, &session),
            "join the synthetic Local description through the asynchronous adapter")) {
        cna_demo_check(
            &demo,
            completions == 3 &&
                cna_network_session_get_session_type(session, &type) == CNA_RESULT_SUCCESS &&
                type == CNA_NETWORK_SESSION_TYPE_LOCAL,
            "asynchronous Local join completes once before returning its owned session");
        cna_demo_check_result(
            &demo, cna_network_session_destroy(session),
            "destroy the asynchronously joined Local session");
        session = CNA_INVALID_HANDLE;
    }
    cna_demo_check_result(
        &demo, cna_available_network_session_destroy(available),
        "release the reusable Local description");
    available = CNA_INVALID_HANDLE;

    session = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_network_session_join_invited(1, &session) == CNA_RESULT_NOT_SUPPORTED &&
            session == CNA_INVALID_HANDLE &&
            cna_network_session_join_invited_with_local_gamers(
                local_gamers, UINT64_C(1), &session) == CNA_RESULT_NOT_SUPPORTED &&
            session == CNA_INVALID_HANDLE,
        "both synchronous invitation overloads require the unavailable service invitation");
    session = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_network_session_join_invited_async(
            1, on_completed, &completions, &session) == CNA_RESULT_NOT_SUPPORTED &&
            session == CNA_INVALID_HANDLE && completions == 3 &&
            cna_network_session_join_invited_with_local_gamers_async(
                local_gamers, UINT64_C(1), on_completed, &completions, &session) ==
                CNA_RESULT_NOT_SUPPORTED &&
            session == CNA_INVALID_HANDLE && completions == 3,
        "both asynchronous invitation overloads refuse without invoking completion");
    cna_demo_check(
        &demo,
        cna_network_session_get_instance_count_ext(&count) == CNA_RESULT_SUCCESS &&
            count == 0 &&
            cna_network_session_get_active_action_count_ext(&count) == CNA_RESULT_SUCCESS &&
            count == 0,
        "all discovery and join results leave no session or pending action behind");

cleanup:
    if (session != CNA_INVALID_HANDLE) {
        (void)cna_network_session_destroy(session);
    }
    if (available != CNA_INVALID_HANDLE) {
        (void)cna_available_network_session_destroy(available);
    }
    if (collection != CNA_INVALID_HANDLE) {
        (void)cna_available_network_session_collection_destroy(collection);
    }
    cna_demo_check_result(
        &demo, cna_gamer_set_signed_in_gamers_ext(NULL, UINT64_C(0)),
        "clear the process-wide signed-in collection");
    if (signed_in != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(signed_in),
            "release the discovery signed-in gamer");
    }
    return cna_demo_finish(&demo);
}
