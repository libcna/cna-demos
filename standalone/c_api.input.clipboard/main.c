/* SPDX-License-Identifier: MS-PL */
/*
 * Clipboard text from C: preserve the desktop clipboard, round-trip explicit UTF-8 through CNA's
 * count/copy boundary, exercise empty text, then restore and verify the original bytes.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ClipboardState {
    CnaDemo* demo;
    int validated;
} ClipboardState;

static CNA_StringView text_view(const char* data, uint64_t bytes)
{
    CNA_StringView view;
    view.data = bytes == UINT64_C(0) ? NULL : data;
    view.byte_length = bytes;
    return view;
}

/* Count and copy are separate because the desktop clipboard can change between calls. Retry a
 * larger answer a few times, then give the caller one exact owned byte sequence. */
static CNA_Result read_clipboard(CNA_Handle game, char** out_text, uint64_t* out_bytes)
{
    int attempt;
    for (attempt = 0; attempt < 4; ++attempt) {
        uint64_t expected = UINT64_C(0);
        uint64_t copied = UINT64_C(0);
        char* text;
        CNA_Result result = cna_clipboard_get_text_size(game, &expected);
        if (result != CNA_RESULT_SUCCESS) {
            return result;
        }
        text = (char*)malloc((size_t)expected + 1U);
        if (text == NULL) {
            return CNA_RESULT_OUT_OF_MEMORY;
        }
        result = cna_clipboard_copy_text(game, expected == UINT64_C(0) ? NULL : text,
                                         expected, &copied);
        if (result == CNA_RESULT_BUFFER_TOO_SMALL) {
            free(text);
            continue;
        }
        if (result != CNA_RESULT_SUCCESS || copied > expected) {
            free(text);
            return result == CNA_RESULT_SUCCESS ? CNA_RESULT_INVALID_STATE : result;
        }
        text[copied] = '\0';
        *out_text = text;
        *out_bytes = copied;
        return CNA_RESULT_SUCCESS;
    }
    return CNA_RESULT_INVALID_STATE;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    static const char probe[] = "cna-demos C clipboard: \xC5\xBElu\xC5\xA5ou\xC4\x8Dk\xC3\xBD";
    ClipboardState* state = (ClipboardState*)context;
    CnaDemo* demo = state->demo;
    char* original = NULL;
    char* restored = NULL;
    uint64_t original_bytes = UINT64_C(0);
    uint64_t restored_bytes = UINT64_C(0);
    uint64_t bytes = UINT64_C(99);
    CNA_Bool accepted = UINT8_C(9);
    CNA_Bool has_text = UINT8_C(9);
    CNA_Result capture;
    CNA_Result restore = CNA_RESULT_INVALID_STATE;
    (void)game_time;
    (void)out_error;

    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    capture = read_clipboard(game, &original, &original_bytes);
    if (!cna_demo_check_result(demo, capture, "capture the original clipboard through size and copy")) {
        return CNA_RESULT_SUCCESS;
    }

    cna_demo_check(
        demo,
        cna_devices_clipboard_set_text_ext(
            game, text_view(probe, (uint64_t)strlen(probe)), &accepted) == CNA_RESULT_SUCCESS &&
            accepted == CNA_TRUE,
        "the device extension reports that the desktop accepted the explicit UTF-8 value");
    cna_demo_check(
        demo,
        cna_clipboard_get_has_text(game, &has_text) == CNA_RESULT_SUCCESS && has_text == CNA_TRUE,
        "the accepted non-empty value makes has-text true");
    cna_demo_check(
        demo,
        cna_clipboard_get_text_size(game, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == (uint64_t)strlen(probe),
        "the clipboard size counts the exact UTF-8 bytes without a terminator");
    {
        char copied[128];
        memset(copied, 0, sizeof(copied));
        cna_demo_check(
            demo,
            cna_clipboard_copy_text(game, copied, sizeof(copied) - 1U, &bytes) == CNA_RESULT_SUCCESS &&
                bytes == (uint64_t)strlen(probe) && memcmp(copied, probe, (size_t)bytes) == 0 &&
                copied[bytes] == '\0',
            "an exact copy returns every requested UTF-8 byte");
    }
    {
        unsigned char guard[128];
        uint64_t needed = UINT64_C(0);
        size_t index;
        int unchanged = 1;
        memset(guard, 0x7f, sizeof(guard));
        cna_demo_check(
            demo,
            cna_clipboard_copy_text(game, (char*)guard, (uint64_t)strlen(probe) - UINT64_C(1),
                                    &needed) == CNA_RESULT_BUFFER_TOO_SMALL &&
                needed == (uint64_t)strlen(probe),
            "a one-byte-short clipboard copy reports the complete requirement");
        for (index = 0U; index < sizeof(guard); ++index) {
            unchanged = unchanged && guard[index] == 0x7fU;
        }
        cna_demo_check(demo, unchanged, "the refused short copy leaves every destination byte unchanged");
    }

    cna_demo_check_result(demo, cna_clipboard_set_text(game, text_view(NULL, UINT64_C(0))),
                          "request an empty clipboard value");
    cna_demo_check(
        demo,
        cna_clipboard_get_has_text(game, &has_text) == CNA_RESULT_SUCCESS && has_text == CNA_FALSE &&
            cna_clipboard_get_text_size(game, &bytes) == CNA_RESULT_SUCCESS && bytes == UINT64_C(0) &&
            cna_clipboard_copy_text(game, NULL, UINT64_C(0), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(0),
        "empty text clears has-text and uses a successful zero-byte size/copy answer");

    restore = cna_clipboard_set_text(game, text_view(original, original_bytes));
    cna_demo_check_result(demo, restore, "restore the original clipboard bytes");
    if (restore == CNA_RESULT_SUCCESS) {
        CNA_Result reread = read_clipboard(game, &restored, &restored_bytes);
        cna_demo_check(
            demo,
            reread == CNA_RESULT_SUCCESS && restored_bytes == original_bytes &&
                (original_bytes == UINT64_C(0) ||
                 memcmp(restored, original, (size_t)original_bytes) == 0),
            "the restored clipboard exactly matches the captured byte sequence");
    }
    free(restored);
    free(original);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    ClipboardState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.input.clipboard", argc, argv, 0);
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
    cna_demo_check_result(&demo, cna_game_run_one_frame(game), "run one frame for clipboard access");
    cna_demo_check(&demo, state.validated == 1, "the update callback completed the clipboard audit once");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
