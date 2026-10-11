/* SPDX-License-Identifier: MS-PL */
/* A host owns the loop: begin once, suppress one draw, draw once, then end once. */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>
#include "CnaDemos/CApiDemo.h"

typedef struct State {
    CnaDemo* demo;
    char trace[64];
    size_t count;
    int begin_draws;
    int bad_begin_arguments;
} State;

static CNA_Result record(State* state, char event)
{
    if (state->count + 1 >= sizeof(state->trace)) return CNA_RESULT_INVALID_STATE;
    state->trace[state->count++] = event;
    state->trace[state->count] = '\0';
    return CNA_RESULT_SUCCESS;
}

static CNA_Result initialize(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    (void)game; (void)time; (void)error;
    return record((State*)context, 'I');
}
static CNA_Result load(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    (void)game; (void)time; (void)error;
    return record((State*)context, 'L');
}
static CNA_Result begin_run(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    (void)game; (void)time; (void)error;
    return record((State*)context, 'B');
}
static CNA_Result update(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    (void)game; (void)time; (void)error;
    return record((State*)context, 'U');
}
static CNA_Result begin_draw(CNA_Handle game, const CNA_GameTime* time, void* context,
                             CNA_Bool* should_draw, CNA_CallbackError* error)
{
    State* state = (State*)context;
    (void)game; (void)error;
    if (time != NULL || *should_draw != CNA_TRUE) ++state->bad_begin_arguments;
    /* The first host frame updates, but deliberately omits both Draw and EndDraw. */
    *should_draw = ++state->begin_draws == 1 ? CNA_FALSE : CNA_TRUE;
    return record(state, 'b');
}
static CNA_Result draw(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    State* state = (State*)context;
    const CNA_Color color = {32, 128, 192, 255};
    CNA_Result result;
    (void)time; (void)error;
    if ((result = record(state, 'D')) != CNA_RESULT_SUCCESS) return result;
    if ((result = cna_game_clear(game, color)) != CNA_RESULT_SUCCESS) return result;
    if (state->demo->screenshot != NULL) {
        CNA_Handle device = CNA_INVALID_HANDLE;
        if ((result = cna_game_get_graphics_device(game, &device)) != CNA_RESULT_SUCCESS) return result;
        cna_demo_capture(state->demo, device);
    }
    return CNA_RESULT_SUCCESS;
}
static CNA_Result end_draw(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    (void)game; (void)time; (void)error;
    return record((State*)context, 'e');
}
static CNA_Result exiting(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    (void)game; (void)time; (void)error;
    return record((State*)context, 'X');
}
static CNA_Result end_run(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    (void)game; (void)time; (void)error;
    return record((State*)context, 'E');
}
static CNA_Result unload(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    (void)game; (void)time; (void)error;
    return record((State*)context, 'Z');
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    State state = {0};
    CNA_GameCallbacks callbacks = {0};
    CNA_GameFrameHooks hooks = {0};
    CNA_GameCreateInfo info = {0};
    CNA_Handle game = CNA_INVALID_HANDLE, manager = CNA_INVALID_HANDLE;
    CNA_Bool running = CNA_FALSE;
    cna_demo_init(&demo, "c_api.game.host_frames", argc, argv, 2);
    state.demo = &demo;
    callbacks.struct_size = sizeof(callbacks);
    callbacks.struct_version = 1;
    callbacks.load_content = load;
    callbacks.update = update;
    callbacks.draw = draw;
    callbacks.exiting = exiting;
    callbacks.unload_content = unload;
    callbacks.context = &state;
    hooks.struct_size = sizeof(hooks);
    hooks.struct_version = 1;
    hooks.initialize = initialize;
    hooks.begin_run = begin_run;
    hooks.end_run = end_run;
    hooks.begin_draw = begin_draw;
    hooks.end_draw = end_draw;
    hooks.context = &state;
    info.struct_size = sizeof(info);
    info.struct_version = 1;
    info.is_fixed_time_step = CNA_FALSE; /* one update per host call, no fixed-step catch-up */
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.callbacks = &callbacks;
    if (!cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the host-driven game"))
        return cna_demo_finish(&demo);
    if (!cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager), "select HiDef for optional capture"))
        goto cleanup;
    if (!cna_demo_check_result(&demo, cna_game_set_frame_hooks_ext(game, &hooks), "install the second callback table"))
        goto cleanup;
    /* Both tables are copied. Their context still points to live caller-owned state. */
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&hooks, 0, sizeof(hooks));
    cna_demo_check(&demo, state.count == 0, "creation and hook installation do not start a run");

    if (!cna_demo_check_result(&demo, cna_game_run_frame_ext(game, &running), "host frame 1")) goto cleanup;
    cna_demo_check(&demo, running == CNA_TRUE && strcmp(state.trace, "ILBUb") == 0,
                   "first frame initializes, loads, begins, updates and suppresses drawing");
    if (!cna_demo_check_result(&demo, cna_game_run_frame_ext(game, &running), "host frame 2")) goto cleanup;
    cna_demo_check(&demo, running == CNA_TRUE && strcmp(state.trace, "ILBUbUbDe") == 0,
                   "second frame updates and draws without restarting the run");
    cna_demo_check(&demo, state.begin_draws == 2 && state.bad_begin_arguments == 0,
                   "BeginDraw receives null time and an initially true draw decision");

    if (!cna_demo_check_result(&demo, cna_game_request_exit(game), "host requests exit between frames")) goto cleanup;
    if (!cna_demo_check_result(&demo, cna_game_run_frame_ext(game, &running), "host observes run completion")) goto cleanup;
    cna_demo_check(&demo, running == CNA_FALSE && strcmp(state.trace, "ILBUbUbDeXE") == 0,
                   "completion raises Exiting then EndRun without an extra update or draw");
    running = CNA_TRUE; /* prove the terminal call writes false rather than preserving the input */
    cna_demo_check_result(&demo, cna_game_run_frame_ext(game, &running), "call again after completion");
    cna_demo_check(&demo, running == CNA_FALSE && strcmp(state.trace, "ILBUbUbDeXE") == 0,
                   "an ended run remains ended and invokes no callbacks again");
cleanup:
    if (manager != CNA_INVALID_HANDLE)
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager), "release the manager");
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the game");
    cna_demo_check(&demo, strcmp(state.trace, "ILBUbUbDeXEZ") == 0, "UnloadContent runs on destruction after EndRun");
    printf("Callback trace: %s (I initialize, L load, B begin, U update, b begin-draw, D draw, e end-draw, X exit, E end, Z unload)\n", state.trace);
    return cna_demo_finish(&demo);
}
