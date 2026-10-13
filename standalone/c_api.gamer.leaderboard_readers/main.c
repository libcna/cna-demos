/* SPDX-License-Identifier: MS-PL */
/* Canonical identities and local empty LeaderboardReader workflows through the C ABI. */
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

static void on_completed(void* context)
{
    ++*(int*)context;
}

static int identity_matches(
    const CNA_LeaderboardIdentity* identity, const char* key, int32_t game_mode)
{
    const size_t length = strlen(key);
    size_t index;
    if (identity->struct_size != (uint32_t)sizeof(*identity) ||
        identity->struct_version != UINT32_C(1) || identity->game_mode != game_mode ||
        length >= sizeof(identity->key) || memcmp(identity->key, key, length) != 0 ||
        identity->key[length] != '\0') {
        return 0;
    }
    for (index = length + 1U; index < sizeof(identity->key); ++index) {
        if (identity->key[index] != '\0') {
            return 0;
        }
    }
    return 1;
}

static int empty_info_matches(
    const CNA_LeaderboardReaderInfo* info, int32_t page_start, CNA_Bool can_page_up)
{
    return info->struct_size == (uint32_t)sizeof(*info) &&
        info->struct_version == UINT32_C(1) && info->page_start == page_start &&
        info->total_leaderboard_size == 0 && info->entry_count == 0 &&
        info->is_disposed == CNA_FALSE && info->can_page_down == CNA_FALSE &&
        info->can_page_up == can_page_up && info->reserved == 0U;
}

static void init_reader_info(CNA_LeaderboardReaderInfo* info)
{
    memset(info, 0xA5, sizeof(*info));
    info->struct_size = (uint32_t)sizeof(*info);
    info->struct_version = UINT32_C(1);
}

static void destroy_reader(CnaDemo* demo, CNA_LeaderboardReaderHandle* reader, const char* what)
{
    if (*reader != CNA_INVALID_HANDLE) {
        cna_demo_check_result(demo, cna_leaderboard_reader_destroy(*reader), what);
        *reader = CNA_INVALID_HANDLE;
    }
}

int main(int argc, char** argv)
{
    static const char AppName[] = "cna-demos-c-api-leaderboard-readers";
    static const char GamerName[] = "Reader Pivot";
    static const char* const Keys[] = {
        "BestScoreLifeTime", "BestScoreRecent", "BestTimeLifeTime", "BestTimeRecent"
    };
    CnaDemo demo;
    CNA_LeaderboardIdentity identity;
    CNA_LeaderboardIdentity returned_identity;
    CNA_LeaderboardIdentity malformed_identity;
    unsigned char malformed_identity_before[sizeof(CNA_LeaderboardIdentity)];
    CNA_LeaderboardReaderInfo info;
    CNA_LeaderboardReaderInfo malformed_info;
    unsigned char malformed_info_before[sizeof(CNA_LeaderboardReaderInfo)];
    CNA_SignedInGamerHandle gamer = CNA_INVALID_HANDLE;
    CNA_GamerHandle gamers[1];
    CNA_LeaderboardReaderHandle direct = CNA_INVALID_HANDLE;
    CNA_LeaderboardReaderHandle pivot = CNA_INVALID_HANDLE;
    CNA_LeaderboardReaderHandle restricted = CNA_INVALID_HANDLE;
    CNA_LeaderboardReaderHandle async_direct = CNA_INVALID_HANDLE;
    CNA_LeaderboardReaderHandle async_pivot = CNA_INVALID_HANDLE;
    CNA_LeaderboardReaderHandle async_restricted = CNA_INVALID_HANDLE;
    CNA_LeaderboardEntryHandle entry = UINT64_C(99);
    CNA_LeaderboardReaderHandle released = CNA_INVALID_HANDLE;
    int completions = 0;
    int identities_ok = 1;
    int index;

    cna_demo_init(&demo, "c_api.gamer.leaderboard_readers", argc, argv, 1);
    cna_demo_check_result(
        &demo, cna_storage_set_app_name_ext(text_view(AppName, sizeof(AppName) - 1U)),
        "select a dedicated offline leaderboard store");
    cna_demo_check_result(
        &demo, cna_gamer_set_signed_in_gamers_ext(NULL, UINT64_C(0)),
        "start with an empty published gamer collection");

    for (index = 0; index < 4; ++index) {
        memset(&identity, 0xA5, sizeof(identity));
        if (cna_leaderboard_identity_init((CNA_LeaderboardKey)index, 7, &identity) !=
                CNA_RESULT_SUCCESS ||
            !identity_matches(&identity, Keys[index], 7)) {
            identities_ok = 0;
        }
    }
    cna_demo_check(
        &demo, identities_ok,
        "all four canonical identities carry exact NUL-padded keys and the requested game mode");
    memset(&malformed_identity, 0xA5, sizeof(malformed_identity));
    memcpy(malformed_identity_before, &malformed_identity, sizeof(malformed_identity));
    cna_demo_check(
        &demo,
        cna_leaderboard_identity_init(
            CNA_LEADERBOARD_KEY_MAXIMUM + UINT32_C(1), 7, &malformed_identity) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(
                &malformed_identity, malformed_identity_before,
                sizeof(malformed_identity)) == 0,
        "an undefined identity key is rejected before changing the output");
    cna_demo_check_result(
        &demo,
        cna_leaderboard_identity_init(CNA_LEADERBOARD_KEY_BEST_SCORE_LIFE_TIME, 7, &identity),
        "select the canonical lifetime-score identity");
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(GamerName, sizeof(GamerName) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_ONE, &gamer),
            "create a valid local pivot gamer")) {
        goto cleanup;
    }
    gamers[0] = gamer;

    if (!cna_demo_check_result(
            &demo, cna_leaderboard_reader_read(&identity, 4, 2, &direct),
            "read an empty page at an explicit starting index")) {
        goto cleanup;
    }
    init_reader_info(&info);
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_get_info(direct, &info) == CNA_RESULT_SUCCESS &&
            empty_info_matches(&info, 4, CNA_TRUE),
        "the local reader preserves its requested start and reports a complete empty page");
    memset(&malformed_info, 0xA5, sizeof(malformed_info));
    malformed_info.struct_size = (uint32_t)sizeof(malformed_info);
    malformed_info.struct_version = UINT32_C(2);
    memcpy(malformed_info_before, &malformed_info, sizeof(malformed_info));
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_get_info(direct, &malformed_info) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&malformed_info, malformed_info_before, sizeof(malformed_info)) == 0,
        "an unknown reader-info version is rejected without changing the output");
    memset(&returned_identity, 0xA5, sizeof(returned_identity));
    returned_identity.struct_size = (uint32_t)sizeof(returned_identity);
    returned_identity.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_get_identity(direct, &returned_identity) ==
                CNA_RESULT_SUCCESS &&
            identity_matches(&returned_identity, Keys[0], 7),
        "the reader returns its exact versioned leaderboard identity");
    entry = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_get_entry_at(direct, 0, &entry) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            entry == CNA_INVALID_HANDLE,
        "entry lookup at the empty-page bound fails with a deterministic invalid handle");
    completions = 0;
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_page_down(direct) == CNA_RESULT_INVALID_STATE &&
            cna_leaderboard_reader_begin_page_down(direct, on_completed, &completions) ==
                CNA_RESULT_INVALID_STATE &&
            completions == 0,
        "synchronous and callback page-down forms refuse an empty final page without completion");
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_page_up(direct) == CNA_RESULT_SUCCESS &&
            cna_leaderboard_reader_get_info(direct, &info) == CNA_RESULT_SUCCESS &&
            empty_info_matches(&info, 2, CNA_TRUE),
        "synchronous page-up moves from the requested sparse start toward zero");
    completions = 0;
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_begin_page_up(direct, on_completed, &completions) ==
                CNA_RESULT_SUCCESS &&
            completions == 1 &&
            cna_leaderboard_reader_get_info(direct, &info) == CNA_RESULT_SUCCESS &&
            empty_info_matches(&info, 0, CNA_FALSE) &&
            cna_leaderboard_reader_begin_page_up(direct, on_completed, &completions) ==
                CNA_RESULT_INVALID_STATE &&
            completions == 1,
        "callback page-up completes once at zero and does not complete a refused repeat");

    cna_demo_check_result(
        &demo,
        cna_leaderboard_reader_read_from_pivot(&identity, gamer, 5, &pivot),
        "read the empty leaderboard centered on a live pivot");
    init_reader_info(&info);
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_get_info(pivot, &info) == CNA_RESULT_SUCCESS &&
            empty_info_matches(&info, 0, CNA_FALSE),
        "an absent pivot falls back to the top of the empty leaderboard");
    cna_demo_check_result(
        &demo,
        cna_leaderboard_reader_read_from_gamers(
            &identity, gamers, UINT64_C(1), gamer, 5, &restricted),
        "read the empty leaderboard restricted to one valid gamer");
    init_reader_info(&info);
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_get_info(restricted, &info) == CNA_RESULT_SUCCESS &&
            empty_info_matches(&info, 0, CNA_FALSE),
        "the gamer-restricted empty result is a valid owned reader");

    completions = 0;
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_begin_read(
            &identity, 0, 3, on_completed, &completions, &async_direct) ==
                CNA_RESULT_SUCCESS &&
            async_direct != CNA_INVALID_HANDLE && completions == 1,
        "the callback explicit-page read completes exactly once before returning");
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_begin_read_from_pivot(
            &identity, gamer, 3, on_completed, &completions, &async_pivot) ==
                CNA_RESULT_SUCCESS &&
            async_pivot != CNA_INVALID_HANDLE && completions == 2,
        "the callback pivot read completes exactly once before returning");
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_begin_read_from_gamers(
            &identity, gamers, UINT64_C(1), gamer, 3, on_completed, &completions,
            &async_restricted) == CNA_RESULT_SUCCESS &&
            async_restricted != CNA_INVALID_HANDLE && completions == 3,
        "the callback gamer-restricted read completes exactly once before returning");

    released = direct;
    destroy_reader(&demo, &direct, "dispose and release the explicitly indexed reader");
    init_reader_info(&info);
    info.page_start = 12345;
    cna_demo_check(
        &demo,
        cna_leaderboard_reader_get_info(released, &info) == CNA_RESULT_INVALID_HANDLE &&
            info.page_start == 12345 &&
            cna_leaderboard_reader_destroy(released) == CNA_RESULT_INVALID_HANDLE,
        "a released reader rejects stale inspection and repeated destruction atomically");

cleanup:
    destroy_reader(&demo, &direct, "release the explicit reader during cleanup");
    destroy_reader(&demo, &pivot, "release the synchronous pivot reader");
    destroy_reader(&demo, &restricted, "release the synchronous restricted reader");
    destroy_reader(&demo, &async_direct, "release the callback explicit reader");
    destroy_reader(&demo, &async_pivot, "release the callback pivot reader");
    destroy_reader(&demo, &async_restricted, "release the callback restricted reader");
    cna_demo_check_result(
        &demo, cna_gamer_set_signed_in_gamers_ext(NULL, UINT64_C(0)),
        "leave the published gamer collection empty");
    if (gamer != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(gamer),
            "release the local pivot gamer");
    }
    return cna_demo_finish(&demo);
}
