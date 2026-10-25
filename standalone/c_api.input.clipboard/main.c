/* SPDX-License-Identifier: MS-PL */
/* Read-only, race-tolerant clipboard text inspection through the C ABI. */
#include <CNA/C/cna.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ClipboardState {
    CnaDemo* demo;
    int validated;
} ClipboardState;

/* Count and copy are separate because the desktop clipboard can change between calls. */
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
    ClipboardState* state = (ClipboardState*)context;
    char* snapshot = NULL;
    uint64_t bytes = UINT64_C(0);
    CNA_Bool has_text = UINT8_C(9);
    (void)game_time;
    (void)out_error;

    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    if (!cna_demo_check_result(state->demo, read_clipboard(game, &snapshot, &bytes),
                               "capture a read-only clipboard snapshot through size and copy")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_clipboard_get_has_text(game, &has_text) == CNA_RESULT_SUCCESS &&
            (has_text == CNA_FALSE || has_text == CNA_TRUE) && snapshot[bytes] == '\0',
        "clipboard text is an exact owned, terminated snapshot without a host write");
    printf("  clipboard text snapshot: %llu byte(s), has-text %u\n",
           (unsigned long long)bytes, (unsigned)has_text);
    free(snapshot);
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
    cna_demo_check(&demo, state.validated == 1, "the update callback completed read-only inspection once");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
