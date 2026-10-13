/* SPDX-License-Identifier: MS-PL */
/* Leaderboard entry values, callbacks, columns and ownership through the C ABI. */
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

static void on_rating_changed(void* context)
{
    ++*(int*)context;
}

int main(int argc, char** argv)
{
    static const char GamerName[] = "Entry Gamer";
    CnaDemo demo;
    CNA_SignedInGamerHandle gamer = CNA_INVALID_HANDLE;
    CNA_GamerHandle borrowed_gamer = CNA_INVALID_HANDLE;
    CNA_LeaderboardEntryHandle first = CNA_INVALID_HANDLE;
    CNA_LeaderboardEntryHandle released_first = CNA_INVALID_HANDLE;
    CNA_LeaderboardEntryHandle same = CNA_INVALID_HANDLE;
    CNA_LeaderboardEntryHandle anonymous = CNA_INVALID_HANDLE;
    CNA_PropertyDictionaryHandle columns = CNA_INVALID_HANDLE;
    CNA_LeaderboardEntryInfo info;
    CNA_LeaderboardEntryInfo malformed;
    unsigned char malformed_before[sizeof(CNA_LeaderboardEntryInfo)];
    CNA_Bool flag = CNA_FALSE;
    uint64_t bytes = UINT64_C(0);
    int32_t count = -1;
    int32_t wins = -1;
    int callbacks = 0;
    char name[32];

    cna_demo_init(&demo, "c_api.gamer.leaderboard_entries", argc, argv, 1);
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(GamerName, sizeof(GamerName) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_ONE, &gamer),
            "create the leaderboard entry's gamer") ||
        !cna_demo_check_result(
            &demo, cna_leaderboard_entry_create_ext(gamer, INT64_C(100), 5, &first),
            "create a gamer-backed leaderboard entry") ||
        !cna_demo_check_result(
            &demo, cna_leaderboard_entry_create_ext(gamer, INT64_C(100), 5, &same),
            "create an independently owned equal entry") ||
        !cna_demo_check_result(
            &demo,
            cna_leaderboard_entry_create_ext(
                CNA_INVALID_HANDLE, INT64_C(100), 5, &anonymous),
            "create an anonymous entry with the same score and rank")) {
        goto cleanup;
    }

    memset(&info, 0xA5, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_leaderboard_entry_get_info(first, &info) == CNA_RESULT_SUCCESS &&
            info.struct_size == (uint32_t)sizeof(info) && info.struct_version == UINT32_C(1) &&
            info.ranking == 5 && info.has_gamer == CNA_TRUE && info.rating == INT64_C(100) &&
            info.reserved[0] == 0U && info.reserved[1] == 0U && info.reserved[2] == 0U,
        "entry info preserves the exact gamer, ranking and 64-bit rating with zero reserve");
    memset(&malformed, 0xA5, sizeof(malformed));
    malformed.struct_size = (uint32_t)sizeof(malformed);
    malformed.struct_version = UINT32_C(2);
    memcpy(malformed_before, &malformed, sizeof(malformed));
    cna_demo_check(
        &demo,
        cna_leaderboard_entry_get_info(first, &malformed) == CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&malformed, malformed_before, sizeof(malformed)) == 0,
        "an unknown entry-info version is rejected without changing the output");

    if (cna_demo_check_result(
            &demo,
            cna_leaderboard_entry_get_gamer(first, &flag, &borrowed_gamer),
            "borrow the gamer named by the entry")) {
        memset(name, 0xA5, sizeof(name));
        cna_demo_check(
            &demo,
            flag == CNA_TRUE && borrowed_gamer != CNA_INVALID_HANDLE &&
                cna_gamer_get_gamertag_size(borrowed_gamer, &bytes) == CNA_RESULT_SUCCESS &&
                bytes == sizeof(GamerName) - 1U &&
                cna_gamer_copy_gamertag(borrowed_gamer, name, bytes, &bytes) ==
                    CNA_RESULT_SUCCESS &&
                bytes == sizeof(GamerName) - 1U &&
                memcmp(name, GamerName, sizeof(GamerName) - 1U) == 0 &&
                (unsigned char)name[bytes] == 0xA5U,
            "the owned borrowed handle exposes the exact source gamer identity");
        cna_demo_check_result(
            &demo, cna_gamer_destroy(borrowed_gamer),
            "release the independent borrowed gamer handle");
        borrowed_gamer = CNA_INVALID_HANDLE;
    }

    borrowed_gamer = UINT64_C(99);
    flag = CNA_TRUE;
    cna_demo_check(
        &demo,
        cna_leaderboard_entry_get_gamer(anonymous, &flag, &borrowed_gamer) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && borrowed_gamer == CNA_INVALID_HANDLE,
        "an anonymous entry reports absence and the implementation initializes its output handle");

    if (!cna_demo_check_result(
            &demo, cna_leaderboard_entry_get_columns(first, &columns),
            "borrow the entry's live columns dictionary")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_property_dictionary_get_count(columns, &count) == CNA_RESULT_SUCCESS && count == 0 &&
            cna_property_dictionary_set_int32(
                columns, text_view("Wins", sizeof("Wins") - 1U), 7) == CNA_RESULT_SUCCESS &&
            cna_property_dictionary_get_int32(
                columns, text_view("Wins", sizeof("Wins") - 1U), &wins) == CNA_RESULT_SUCCESS &&
            wins == 7,
        "the entry's own initially empty columns are mutable through their alias handle");

    flag = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_leaderboard_entry_equals(first, same, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_leaderboard_entry_equals(first, anonymous, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "entry equality includes gamer, rating and ranking but ignores changed columns");
    cna_demo_check(
        &demo,
        cna_leaderboard_entry_set_rating_changed_hook_ext(
            first, on_rating_changed, &callbacks) == CNA_RESULT_SUCCESS &&
            cna_leaderboard_entry_set_rating(first, INT64_C(250)) == CNA_RESULT_SUCCESS &&
            callbacks == 1 &&
            cna_leaderboard_entry_get_info(first, &info) == CNA_RESULT_SUCCESS &&
            info.rating == INT64_C(250),
        "the rating hook runs exactly once before a changed rating returns");
    cna_demo_check(
        &demo,
        cna_leaderboard_entry_equals(first, same, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_leaderboard_entry_set_rating(same, INT64_C(250)) == CNA_RESULT_SUCCESS &&
            cna_leaderboard_entry_equals(first, same, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "rating mutation changes equality while columns remain excluded");
    cna_demo_check(
        &demo,
        cna_leaderboard_entry_set_rating_changed_hook_ext(first, NULL, NULL) ==
                CNA_RESULT_SUCCESS &&
            cna_leaderboard_entry_set_rating(first, INT64_C(260)) == CNA_RESULT_SUCCESS &&
            callbacks == 1,
        "detaching the single rating hook prevents later callbacks");

    released_first = first;
    cna_demo_check_result(
        &demo, cna_leaderboard_entry_destroy(first),
        "release the entry while its columns alias remains live");
    first = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_property_dictionary_get_int32(
            columns, text_view("Wins", sizeof("Wins") - 1U), &wins) == CNA_RESULT_SUCCESS &&
            wins == 7,
        "the columns handle retains its entry storage after the entry handle is released");
    cna_demo_check_result(
        &demo, cna_property_dictionary_destroy(columns),
        "release the retained columns alias");
    columns = CNA_INVALID_HANDLE;
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.rating = INT64_C(999);
    cna_demo_check(
        &demo,
        cna_leaderboard_entry_get_info(released_first, &info) == CNA_RESULT_INVALID_HANDLE &&
            info.rating == INT64_C(999),
        "a released entry rejects stale reads without changing the caller's payload");

cleanup:
    if (borrowed_gamer != CNA_INVALID_HANDLE) {
        (void)cna_gamer_destroy(borrowed_gamer);
    }
    if (columns != CNA_INVALID_HANDLE) {
        (void)cna_property_dictionary_destroy(columns);
    }
    if (first != CNA_INVALID_HANDLE) {
        (void)cna_leaderboard_entry_destroy(first);
    }
    if (same != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_leaderboard_entry_destroy(same),
            "release the equal entry snapshot");
    }
    if (anonymous != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_leaderboard_entry_destroy(anonymous),
            "release the anonymous entry snapshot");
    }
    if (gamer != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(gamer),
            "release the entry's source gamer");
    }
    return cna_demo_finish(&demo);
}
