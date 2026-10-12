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
    int fail_volume;
    int copied_diagnostic;
    char message[512], exception_type[128], parameter[32];
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
    if (request->result == CNA_RESULT_SUCCESS && request->fail_volume) {
        /* Unlike an invalid target step, this reaches the native setter after handle affinity
           dispatch. CNA refuses 2.0 before initializing audio or changing the global volume. */
        request->result = cna_sound_effect_set_master_volume(request->game, 2.0f);
    } else if (request->result == CNA_RESULT_SUCCESS && request->change_step) {
        request->result = cna_game_set_target_elapsed_time_ticks(request->game, CHANGED_TICKS);
        if (request->result == CNA_RESULT_SUCCESS)
            request->result = cna_game_get_target_elapsed_time_ticks(request->game, &request->after);
    }
    request->error.struct_size = (uint32_t)sizeof(request->error);
    request->error.struct_version = UINT32_C(1);
    request->error_query = cna_error_get_last_info(&request->error);
    if (request->fail_volume) {
        uint64_t needed = 0, copied = 0;
        /* Query on the worker: the owner must forward the actual error record, including
           canonical details. Leave room for our terminators outside each copied payload. */
        request->copied_diagnostic =
            cna_error_get_last_message_size(&needed) == CNA_RESULT_SUCCESS &&
            needed > 0 && needed < sizeof(request->message) &&
            cna_error_copy_last_message(request->message, needed, &copied) == CNA_RESULT_SUCCESS &&
            copied == needed;
        if (request->copied_diagnostic) request->message[copied] = '\0';
        request->copied_diagnostic = request->copied_diagnostic &&
            cna_error_get_last_exception_type_size_ext(&needed) == CNA_RESULT_SUCCESS &&
            needed > 0 && needed < sizeof(request->exception_type) &&
            cna_error_copy_last_exception_type_ext(request->exception_type, needed, &copied) == CNA_RESULT_SUCCESS &&
            copied == needed;
        if (request->copied_diagnostic) request->exception_type[copied] = '\0';
        request->copied_diagnostic = request->copied_diagnostic &&
            cna_error_get_last_exception_param_name_size_ext(&needed) == CNA_RESULT_SUCCESS &&
            needed > 0 && needed < sizeof(request->parameter) &&
            cna_error_copy_last_exception_param_name_ext(request->parameter, needed, &copied) == CNA_RESULT_SUCCESS &&
            copied == needed;
        if (request->copied_diagnostic) request->parameter[copied] = '\0';
    }
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
            Request refused;
            memset(&refused, 0, sizeof(refused));
            refused.game = game;
            refused.fail_volume = 1;
            if (!run_worker(&demo, &refused, 1)) break;
            cna_demo_check(&demo, refused.before == CHANGED_TICKS &&
                           refused.result == CNA_RESULT_INVALID_ARGUMENT &&
                           refused.error_query == CNA_RESULT_SUCCESS &&
                           refused.error.result == CNA_RESULT_INVALID_ARGUMENT &&
                           refused.error.category == CNA_ERROR_CATEGORY_ARGUMENT,
                           "a served native failure returns its result and category to the worker");
            cna_demo_check(&demo, refused.copied_diagnostic &&
                           strcmp(refused.exception_type, "System.ArgumentOutOfRangeException") == 0 &&
                           strcmp(refused.parameter, "value") == 0 &&
                           strcmp(refused.message, "Specified argument was out of the range of valid values.\n"
                                                   "Parameter name: value") == 0,
                           "the worker receives the native exception type, parameter and message");
            if (refused.copied_diagnostic)
                printf("Served diagnostic: %s (%s): %s\n",
                       refused.exception_type, refused.parameter, refused.message);
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
