/* SPDX-License-Identifier: MS-PL */
/* Versioned network-session event payload values and validation through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int four_zero_bytes(const uint8_t value[4])
{
    return value[0] == 0U && value[1] == 0U && value[2] == 0U && value[3] == 0U;
}

static int seven_zero_bytes(const uint8_t value[7])
{
    int i;
    for (i = 0; i < 7; ++i) {
        if (value[i] != 0U) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv)
{
    static const char OldName[] = "Old Host";
    static const char NewName[] = "New Host";
    static const CNA_NetworkSessionEndReason Reasons[] = {
        CNA_NETWORK_SESSION_END_REASON_CLIENT_SIGNED_OUT,
        CNA_NETWORK_SESSION_END_REASON_HOST_ENDED_SESSION,
        CNA_NETWORK_SESSION_END_REASON_REMOVED_BY_HOST,
        CNA_NETWORK_SESSION_END_REASON_DISCONNECTED};
    CnaDemo demo;
    CNA_NetworkGamerHandle old_gamer = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle new_gamer = CNA_INVALID_HANDLE;
    CNA_GameEndedEventInfo ended;
    CNA_GameStartedEventInfo started;
    CNA_GamerJoinedEventInfo joined;
    CNA_GamerLeftEventInfo left;
    CNA_HostChangedEventInfo host;
    CNA_NetworkSessionEndedEventInfo session_ended;
    CNA_WriteLeaderboardsEventInfo leaderboards;
    CNA_Bool all_reasons = CNA_TRUE;
    size_t i;

    cna_demo_init(&demo, "c_api.net.session_event_values", argc, argv, 1);
    if (!cna_demo_check_result(
            &demo,
            cna_network_gamer_create(
                CNA_INVALID_HANDLE,
                (CNA_StringView){OldName, (uint64_t)(sizeof(OldName) - 1U)},
                &old_gamer),
            "create the old-host identity") ||
        !cna_demo_check_result(
            &demo,
            cna_network_gamer_create(
                CNA_INVALID_HANDLE,
                (CNA_StringView){NewName, (uint64_t)(sizeof(NewName) - 1U)},
                &new_gamer),
            "create the new-host identity")) {
        goto cleanup;
    }

    memset(&ended, 0xA5, sizeof(ended));
    ended.struct_size = (uint32_t)sizeof(ended);
    ended.struct_version = UINT32_C(1);
    memset(&started, 0xA5, sizeof(started));
    started.struct_size = (uint32_t)sizeof(started);
    started.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_game_ended_event_info_init(&ended) == CNA_RESULT_SUCCESS &&
            ended.struct_size == sizeof(ended) && ended.struct_version == UINT32_C(1) &&
            cna_game_started_event_info_init(&started) == CNA_RESULT_SUCCESS &&
            started.struct_size == sizeof(started) && started.struct_version == UINT32_C(1),
        "the two payload-free game transition descriptions accept their canonical prefixes");
    ended.struct_size = (uint32_t)sizeof(ended) - 1U;
    started.struct_version = UINT32_C(2);
    cna_demo_check(
        &demo,
        cna_game_ended_event_info_init(&ended) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_game_started_event_info_init(&started) == CNA_RESULT_INVALID_ARGUMENT,
        "undersized and unknown-version transition descriptions are refused");

    memset(&joined, 0xA5, sizeof(joined));
    joined.struct_size = (uint32_t)sizeof(joined);
    joined.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_gamer_joined_event_info_init(old_gamer, &joined) == CNA_RESULT_SUCCESS &&
            joined.gamer == old_gamer,
        "a joined description preserves the exact validated gamer identity");
    memset(&left, 0xA5, sizeof(left));
    left.struct_size = (uint32_t)sizeof(left);
    left.struct_version = UINT32_C(1);
    left.gamer = UINT64_C(77);
    cna_demo_check(
        &demo,
        cna_gamer_left_event_info_init(UINT64_C(123456), &left) ==
                CNA_RESULT_INVALID_HANDLE &&
            left.gamer == UINT64_C(77) &&
            cna_gamer_left_event_info_init(new_gamer, &left) == CNA_RESULT_SUCCESS &&
            left.gamer == new_gamer,
        "a left description refuses a stale identity atomically then accepts a live gamer");

    memset(&host, 0xA5, sizeof(host));
    host.struct_size = (uint32_t)sizeof(host);
    host.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_host_changed_event_info_init(old_gamer, new_gamer, &host) ==
                CNA_RESULT_SUCCESS &&
            host.old_host == old_gamer && host.new_host == new_gamer,
        "host-change initialization preserves two distinct validated gamer identities");
    host.old_host = UINT64_C(11);
    host.new_host = UINT64_C(12);
    cna_demo_check(
        &demo,
        cna_host_changed_event_info_init(old_gamer, UINT64_C(123456), &host) ==
                CNA_RESULT_INVALID_HANDLE &&
            host.old_host == UINT64_C(11) && host.new_host == UINT64_C(12),
        "an invalid new host refuses the complete host-change payload atomically");

    for (i = 0U; i < sizeof(Reasons) / sizeof(Reasons[0]); ++i) {
        memset(&session_ended, 0xA5, sizeof(session_ended));
        session_ended.struct_size = (uint32_t)sizeof(session_ended);
        session_ended.struct_version = UINT32_C(1);
        if (cna_network_session_ended_event_info_init(Reasons[i], &session_ended) !=
                CNA_RESULT_SUCCESS ||
            session_ended.end_reason != Reasons[i] ||
            !four_zero_bytes(session_ended.reserved)) {
            all_reasons = CNA_FALSE;
        }
    }
    cna_demo_check(
        &demo, all_reasons == CNA_TRUE,
        "all four session-end identities round-trip with canonical reserved bytes");
    memset(&session_ended, 0xA5, sizeof(session_ended));
    session_ended.struct_size = (uint32_t)sizeof(session_ended);
    session_ended.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_network_session_ended_event_info_init(
            (CNA_NetworkSessionEndReason)UINT32_C(4), &session_ended) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            session_ended.end_reason == (CNA_NetworkSessionEndReason)UINT32_C(0xA5A5A5A5),
        "an unknown session-end identity is refused before changing the payload");

    memset(&leaderboards, 0xA5, sizeof(leaderboards));
    leaderboards.struct_size = (uint32_t)sizeof(leaderboards);
    leaderboards.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_write_leaderboards_event_info_init(old_gamer, CNA_TRUE, &leaderboards) ==
                CNA_RESULT_SUCCESS &&
            leaderboards.gamer == old_gamer && leaderboards.is_leaving == CNA_TRUE &&
            seven_zero_bytes(leaderboards.reserved),
        "a leaderboard-write description preserves its gamer and canonical leaving flag");
    leaderboards.gamer = UINT64_C(55);
    leaderboards.is_leaving = UINT8_C(55);
    cna_demo_check(
        &demo,
        cna_write_leaderboards_event_info_init(old_gamer, (CNA_Bool)2, &leaderboards) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            leaderboards.gamer == UINT64_C(55) && leaderboards.is_leaving == UINT8_C(55),
        "a noncanonical leaving flag is refused before changing leaderboard-write output");
    memset(&leaderboards, 0xA5, sizeof(leaderboards));
    leaderboards.struct_size = (uint32_t)sizeof(leaderboards);
    leaderboards.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_write_leaderboards_event_info_init(
            CNA_INVALID_HANDLE, CNA_FALSE, &leaderboards) == CNA_RESULT_SUCCESS &&
            leaderboards.gamer == CNA_INVALID_HANDLE &&
            leaderboards.is_leaving == CNA_FALSE &&
            seven_zero_bytes(leaderboards.reserved),
        "the optional leaderboard gamer accepts a canonical absent identity");

cleanup:
    if (new_gamer != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo, cna_network_gamer_destroy(new_gamer) == CNA_RESULT_SUCCESS,
            "release the new-host identity");
    }
    if (old_gamer != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo, cna_network_gamer_destroy(old_gamer) == CNA_RESULT_SUCCESS,
            "release the old-host identity");
    }
    return cna_demo_finish(&demo);
}
