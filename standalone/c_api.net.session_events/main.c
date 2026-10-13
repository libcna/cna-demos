/* SPDX-License-Identifier: MS-PL */
/* NetworkSession event subscriptions, scoped payloads and injected Local events through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct EventState {
    int started;
    int ended;
    int joined;
    int left;
    int host_changed;
    int session_ended;
    int leaderboards;
    int invites;
    int valid_prefixes;
    int valid_joined_payloads;
    int valid_left_payloads;
    int valid_host_payloads;
    int valid_end_reasons;
    CNA_NetworkGamerHandle last_scoped_gamer;
} EventState;

static void on_started(
    CNA_NetworkSessionHandle session,
    const CNA_GameStartedEventInfo* info,
    void* context)
{
    EventState* state = (EventState*)context;
    (void)session;
    state->started += 1;
    if (info != NULL && info->struct_size == sizeof(*info) &&
        info->struct_version == UINT32_C(1)) {
        state->valid_prefixes += 1;
    }
}

static void on_ended(
    CNA_NetworkSessionHandle session,
    const CNA_GameEndedEventInfo* info,
    void* context)
{
    EventState* state = (EventState*)context;
    (void)session;
    state->ended += 1;
    if (info != NULL && info->struct_size == sizeof(*info) &&
        info->struct_version == UINT32_C(1)) {
        state->valid_prefixes += 1;
    }
}

static void on_joined(
    CNA_NetworkSessionHandle session,
    const CNA_GamerJoinedEventInfo* info,
    void* context)
{
    EventState* state = (EventState*)context;
    CNA_Bool local = CNA_FALSE;
    (void)session;
    state->joined += 1;
    if (info != NULL && info->struct_size == sizeof(*info) &&
        info->struct_version == UINT32_C(1) && info->gamer != CNA_INVALID_HANDLE &&
        cna_network_gamer_get_is_local(info->gamer, &local) == CNA_RESULT_SUCCESS) {
        state->valid_joined_payloads += 1;
        state->last_scoped_gamer = info->gamer;
    }
}

static void on_left(
    CNA_NetworkSessionHandle session,
    const CNA_GamerLeftEventInfo* info,
    void* context)
{
    EventState* state = (EventState*)context;
    CNA_Bool departed = CNA_FALSE;
    (void)session;
    state->left += 1;
    if (info != NULL && info->struct_size == sizeof(*info) &&
        info->struct_version == UINT32_C(1) && info->gamer != CNA_INVALID_HANDLE &&
        cna_network_gamer_get_has_left_session(info->gamer, &departed) == CNA_RESULT_SUCCESS &&
        departed == CNA_TRUE) {
        state->valid_left_payloads += 1;
        state->last_scoped_gamer = info->gamer;
    }
}

static void on_host_changed(
    CNA_NetworkSessionHandle session,
    const CNA_HostChangedEventInfo* info,
    void* context)
{
    EventState* state = (EventState*)context;
    CNA_Bool host = CNA_FALSE;
    (void)session;
    state->host_changed += 1;
    if (info != NULL && info->struct_size == sizeof(*info) &&
        info->struct_version == UINT32_C(1) && info->new_host != CNA_INVALID_HANDLE &&
        cna_network_gamer_get_is_host(info->new_host, &host) == CNA_RESULT_SUCCESS &&
        host == CNA_TRUE) {
        state->valid_host_payloads += 1;
        state->last_scoped_gamer = info->new_host;
    }
}

static void on_session_ended(
    CNA_NetworkSessionHandle session,
    const CNA_NetworkSessionEndedEventInfo* info,
    void* context)
{
    EventState* state = (EventState*)context;
    (void)session;
    state->session_ended += 1;
    if (info != NULL && info->struct_size == sizeof(*info) &&
        info->struct_version == UINT32_C(1) &&
        info->end_reason == CNA_NETWORK_SESSION_END_REASON_DISCONNECTED &&
        info->reserved[0] == 0U && info->reserved[1] == 0U &&
        info->reserved[2] == 0U && info->reserved[3] == 0U) {
        state->valid_end_reasons += 1;
    }
}

static void on_leaderboards(
    CNA_NetworkSessionHandle session,
    const CNA_WriteLeaderboardsEventInfo* info,
    void* context)
{
    EventState* state = (EventState*)context;
    (void)session;
    (void)info;
    state->leaderboards += 1;
}

static void on_invite(const CNA_InviteAcceptedEventInfo* info, void* context)
{
    EventState* state = (EventState*)context;
    (void)info;
    state->invites += 1;
}

int main(int argc, char** argv)
{
    static const char LocalName[] = "Event Host";
    static const char RemoteName[] = "Late Remote";
    CnaDemo demo;
    EventState state;
    CNA_SignedInGamerHandle signed_in = CNA_INVALID_HANDLE;
    CNA_NetworkSessionHandle session = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle remote = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle remote_view = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle local_view = CNA_INVALID_HANDLE;
    CNA_NetworkSessionEventRegistrationHandle registrations[10];
    CNA_NetworkSessionEventRegistrationHandle rejected = UINT64_C(99);
    CNA_Handle locals[1];
    CNA_NetworkEventInfo event;
    CNA_Bool bool_value = CNA_FALSE;
    int subscriptions_ok = 1;
    int releases_ok = 1;
    int index;

    cna_demo_init(&demo, "c_api.net.session_events", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.last_scoped_gamer = CNA_INVALID_HANDLE;
    for (index = 0; index < 10; ++index) {
        registrations[index] = CNA_INVALID_HANDLE;
    }
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                (CNA_StringView){LocalName, (uint64_t)(sizeof(LocalName) - 1U)},
                CNA_FALSE, CNA_FALSE, CNA_PLAYER_INDEX_ONE, &signed_in),
            "create the event host's signed-in identity")) {
        goto cleanup;
    }
    locals[0] = signed_in;
    if (!cna_demo_check_result(
            &demo,
            cna_network_session_create_with_local_gamers(
                CNA_NETWORK_SESSION_TYPE_LOCAL, locals, UINT64_C(1), 8, 0,
                CNA_INVALID_HANDLE, &session),
            "create the Local event session")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_session_subscribe_game_started(session, NULL, NULL, &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE &&
            cna_network_session_subscribe_invite_accepted(on_invite, &state, NULL) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "subscription creation refuses null callbacks and output handles atomically");

    subscriptions_ok = subscriptions_ok &&
        cna_network_session_subscribe_game_started(
            session, on_started, &state, &registrations[0]) == CNA_RESULT_SUCCESS;
    subscriptions_ok = subscriptions_ok &&
        cna_network_session_subscribe_game_ended(
            session, on_ended, &state, &registrations[1]) == CNA_RESULT_SUCCESS;
    subscriptions_ok = subscriptions_ok &&
        cna_network_session_subscribe_gamer_joined(
            session, on_joined, &state, &registrations[2]) == CNA_RESULT_SUCCESS;
    subscriptions_ok = subscriptions_ok &&
        cna_network_session_subscribe_gamer_left(
            session, on_left, &state, &registrations[3]) == CNA_RESULT_SUCCESS;
    subscriptions_ok = subscriptions_ok &&
        cna_network_session_subscribe_host_changed(
            session, on_host_changed, &state, &registrations[4]) == CNA_RESULT_SUCCESS;
    subscriptions_ok = subscriptions_ok &&
        cna_network_session_subscribe_session_ended(
            session, on_session_ended, &state, &registrations[5]) == CNA_RESULT_SUCCESS;
    subscriptions_ok = subscriptions_ok &&
        cna_network_session_subscribe_write_arbitrated_leaderboard(
            session, on_leaderboards, &state, &registrations[6]) == CNA_RESULT_SUCCESS;
    subscriptions_ok = subscriptions_ok &&
        cna_network_session_subscribe_write_unarbitrated_leaderboard(
            session, on_leaderboards, &state, &registrations[7]) == CNA_RESULT_SUCCESS;
    subscriptions_ok = subscriptions_ok &&
        cna_network_session_subscribe_write_true_skill(
            session, on_leaderboards, &state, &registrations[8]) == CNA_RESULT_SUCCESS;
    subscriptions_ok = subscriptions_ok &&
        cna_network_session_subscribe_invite_accepted(
            on_invite, &state, &registrations[9]) == CNA_RESULT_SUCCESS;
    cna_demo_check(
        &demo, subscriptions_ok,
        "subscribe to all nine session event families and the process-wide invite event");
    cna_demo_check(
        &demo,
        state.joined == 1 && state.valid_joined_payloads == 1 &&
            state.last_scoped_gamer != CNA_INVALID_HANDLE &&
            cna_network_gamer_get_is_local(state.last_scoped_gamer, &bool_value) ==
                CNA_RESULT_INVALID_HANDLE,
        "GamerJoined replays the existing participant through a callback-scoped handle");

    if (!cna_demo_check_result(
            &demo,
            cna_network_gamer_create(
                session, (CNA_StringView){RemoteName, (uint64_t)(sizeof(RemoteName) - 1U)},
                &remote),
            "create a late remote gamer bound to the session")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_session_add_remote_gamer_ext(session, remote) == CNA_RESULT_SUCCESS &&
            cna_network_gamer_destroy(remote) == CNA_RESULT_SUCCESS &&
            cna_network_session_update(session) == CNA_RESULT_SUCCESS &&
            state.joined == 2 && state.valid_joined_payloads == 2 &&
            cna_network_gamer_get_is_local(state.last_scoped_gamer, &bool_value) ==
                CNA_RESULT_INVALID_HANDLE,
        "a retained remote addition raises GamerJoined with another scoped payload");
    remote = CNA_INVALID_HANDLE;
    if (!cna_demo_check_result(
            &demo,
            cna_network_session_get_gamer(
                session, CNA_NETWORK_SESSION_ROSTER_REMOTE, 0, &remote_view),
            "borrow the retained remote gamer for removal")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_session_remove_gamer_ext(
            session, remote_view, CNA_NETWORK_SESSION_END_REASON_DISCONNECTED) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_remove_gamer_ext(session, remote_view, UINT32_C(9)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_network_session_update(session) == CNA_RESULT_SUCCESS && state.left == 1 &&
            state.valid_left_payloads == 1 &&
            cna_network_gamer_get_has_left_session(state.last_scoped_gamer, &bool_value) ==
                CNA_RESULT_INVALID_HANDLE,
        "remote removal raises GamerLeft with departure state and rejects an unknown reason");
    cna_demo_check(
        &demo,
        cna_network_gamer_destroy(remote_view) == CNA_RESULT_SUCCESS,
        "release the caller's remote roster view");
    remote_view = CNA_INVALID_HANDLE;

    cna_demo_check(
        &demo,
        cna_network_session_start_game(session) == CNA_RESULT_SUCCESS &&
            cna_network_session_update(session) == CNA_RESULT_SUCCESS && state.started == 1 &&
            state.valid_prefixes == 1 &&
            cna_network_session_end_game(session) == CNA_RESULT_SUCCESS &&
            cna_network_session_update(session) == CNA_RESULT_SUCCESS && state.ended == 1 &&
            state.valid_prefixes == 2,
        "queued game transitions raise exact versioned GameStarted and GameEnded payloads");
    if (!cna_demo_check_result(
            &demo,
            cna_network_session_get_gamer(
                session, CNA_NETWORK_SESSION_ROSTER_LOCAL, 0, &local_view),
            "borrow the local host for an injected host-change event")) {
        goto cleanup;
    }
    memset(&event, 0, sizeof(event));
    event.struct_size = (uint32_t)sizeof(event);
    event.struct_version = UINT32_C(1);
    event.type = CNA_NETWORK_EVENT_TYPE_HOST_CHANGE;
    event.reliable = CNA_SEND_DATA_OPTIONS_NONE;
    event.state = CNA_NETWORK_SESSION_STATE_LOBBY;
    event.reason = CNA_NETWORK_SESSION_END_REASON_DISCONNECTED;
    event.gamer = local_view;
    event.sender = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_network_session_send_network_event_ext(session, &event) == CNA_RESULT_SUCCESS &&
            cna_network_session_update(session) == CNA_RESULT_SUCCESS &&
            state.host_changed == 1 && state.valid_host_payloads == 1 &&
            cna_network_gamer_get_is_host(state.last_scoped_gamer, &bool_value) ==
                CNA_RESULT_INVALID_HANDLE,
        "an injected host change raises an exact callback with a scoped new-host view");
    event.type = UINT32_C(9);
    cna_demo_check(
        &demo,
        cna_network_session_send_network_event_ext(session, &event) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "an injected event with an unknown type is refused before queuing");
    event.type = CNA_NETWORK_EVENT_TYPE_STATE_CHANGE;
    event.state = CNA_NETWORK_SESSION_STATE_ENDED;
    event.gamer = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_network_session_send_network_event_ext(session, &event) == CNA_RESULT_SUCCESS &&
            cna_network_session_update(session) == CNA_RESULT_SUCCESS &&
            state.session_ended == 1 && state.valid_end_reasons == 1,
        "an injected ended-state event raises SessionEnded with the requested reason");
    cna_demo_check(
        &demo,
        state.leaderboards == 0 && state.invites == 0,
        "the Local workflow does not invent leaderboard-write or accepted-invite events");

    for (index = 0; index < 10; ++index) {
        if (registrations[index] == CNA_INVALID_HANDLE ||
            cna_network_session_unsubscribe(registrations[index]) != CNA_RESULT_SUCCESS ||
            cna_network_session_unsubscribe(registrations[index]) != CNA_RESULT_INVALID_HANDLE) {
            releases_ok = 0;
        }
        registrations[index] = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo, releases_ok,
        "all ten registrations unsubscribe and release exactly once");

cleanup:
    for (index = 0; index < 10; ++index) {
        if (registrations[index] != CNA_INVALID_HANDLE) {
            (void)cna_network_session_unsubscribe(registrations[index]);
        }
    }
    if (local_view != CNA_INVALID_HANDLE) {
        (void)cna_network_gamer_destroy(local_view);
    }
    if (remote_view != CNA_INVALID_HANDLE) {
        (void)cna_network_gamer_destroy(remote_view);
    }
    if (remote != CNA_INVALID_HANDLE) {
        (void)cna_network_gamer_destroy(remote);
    }
    if (session != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo, cna_network_session_destroy(session) == CNA_RESULT_SUCCESS,
            "release the ended Local session");
    }
    if (signed_in != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo, cna_signed_in_gamer_destroy(signed_in) == CNA_RESULT_SUCCESS,
            "release the event host identity");
    }
    return cna_demo_finish(&demo);
}
