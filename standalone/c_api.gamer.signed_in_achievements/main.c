/* SPDX-License-Identifier: MS-PL */
/* SignedInGamer achievement award/read adapters and offline catalog persistence through the C ABI. */
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

static void release_achievement(CNA_AchievementHandle* achievement)
{
    if (*achievement != CNA_INVALID_HANDLE) {
        (void)cna_achievement_destroy(*achievement);
        *achievement = CNA_INVALID_HANDLE;
    }
}

static void release_collection(CNA_AchievementCollectionHandle* collection)
{
    if (*collection != CNA_INVALID_HANDLE) {
        (void)cna_achievement_collection_destroy(*collection);
        *collection = CNA_INVALID_HANDLE;
    }
}

static CNA_AchievementInfo achievement_info(CNA_AchievementHandle achievement)
{
    CNA_AchievementInfo info;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    if (cna_achievement_get_info(achievement, &info) != CNA_RESULT_SUCCESS) {
        info.struct_version = UINT32_C(0);
    }
    return info;
}

int main(int argc, char** argv)
{
    static const char AppName[] = "cna-demos-c-api-signed-in-achievements";
    static const char Gamertag[] = "C API Achievement Reader";
    static const char FirstKey[] = "first_steps";
    static const char ExplorerKey[] = "explorer";
    static const char FirstName[] = "First Steps";
    static const char FirstHow[] = "Run the achievement demo.";
    CnaDemo demo;
    CNA_SignedInGamerHandle gamer = CNA_INVALID_HANDLE;
    CNA_AchievementCollectionHandle before = CNA_INVALID_HANDLE;
    CNA_AchievementCollectionHandle after = CNA_INVALID_HANDLE;
    CNA_AchievementCollectionHandle repeated = CNA_INVALID_HANDLE;
    CNA_AchievementCollectionHandle async_read = CNA_INVALID_HANDLE;
    CNA_AchievementCollectionHandle reloaded = CNA_INVALID_HANDLE;
    CNA_AchievementHandle first = CNA_INVALID_HANDLE;
    CNA_AchievementHandle explorer = CNA_INVALID_HANDLE;
    CNA_AchievementInfo before_first;
    CNA_AchievementInfo after_first;
    CNA_AchievementInfo after_explorer;
    CNA_AchievementInfo current;
    int32_t count = -1;
    int completions = 0;
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    char text[64];

    cna_demo_init(&demo, "c_api.gamer.signed_in_achievements", argc, argv, 1);
    if (!cna_demo_check_result(
            &demo,
            cna_storage_set_app_name_ext(text_view(AppName, sizeof(AppName) - 1U)),
            "select the demo's isolated persistent application store") ||
        !cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(Gamertag, sizeof(Gamertag) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_ONE, &gamer),
            "create the offline achievement owner")) {
        goto cleanup;
    }

    if (!cna_demo_check_result(
            &demo, cna_signed_in_gamer_get_achievements(gamer, &before),
            "read the title achievement catalog synchronously")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_count(before, &count) == CNA_RESULT_SUCCESS &&
            count == 3,
        "the synchronous result contains all three title-catalog entries");
    if (!cna_demo_check_result(
            &demo,
            cna_achievement_collection_get_by_key(
                before, text_view(FirstKey, sizeof(FirstKey) - 1U), &first),
            "look up the first catalog achievement by key")) {
        goto cleanup;
    }
    before_first = achievement_info(first);
    cna_demo_check(
        &demo,
        before_first.struct_version == UINT32_C(1) && before_first.gamer_score == 10 &&
            before_first.display_before_earned == CNA_TRUE &&
            ((!before_first.is_earned && before_first.earned_date_time_ticks == INT64_C(0)) ||
             (before_first.is_earned &&
              before_first.earned_date_time_ticks > INT64_C(0))),
        "the catalog supplies score and visibility while prior earned state stays coherent");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_achievement_get_name_size(first, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(FirstName) - 1U &&
            cna_achievement_copy_name(first, text, bytes, &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, FirstName, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "the synchronous result carries exact catalog display text");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_achievement_get_how_to_earn_size(first, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(FirstHow) - 1U &&
            cna_achievement_copy_how_to_earn(first, text, bytes, &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, FirstHow, (size_t)bytes) == 0,
        "the catalog's how-to-earn text survives the C collection adapter");
    release_achievement(&first);
    release_collection(&before);

    cna_demo_check_result(
        &demo,
        cna_signed_in_gamer_award_achievement(
            gamer, text_view(FirstKey, sizeof(FirstKey) - 1U)),
        "award the first catalog achievement synchronously");
    if (!cna_demo_check_result(
            &demo, cna_signed_in_gamer_get_achievements(gamer, &after),
            "read the catalog after the synchronous award") ||
        !cna_demo_check_result(
            &demo,
            cna_achievement_collection_get_by_key(
                after, text_view(FirstKey, sizeof(FirstKey) - 1U), &first),
            "read back the synchronously awarded value")) {
        goto cleanup;
    }
    after_first = achievement_info(first);
    cna_demo_check(
        &demo,
        after_first.struct_version == UINT32_C(1) && after_first.is_earned == CNA_TRUE &&
            after_first.earned_date_time_ticks > INT64_C(0) &&
            (!before_first.is_earned ||
             after_first.earned_date_time_ticks == before_first.earned_date_time_ticks),
        "the synchronous award is earned and preserves an existing first-earned date");
    release_achievement(&first);
    release_collection(&after);

    cna_demo_check_result(
        &demo,
        cna_signed_in_gamer_award_achievement(
            gamer, text_view(FirstKey, sizeof(FirstKey) - 1U)),
        "repeat the synchronous award as an idempotent operation");
    if (!cna_demo_check_result(
            &demo, cna_signed_in_gamer_get_achievements(gamer, &repeated),
            "read the catalog after the repeated award") ||
        !cna_demo_check_result(
            &demo,
            cna_achievement_collection_get_by_key(
                repeated, text_view(FirstKey, sizeof(FirstKey) - 1U), &first),
            "read the repeatedly awarded value")) {
        goto cleanup;
    }
    current = achievement_info(first);
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_count(repeated, &count) == CNA_RESULT_SUCCESS &&
            count == 3 && current.struct_version == UINT32_C(1) &&
            current.earned_date_time_ticks == after_first.earned_date_time_ticks,
        "a repeated award neither duplicates the catalog entry nor changes its first date");
    release_achievement(&first);
    release_collection(&repeated);

    completions = 0;
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_begin_award_achievement(
            gamer, text_view(ExplorerKey, sizeof(ExplorerKey) - 1U),
            on_completed, &completions) == CNA_RESULT_SUCCESS &&
            completions == 1,
        "the asynchronous award adapter completes exactly once before returning");
    completions = 0;
    if (!cna_demo_check(
            &demo,
            cna_signed_in_gamer_begin_get_achievements(
                gamer, on_completed, &completions, &async_read) == CNA_RESULT_SUCCESS &&
                completions == 1 && async_read != CNA_INVALID_HANDLE,
            "the asynchronous read completes once with an owned collection")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_achievement_collection_get_count(async_read, &count) == CNA_RESULT_SUCCESS &&
            count == 3,
        "the asynchronous result retains the complete catalog");
    if (!cna_demo_check_result(
            &demo,
            cna_achievement_collection_get_by_key(
                async_read, text_view(FirstKey, sizeof(FirstKey) - 1U), &first),
            "read the first value from the asynchronous result") ||
        !cna_demo_check_result(
            &demo,
            cna_achievement_collection_get_by_key(
                async_read, text_view(ExplorerKey, sizeof(ExplorerKey) - 1U), &explorer),
            "read the asynchronously awarded value")) {
        goto cleanup;
    }
    current = achievement_info(first);
    after_explorer = achievement_info(explorer);
    cna_demo_check(
        &demo,
        current.struct_version == UINT32_C(1) && current.is_earned == CNA_TRUE &&
            current.earned_date_time_ticks == after_first.earned_date_time_ticks &&
            after_explorer.struct_version == UINT32_C(1) &&
            after_explorer.is_earned == CNA_TRUE &&
            after_explorer.earned_date_time_ticks > INT64_C(0),
        "the asynchronous result contains both earned values and stable timestamps");
    release_achievement(&explorer);
    release_achievement(&first);
    release_collection(&async_read);

    cna_demo_check_result(
        &demo, cna_signed_in_gamer_destroy(gamer),
        "release the writer before constructing a fresh reader");
    gamer = CNA_INVALID_HANDLE;
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(Gamertag, sizeof(Gamertag) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_ONE, &gamer),
            "recreate the same offline profile") ||
        !cna_demo_check_result(
            &demo, cna_signed_in_gamer_get_achievements(gamer, &reloaded),
            "load its achievements through a fresh gamer object")) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo,
            cna_achievement_collection_get_by_key(
                reloaded, text_view(FirstKey, sizeof(FirstKey) - 1U), &first),
            "look up the persisted synchronous award") ||
        !cna_demo_check_result(
            &demo,
            cna_achievement_collection_get_by_key(
                reloaded, text_view(ExplorerKey, sizeof(ExplorerKey) - 1U), &explorer),
            "look up the persisted asynchronous award")) {
        goto cleanup;
    }
    current = achievement_info(first);
    {
        const CNA_AchievementInfo reloaded_explorer = achievement_info(explorer);
        cna_demo_check(
            &demo,
            current.struct_version == UINT32_C(1) && current.is_earned == CNA_TRUE &&
                current.earned_date_time_ticks == after_first.earned_date_time_ticks &&
                reloaded_explorer.struct_version == UINT32_C(1) &&
                reloaded_explorer.is_earned == CNA_TRUE &&
                reloaded_explorer.earned_date_time_ticks ==
                    after_explorer.earned_date_time_ticks,
            "both sync and async awards persist across complete gamer-object replacement");
    }

cleanup:
    release_achievement(&explorer);
    release_achievement(&first);
    release_collection(&reloaded);
    release_collection(&async_read);
    release_collection(&repeated);
    release_collection(&after);
    release_collection(&before);
    if (gamer != CNA_INVALID_HANDLE) {
        (void)cna_signed_in_gamer_destroy(gamer);
    }
    return cna_demo_finish(&demo);
}
