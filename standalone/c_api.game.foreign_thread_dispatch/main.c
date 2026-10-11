/* SPDX-License-Identifier: MS-PL */
/* An owner thread explicitly serves worker calls on its game handle. No game loop or drawing. */
#include <CNA/C/cna.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <time.h>

#include "CnaDemos/CApiDemo.h"

#define INITIAL_TICKS INT64_C(166667)
#define CHANGED_TICKS INT64_C(333333)

typedef struct Request {
    CNA_Handle game;
    int change_step;
    atomic_int done;
    CNA_Result result;
    CNA_Result error_query;
    CNA_ErrorInfo error;
    int64_t before, after;
} Request;

static int worker(void* argument)
{
    Request* request = (Request*)argument;
    /* With dispatch off, the same handle is rejected. With it on, this call blocks until
       its owner pumps the queue. The out parameters stay alive for the whole call. */
    request->result = cna_game_get_target_elapsed_time_ticks(request->game, &request->before);
    if (request->result == CNA_RESULT_SUCCESS && request->change_step) {
        request->result = cna_game_set_target_elapsed_time_ticks(request->game, CHANGED_TICKS);
        if (request->result == CNA_RESULT_SUCCESS)
            request->result = cna_game_get_target_elapsed_time_ticks(request->game, &request->after);
    }
    request->error.struct_size = (uint32_t)sizeof(request->error);
    request->error.struct_version = UINT32_C(1);
    request->error_query = cna_error_get_last_info(&request->error);
    atomic_store_explicit(&request->done, 1, memory_order_release);
    return 0;
}

static int run_worker(CnaDemo* demo, Request* request, int pump)
{
    thrd_t thread;
    const struct timespec pause = {0, 2000000L};
    atomic_init(&request->done, 0);
    if (!cna_demo_check(demo, thrd_create(&thread, worker, request) == thrd_success,
                        "a worker starts")) return 0;
    int pumped = 1;
    for (int attempt = 0; attempt < 2500 && !atomic_load_explicit(&request->done, memory_order_acquire); ++attempt) {
        /* Joining here without pumping would deadlock: the worker waits for this owner.
           A normal Game loop pumps automatically; this host explicitly serves the calls. */
        if (pump && cna_game_run_foreign_thread_calls_ext(request->game) != CNA_RESULT_SUCCESS) {
            pumped = 0;
            break;
        }
        (void)thrd_sleep(&pause, NULL);
    }
    const int finished = atomic_load_explicit(&request->done, memory_order_acquire);
    if (!finished) {
        /* Release queued calls before joining on a failure path. Never abandon stack-backed
           request storage or enter an unbounded join if the runtime itself stops responding. */
        (void)cna_game_set_foreign_thread_calls_ext(request->game, CNA_FALSE);
        for (int attempt = 0; attempt < 2500 && !atomic_load_explicit(&request->done, memory_order_acquire); ++attempt)
            (void)thrd_sleep(&pause, NULL);
        if (!atomic_load_explicit(&request->done, memory_order_acquire)) {
            cna_demo_check(demo, 0, "worker did not return after dispatch shutdown");
            exit(EXIT_FAILURE);
        }
    }
    int result = 1;
    const int joined = thrd_join(thread, &result);
    return cna_demo_check(demo, pumped && finished && joined == thrd_success && result == 0,
                          "the worker completes within the bounded service loop");
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GameCreateInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    cna_demo_init(&demo, "c_api.game.foreign_thread_dispatch", argc, argv, 0);
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INITIAL_TICKS;
    if (!cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the game on its owner thread"))
        return cna_demo_finish(&demo);

    for (int phase = 0; phase < 3; ++phase) {
        const int enabled = phase == 1;
        Request request;
        memset(&request, 0, sizeof(request));
        request.game = game;
        request.change_step = enabled;
        /* The first phase tests the default. The third restores strict affinity. */
        if (phase > 0 && !cna_demo_check_result(&demo,
                cna_game_set_foreign_thread_calls_ext(game, enabled ? CNA_TRUE : CNA_FALSE),
                enabled ? "enable owner-thread dispatch" : "disable owner-thread dispatch")) break;
        if (!run_worker(&demo, &request, enabled)) break;
        if (enabled) {
            int64_t owner_ticks = 0;
            cna_demo_check(&demo, request.result == CNA_RESULT_SUCCESS && request.before == INITIAL_TICKS &&
                           request.after == CHANGED_TICKS &&
                           cna_game_get_target_elapsed_time_ticks(game, &owner_ticks) == CNA_RESULT_SUCCESS &&
                           owner_ticks == CHANGED_TICKS,
                           "served worker reads and changes the owner's actual target step");
        } else {
            cna_demo_check(&demo, request.result == CNA_RESULT_THREAD && request.error_query == CNA_RESULT_SUCCESS &&
                           request.error.result == CNA_RESULT_THREAD && request.error.category == CNA_ERROR_CATEGORY_THREAD,
                           phase == 0 ? "foreign handle calls are refused by default" :
                                        "disabling dispatch restores thread-affinity rejection");
        }
    }
    cna_demo_check_result(&demo, cna_game_set_foreign_thread_calls_ext(game, CNA_FALSE), "leave dispatch disabled");
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the game on its owner thread");
    return cna_demo_finish(&demo);
}
