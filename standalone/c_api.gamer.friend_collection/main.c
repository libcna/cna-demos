/* SPDX-License-Identifier: MS-PL */
/* FriendGamer values and mutable GamerCollection iteration through the C ABI. */
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
    static const char FirstTag[] = "FriendOne";
    static const char FirstName[] = "Friend One";
    static const char SecondTag[] = "FriendTwo";
    static const char SecondName[] = "Friend Two";
    CnaDemo demo;
    CNA_GamerHandle first = CNA_INVALID_HANDLE;
    CNA_GamerHandle second = CNA_INVALID_HANDLE;
    CNA_GamerHandle inputs[2];
    CNA_GamerHandle borrowed = CNA_INVALID_HANDLE;
    CNA_GamerHandle copied[4];
    CNA_GamerHandle before[4];
    CNA_GamerCollectionHandle collection = CNA_INVALID_HANDLE;
    CNA_GamerEnumeratorHandle enumerator = CNA_INVALID_HANDLE;
    CNA_FriendGamerInfo info;
    CNA_FriendGamerInfo malformed;
    CNA_Bool flag = CNA_FALSE;
    int32_t count = -1;
    int32_t index = -2;
    uint64_t bytes = UINT64_C(0);
    uint64_t copied_count = UINT64_C(0);
    char presence[64];
    char untouched[64];

    cna_demo_init(&demo, "c_api.gamer.friend_collection", argc, argv, 1);
    if (!cna_demo_check_result(
            &demo,
            cna_friend_gamer_create_ext(
                text_view(FirstTag, sizeof(FirstTag) - 1U),
                text_view(FirstName, sizeof(FirstName) - 1U), CNA_TRUE, CNA_TRUE,
                CNA_FALSE, CNA_FALSE, CNA_FALSE, CNA_TRUE, &first),
            "create the online playing friend") ||
        !cna_demo_check_result(
            &demo,
            cna_friend_gamer_create_ext(
                text_view(SecondTag, sizeof(SecondTag) - 1U),
                text_view(SecondName, sizeof(SecondName) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_TRUE, CNA_TRUE, CNA_TRUE, CNA_FALSE, &second),
            "create the offline busy friend")) {
        goto cleanup;
    }

    memset(&info, 0xA5, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_friend_gamer_get_info(first, &info) == CNA_RESULT_SUCCESS &&
            info.friend_request_received_from == CNA_TRUE &&
            info.friend_request_sent_to == CNA_FALSE && info.has_voice == CNA_FALSE &&
            info.invite_accepted == CNA_FALSE && info.invite_received_from == CNA_FALSE &&
            info.invite_rejected == CNA_FALSE && info.invite_sent_to == CNA_FALSE &&
            info.is_away == CNA_FALSE && info.is_busy == CNA_FALSE &&
            info.is_joinable == CNA_FALSE && info.is_online == CNA_TRUE &&
            info.is_playing == CNA_TRUE && info.reserved[0] == 0U &&
            info.reserved[1] == 0U && info.reserved[2] == 0U && info.reserved[3] == 0U,
        "the first friend snapshot preserves its request and activity predicates");
    malformed = info;
    malformed.struct_version = UINT32_C(2);
    cna_demo_check(
        &demo,
        cna_friend_gamer_get_info(second, &info) == CNA_RESULT_SUCCESS &&
            info.friend_request_received_from == CNA_FALSE &&
            info.friend_request_sent_to == CNA_TRUE && info.is_away == CNA_TRUE &&
            info.is_busy == CNA_TRUE && info.is_online == CNA_FALSE &&
            info.is_playing == CNA_FALSE &&
            cna_friend_gamer_get_info(second, &malformed) == CNA_RESULT_INVALID_ARGUMENT,
        "the second friend has independent predicates and rejects an unknown snapshot version");

    memset(presence, 0xA5, sizeof(presence));
    memset(untouched, 0xA5, sizeof(untouched));
    cna_demo_check(
        &demo,
        cna_friend_gamer_get_presence_size(first, &bytes) == CNA_RESULT_SUCCESS &&
            bytes < sizeof(presence) &&
            (bytes == 0U ||
             (cna_friend_gamer_copy_presence(first, presence, bytes - 1U, &copied_count) ==
                  CNA_RESULT_BUFFER_TOO_SMALL &&
              copied_count == bytes && memcmp(presence, untouched, sizeof(presence)) == 0)) &&
            cna_friend_gamer_copy_presence(first, presence, bytes, &copied_count) ==
                CNA_RESULT_SUCCESS &&
            copied_count == bytes && (unsigned char)presence[bytes] == 0xA5U,
        "friend presence uses an exact atomic free-text buffer contract");

    inputs[0] = first;
    inputs[1] = second;
    if (!cna_demo_check_result(
            &demo,
            cna_friend_collection_create_ext(inputs, UINT64_C(2), &collection),
            "create a retained two-friend collection")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_gamer_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 2 &&
            cna_friend_collection_get_is_disposed(collection, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "the new friend collection is live with both entries");
    cna_demo_check(
        &demo,
        cna_gamer_collection_get_at(collection, 0, &borrowed) == CNA_RESULT_SUCCESS &&
            borrowed == first &&
            cna_gamer_collection_get_at(collection, 1, &borrowed) == CNA_RESULT_SUCCESS &&
            borrowed == second,
        "indexed collection reads retain the authored friend order");
    borrowed = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_gamer_collection_get_at(collection, -1, &borrowed) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            borrowed == CNA_INVALID_HANDLE &&
            cna_gamer_collection_get_at(collection, 2, &borrowed) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            borrowed == CNA_INVALID_HANDLE,
        "both collection bounds are refused without publishing a gamer");
    cna_demo_check(
        &demo,
        cna_gamer_collection_index_of(collection, second, &index) == CNA_RESULT_SUCCESS &&
            index == 1 &&
            cna_gamer_collection_contains(collection, first, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "IndexOf and Contains identify the exact retained friend handles");

    memset(copied, 0xA5, sizeof(copied));
    memcpy(before, copied, sizeof(before));
    cna_demo_check(
        &demo,
        cna_gamer_collection_copy_to(
            collection, copied, UINT64_C(2), 1, &copied_count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied_count == UINT64_C(2) && memcmp(copied, before, sizeof(copied)) == 0,
        "a short shifted collection copy reports its count without a partial write");
    cna_demo_check(
        &demo,
        cna_gamer_collection_copy_to(
            collection, copied, UINT64_C(4), 1, &copied_count) == CNA_RESULT_SUCCESS &&
            copied_count == UINT64_C(2) && copied[1] == first && copied[2] == second,
        "a sufficient shifted collection copy preserves both ordered handles");

    if (cna_demo_check_result(
            &demo, cna_gamer_collection_create_enumerator(collection, &enumerator),
            "create a cursor over the friend collection")) {
        cna_demo_check(
            &demo,
            cna_gamer_enumerator_get_current(enumerator, &borrowed) ==
                    CNA_RESULT_INVALID_STATE &&
                cna_gamer_enumerator_move_next(enumerator, &flag) == CNA_RESULT_SUCCESS &&
                flag == CNA_TRUE &&
                cna_gamer_enumerator_get_current(enumerator, &borrowed) ==
                    CNA_RESULT_SUCCESS &&
                borrowed == first &&
                cna_gamer_enumerator_move_next(enumerator, &flag) == CNA_RESULT_SUCCESS &&
                flag == CNA_TRUE &&
                cna_gamer_enumerator_get_current(enumerator, &borrowed) ==
                    CNA_RESULT_SUCCESS &&
                borrowed == second &&
                cna_gamer_enumerator_move_next(enumerator, &flag) == CNA_RESULT_SUCCESS &&
                flag == CNA_FALSE,
            "the cursor starts before the first item and walks both friends in order");
        cna_demo_check(
            &demo,
            cna_gamer_enumerator_reset(enumerator) == CNA_RESULT_SUCCESS &&
                cna_gamer_enumerator_move_next(enumerator, &flag) == CNA_RESULT_SUCCESS &&
                flag == CNA_TRUE &&
                cna_gamer_enumerator_get_current(enumerator, &borrowed) ==
                    CNA_RESULT_SUCCESS &&
                borrowed == first,
            "cursor reset returns iteration to the first friend");
        cna_demo_check_result(
            &demo, cna_gamer_enumerator_destroy(enumerator),
            "release the collection cursor");
        enumerator = CNA_INVALID_HANDLE;
    }

    cna_demo_check(
        &demo,
        cna_gamer_collection_remove(collection, first) == CNA_RESULT_SUCCESS &&
            cna_gamer_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 1 &&
            cna_gamer_collection_remove(collection, first) == CNA_RESULT_SUCCESS &&
            cna_gamer_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 1,
        "remove drops a present friend and treats a repeated removal as a successful no-op");
    cna_demo_check(
        &demo,
        cna_gamer_collection_add(collection, first) == CNA_RESULT_SUCCESS &&
            cna_gamer_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 2 &&
            cna_gamer_collection_clear(collection) == CNA_RESULT_SUCCESS &&
            cna_gamer_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 0,
        "add restores the removed friend and Clear empties the mutable collection");
    cna_demo_check_result(
        &demo, cna_gamer_collection_destroy(collection),
        "dispose and release the friend collection");
    collection = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_gamer_destroy(second) == CNA_RESULT_SUCCESS &&
            cna_gamer_destroy(first) == CNA_RESULT_SUCCESS &&
            cna_gamer_destroy(first) == CNA_RESULT_INVALID_HANDLE,
        "both caller-owned friend handles release independently and only once");
    second = CNA_INVALID_HANDLE;
    first = CNA_INVALID_HANDLE;

cleanup:
    if (enumerator != CNA_INVALID_HANDLE) {
        (void)cna_gamer_enumerator_destroy(enumerator);
    }
    if (collection != CNA_INVALID_HANDLE) {
        (void)cna_gamer_collection_destroy(collection);
    }
    if (second != CNA_INVALID_HANDLE) {
        (void)cna_gamer_destroy(second);
    }
    if (first != CNA_INVALID_HANDLE) {
        (void)cna_gamer_destroy(first);
    }
    return cna_demo_finish(&demo);
}
