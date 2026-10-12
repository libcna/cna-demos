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

enum { ORDINARY_CALL, DRIVE_FRAME, DESTROY_GAME };

typedef struct Request {
    CNA_Handle game;
    int change_step;
    int fail_volume;
    int owner_operation;
    int cancelled;
    int cancel_pending;
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
    /* Ordinary calls can wait for owner dispatch; driving and destruction must still refuse.
       Every out parameter stays alive for the whole call. */
    if (request->cancel_pending) {
        /* This permitted property write should queue, then be cancelled without changing the
           owner's value. Start with the setter itself, not an earlier queued read. */
        request->result = cna_game_set_target_elapsed_time_ticks(request->game, INITIAL_TICKS);
    } else if (request->owner_operation != ORDINARY_CALL) {
        CNA_Bool running = CNA_TRUE;
        /* These routes must refuse a foreign caller even while ordinary dispatch is enabled.
           The owner deliberately NEVER pumps this request. */
        request->result = request->owner_operation == DRIVE_FRAME ?
            cna_game_run_frame_ext(request->game, &running) : cna_game_destroy(request->game);
    } else {
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
    const int attempts = request->owner_operation == ORDINARY_CALL && !request->cancel_pending ? 2500 : 500;
    for (int attempt = 0; attempt < attempts && !atomic_load_explicit(&request->done, memory_order_acquire); ++attempt) {
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
        request->cancelled = 1;
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
    if (request->cancel_pending)
        return cna_demo_check(demo, joined == thrd_success && result == 0,
                              "the pending property worker is released before joining");
    if (request->owner_operation != ORDINARY_CALL)
        return cna_demo_check(demo, joined == thrd_success && result == 0,
                              "the owner-only worker returns or is safely cancelled before joining");
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
    for (int operation = DRIVE_FRAME; operation <= DESTROY_GAME; ++operation) {
        Request request;
        memset(&request, 0, sizeof(request));
        request.game = game;
        request.owner_operation = operation;
        /* Re-enable for each attempt: cancellation of ISSUE-050 turns dispatch off. */
        if (!cna_demo_check_result(&demo, cna_game_set_foreign_thread_calls_ext(game, CNA_TRUE),
                                   "enable dispatch for an owner-only operation probe")) break;
        if (!run_worker(&demo, &request, 0)) break;
        /* A worker delayed until after shutdown could return THREAD without testing the enabled
           state. Report that as inconclusive, never as proof that the owner-only contract works. */
        const int conclusive = !request.cancelled || request.result == CNA_RESULT_INVALID_STATE;
        cna_demo_check(&demo, conclusive, "the owner-only probe reaches a conclusive call or cancellation result");
        if (conclusive)
            cna_demo_check(&demo, !request.cancelled && request.result == CNA_RESULT_THREAD &&
                           request.error_query == CNA_RESULT_SUCCESS && request.error.result == CNA_RESULT_THREAD &&
                           request.error.category == CNA_ERROR_CATEGORY_THREAD,
                           operation == DRIVE_FRAME ?
                           "foreign frame driving is refused without queuing (CNA-DEMOS-ISSUE-050)" :
                           "foreign game destruction is refused without queuing (CNA-DEMOS-ISSUE-050)");
        printf("Owner-only %s: cancelled=%d, result=%u, category=%u\n",
               operation == DRIVE_FRAME ? "RunFrameEXT" : "Destroy", request.cancelled,
               (unsigned)request.result, (unsigned)request.error.category);
        int64_t owner_ticks = 0;
        cna_demo_check(&demo, cna_game_get_target_elapsed_time_ticks(game, &owner_ticks) == CNA_RESULT_SUCCESS &&
                       owner_ticks == CHANGED_TICKS,
                       "the owner still has its unchanged game after the forbidden operation probe");
    }
    /* A permitted property call has the opposite contract from the owner-only probes above:
       it should wait for dispatch, and disabling the feature must cancel it without execution. */
    Request pending;
    memset(&pending, 0, sizeof(pending));
    pending.game = game;
    pending.cancel_pending = 1;
    if (cna_demo_check_result(&demo, cna_game_set_foreign_thread_calls_ext(game, CNA_TRUE),
                              "enable dispatch for a pending property write") &&
        run_worker(&demo, &pending, 0)) {
        cna_demo_check(&demo, pending.cancelled && pending.result == CNA_RESULT_INVALID_STATE &&
                       pending.error_query == CNA_RESULT_SUCCESS &&
                       pending.error.result == CNA_RESULT_INVALID_STATE && pending.error.category == CNA_ERROR_CATEGORY_STATE,
                       "disabling dispatch cancels an actually queued property write with INVALID_STATE");
        printf("Cancelled property: cancelled=%d, result=%u, category=%u\n", pending.cancelled,
               (unsigned)pending.result, (unsigned)pending.error.category);
        int64_t ticks = 0;
        cna_demo_check(&demo, cna_game_get_target_elapsed_time_ticks(game, &ticks) == CNA_RESULT_SUCCESS &&
                       ticks == CHANGED_TICKS, "the cancelled setter leaves the owner's target step unchanged");
        cna_demo_check(&demo, cna_game_run_foreign_thread_calls_ext(game) == CNA_RESULT_SUCCESS &&
                       cna_game_get_target_elapsed_time_ticks(game, &ticks) == CNA_RESULT_SUCCESS && ticks == CHANGED_TICKS,
                       "pumping disabled dispatch does not replay the cancelled setter");
        Request resumed;
        memset(&resumed, 0, sizeof(resumed));
        resumed.game = game;
        if (cna_demo_check_result(&demo, cna_game_set_foreign_thread_calls_ext(game, CNA_TRUE),
                                  "re-enable dispatch after cancelling the pending write") &&
            run_worker(&demo, &resumed, 1))
            cna_demo_check(&demo, resumed.result == CNA_RESULT_SUCCESS && resumed.before == CHANGED_TICKS,
                           "a fresh worker is served after re-enabling and sees no replay of the cancelled setter");
    }
    cna_demo_check_result(&demo, cna_game_set_foreign_thread_calls_ext(game, CNA_FALSE), "leave dispatch disabled");
    /* Re-enable only for the final destruction boundary: do not pump this permitted setter.
       An INVALID_STATE worker result proves actual queue cancellation; a worker delayed until
       after destruction sees an invalid handle and cannot pass that assertion. */
    Request dying;
    memset(&dying, 0, sizeof(dying));
    dying.game = game;
    dying.cancel_pending = 1;
    atomic_init(&dying.done, 0);
    thrd_t final_thread;
    int final_started = 0, pending_at_destroy = 0;
    const struct timespec final_pause = {0, 2000000L};
    if (cna_demo_check_result(&demo, cna_game_set_foreign_thread_calls_ext(game, CNA_TRUE),
                              "enable dispatch for owner-destruction cancellation"))
        final_started = cna_demo_check(&demo, thrd_create(&final_thread, worker, &dying) == thrd_success,
                                       "start a property worker before owner destruction");
    if (final_started) {
        for (int attempt = 0; attempt < 500 && !atomic_load_explicit(&dying.done, memory_order_acquire); ++attempt)
            (void)thrd_sleep(&final_pause, NULL);
        pending_at_destroy = !atomic_load_explicit(&dying.done, memory_order_acquire);
        int64_t ticks = 0;
        cna_demo_check(&demo, pending_at_destroy &&
                       cna_game_get_target_elapsed_time_ticks(game, &ticks) == CNA_RESULT_SUCCESS && ticks == CHANGED_TICKS,
                       "the unserved property worker waits while the owner retains its previous target step");
    }
    const CNA_Result destroy_result = cna_game_destroy(game);
    cna_demo_check_result(&demo, destroy_result, "destroy the game on its owner thread");
    if (final_started) {
        if (destroy_result != CNA_RESULT_SUCCESS)
            (void)cna_game_set_foreign_thread_calls_ext(game, CNA_FALSE); /* Failed-destroy cleanup only. */
        for (int attempt = 0; attempt < 2500 && !atomic_load_explicit(&dying.done, memory_order_acquire); ++attempt)
            (void)thrd_sleep(&final_pause, NULL);
        if (!atomic_load_explicit(&dying.done, memory_order_acquire)) {
            cna_demo_check(&demo, 0, "the property worker returns after bounded owner-destruction cleanup");
            exit(EXIT_FAILURE);
        }
        int thread_result = 1;
        const int joined = thrd_join(final_thread, &thread_result);
        cna_demo_check(&demo, joined == thrd_success && thread_result == 0,
                       "join the released property worker after owner destruction");
        cna_demo_check(&demo, pending_at_destroy && destroy_result == CNA_RESULT_SUCCESS &&
                       dying.result == CNA_RESULT_INVALID_STATE && dying.error_query == CNA_RESULT_SUCCESS &&
                       dying.error.result == CNA_RESULT_INVALID_STATE && dying.error.category == CNA_ERROR_CATEGORY_STATE,
                       "owner destruction cancels an actually queued call with the worker's INVALID_STATE error record");
        int64_t untouched = INT64_C(-12345);
        cna_demo_check(&demo, cna_game_get_target_elapsed_time_ticks(game, &untouched) == CNA_RESULT_INVALID_HANDLE &&
                       untouched == INT64_C(-12345),
                       "the destroyed owner handle is invalid and a refused query preserves its output");
        printf("Destroyed property: pending=%d, result=%u, category=%u\n", pending_at_destroy,
               (unsigned)dying.result, (unsigned)dying.error.category);
    }
    if (destroy_result != CNA_RESULT_SUCCESS) (void)cna_game_destroy(game);
    return cna_demo_finish(&demo);
}
