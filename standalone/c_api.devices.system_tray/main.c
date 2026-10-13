/* SPDX-License-Identifier: MS-PL */
/*
 * System tray from C: inspect host availability without creating an icon, then use CNA's public
 * per-handle test tray for entry state, callbacks, upper bounds and owned lifetime.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ClickRecord {
    int calls;
    int last_id;
    int id;
} ClickRecord;

typedef struct TrayDemoState {
    CnaDemo* demo;
    int validated;
} TrayDemoState;

static CNA_StringView text_view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static void on_entry_click(void* context)
{
    ClickRecord* record = (ClickRecord*)context;
    ++record->calls;
    record->last_id = record->id;
}

static void tray_contract(CNA_Handle game, CnaDemo* demo)
{
    CNA_SystemTrayHandle tray = CNA_INVALID_HANDLE;
    CNA_Bool supported = UINT8_C(9);
    CNA_Bool value = UINT8_C(9);
    uint64_t open_index = UINT64_MAX;
    uint64_t mute_index = UINT64_MAX;
    uint64_t quit_index = UINT64_MAX;
    ClickRecord open_clicks;
    ClickRecord quit_clicks;

    memset(&open_clicks, 0, sizeof(open_clicks));
    memset(&quit_clicks, 0, sizeof(quit_clicks));
    open_clicks.id = 10;
    quit_clicks.id = 30;

    cna_demo_check(
        demo,
        cna_system_tray_get_is_supported_ext(game, &supported) == CNA_RESULT_SUCCESS &&
            (supported == CNA_FALSE || supported == CNA_TRUE),
        "real tray availability is readable without creating a host icon");
    printf("  platform system tray supported: %u\n", (unsigned)supported);

    if (!cna_demo_check(
            demo,
            cna_system_tray_create_with_test_backend_ext(
                game, text_view("CNA demo"), &tray) == CNA_RESULT_SUCCESS &&
                tray != CNA_INVALID_HANDLE,
            "the in-memory backend creates one owned tray handle without host UI")) {
        return;
    }

    cna_demo_check(
        demo,
        cna_system_tray_set_tooltip(tray, text_view("CNA tray model")) == CNA_RESULT_SUCCESS &&
            cna_system_tray_add_entry(
                tray, text_view("Open"), CNA_FALSE, CNA_FALSE, CNA_TRUE,
                on_entry_click, &open_clicks, &open_index) == CNA_RESULT_SUCCESS &&
            cna_system_tray_add_entry(
                tray, text_view("Mute"), CNA_TRUE, CNA_TRUE, CNA_TRUE,
                NULL, NULL, &mute_index) == CNA_RESULT_SUCCESS &&
            cna_system_tray_add_entry(
                tray, text_view("Quit"), CNA_FALSE, CNA_FALSE, CNA_FALSE,
                on_entry_click, &quit_clicks, &quit_index) == CNA_RESULT_SUCCESS &&
            open_index == UINT64_C(0) && mute_index == UINT64_C(1) &&
            quit_index == UINT64_C(2),
        "tooltip update and three menu additions publish contiguous stable indices");

    cna_demo_check(
        demo,
        cna_system_tray_add_entry(
            tray, text_view("Missing output"), CNA_FALSE, CNA_FALSE, CNA_TRUE,
            NULL, NULL, NULL) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_system_tray_click_entry_for_tests_ext(tray, UINT64_C(3)) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "a missing index output refuses the add before mutation");

    cna_demo_check(
        demo,
        cna_system_tray_get_entry_checked(tray, mute_index, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE &&
            cna_system_tray_set_entry_checked(tray, mute_index, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_system_tray_get_entry_checked(tray, mute_index, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE,
        "a checkable entry exposes and mutates its initial checked state");

    cna_demo_check(
        demo,
        cna_system_tray_get_entry_enabled(tray, open_index, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE &&
            cna_system_tray_get_entry_enabled(tray, quit_index, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_system_tray_set_entry_enabled(tray, open_index, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_system_tray_set_entry_enabled(tray, quit_index, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_system_tray_get_entry_enabled(tray, open_index, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_system_tray_get_entry_enabled(tray, quit_index, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE,
        "enabled state starts per entry and round-trips independently");

    cna_demo_check(
        demo,
        cna_system_tray_set_entry_label(
            tray, open_index, text_view("Show window")) == CNA_RESULT_SUCCESS,
        "an existing menu entry accepts a replacement label");

    cna_demo_check(
        demo,
        cna_system_tray_set_entry_label(
            tray, UINT64_C(99), text_view("Ghost")) == CNA_RESULT_SUCCESS &&
            cna_system_tray_set_entry_checked(
                tray, UINT64_C(99), CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_system_tray_set_entry_enabled(
                tray, UINT64_C(99), CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_system_tray_get_entry_checked(
                tray, UINT64_C(99), &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_system_tray_get_entry_enabled(
                tray, UINT64_C(99), &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE,
        "upper-bound entry mutations are ignored and both state reads return false");

    cna_demo_check(
        demo,
        cna_system_tray_click_entry_for_tests_ext(tray, open_index) == CNA_RESULT_SUCCESS &&
            open_clicks.calls == 1 && open_clicks.last_id == 10 && quit_clicks.calls == 0 &&
            cna_system_tray_click_entry_for_tests_ext(tray, quit_index) == CNA_RESULT_SUCCESS &&
            open_clicks.calls == 1 && quit_clicks.calls == 1 && quit_clicks.last_id == 30,
        "test activations synchronously invoke only the selected entry's context callback");

    cna_demo_check(
        demo,
        cna_system_tray_click_entry_for_tests_ext(tray, mute_index) == CNA_RESULT_SUCCESS &&
            open_clicks.calls == 1 && quit_clicks.calls == 1 &&
            cna_system_tray_click_entry_for_tests_ext(tray, UINT64_C(99)) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "an entry without a handler is silent and an absent entry is refused");

    cna_demo_check_result(demo, cna_system_tray_destroy(tray),
                          "destroy removes the in-memory tray and releases its handle");
    cna_demo_check(
        demo,
        cna_system_tray_destroy(tray) == CNA_RESULT_INVALID_HANDLE &&
            cna_system_tray_set_tooltip(tray, text_view("gone")) == CNA_RESULT_INVALID_HANDLE &&
            cna_system_tray_get_entry_checked(tray, mute_index, &value) ==
                CNA_RESULT_INVALID_HANDLE,
        "the released tray refuses repeated destruction and stale property access");
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    TrayDemoState* state = (TrayDemoState*)context;
    (void)game_time;
    (void)out_error;
    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    tray_contract(game, state->demo);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    TrayDemoState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.devices.system_tray", argc, argv, 0);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                          "run one frame for owner-thread tray operations");
    cna_demo_check(&demo, state.validated == 1,
                   "the update callback completed the tray audit once");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
