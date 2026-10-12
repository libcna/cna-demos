/* SPDX-License-Identifier: MS-PL */
/* CNA-DEMOS-ISSUE-050: a diagnostic tool, not a catalog demo.
 * Run with no argument for RunFrameEXT, or "destroy" for game destruction.
 * Exit 0: direct THREAD refusal; 1: forbidden operation demonstrably queued;
 * 2: unexpected/inconclusive. See docs/CNA_ISSUES.md for build/run commands.
 * Never pump: even a broken dispatch policy cannot execute the forbidden operation.
 */
#include <CNA/C/cna.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

typedef struct Probe {
    CNA_Handle game;
    int destroy;
    atomic_int ready, done;
    CNA_Result result, diagnostic;
    CNA_ErrorInfo error;
} Probe;

static int run(void* opaque)
{
    Probe* probe = opaque;
    CNA_Bool running = CNA_TRUE;
    atomic_store_explicit(&probe->ready, 1, memory_order_release);
    probe->result = probe->destroy ? cna_game_destroy(probe->game) :
                                    cna_game_run_frame_ext(probe->game, &running);
    probe->error.struct_size = sizeof(probe->error);
    probe->error.struct_version = 1;
    probe->diagnostic = cna_error_get_last_info(&probe->error);
    atomic_store_explicit(&probe->done, 1, memory_order_release);
    return 0;
}

int main(int argc, char** argv)
{
    Probe probe = {0};
    CNA_GameCreateInfo info = {0};
    thrd_t thread;
    const struct timespec pause = {0, 2000000L};
    info.struct_size = sizeof(info);
    info.struct_version = 1;
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = 166667;
    probe.destroy = argc > 1 && strcmp(argv[1], "destroy") == 0;
    atomic_init(&probe.ready, 0);
    atomic_init(&probe.done, 0);
    if (cna_game_create(&info, &probe.game) != CNA_RESULT_SUCCESS) return 2;
    if (cna_game_set_foreign_thread_calls_ext(probe.game, CNA_TRUE) != CNA_RESULT_SUCCESS) return 2;
    if (thrd_create(&thread, run, &probe) != thrd_success) return 2;
    for (int n=0; n<1000 && !atomic_load_explicit(&probe.ready, memory_order_acquire); ++n)
        (void)thrd_sleep(&pause, NULL);
    for (int n=0; n<500 && !atomic_load_explicit(&probe.done, memory_order_acquire); ++n)
        (void)thrd_sleep(&pause, NULL);
    const int cancelled = !atomic_load_explicit(&probe.done, memory_order_acquire);
    /* No queue pumping at any point. Cancellation frees a wrongly queued caller safely. */
    const CNA_Result disable = cna_game_set_foreign_thread_calls_ext(probe.game, CNA_FALSE);
    for (int n=0; n<1000 && !atomic_load_explicit(&probe.done, memory_order_acquire); ++n)
        (void)thrd_sleep(&pause, NULL);
    if (!atomic_load_explicit(&probe.done, memory_order_acquire)) {
        fprintf(stderr, "Worker did not return after cancellation; ending isolated process.\n");
        exit(2);
    }
    int thread_result = 1;
    const int joined = thrd_join(thread, &thread_result);
    int64_t ticks = 0;
    const CNA_Result owner_read = cna_game_get_target_elapsed_time_ticks(probe.game, &ticks);
    const CNA_Result cleanup = cna_game_destroy(probe.game);
    printf("%s: cancelled=%d, result=%u, diagnostic=%u, category=%u, disable=%u, owner_read=%u, ticks=%lld, cleanup=%u\n",
           probe.destroy ? "cna_game_destroy" : "cna_game_run_frame_ext", cancelled,
           (unsigned)probe.result, (unsigned)probe.diagnostic, (unsigned)probe.error.category,
           (unsigned)disable, (unsigned)owner_read, (long long)ticks, (unsigned)cleanup);
    if (joined != thrd_success || thread_result != 0 || disable != CNA_RESULT_SUCCESS ||
        owner_read != CNA_RESULT_SUCCESS || ticks != 166667 || cleanup != CNA_RESULT_SUCCESS) return 2;
    if (!cancelled && probe.result == CNA_RESULT_THREAD && probe.diagnostic == CNA_RESULT_SUCCESS &&
        probe.error.result == CNA_RESULT_THREAD && probe.error.category == CNA_ERROR_CATEGORY_THREAD) return 0;
    if (cancelled && probe.result == CNA_RESULT_INVALID_STATE) {
        puts("Forbidden operation was queued: only explicit dispatch cancellation released it.");
        return 1;
    }
    puts("Unexpected or scheduling-inconclusive result; inspect before classifying.");
    return 2;
}
