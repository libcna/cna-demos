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

typedef enum FailurePoint {
    FAILURE_INITIALIZE,
    FAILURE_LOAD,
    FAILURE_BEGIN_RUN,
    FAILURE_BEGIN_DRAW,
    FAILURE_DRAW,
    FAILURE_END_DRAW,
    FAILURE_EXITING,
    FAILURE_END_RUN,
    FAILURE_UNLOAD,
    FAILURE_POINT_COUNT
} FailurePoint;

typedef struct FailureState {
    char trace[64];
    unsigned length;
    unsigned failures;
    unsigned unloads;
    FailurePoint point;
    CNA_Result exit_request;
} FailureState;

static const char* const failure_names[FAILURE_POINT_COUNT] = {
    "initialize", "load_content", "begin_run", "begin_draw", "draw", "end_draw", "exiting",
    "end_run", "unload_content"
};

static CNA_Result failure_step(
    FailureState* state,
    FailurePoint point,
    char marker,
    CNA_CallbackError* error)
{
    if (state->length + 1 >= sizeof(state->trace)) return CNA_RESULT_INVALID_STATE;
    state->trace[state->length++] = marker;
    state->trace[state->length] = '\0';
    if (state->point != point) return CNA_RESULT_SUCCESS;
    ++state->failures;
    error->message.data = failure_names[point];
    error->message.byte_length = strlen(failure_names[point]);
    return CNA_RESULT_IO;
}

#define FAILURE_HOOK(name, point, marker) \
static CNA_Result name(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error) \
{ (void)game; (void)time; return failure_step(context, point, marker, error); }
FAILURE_HOOK(failure_initialize, FAILURE_INITIALIZE, 'I')
FAILURE_HOOK(failure_load, FAILURE_LOAD, 'L')
FAILURE_HOOK(failure_begin_run, FAILURE_BEGIN_RUN, 'B')
FAILURE_HOOK(failure_draw, FAILURE_DRAW, 'D')
FAILURE_HOOK(failure_end_draw, FAILURE_END_DRAW, 'A')
FAILURE_HOOK(failure_exiting, FAILURE_EXITING, 'X')
FAILURE_HOOK(failure_end_run, FAILURE_END_RUN, 'E')

static CNA_Result failure_update(
    CNA_Handle game,
    const CNA_GameTime* time,
    void* context,
    CNA_CallbackError* error)
{
    FailureState* state = context;
    CNA_Result result;
    (void)time;
    result = failure_step(state, FAILURE_POINT_COUNT, 'U', error);
    if (result == CNA_RESULT_SUCCESS && state->point >= FAILURE_EXITING) {
        state->exit_request = cna_game_request_exit(game);
        return state->exit_request;
    }
    return result;
}

static CNA_Result failure_begin_draw(
    CNA_Handle game,
    const CNA_GameTime* time,
    void* context,
    CNA_Bool* should_draw,
    CNA_CallbackError* error)
{
    (void)game;
    (void)time;
    (void)should_draw;
    return failure_step(context, FAILURE_BEGIN_DRAW, 'P', error);
}

static CNA_Result failure_unload(
    CNA_Handle game,
    const CNA_GameTime* time,
    void* context,
    CNA_CallbackError* error)
{
    FailureState* state = context;
    (void)game;
    (void)time;
    ++state->unloads;
    return failure_step(state, FAILURE_UNLOAD, 'Z', error);
}

static int has_last_diagnostic(const char* expected)
{
    CNA_ErrorInfo info = {0};
    char message[64] = {0};
    uint64_t needed = 0, copied = 0;
    info.struct_size = sizeof(info);
    info.struct_version = 1;
    return cna_error_get_last_info(&info) == CNA_RESULT_SUCCESS &&
           info.result == CNA_RESULT_CALLBACK && info.category == CNA_ERROR_CATEGORY_CALLBACK &&
           cna_error_get_last_message_size(&needed) == CNA_RESULT_SUCCESS && needed < sizeof(message) &&
           cna_error_copy_last_message(message, needed, &copied) == CNA_RESULT_SUCCESS && copied == needed &&
           strcmp(message, expected) == 0;
}

static void run_failure_scenario(CnaDemo* demo, FailurePoint point)
{
    FailureState state = {0};
    CNA_GameCallbacks callbacks = {0};
    CNA_GameFrameHooks hooks = {0};
    CNA_GameCreateInfo info = {0};
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Bool running = CNA_TRUE;
    CNA_Result run_result, terminal_result, destroy_result;
    unsigned terminal_length;
    char label[160];

    state.point = point;
    state.exit_request = CNA_RESULT_INTERNAL;
    callbacks.struct_size = sizeof(callbacks);
    callbacks.struct_version = 1;
    callbacks.load_content = failure_load;
    callbacks.update = failure_update;
    callbacks.draw = failure_draw;
    callbacks.unload_content = failure_unload;
    callbacks.exiting = failure_exiting;
    callbacks.context = &state;
    hooks.struct_size = sizeof(hooks);
    hooks.struct_version = 1;
    hooks.initialize = failure_initialize;
    hooks.begin_run = failure_begin_run;
    hooks.end_run = failure_end_run;
    hooks.begin_draw = failure_begin_draw;
    hooks.end_draw = failure_end_draw;
    hooks.context = &state;
    info.struct_size = sizeof(info);
    info.struct_version = 1;
    info.is_fixed_time_step = CNA_FALSE;
    info.target_elapsed_time_ticks = 166667;
    info.callbacks = &callbacks;

    snprintf(label, sizeof(label), "create the %s failure scenario", failure_names[point]);
    if (!cna_demo_check_result(demo, cna_game_create(&info, &game), label)) return;
    snprintf(label, sizeof(label), "install hooks for the %s failure scenario", failure_names[point]);
    if (!cna_demo_check_result(demo, cna_game_set_frame_hooks_ext(game, &hooks), label)) {
        (void)cna_game_destroy(game);
        return;
    }

    run_result = cna_game_run_frame_ext(game, &running);
    snprintf(label, sizeof(label), "%s failure reaches its documented run result", failure_names[point]);
    cna_demo_check(demo,
        point == FAILURE_UNLOAD
            ? run_result == CNA_RESULT_SUCCESS && running == CNA_FALSE && state.failures == 0 &&
                  state.exit_request == CNA_RESULT_SUCCESS
            : run_result == CNA_RESULT_CALLBACK && running == CNA_FALSE && state.failures == 1 &&
                  (point < FAILURE_EXITING || state.exit_request == CNA_RESULT_SUCCESS),
        label);
    if (point != FAILURE_UNLOAD) {
        snprintf(label, sizeof(label), "%s failure retains its callback diagnostic", failure_names[point]);
        cna_demo_check(demo, has_last_diagnostic(failure_names[point]), label);
    }

    terminal_length = state.length;
    running = CNA_TRUE;
    terminal_result = cna_game_run_frame_ext(game, &running);
    snprintf(label, sizeof(label), "%s completion is terminal without callback replay", failure_names[point]);
    cna_demo_check(demo,
        terminal_result == (point == FAILURE_UNLOAD ? CNA_RESULT_SUCCESS : CNA_RESULT_CALLBACK) &&
            running == CNA_FALSE && state.length == terminal_length,
        label);

    snprintf(label, sizeof(label), "%s failure still delivers promised exiting then end_run hooks%s",
             failure_names[point], point <= FAILURE_EXITING ? " (CNA-DEMOS-ISSUE-051)" : "");
    cna_demo_check(demo, strstr(state.trace, "XE") != NULL, label);

    destroy_result = cna_game_destroy(game);
    snprintf(label, sizeof(label), "%s destruction reports CALLBACK and unloads exactly once",
             failure_names[point]);
    cna_demo_check(demo,
        destroy_result == CNA_RESULT_CALLBACK && state.unloads == 1 && state.failures == 1,
        label);
    if (point == FAILURE_UNLOAD) {
        cna_demo_check(demo, has_last_diagnostic(failure_names[point]),
                       "unload_content failure retains its callback diagnostic after handle release");
    }
    printf("  %s failure trace: %s\n", failure_names[point], state.trace);
}

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
    {
        FailurePoint point;
        for (point = FAILURE_INITIALIZE; point < FAILURE_POINT_COUNT; point = (FailurePoint)(point + 1)) {
            run_failure_scenario(&demo, point);
        }
    }
    return cna_demo_finish(&demo);
}
