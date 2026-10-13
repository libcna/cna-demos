/* SPDX-License-Identifier: MS-PL */
/* SignedInGamer state and the process-wide positional/player-index collection through the C ABI. */
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

int main(int argc, char** argv)
{
    static const char PrimaryName[] = "Signed Player";
    static const char OutsiderName[] = "Outside Player";
    CnaDemo demo;
    CNA_SignedInGamerHandle primary = CNA_INVALID_HANDLE;
    CNA_SignedInGamerHandle outsider = CNA_INVALID_HANDLE;
    CNA_SignedInGamerHandle published[1];
    CNA_SignedInGamerHandle positional = CNA_INVALID_HANDLE;
    CNA_SignedInGamerHandle indexed = CNA_INVALID_HANDLE;
    CNA_SignedInGamerHandle refused = UINT64_C(99);
    CNA_GamerCollectionHandle friends = CNA_INVALID_HANDLE;
    CNA_GamerPresence presence;
    CNA_GamerPresence malformed_presence;
    CNA_GamerPrivileges privileges;
    CNA_InviteAcceptedEventInfo invite;
    CNA_InviteAcceptedEventInfo malformed_invite;
    CNA_PlayerIndex player = UINT32_C(99);
    CNA_Bool flag = CNA_FALSE;
    int32_t count = -1;
    int32_t position = -2;
    int32_t party_size = -1;

    cna_demo_init(&demo, "c_api.gamer.signed_in_state", argc, argv, 1);
    cna_demo_check_result(
        &demo, cna_gamer_set_signed_in_gamers_ext(NULL, UINT64_C(0)),
        "start with an empty process-wide signed-in collection");
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(PrimaryName, sizeof(PrimaryName) - 1U), CNA_TRUE, CNA_TRUE,
                CNA_PLAYER_INDEX_TWO, &primary),
            "create the primary online guest at player index two") ||
        !cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(OutsiderName, sizeof(OutsiderName) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_FOUR, &outsider),
            "create an unpublished offline outsider")) {
        goto cleanup;
    }

    memset(&invite, 0xA5, sizeof(invite));
    invite.struct_size = (uint32_t)sizeof(invite);
    invite.struct_version = UINT32_C(1);
    malformed_invite = invite;
    malformed_invite.struct_version = UINT32_C(2);
    cna_demo_check(
        &demo,
        cna_invite_accepted_event_info_init(primary, CNA_TRUE, &invite) ==
                CNA_RESULT_SUCCESS &&
            invite.gamer == primary && invite.is_current_session == CNA_TRUE &&
            invite.reserved[0] == 0U && invite.reserved[1] == 0U &&
            invite.reserved[2] == 0U && invite.reserved[3] == 0U &&
            invite.reserved[4] == 0U && invite.reserved[5] == 0U &&
            invite.reserved[6] == 0U &&
            cna_invite_accepted_event_info_init(primary, CNA_FALSE, &malformed_invite) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "accepted-invite values retain the live gamer and reject an unknown structure version");
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_get_is_guest(primary, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_signed_in_gamer_get_is_signed_in_to_live(primary, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_signed_in_gamer_get_player_index(primary, &player) == CNA_RESULT_SUCCESS &&
            player == CNA_PLAYER_INDEX_TWO,
        "guest, online-service and player-index identities retain their constructor values");
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_get_party_size(primary, &party_size) == CNA_RESULT_SUCCESS &&
            cna_signed_in_gamer_set_party_size(primary, 4) == CNA_RESULT_SUCCESS &&
            cna_signed_in_gamer_get_party_size(primary, &party_size) == CNA_RESULT_SUCCESS &&
            party_size == 4,
        "the host-controlled party size changes and reads back exactly");

    memset(&presence, 0xA5, sizeof(presence));
    presence.struct_size = (uint32_t)sizeof(presence);
    presence.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_gamer_presence_init(&presence) == CNA_RESULT_SUCCESS &&
            presence.presence_mode == CNA_GAMER_PRESENCE_MODE_NONE &&
            presence.presence_value == 0,
        "GamerPresence initializes to the canonical no-mode zero value");
    presence.presence_mode = CNA_GAMER_PRESENCE_MODE_CORNFLOWER_BLUE;
    presence.presence_value = 7;
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_set_presence(primary, &presence) == CNA_RESULT_SUCCESS &&
            cna_gamer_presence_init(&presence) == CNA_RESULT_SUCCESS &&
            cna_signed_in_gamer_get_presence(primary, &presence) == CNA_RESULT_SUCCESS &&
            presence.presence_mode == CNA_GAMER_PRESENCE_MODE_CORNFLOWER_BLUE &&
            presence.presence_value == 7,
        "the complete structured presence round-trips through the signed-in gamer");
    malformed_presence = presence;
    malformed_presence.presence_mode = CNA_GAMER_PRESENCE_MODE_MAXIMUM + UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_set_presence(primary, &malformed_presence) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_signed_in_gamer_set_presence_mode_string_ext(
                primary, text_view("Custom presence", sizeof("Custom presence") - 1U)) ==
                CNA_RESULT_SUCCESS &&
            cna_signed_in_gamer_get_presence(primary, &presence) == CNA_RESULT_SUCCESS &&
            presence.presence_mode == CNA_GAMER_PRESENCE_MODE_CORNFLOWER_BLUE &&
            presence.presence_value == 7,
        "invalid structured presence is refused and free text does not replace its mode/value");

    memset(&privileges, 0xA5, sizeof(privileges));
    privileges.struct_size = (uint32_t)sizeof(privileges);
    privileges.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_get_privileges(primary, &privileges) == CNA_RESULT_SUCCESS &&
            privileges.allow_communication <= CNA_GAMER_PRIVILEGE_SETTING_MAXIMUM &&
            privileges.allow_profile_viewing <= CNA_GAMER_PRIVILEGE_SETTING_MAXIMUM &&
            privileges.allow_user_created_content <= CNA_GAMER_PRIVILEGE_SETTING_MAXIMUM &&
            privileges.allow_online_sessions <= CNA_TRUE &&
            privileges.allow_premium_content <= CNA_TRUE &&
            privileges.allow_purchase_content <= CNA_TRUE &&
            privileges.allow_trade_content <= CNA_TRUE && privileges.reserved[0] == 0U &&
            privileges.reserved[1] == 0U && privileges.reserved[2] == 0U &&
            privileges.reserved[3] == 0U,
        "privileges publish bounded graded choices, canonical booleans and zero reserved bytes");
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_is_friend(primary, outsider, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "an explicit offline gamer has no service friendship with another local fixture");
    if (cna_demo_check_result(
            &demo, cna_signed_in_gamer_get_friends(primary, &friends),
            "get the offline gamer's owned friend collection")) {
        cna_demo_check(
            &demo,
            cna_gamer_collection_get_count(friends, &count) == CNA_RESULT_SUCCESS &&
                count == 0,
            "the offline friend collection is an ordinary empty result");
        cna_demo_check_result(
            &demo, cna_gamer_collection_destroy(friends),
            "release the empty friend collection");
        friends = CNA_INVALID_HANDLE;
    }

    cna_demo_check(
        &demo,
        cna_gamer_get_signed_in_gamer_at_player_index(
            CNA_PLAYER_INDEX_TWO, &flag, &refused) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && refused == CNA_INVALID_HANDLE,
        "a valid but empty player slot succeeds with no gamer handle");
    published[0] = primary;
    cna_demo_check(
        &demo,
        cna_gamer_set_signed_in_gamers_ext(published, UINT64_C(1)) ==
                CNA_RESULT_SUCCESS &&
            cna_gamer_get_signed_in_gamer_count(&count) == CNA_RESULT_SUCCESS && count == 1,
        "publish exactly one signed-in gamer");
    if (cna_demo_check_result(
            &demo, cna_gamer_get_signed_in_gamer_at(0, &positional),
            "borrow the gamer at collection position zero")) {
        cna_demo_check(
            &demo,
            cna_gamer_signed_in_index_of(positional, &position) == CNA_RESULT_SUCCESS &&
                position == 0 &&
                cna_gamer_signed_in_contains(positional, &flag) == CNA_RESULT_SUCCESS &&
                flag == CNA_TRUE,
            "positional lookup, IndexOf and Contains agree on the published gamer");
    }
    cna_demo_check(
        &demo,
        cna_gamer_signed_in_index_of(outsider, &position) == CNA_RESULT_SUCCESS &&
            position == -1 &&
            cna_gamer_signed_in_contains(outsider, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "a live unpublished gamer is a successful missing collection result");
    if (cna_demo_check_result(
            &demo,
            cna_gamer_get_signed_in_gamer_at_player_index(
                CNA_PLAYER_INDEX_TWO, &flag, &indexed),
            "borrow the published gamer by player index")) {
        cna_demo_check(
            &demo, flag == CNA_TRUE && indexed != CNA_INVALID_HANDLE,
            "player-index lookup is distinct from positional lookup and finds player two");
    }
    refused = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_gamer_get_signed_in_gamer_at_player_index(
            CNA_PLAYER_INDEX_ONE, &flag, &refused) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && refused == CNA_INVALID_HANDLE,
        "an unoccupied player index clears the output handle while reporting absence");
    cna_demo_check(
        &demo,
        cna_gamer_get_signed_in_gamer_at(1, &refused) == CNA_RESULT_INVALID_ARGUMENT &&
            refused == CNA_INVALID_HANDLE,
        "the positional upper bound is refused without publishing a handle");
    if (indexed != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(indexed),
            "release the independent player-index view");
        indexed = CNA_INVALID_HANDLE;
    }
    if (positional != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(positional),
            "release the independent positional view");
        positional = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_destroy(primary) == CNA_RESULT_INVALID_STATE &&
            cna_gamer_get_signed_in_gamer_count(&count) == CNA_RESULT_SUCCESS && count == 1,
        "the published collection retains its source against premature destruction");

cleanup:
    if (friends != CNA_INVALID_HANDLE) {
        (void)cna_gamer_collection_destroy(friends);
    }
    if (indexed != CNA_INVALID_HANDLE) {
        (void)cna_signed_in_gamer_destroy(indexed);
    }
    if (positional != CNA_INVALID_HANDLE) {
        (void)cna_signed_in_gamer_destroy(positional);
    }
    cna_demo_check_result(
        &demo, cna_gamer_set_signed_in_gamers_ext(NULL, UINT64_C(0)),
        "clear the process-wide signed-in collection");
    if (outsider != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(outsider),
            "release the unpublished outsider");
    }
    if (primary != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(primary),
            "release the formerly published gamer");
    }
    return cna_demo_finish(&demo);
}
