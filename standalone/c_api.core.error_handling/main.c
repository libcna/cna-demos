/* SPDX-License-Identifier: MS-PL */
/*
 * How the C ABI reports errors. Every fallible route returns a CNA_Result; the failing thread
 * also keeps a structured diagnostic of its last failure (cna_error_get_last_info) and a UTF-8
 * message (count, then copy). This program deliberately fails calls and queries their diagnostics.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static const char callback_message[] = "the level file is missing";

typedef struct CallbackState {
    int updates;
    int unloads;
    int cleanup_context_valid;
} CallbackState;

/* Reads the calling thread's last diagnostic: its category and its message. */
static CNA_ErrorCategory last_error(char* message, size_t capacity)
{
    CNA_ErrorInfo info;
    uint64_t length = 0U;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    message[0] = '\0';
    if (cna_error_get_last_info(&info) != CNA_RESULT_SUCCESS) {
        return CNA_ERROR_CATEGORY_NONE;
    }
    /* Count, then copy: the count never includes a terminator, so leave room for one. */
    if (cna_error_get_last_message_size(&length) == CNA_RESULT_SUCCESS && length < capacity &&
        cna_error_copy_last_message(message, capacity - 1U, &length) == CNA_RESULT_SUCCESS) {
        message[length] = '\0';
    }
    printf("    result %u, category %u: %s\n", (unsigned)info.result, (unsigned)info.category, message);
    return info.category;
}

/* An update callback that fails on its second frame, explaining why. */
static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    CallbackState* state = (CallbackState*)context;
    (void)game;
    (void)game_time;
    if (++state->updates == 2) {
        /* The message is borrowed: it only has to stay valid until the callback returns. */
        out_error->message.data = callback_message;
        out_error->message.byte_length = sizeof(callback_message) - 1U;
        return CNA_RESULT_IO;
    }
    return CNA_RESULT_SUCCESS;
}

/* Ordinary frame callbacks stop after failure; content cleanup must still run at destruction. */
static CNA_Result unload(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                          CNA_CallbackError* out_error)
{
    CallbackState* state = (CallbackState*)context;
    (void)game; (void)out_error;
    ++state->unloads;
    state->cleanup_context_valid = state->updates == 2 && game_time == NULL;
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Bool active = CNA_FALSE;
    CNA_Result result;
    char message[512];
    char name[256];
    uint64_t needed = 0U;
    uint64_t reported = 0U;
    CallbackState state = {0};

    cna_demo_init(&demo, "c_api.core.error_handling", argc, argv, 3);

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
    callbacks.unload_content = unload;
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

    /* 1. Passing the wrong kind of thing: an argument failure. */
    printf("1. a null output pointer\n");
    result = cna_game_get_is_active(game, NULL);
    cna_demo_check(&demo, result == CNA_RESULT_INVALID_ARGUMENT && last_error(message, sizeof(message)) ==
                                                                        CNA_ERROR_CATEGORY_ARGUMENT,
                   "INVALID_ARGUMENT, category ARGUMENT");

    cna_demo_check(&demo, cna_error_get_last_exception_type_size_ext(&needed) == CNA_RESULT_SUCCESS &&
                   needed == 0U && cna_error_get_last_exception_param_name_size_ext(&reported) == CNA_RESULT_SUCCESS &&
                   reported == 0U, "a C-layer argument check has no canonical exception type or parameter");

    /* Diagnostic queries have a special contract: even a bad query preserves the original
       failure so the binding can recover and retrieve it correctly. */
    {
        char original[sizeof(message)];
        strcpy(original, message);
        result = cna_error_get_last_info(NULL);
        cna_demo_check(&demo, result == CNA_RESULT_INVALID_ARGUMENT &&
                       last_error(message, sizeof(message)) == CNA_ERROR_CATEGORY_ARGUMENT &&
                       strcmp(message, original) == 0,
                       "a null diagnostic query preserves the original argument diagnostic");
        /* Allocate the full C object even when describing a deliberately short layout. */
        {
            CNA_ErrorInfo incompatible;
            memset(&incompatible, 0, sizeof(incompatible));
            incompatible.struct_size = (uint32_t)sizeof(incompatible) - 1U;
            incompatible.struct_version = UINT32_C(1);
            result = cna_error_get_last_info(&incompatible);
            cna_demo_check(&demo, result == CNA_RESULT_INVALID_ARGUMENT &&
                           last_error(message, sizeof(message)) == CNA_ERROR_CATEGORY_ARGUMENT &&
                           strcmp(message, original) == 0,
                           "an undersized diagnostic structure is refused without replacing the original error");
            incompatible.struct_size = (uint32_t)sizeof(incompatible);
            incompatible.struct_version = UINT32_C(2);
            result = cna_error_get_last_info(&incompatible);
            cna_demo_check(&demo, result == CNA_RESULT_INVALID_ARGUMENT &&
                           last_error(message, sizeof(message)) == CNA_ERROR_CATEGORY_ARGUMENT &&
                           strcmp(message, original) == 0,
                           "an unsupported diagnostic structure version preserves the original error");
        }
        /* A future caller may append fields after the version-one output prefix. The
           library fills known fields, preserves the unknown tail and keeps its diagnostic. */
        {
            struct { CNA_ErrorInfo prefix; uint64_t future[2]; } extended = {0};
            extended.prefix.struct_size = (uint32_t)sizeof(extended);
            extended.prefix.struct_version = UINT32_C(1);
            extended.future[0] = UINT64_C(0x123456789abcdef0);
            extended.future[1] = UINT64_C(0xfedcba9876543210);
            result = cna_error_get_last_info(&extended.prefix);
            cna_demo_check(&demo, result == CNA_RESULT_SUCCESS &&
                           extended.prefix.result == CNA_RESULT_INVALID_ARGUMENT &&
                           extended.prefix.category == CNA_ERROR_CATEGORY_ARGUMENT &&
                           extended.prefix.message_byte_length == strlen(original),
                           "a larger compatible diagnostic prefix receives the original error fields");
            cna_demo_check(&demo, extended.future[0] == UINT64_C(0x123456789abcdef0) &&
                           extended.future[1] == UINT64_C(0xfedcba9876543210) &&
                           last_error(message, sizeof(message)) == CNA_ERROR_CATEGORY_ARGUMENT &&
                           strcmp(message, original) == 0,
                           "a larger diagnostic query preserves unknown tail fields and the thread's original error");
        }
        memset(name, '#', sizeof(name));
        result = cna_error_copy_last_message(name, 1U, &reported);
        cna_demo_check(&demo, result == CNA_RESULT_BUFFER_TOO_SMALL && reported > 1U && name[0] == '#',
                       "a short diagnostic copy reports its required size without a partial write");
        cna_demo_check(&demo, last_error(message, sizeof(message)) == CNA_ERROR_CATEGORY_ARGUMENT &&
                       strcmp(message, original) == 0,
                       "the failed copy also preserves the original diagnostic");
    }

    /* 2. A canonical argument exception has more information than its common result/category.
       Invalid volume is rejected before audio initialization or a global volume change. */
    printf("2. a canonical argument exception\n");
    result = cna_sound_effect_set_master_volume(game, 2.0f);
    cna_demo_check(&demo, result == CNA_RESULT_INVALID_ARGUMENT &&
                   last_error(message, sizeof(message)) == CNA_ERROR_CATEGORY_ARGUMENT,
                   "the canonical range exception also maps to INVALID_ARGUMENT, category ARGUMENT");
    if (cna_demo_check(&demo, cna_error_get_last_exception_type_size_ext(&needed) == CNA_RESULT_SUCCESS &&
                      needed > 0U && needed < sizeof(name), "the canonical exception type fits the caller's buffer")) {
        result = cna_error_copy_last_exception_type_ext(name, needed, &reported);
        if (result == CNA_RESULT_SUCCESS && reported == needed) name[needed] = '\0';
        cna_demo_check(&demo, result == CNA_RESULT_SUCCESS && reported == needed &&
                       strcmp(name, "System.ArgumentOutOfRangeException") == 0,
                       "the structured type identifies ArgumentOutOfRangeException without parsing its message");
    }
    if (cna_demo_check(&demo, cna_error_get_last_exception_param_name_size_ext(&needed) == CNA_RESULT_SUCCESS &&
                      needed > 0U && needed < sizeof(name), "the canonical parameter name fits the caller's buffer")) {
        result = cna_error_copy_last_exception_param_name_ext(name, needed, &reported);
        if (result == CNA_RESULT_SUCCESS && reported == needed) name[needed] = '\0';
        cna_demo_check(&demo, result == CNA_RESULT_SUCCESS && reported == needed && strcmp(name, "value") == 0,
                       "the structured parameter name is value");
    }

    /* 3. A handle that names nothing (never issued, or already destroyed): a handle failure. */
    printf("3. a handle that was never issued\n");
    result = cna_game_get_is_active(UINT64_C(0xDEADBEEF), &active);
    cna_demo_check(&demo, result == CNA_RESULT_INVALID_HANDLE && last_error(message, sizeof(message)) ==
                                                                       CNA_ERROR_CATEGORY_HANDLE,
                   "INVALID_HANDLE, category HANDLE");
    cna_demo_check(&demo, cna_error_get_last_exception_type_size_ext(&needed) == CNA_RESULT_SUCCESS &&
                   needed == 0U && cna_error_get_last_exception_param_name_size_ext(&reported) == CNA_RESULT_SUCCESS &&
                   reported == 0U, "a later C-layer failure clears the previous canonical exception details");

    /* 4. Strings: ask for the size, then copy. Too small a buffer is refused without a partial write. */
    printf("4. a buffer one byte too small\n");
    if (cna_demo_check_result(&demo, cna_game_get_type_name_size(game, &needed), "cna_game_get_type_name_size") &&
        cna_demo_check(&demo, needed > 1U && needed < sizeof(name), "the type name fits the demo's buffer")) {
        memset(name, '#', sizeof(name));
        result = cna_game_copy_type_name(game, name, needed - 1U, &reported);
        last_error(message, sizeof(message));
        cna_demo_check(&demo, result == CNA_RESULT_BUFFER_TOO_SMALL && reported == needed,
                       "BUFFER_TOO_SMALL, and the required size is reported");
        {
            size_t i;
            int unchanged = 1;
            for (i = 0U; i < sizeof(name); ++i) unchanged = unchanged && name[i] == '#';
            cna_demo_check(&demo, unchanged, "the refused copy leaves every destination and guard byte intact");
        }
        result = cna_game_copy_type_name(game, name, needed, &reported);
        if (cna_demo_check_result(&demo, result, "an exactly sized byte buffer succeeds")) {
            cna_demo_check(&demo, reported == needed && name[needed] == '#',
                           "the copy reports exactly the payload bytes and writes no terminator past capacity");
            name[needed] = '\0'; /* caller supplies the terminator outside the exact payload capacity */
            printf("    the game's type name: %s\n", name);
        }
    }

    /* 5. A callback's failure stops the frame; the caller sees CALLBACK and the callback's message. */
    printf("5. an update callback that fails\n");
    cna_demo_check_result(&demo, cna_game_run_one_frame(game), "the first frame runs");
    result = cna_game_run_one_frame(game);
    cna_demo_check(&demo, result == CNA_RESULT_CALLBACK &&
                              last_error(message, sizeof(message)) == CNA_ERROR_CATEGORY_CALLBACK,
                   "the second frame returns CALLBACK, category CALLBACK");
    cna_demo_check(&demo, strstr(message, callback_message) != NULL, "the diagnostic carries the callback's message");

    result = cna_game_run_one_frame(game);
    cna_demo_check(&demo, result == CNA_RESULT_CALLBACK && state.updates == 2 && state.unloads == 0 &&
                   last_error(message, sizeof(message)) == CNA_ERROR_CATEGORY_CALLBACK &&
                   strcmp(message, callback_message) == 0,
                   "repeating a failed frame preserves its diagnostic without another update or early unload");

    /* A game whose callback failed still releases everything when destroyed, and that last call
       reports the failure once more, so a caller that only checks the final result still sees it. */
    result = cna_game_destroy(game);
    cna_demo_check(&demo, result == CNA_RESULT_CALLBACK, "cna_game_destroy releases it and reports CALLBACK again");
    cna_demo_check(&demo, state.unloads == 1 && state.cleanup_context_valid,
                   "UnloadContent runs once after the failed Update with its caller-owned context and null time");
    cna_demo_check(&demo, last_error(message, sizeof(message)) == CNA_ERROR_CATEGORY_CALLBACK &&
                   strcmp(message, callback_message) == 0,
                   "successful unload cleanup preserves the original callback diagnostic");
    printf("Callback counts: %d updates, %d unloads after failure\n", state.updates, state.unloads);
    cna_demo_check(&demo, cna_game_get_is_active(game, &active) == CNA_RESULT_INVALID_HANDLE,
                   "afterwards the handle names nothing: INVALID_HANDLE");
    return cna_demo_finish(&demo);
}
