/* SPDX-License-Identifier: MS-PL */
/* Host-owned frame failure: terminal state and cleanup versus promised C completion hooks. */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>
#include "CnaDemos/CApiDemo.h"

typedef struct State { char trace[32]; unsigned length, updates, exits, ends, unloads; } State;
static CNA_Result record(State* state, char marker)
{
    if (state->length + 1 >= sizeof(state->trace)) return CNA_RESULT_INVALID_STATE;
    state->trace[state->length++] = marker;
    state->trace[state->length] = '\0';
    return CNA_RESULT_SUCCESS;
}
#define HOOK(name, marker) \
static CNA_Result name(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error) \
{ (void)game; (void)time; (void)error; return record((State*)context, marker); }
HOOK(initialize, 'I')
HOOK(load, 'L')
HOOK(begin, 'B')
HOOK(draw, 'D')
static CNA_Result update(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    static const char message[] = "host frame update failed";
    State* state = context;
    (void)game; (void)time;
    if (record(state, 'U') != CNA_RESULT_SUCCESS) return CNA_RESULT_INVALID_STATE;
    if (++state->updates == 2) {
        error->message.data = message;
        error->message.byte_length = sizeof(message) - 1;
        return CNA_RESULT_IO;
    }
    return CNA_RESULT_SUCCESS;
}
static CNA_Result exiting(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{ State* state = context; (void)game; (void)time; (void)error; ++state->exits; return record(state, 'X'); }
static CNA_Result end(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{ State* state = context; (void)game; (void)time; (void)error; ++state->ends; return record(state, 'E'); }
static CNA_Result unload(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{ State* state = context; (void)game; (void)time; (void)error; ++state->unloads; return record(state, 'Z'); }
int main(int argc, char** argv)
{
    CnaDemo demo;
    cna_demo_init(&demo, "c_api.game.host_failure", argc, argv, 3);
    State state = {0};
    CNA_GameCallbacks callbacks = {0};
    callbacks.struct_size = sizeof(callbacks); callbacks.struct_version = 1;
    callbacks.load_content = load; callbacks.update = update; callbacks.draw = draw;
    callbacks.exiting = exiting; callbacks.unload_content = unload; callbacks.context = &state;
    CNA_GameFrameHooks hooks = {0};
    hooks.struct_size = sizeof(hooks); hooks.struct_version = 1;
    hooks.initialize = initialize; hooks.begin_run = begin; hooks.end_run = end; hooks.context = &state;
    CNA_GameCreateInfo info = {0};
    info.struct_size = sizeof(info); info.struct_version = 1;
    info.is_fixed_time_step = CNA_FALSE; info.target_elapsed_time_ticks = 166667; info.callbacks = &callbacks;
    CNA_Handle game = CNA_INVALID_HANDLE;
    if (!cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the host failure game"))
        return cna_demo_finish(&demo);
    if (!cna_demo_check_result(&demo, cna_game_set_frame_hooks_ext(game, &hooks), "install host completion hooks")) {
        (void)cna_game_destroy(game);
        return cna_demo_finish(&demo);
    }
    CNA_Bool running = CNA_FALSE;
    CNA_Result result = cna_game_run_frame_ext(game, &running);
    cna_demo_check(&demo, result == CNA_RESULT_SUCCESS && running == CNA_TRUE && strcmp(state.trace, "ILBUD") == 0,
                   "the first host frame initializes, begins, updates and draws normally");
    result = cna_game_run_frame_ext(game, &running);
    cna_demo_check(&demo, result == CNA_RESULT_CALLBACK && running == CNA_FALSE && state.updates == 2 && state.unloads == 0,
                   "a failed Update returns CALLBACK and ends the host run without unloading yet");
    CNA_ErrorInfo error = {0};
    error.struct_size = sizeof(error); error.struct_version = 1;
    char message[128] = {0};
    uint64_t needed = 0, copied = 0;
    cna_demo_check(&demo, cna_error_get_last_info(&error) == CNA_RESULT_SUCCESS &&
                   error.result == CNA_RESULT_CALLBACK && error.category == CNA_ERROR_CATEGORY_CALLBACK &&
                   cna_error_get_last_message_size(&needed) == CNA_RESULT_SUCCESS && needed < sizeof(message) &&
                   cna_error_copy_last_message(message, needed, &copied) == CNA_RESULT_SUCCESS && copied == needed &&
                   strcmp(message, "host frame update failed") == 0,
                   "host failure retains the callback category and its supplied diagnostic");
    cna_demo_check(&demo, state.exits == 1 && state.ends == 1 && strcmp(state.trace, "ILBUDUXE") == 0,
                   "failed host completion invokes the promised C exiting then end_run hooks (CNA-DEMOS-ISSUE-051)");
    const unsigned length = state.length, exits = state.exits, ends = state.ends;
    running = CNA_TRUE;
    result = cna_game_run_frame_ext(game, &running);
    cna_demo_check(&demo, result == CNA_RESULT_CALLBACK && running == CNA_FALSE &&
                   state.length == length && state.updates == 2 && state.unloads == 0,
                   "a later host call remains terminal without replaying any callback");
    result = cna_game_destroy(game);
    cna_demo_check(&demo, result == CNA_RESULT_CALLBACK, "destruction releases the failed game and reports CALLBACK");
    cna_demo_check(&demo, state.unloads == 1 && state.length == length + 1 && state.trace[length] == 'Z' &&
                   state.exits == exits && state.ends == ends,
                   "destruction unloads exactly once without repeating completion hooks");
    int64_t ticks = 0;
    cna_demo_check(&demo, cna_game_get_target_elapsed_time_ticks(game, &ticks) == CNA_RESULT_INVALID_HANDLE,
                   "the destroyed host handle is invalid even though destruction returned CALLBACK");
    printf("Host failure trace: %s (I initialize, L load, B begin, U update, D draw, X exiting, E end, Z unload)\n",
           state.trace);
    return cna_demo_finish(&demo);
}
