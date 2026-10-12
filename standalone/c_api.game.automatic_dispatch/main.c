/* SPDX-License-Identifier: MS-PL */
/* Observe automatic Update service, then isolate Draw service with a BeginDraw worker. */
#include <CNA/C/cna.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include "CnaDemos/CApiDemo.h"

#define INITIAL_TICKS INT64_C(166667)
#define CHANGED_TICKS INT64_C(333333)
#define DRAW_TICKS INT64_C(500000)

typedef struct State {
    CNA_Handle game;
    thrd_t owner;
    atomic_int done;
    CNA_Result result, borrow_result;
    CNA_Handle expired_device; /* Numeric identifier retained only for an explicit expiry check. */
    int64_t before, after;
    int updates, draws, suppressed, saw_change, wrong_thread;
    thrd_t draw_worker;
    atomic_int draw_entered, draw_done;
    CNA_Result draw_result;
    int draw_phase, draw_started, draw_pending, draw_begin_updates;
    int64_t draw_before, draw_observed;
} State;

static int worker(void* context)
{
    State* state = (State*)context;
    /* No explicit queue API: ordinary handle calls block until a frame serves them. */
    state->result = cna_game_get_target_elapsed_time_ticks(state->game, &state->before);
    if (state->result == CNA_RESULT_SUCCESS)
        state->result = cna_game_set_target_elapsed_time_ticks(state->game, CHANGED_TICKS);
    if (state->result == CNA_RESULT_SUCCESS)
        state->result = cna_game_get_target_elapsed_time_ticks(state->game, &state->after);
    /* This separate request requires an active callback scope. Do not use or release the
       returned borrow on the worker: a later queued call may belong to a different frame. */
    if (state->result == CNA_RESULT_SUCCESS)
        state->borrow_result = cna_game_get_graphics_device(state->game, &state->expired_device);
    atomic_store_explicit(&state->done, 1, memory_order_release);
    return 0;
}

static int draw_worker(void* context)
{
    State* state = (State*)context;
    atomic_store_explicit(&state->draw_entered, 1, memory_order_release);
    state->draw_result = cna_game_set_target_elapsed_time_ticks(state->game, DRAW_TICKS);
    atomic_store_explicit(&state->draw_done, 1, memory_order_release);
    return 0;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    State* state = (State*)context;
    int64_t ticks = 0;
    (void)error;
    ++state->updates;
    if (!thrd_equal(state->owner, thrd_current())) ++state->wrong_thread;
    CNA_Result result = cna_game_get_target_elapsed_time_ticks(game, &ticks);
    if (result == CNA_RESULT_SUCCESS && time != NULL && ticks == CHANGED_TICKS) state->saw_change = 1;
    return result;
}

static CNA_Result begin_draw(CNA_Handle game, const CNA_GameTime* time, void* context,
                             CNA_Bool* should_draw, CNA_CallbackError* error)
{
    State* state = (State*)context;
    (void)game; (void)time; (void)error;
    if (!state->draw_phase) {
        ++state->suppressed;
        *should_draw = CNA_FALSE; /* Preserve the original Update-only control. */
        return CNA_RESULT_SUCCESS;
    }
    *should_draw = CNA_TRUE;
    if (state->draw_started) return CNA_RESULT_SUCCESS; /* Never launch a second worker. */
    state->draw_begin_updates = state->updates;
    /* Update has already returned. This one setter can only be served by the coming Draw.
       An entered flag alone is not proof of queuing: the first Draw must observe its write. */
    state->draw_started = thrd_create(&state->draw_worker, draw_worker, state) == thrd_success;
    if (state->draw_started) {
        const struct timespec pause = {0, 2000000L};
        for (int attempt = 0; attempt < 500 && !atomic_load_explicit(&state->draw_entered, memory_order_acquire); ++attempt)
            (void)thrd_sleep(&pause, NULL);
        for (int attempt = 0; attempt < 100 && !atomic_load_explicit(&state->draw_done, memory_order_acquire); ++attempt)
            (void)thrd_sleep(&pause, NULL);
        state->draw_pending = atomic_load_explicit(&state->draw_entered, memory_order_acquire) &&
            !atomic_load_explicit(&state->draw_done, memory_order_acquire) &&
            cna_game_get_target_elapsed_time_ticks(game, &state->draw_before) == CNA_RESULT_SUCCESS &&
            state->draw_before == CHANGED_TICKS;
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(CNA_Handle game, const CNA_GameTime* time, void* context, CNA_CallbackError* error)
{
    State* state = (State*)context;
    (void)error;
    ++state->draws;
    if (!thrd_equal(state->owner, thrd_current()) || time == NULL) ++state->wrong_thread;
    if (state->draw_phase)
        return cna_game_get_target_elapsed_time_ticks(game, &state->draw_observed);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    State state = {0};
    CNA_GameCallbacks callbacks = {0};
    CNA_GameFrameHooks hooks = {0};
    CNA_GameCreateInfo info = {0};
    CNA_Bool running = CNA_TRUE;
    thrd_t thread;
    const struct timespec pause = {0, 2000000L};
    int frame_ok = 1, joined_result = 1;
    cna_demo_init(&demo, "c_api.game.automatic_dispatch", argc, argv, 0);
    state.owner = thrd_current();
    atomic_init(&state.done, 0);
    atomic_init(&state.draw_entered, 0);
    atomic_init(&state.draw_done, 0);
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.context = &state;
    callbacks.update = update;
    callbacks.draw = draw;
    hooks.struct_size = (uint32_t)sizeof(hooks);
    hooks.struct_version = UINT32_C(1);
    hooks.context = &state;
    hooks.begin_draw = begin_draw;
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.callbacks = &callbacks;
    info.is_fixed_time_step = CNA_FALSE;
    info.target_elapsed_time_ticks = INITIAL_TICKS;
    if (!cna_demo_check_result(&demo, cna_game_create(&info, &state.game), "create an owner-thread game with Update observation"))
        return cna_demo_finish(&demo);
    if (!cna_demo_check_result(&demo, cna_game_set_frame_hooks_ext(state.game, &hooks), "suppress Draw to isolate automatic Update dispatch")) goto cleanup;
    /* Start before requesting a borrow: pre-run borrowing is a separate permitted API route. */
    if (!cna_demo_check(&demo, cna_game_run_frame_ext(state.game, &running) == CNA_RESULT_SUCCESS &&
                        running && state.updates > 0 && !state.saw_change,
                        "start the run with the original target step before launching the worker")) goto cleanup;
    if (!cna_demo_check_result(&demo, cna_game_set_foreign_thread_calls_ext(state.game, CNA_TRUE), "enable foreign-thread calls during normal frames")) goto cleanup;
    if (!cna_demo_check(&demo, thrd_create(&thread, worker, &state) == thrd_success, "start the ordinary property worker")) goto cleanup;

    for (int attempt = 0; attempt < 2500 && !atomic_load_explicit(&state.done, memory_order_acquire); ++attempt) {
        /* Never call cna_game_run_foreign_thread_calls_ext: the game does its normal Update. */
        if (cna_game_run_frame_ext(state.game, &running) != CNA_RESULT_SUCCESS || !running) {
            frame_ok = 0;
            break;
        }
        (void)thrd_sleep(&pause, NULL);
    }
    const int finished = atomic_load_explicit(&state.done, memory_order_acquire);
    cna_demo_check(&demo, frame_ok && finished, "normal host frames complete the worker without an explicit pump");
    /* A final owner update observes the write even if worker completion followed the prior callback. */
    if (frame_ok && finished)
        frame_ok = cna_game_run_frame_ext(state.game, &running) == CNA_RESULT_SUCCESS && running;
    cna_demo_check_result(&demo, cna_game_set_foreign_thread_calls_ext(state.game, CNA_FALSE), "disable dispatch before joining or cancellation cleanup");
    for (int attempt = 0; attempt < 2500 && !atomic_load_explicit(&state.done, memory_order_acquire); ++attempt)
        (void)thrd_sleep(&pause, NULL);
    if (!atomic_load_explicit(&state.done, memory_order_acquire)) {
        cna_demo_check(&demo, 0, "the worker returns after bounded dispatch shutdown");
        exit(EXIT_FAILURE); /* Keep live stack-backed request data out of destruction paths. */
    }
    const int joined = thrd_join(thread, &joined_result);
    cna_demo_check(&demo, joined == thrd_success && joined_result == 0, "join the completed worker after dispatch shutdown");
    cna_demo_check(&demo, finished && state.result == CNA_RESULT_SUCCESS && state.before == INITIAL_TICKS && state.after == CHANGED_TICKS,
                   "automatically served reads surround the worker's actual target-step write");
    cna_demo_check(&demo, frame_ok && state.updates > 0 && state.saw_change && state.wrong_thread == 0,
                   "the owner's Update observes the new value with valid frame timing");
    cna_demo_check(&demo, state.suppressed > 0 && state.draws == 0,
                   "automatic dispatch succeeds while every Draw is suppressed");
    cna_demo_check(&demo, finished && state.borrow_result == CNA_RESULT_SUCCESS && state.expired_device != CNA_INVALID_HANDLE,
                   "automatic service supplies the callback scope required to borrow the device after startup");
    uint64_t name_bytes = 0;
    cna_demo_check(&demo, state.expired_device != CNA_INVALID_HANDLE &&
                   cna_graphics_device_get_renderer_name_size(state.expired_device, &name_bytes) == CNA_RESULT_INVALID_HANDLE,
                   "the automatically served device borrow expires before returning to the host");
    printf("Automatic dispatch: before=%lld, after=%lld, observed=%d, draws=%d\n",
           (long long)state.before, (long long)state.after, state.saw_change, state.draws);
    if (!cna_demo_check_result(&demo, cna_game_set_foreign_thread_calls_ext(state.game, CNA_TRUE),
                               "enable dispatch for the isolated Draw service phase")) goto cleanup;
    state.draw_phase = 1;
    const CNA_Result draw_frame = cna_game_run_frame_ext(state.game, &running);
    state.draw_phase = 0;
    cna_demo_check(&demo, state.draw_started, "start a permitted setter inside BeginDraw after Update has returned");
    cna_demo_check(&demo, draw_frame == CNA_RESULT_SUCCESS && running && state.draw_pending &&
                   state.draws == 1 && state.updates == state.draw_begin_updates &&
                   state.draw_observed == DRAW_TICKS && state.wrong_thread == 0,
                   "the first Draw observes the pending write without any intervening Update or explicit pump");
    cna_demo_check_result(&demo, cna_game_set_foreign_thread_calls_ext(state.game, CNA_FALSE),
                          "disable Draw-phase dispatch before bounded worker cleanup");
    if (state.draw_started) {
        for (int attempt = 0; attempt < 2500 && !atomic_load_explicit(&state.draw_done, memory_order_acquire); ++attempt)
            (void)thrd_sleep(&pause, NULL);
        if (!atomic_load_explicit(&state.draw_done, memory_order_acquire)) {
            cna_demo_check(&demo, 0, "the Draw-phase worker returns after bounded dispatch shutdown");
            exit(EXIT_FAILURE);
        }
    }
    int draw_join_result = 1;
    const int draw_joined = state.draw_started ? thrd_join(state.draw_worker, &draw_join_result) : thrd_error;
    cna_demo_check(&demo, draw_joined == thrd_success && draw_join_result == 0,
                   "join the Draw-served worker after dispatch shutdown");
    cna_demo_check(&demo, state.draw_started && atomic_load_explicit(&state.draw_done, memory_order_acquire) &&
                   state.draw_result == CNA_RESULT_SUCCESS && state.draw_observed == DRAW_TICKS,
                   "the Draw-phase setter completed successfully rather than being cancelled at cleanup");
    printf("Automatic Draw: pending=%d, before=%lld, observed=%lld, updates_between=%d, result=%d\n",
           state.draw_pending, (long long)state.draw_before, (long long)state.draw_observed,
           state.updates - state.draw_begin_updates, (int)state.draw_result);
    cna_demo_check(&demo, cna_game_request_exit(state.game) == CNA_RESULT_SUCCESS &&
                   cna_game_run_frame_ext(state.game, &running) == CNA_RESULT_SUCCESS && !running,
                   "the owner ends its host-driven run after the worker finishes");
cleanup:
    cna_demo_check_result(&demo, cna_game_destroy(state.game), "destroy the game on its owner thread");
    return cna_demo_finish(&demo);
}
