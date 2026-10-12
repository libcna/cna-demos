/* SPDX-License-Identifier: MS-PL */
/* Diagnose the public host-frame completion contract after a failed Update callback.
   Exit 0: promised exit hooks run; 1: both omitted (ISSUE-051); 2: other/inconclusive outcome. */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

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
int main(void)
{
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
    if (cna_game_create(&info, &game) != CNA_RESULT_SUCCESS) return 2;
    if (cna_game_set_frame_hooks_ext(game, &hooks) != CNA_RESULT_SUCCESS) return 2;
    CNA_Bool running = CNA_FALSE;
    CNA_Result first = cna_game_run_frame_ext(game, &running);
    printf("first: result=%u running=%u trace=%s\n", (unsigned)first, (unsigned)running, state.trace);
    if (first != CNA_RESULT_SUCCESS || running != CNA_TRUE || strcmp(state.trace,"ILBUD") != 0) return 2;
    CNA_Result failed = cna_game_run_frame_ext(game, &running);
    printf("failed: result=%u running=%u trace=%s\n", (unsigned)failed, (unsigned)running, state.trace);
    if (failed != CNA_RESULT_CALLBACK || running != CNA_FALSE || state.updates != 2) return 2;
    const unsigned length = state.length;
    running = CNA_TRUE;
    CNA_Result terminal = cna_game_run_frame_ext(game, &running);
    printf("terminal: result=%u running=%u trace=%s\n", (unsigned)terminal, (unsigned)running, state.trace);
    if (terminal != CNA_RESULT_CALLBACK || running != CNA_FALSE || state.length != length) return 2;
    CNA_Result destroyed = cna_game_destroy(game);
    printf("destroy: result=%u exits=%u ends=%u unloads=%u trace=%s\n", (unsigned)destroyed,
           state.exits, state.ends, state.unloads, state.trace);
    if (destroyed != CNA_RESULT_CALLBACK || state.unloads != 1) return 2;
    if (state.exits == 1 && state.ends == 1 && strcmp(state.trace,"ILBUDUXEZ") == 0) return 0;
    if (state.exits == 0 && state.ends == 0 && strcmp(state.trace,"ILBUDUZ") == 0) return 1;
    return 2;
}
