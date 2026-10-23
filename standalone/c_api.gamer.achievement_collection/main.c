/* SPDX-License-Identifier: MS-PL */
/* Achievement values and read-only AchievementCollection behavior through the C ABI. */
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
    static const char FirstKey[] = "FirstSteps";
    static const char FirstName[] = "First Steps";
    static const char FirstDescription[] = "Begin the game";
    static const char OtherKey[] = "LastSteps";
    static const char OtherName[] = "Last Steps";
    static const char OtherDescription[] = "Finish the game";
    CnaDemo demo;
    CNA_AchievementHandle first = CNA_INVALID_HANDLE;
    CNA_AchievementHandle same = CNA_INVALID_HANDLE;
    CNA_AchievementHandle other = CNA_INVALID_HANDLE;
    CNA_AchievementHandle by_index = CNA_INVALID_HANDLE;
    CNA_AchievementHandle by_key = CNA_INVALID_HANDLE;
    CNA_AchievementHandle copied[4];
    CNA_AchievementHandle before[4];
    CNA_AchievementHandle inputs[2];
    CNA_AchievementCollectionHandle collection = CNA_INVALID_HANDLE;
    CNA_AchievementCollectionHandle empty = CNA_INVALID_HANDLE;
    CNA_AchievementInfo info;
    CNA_AchievementInfo malformed;
    CNA_Bool flag = CNA_TRUE;
    int32_t count = -1;
    int32_t index = -2;
    uint64_t bytes = UINT64_C(0);
    uint64_t copied_bytes = UINT64_C(0);
    uint64_t copied_count = UINT64_C(0);
    char text[64];
    char untouched[64];

    cna_demo_init(&demo, "c_api.gamer.achievement_collection", argc, argv, 1);
    if (!cna_demo_check_result(
            &demo,
            cna_achievement_create_ext(
                text_view(FirstKey, sizeof(FirstKey) - 1U),
                text_view(FirstName, sizeof(FirstName) - 1U),
                text_view(FirstDescription, sizeof(FirstDescription) - 1U),
                CNA_TRUE, CNA_TRUE, INT64_C(637000000000000000), &first),
            "create an earned achievement value") ||
        !cna_demo_check_result(
            &demo,
            cna_achievement_create_ext(
                text_view(FirstKey, sizeof(FirstKey) - 1U),
                text_view(FirstName, sizeof(FirstName) - 1U),
                text_view(FirstDescription, sizeof(FirstDescription) - 1U),
                CNA_TRUE, CNA_TRUE, INT64_C(637000000000000000), &same),
            "create an independently owned equal achievement") ||
        !cna_demo_check_result(
            &demo,
            cna_achievement_create_ext(
                text_view(OtherKey, sizeof(OtherKey) - 1U),
                text_view(OtherName, sizeof(OtherName) - 1U),
                text_view(OtherDescription, sizeof(OtherDescription) - 1U),
                CNA_FALSE, CNA_FALSE, INT64_C(0), &other),
            "create a distinct unearned achievement")) {
        goto cleanup;
    }

    memset(&info, 0xA5, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    malformed = info;
    malformed.struct_version = UINT32_C(2);
    cna_demo_check(
        &demo,
        cna_achievement_get_info(first, &info) == CNA_RESULT_SUCCESS &&
            info.struct_size == sizeof(info) && info.struct_version == UINT32_C(1) &&
            info.gamer_score == 0 && info.display_before_earned == CNA_TRUE &&
            info.earned_online == CNA_FALSE && info.is_earned == CNA_TRUE &&
            info.reserved == 0U &&
            info.earned_date_time_ticks == INT64_C(637000000000000000),
        "the versioned snapshot preserves score, visibility, earned state and timestamp");
    cna_demo_check(
        &demo,
        cna_achievement_get_info(first, &malformed) == CNA_RESULT_INVALID_ARGUMENT,
        "the snapshot refuses an unknown structure version");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_achievement_get_key_size(first, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(FirstKey) - 1U &&
            cna_achievement_copy_key(
                first, text, sizeof(FirstKey) - 1U, &copied_bytes) == CNA_RESULT_SUCCESS &&
            copied_bytes == sizeof(FirstKey) - 1U &&
            memcmp(text, FirstKey, sizeof(FirstKey) - 1U) == 0 &&
            (unsigned char)text[sizeof(FirstKey) - 1U] == 0xA5U,
        "achievement key size and exact unterminated copy agree");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_achievement_get_name_size(first, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(FirstName) - 1U &&
            cna_achievement_copy_name(
                first, text, sizeof(FirstName) - 1U, &copied_bytes) == CNA_RESULT_SUCCESS &&
            copied_bytes == sizeof(FirstName) - 1U &&
            memcmp(text, FirstName, sizeof(FirstName) - 1U) == 0 &&
            (unsigned char)text[sizeof(FirstName) - 1U] == 0xA5U,
        "achievement name size and exact unterminated copy agree");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_achievement_get_description_size(first, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(FirstDescription) - 1U &&
            cna_achievement_copy_description(
                first, text, sizeof(FirstDescription) - 1U, &copied_bytes) ==
                CNA_RESULT_SUCCESS &&
            copied_bytes == sizeof(FirstDescription) - 1U &&
            memcmp(text, FirstDescription, sizeof(FirstDescription) - 1U) == 0 &&
            (unsigned char)text[sizeof(FirstDescription) - 1U] == 0xA5U,
        "achievement description size and exact unterminated copy agree");

    memset(text, 0xA5, sizeof(text));
    memset(untouched, 0xA5, sizeof(untouched));
    cna_demo_check(
        &demo,
        cna_achievement_copy_key(
            first, text, sizeof(FirstKey) - 2U, &copied_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied_bytes == sizeof(FirstKey) - 1U &&
            memcmp(text, untouched, sizeof(text)) == 0,
        "a short achievement text copy reports its size without a partial write");
    cna_demo_check(
        &demo,
        cna_achievement_get_how_to_earn_size(first, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(0) &&
            cna_achievement_copy_how_to_earn(
                first, NULL, UINT64_C(0), &copied_bytes) == CNA_RESULT_SUCCESS &&
            copied_bytes == UINT64_C(0),
        "a factory value has present but empty how-to-earn text");
    bytes = UINT64_C(99);
    copied_bytes = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_achievement_get_picture_size(first, &bytes) == CNA_RESULT_NOT_SUPPORTED &&
            cna_achievement_copy_picture(
                first, NULL, UINT64_C(0), &copied_bytes) == CNA_RESULT_NOT_SUPPORTED,
        "both picture routes truthfully refuse a value without configured artwork");

    cna_demo_check(
        &demo,
        cna_achievement_equals(first, same, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_achievement_equals(first, other, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "achievement equality compares complete values rather than handle identity");

    inputs[0] = first;
    inputs[1] = other;
    if (!cna_demo_check_result(
            &demo,
            cna_achievement_collection_create_ext(inputs, UINT64_C(2), &collection),
            "create a collection that copies two achievement values")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 2 &&
            cna_achievement_collection_get_is_disposed(collection, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "the new collection is live with both copied values");
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_is_read_only(collection, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "XNA achievement collections report read-only");

    cna_demo_check_result(
        &demo, cna_achievement_destroy(first),
        "release a source after the collection has copied it");
    first = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_at(collection, 0, &by_index) ==
                CNA_RESULT_SUCCESS &&
            by_index != CNA_INVALID_HANDLE &&
            cna_achievement_equals(by_index, same, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "indexed lookup returns an independent value equal to the released source");
    by_key = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_by_key(
            collection, text_view(OtherKey, sizeof(OtherKey) - 1U), &by_key) ==
                CNA_RESULT_SUCCESS &&
            by_key != CNA_INVALID_HANDLE &&
            cna_achievement_equals(by_key, other, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "key lookup returns an owned copy of the matching value");
    cna_demo_check(
        &demo,
        cna_achievement_collection_index_of(collection, other, &index) ==
                CNA_RESULT_SUCCESS &&
            index == 1 &&
            cna_achievement_collection_contains(collection, same, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "IndexOf and Contains use achievement value equality");
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_at(collection, -1, &copied[0]) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            copied[0] == CNA_INVALID_HANDLE &&
            cna_achievement_collection_get_at(collection, 2, &copied[0]) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            copied[0] == CNA_INVALID_HANDLE,
        "both indexed bounds are refused without publishing a value");
    copied[0] = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_by_key(
            collection, text_view("missing", sizeof("missing") - 1U), &copied[0]) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            copied[0] == CNA_INVALID_HANDLE,
        "an unknown key returns INVALID_ARGUMENT without publishing a value");

    memset(copied, 0xA5, sizeof(copied));
    memcpy(before, copied, sizeof(before));
    cna_demo_check(
        &demo,
        cna_achievement_collection_copy_to(
            collection, copied, UINT64_C(2), 1, &copied_count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied_count == UINT64_C(2) && memcmp(copied, before, sizeof(copied)) == 0,
        "a short shifted collection copy reports its count without publishing handles");
    cna_demo_check(
        &demo,
        cna_achievement_collection_copy_to(
            collection, copied, UINT64_C(4), 1, &copied_count) == CNA_RESULT_SUCCESS &&
            copied_count == UINT64_C(2) && copied[1] != CNA_INVALID_HANDLE &&
            copied[2] != CNA_INVALID_HANDLE &&
            cna_achievement_equals(copied[1], same, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "a sufficient shifted copy publishes independent ordered value handles");
    (void)cna_achievement_destroy(copied[1]);
    (void)cna_achievement_destroy(copied[2]);
    copied[1] = CNA_INVALID_HANDLE;
    copied[2] = CNA_INVALID_HANDLE;

    /* Genuine XNA refuses ICollection/IList mutation, even with invalid indices. */
    cna_demo_check(
        &demo,
        cna_achievement_collection_add(collection, same) == CNA_RESULT_NOT_SUPPORTED &&
            cna_achievement_collection_insert(collection, 1, other) == CNA_RESULT_NOT_SUPPORTED &&
            cna_achievement_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 2,
        "Add and Insert refuse modification and preserve the collection count");
    cna_demo_check(
        &demo,
        cna_achievement_collection_insert(collection, -1, other) == CNA_RESULT_NOT_SUPPORTED &&
            cna_achievement_collection_remove_at(collection, 4) == CNA_RESULT_NOT_SUPPORTED &&
            cna_achievement_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 2,
        "modification refusal precedes insertion and removal index validation");
    flag = CNA_TRUE;
    cna_demo_check(
        &demo,
        cna_achievement_collection_remove(collection, same, &flag) == CNA_RESULT_NOT_SUPPORTED &&
            flag == CNA_TRUE &&
            cna_achievement_collection_remove_at(collection, 1) == CNA_RESULT_NOT_SUPPORTED &&
            cna_achievement_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 2,
        "value and indexed removal refuse without publishing a removal result");
    cna_demo_check_result(
        &demo, cna_achievement_collection_create_ext(NULL, 0, &empty),
        "create an empty collection for absent-value and empty-clear refusal");
    flag = CNA_TRUE;
    cna_demo_check(
        &demo,
        cna_achievement_collection_remove(empty, same, &flag) == CNA_RESULT_NOT_SUPPORTED &&
            flag == CNA_TRUE && cna_achievement_collection_clear(empty) == CNA_RESULT_NOT_SUPPORTED &&
            cna_achievement_collection_get_count(empty, &count) == CNA_RESULT_SUCCESS && count == 0,
        "removing an absent value and clearing an empty collection still refuse modification");
    cna_demo_check(
        &demo,
        cna_achievement_collection_clear(collection) == CNA_RESULT_NOT_SUPPORTED &&
            cna_achievement_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS && count == 2,
        "Clear refuses without changing the collection count");
    cna_demo_check(
        &demo,
        cna_achievement_collection_copy_to(collection, copied, 4, 1, &copied_count) == CNA_RESULT_SUCCESS &&
            copied_count == 2 &&
            cna_achievement_equals(copied[1], same, &flag) == CNA_RESULT_SUCCESS && flag == CNA_TRUE &&
            cna_achievement_equals(copied[2], other, &flag) == CNA_RESULT_SUCCESS && flag == CNA_TRUE,
        "every refused mutation preserves both complete values and their order");
    (void)cna_achievement_destroy(copied[1]);
    (void)cna_achievement_destroy(copied[2]);
    copied[1] = CNA_INVALID_HANDLE;
    copied[2] = CNA_INVALID_HANDLE;
    copied[0] = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_by_key(collection, text_view("", 0), &copied[0]) ==
            CNA_RESULT_INVALID_ARGUMENT && copied[0] == CNA_INVALID_HANDLE,
        "an empty absent key returns INVALID_ARGUMENT and clears the output");

    cna_demo_check_result(
        &demo, cna_achievement_collection_destroy(collection),
        "dispose and release the achievement collection");
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_count(collection, &count) == CNA_RESULT_INVALID_HANDLE,
        "a released collection handle is rejected");
    collection = CNA_INVALID_HANDLE;

cleanup:
    if (empty != CNA_INVALID_HANDLE) {
        (void)cna_achievement_collection_destroy(empty);
    }
    if (collection != CNA_INVALID_HANDLE) {
        (void)cna_achievement_collection_destroy(collection);
    }
    if (by_key != CNA_INVALID_HANDLE) {
        (void)cna_achievement_destroy(by_key);
    }
    if (by_index != CNA_INVALID_HANDLE) {
        (void)cna_achievement_destroy(by_index);
    }
    if (other != CNA_INVALID_HANDLE) {
        (void)cna_achievement_destroy(other);
    }
    if (same != CNA_INVALID_HANDLE) {
        (void)cna_achievement_destroy(same);
    }
    if (first != CNA_INVALID_HANDLE) {
        (void)cna_achievement_destroy(first);
    }
    return cna_demo_finish(&demo);
}
