/* SPDX-License-Identifier: MS-PL */
/*
 * Dialogs from C: inspect real availability without opening UI, then use CNA's public immediate
 * test service for message choices, file-result callbacks, cancellation and invalid input.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct DialogResult {
    int calls;
    int valid;
    int files_were_null;
    uint64_t count;
    char first[128];
    char second[128];
} DialogResult;

typedef struct DialogDemoState {
    CnaDemo* demo;
    int validated;
} DialogDemoState;

static CNA_StringView text_view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static void copy_borrowed_text(char* destination, size_t capacity, CNA_StringView source, int* valid)
{
    if (source.byte_length >= (uint64_t)capacity ||
        (source.byte_length != UINT64_C(0) && source.data == NULL)) {
        *valid = 0;
        return;
    }
    if (source.byte_length != UINT64_C(0)) {
        memcpy(destination, source.data, (size_t)source.byte_length);
    }
    destination[source.byte_length] = '\0';
}

static void on_file_result(const CNA_StringView* files, uint64_t count, void* context)
{
    DialogResult* result = (DialogResult*)context;
    ++result->calls;
    result->valid = count <= UINT64_C(2) && (count == UINT64_C(0) || files != NULL);
    result->files_were_null = files == NULL;
    result->count = count;
    result->first[0] = '\0';
    result->second[0] = '\0';
    if (result->valid && count > UINT64_C(0)) {
        copy_borrowed_text(result->first, sizeof(result->first), files[0], &result->valid);
    }
    if (result->valid && count > UINT64_C(1)) {
        copy_borrowed_text(result->second, sizeof(result->second), files[1], &result->valid);
    }
}

static void message_box_contract(CNA_Handle game, CnaDemo* demo)
{
    CNA_MessageBoxTestLog log;
    CNA_StringView buttons[2];
    CNA_Bool supported = UINT8_C(9);
    int32_t chosen = -99;

    buttons[0] = text_view("Keep");
    buttons[1] = text_view("Discard");
    cna_demo_check(
        demo,
        cna_message_box_get_is_supported_ext(game, &supported) == CNA_RESULT_SUCCESS &&
            (supported == CNA_FALSE || supported == CNA_TRUE) &&
            cna_message_box_get_test_log_ext(game, &log) == CNA_RESULT_INVALID_STATE,
        "real message-box availability is readable without opening a modal window");
    printf("  platform message boxes supported: %u\n", (unsigned)supported);

    cna_demo_check_result(
        demo, cna_message_box_set_test_backend_ext(game, CNA_TRUE, 1),
        "install the immediate message-box backend with choice one");
    memset(&log, 0x7f, sizeof(log));
    cna_demo_check(
        demo,
        cna_message_box_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.struct_size == (uint32_t)sizeof(log) && log.struct_version == UINT32_C(1) &&
            log.simple_calls == UINT32_C(0) && log.choice_calls == UINT32_C(0) &&
            log.last_type == CNA_MESSAGE_BOX_TYPE_ERROR &&
            log.last_button_count == UINT32_C(0),
        "a newly installed message service exposes a versioned zero-request log");

    cna_demo_check(
        demo,
        cna_message_box_show_simple_ext(
            game, CNA_MESSAGE_BOX_TYPE_WARNING, text_view("Unsaved game"),
            text_view("Progress remains in memory.")) == CNA_RESULT_SUCCESS &&
            cna_message_box_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.simple_calls == UINT32_C(1) && log.choice_calls == UINT32_C(0) &&
            log.last_type == CNA_MESSAGE_BOX_TYPE_WARNING,
        "a dismiss-only warning reaches the test service with its severity");

    cna_demo_check(
        demo,
        cna_message_box_show_ext(
            game, CNA_MESSAGE_BOX_TYPE_INFORMATION, text_view("Save slot"),
            text_view("Keep this version?"), buttons, UINT64_C(2), &chosen) ==
                CNA_RESULT_SUCCESS &&
            chosen == 1 &&
            cna_message_box_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.simple_calls == UINT32_C(1) && log.choice_calls == UINT32_C(1) &&
            log.last_type == CNA_MESSAGE_BOX_TYPE_INFORMATION &&
            log.last_button_count == UINT32_C(2),
        "a two-button information request returns the authored choice and records its shape");

    cna_demo_check(
        demo,
        cna_message_box_show_simple_ext(
            game, CNA_MESSAGE_BOX_TYPE_MAXIMUM + UINT32_C(1), text_view("bad"),
            text_view("bad")) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_message_box_show_ext(
                game, CNA_MESSAGE_BOX_TYPE_ERROR, text_view("bad"), text_view("bad"),
                buttons, UINT64_C(0), &chosen) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_message_box_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.simple_calls == UINT32_C(1) && log.choice_calls == UINT32_C(1),
        "undefined severity and an empty choice list are refused before service dispatch");

    chosen = -99;
    cna_demo_check(
        demo,
        cna_message_box_set_test_backend_ext(game, CNA_TRUE, -1) == CNA_RESULT_SUCCESS &&
            cna_message_box_show_ext(
                game, CNA_MESSAGE_BOX_TYPE_ERROR, text_view("Close"), text_view("No choice"),
                buttons, UINT64_C(2), &chosen) == CNA_RESULT_SUCCESS &&
            chosen == -1 &&
            cna_message_box_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.simple_calls == UINT32_C(0) && log.choice_calls == UINT32_C(1),
        "replacing the service resets its log and can report closure without a chosen button");

    cna_demo_check(
        demo,
        cna_message_box_set_test_backend_ext(game, CNA_FALSE, 0) == CNA_RESULT_SUCCESS &&
            cna_message_box_get_test_log_ext(game, &log) == CNA_RESULT_INVALID_STATE,
        "removing the message-box test service restores the platform service and releases its log");
}

static void file_dialog_contract(CNA_Handle game, CnaDemo* demo)
{
    CNA_FileDialogFilter filter;
    CNA_FileDialogFilter broken;
    CNA_StringView two_results[2];
    CNA_StringView one_result[1];
    CNA_Bool supported = UINT8_C(9);
    DialogResult result;

    memset(&filter, 0, sizeof(filter));
    filter.struct_size = (uint32_t)sizeof(filter);
    filter.struct_version = UINT32_C(1);
    filter.name = text_view("Saved games");
    filter.pattern = text_view("*.sav");
    two_results[0] = text_view("/virtual/one.sav");
    two_results[1] = text_view("/virtual/two.sav");
    one_result[0] = text_view("/virtual/result.sav");
    memset(&result, 0, sizeof(result));

    cna_demo_check(
        demo,
        cna_file_dialog_get_is_supported_ext(game, &supported) == CNA_RESULT_SUCCESS &&
            (supported == CNA_FALSE || supported == CNA_TRUE),
        "real file-dialog availability is readable without opening a picker");
    printf("  platform file dialogs supported: %u\n", (unsigned)supported);

    cna_demo_check_result(
        demo,
        cna_file_dialog_set_test_backend_ext(game, CNA_TRUE, two_results, UINT64_C(2)),
        "install an immediate two-path file-dialog answer");
    cna_demo_check(
        demo,
        cna_file_dialog_show_open_file_ext(
            game, on_file_result, &result, &filter, UINT64_C(1), text_view("/virtual"),
            CNA_TRUE) == CNA_RESULT_SUCCESS &&
            result.calls == 1 && result.valid && result.count == UINT64_C(2) &&
            strcmp(result.first, "/virtual/one.sav") == 0 &&
            strcmp(result.second, "/virtual/two.sav") == 0,
        "open-file returns two borrowed paths synchronously through the test callback");

    cna_demo_check_result(
        demo,
        cna_file_dialog_set_test_backend_ext(game, CNA_TRUE, one_result, UINT64_C(1)),
        "replace the file-dialog answer with one copied path");
    cna_demo_check(
        demo,
        cna_file_dialog_show_save_file_ext(
            game, on_file_result, &result, &filter, UINT64_C(1), text_view("/virtual")) ==
                CNA_RESULT_SUCCESS &&
            result.calls == 2 && result.valid && result.count == UINT64_C(1) &&
            strcmp(result.first, "/virtual/result.sav") == 0 &&
            cna_file_dialog_show_open_folder_ext(
                game, on_file_result, &result, text_view("/virtual"), CNA_FALSE) ==
                CNA_RESULT_SUCCESS &&
            result.calls == 3 && result.valid && result.count == UINT64_C(1) &&
            strcmp(result.first, "/virtual/result.sav") == 0,
        "save-file and open-folder routes deliver the copied authored answer");

    cna_demo_check(
        demo,
        cna_file_dialog_set_test_backend_ext(game, CNA_TRUE, NULL, UINT64_C(0)) ==
                CNA_RESULT_SUCCESS &&
            cna_file_dialog_show_open_file_ext(
                game, on_file_result, &result, NULL, UINT64_C(0), text_view(""), CNA_FALSE) ==
                CNA_RESULT_SUCCESS &&
            result.calls == 4 && result.valid && result.count == UINT64_C(0) &&
            result.files_were_null,
        "cancellation is delivered as an empty callback result with a null borrowed array");

    broken = filter;
    broken.struct_version = UINT32_C(0);
    cna_demo_check(
        demo,
        cna_file_dialog_show_open_file_ext(
            game, NULL, &result, NULL, UINT64_C(0), text_view(""), CNA_FALSE) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_file_dialog_show_open_file_ext(
                game, on_file_result, &result, NULL, UINT64_C(1), text_view(""), CNA_FALSE) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_file_dialog_show_save_file_ext(
                game, on_file_result, &result, &broken, UINT64_C(1), text_view("")) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_file_dialog_show_open_folder_ext(
                game, NULL, &result, text_view(""), CNA_FALSE) == CNA_RESULT_INVALID_ARGUMENT &&
            result.calls == 4,
        "null callbacks, missing filters and an unversioned filter are refused before dispatch");

    cna_demo_check_result(
        demo,
        cna_file_dialog_set_test_backend_ext(game, CNA_FALSE, NULL, UINT64_C(0)),
        "remove the file-dialog test service and restore the platform service");
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    DialogDemoState* state = (DialogDemoState*)context;
    (void)game_time;
    (void)out_error;
    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    message_box_contract(game, state->demo);
    file_dialog_contract(game, state->demo);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    DialogDemoState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.devices.dialogs", argc, argv, 0);
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
                          "run one frame for owner-thread dialog requests");
    cna_demo_check(&demo, state.validated == 1,
                   "the update callback completed the dialog audit once");
    (void)cna_message_box_set_test_backend_ext(game, CNA_FALSE, 0);
    (void)cna_file_dialog_set_test_backend_ext(game, CNA_FALSE, NULL, UINT64_C(0));
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
