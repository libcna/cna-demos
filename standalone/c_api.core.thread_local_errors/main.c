/* SPDX-License-Identifier: MS-PL */
/* Independent thread diagnostics, followed by an owner-native exception versus worker C validation. */
#include <CNA/C/cna.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>
#include <time.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ErrorSnapshot {
    CNA_ErrorInfo info;
    char message[256];
    char exception_type[128], parameter[32];
} ErrorSnapshot;

typedef struct Exchange {
    atomic_int phase;
    ErrorSnapshot first_before, first_after;
    ErrorSnapshot second_before, second_after;
} Exchange;

/* Error queries preserve the diagnostic they read. All CNA calls stay visible here. */
static int read_error(ErrorSnapshot* snapshot)
{
    uint64_t length = 0U, copied = 0U;
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->info.struct_size = (uint32_t)sizeof(snapshot->info);
    snapshot->info.struct_version = UINT32_C(1);
    if (cna_error_get_last_info(&snapshot->info) != CNA_RESULT_SUCCESS ||
        cna_error_get_last_message_size(&length) != CNA_RESULT_SUCCESS ||
        length == 0U || length >= sizeof(snapshot->message) ||
        cna_error_copy_last_message(snapshot->message, length, &copied) != CNA_RESULT_SUCCESS ||
        copied != length || snapshot->info.message_byte_length != length) return 0;
    snapshot->message[length] = '\0';
    length = copied = 0;
    if (cna_error_get_last_exception_type_size_ext(&length) != CNA_RESULT_SUCCESS ||
        length >= sizeof(snapshot->exception_type)) return 0;
    if (length > 0 && (cna_error_copy_last_exception_type_ext(snapshot->exception_type, length, &copied) != CNA_RESULT_SUCCESS ||
                       copied != length)) return 0;
    snapshot->exception_type[length] = '\0';
    length = copied = 0;
    if (cna_error_get_last_exception_param_name_size_ext(&length) != CNA_RESULT_SUCCESS ||
        length >= sizeof(snapshot->parameter)) return 0;
    if (length > 0 && (cna_error_copy_last_exception_param_name_ext(snapshot->parameter, length, &copied) != CNA_RESULT_SUCCESS ||
                       copied != length)) return 0;
    snapshot->parameter[length] = '\0';
    return 1;
}

static int same_error(const ErrorSnapshot* left, const ErrorSnapshot* right)
{
    return left->info.result == right->info.result && left->info.category == right->info.category &&
           strcmp(left->message, right->message) == 0 &&
           strcmp(left->exception_type, right->exception_type) == 0 &&
           strcmp(left->parameter, right->parameter) == 0;
}

/* Bounded waits avoid a deadlock if a worker cannot produce its diagnostic. Phase -1 aborts.
   The atomics synchronize the exchange; they do not protect CNA's error storage for it. */
static int await_phase(Exchange* exchange, int wanted)
{
    const struct timespec pause = {0, 2000000L};
    for (int attempt = 0; attempt < 2500; ++attempt) {
        int phase = atomic_load_explicit(&exchange->phase, memory_order_acquire);
        if (phase < 0) return 0;
        if (phase >= wanted) return 1;
        (void)thrd_sleep(&pause, NULL);
    }
    atomic_store_explicit(&exchange->phase, -1, memory_order_release);
    return 0;
}

static int abort_exchange(Exchange* exchange)
{
    atomic_store_explicit(&exchange->phase, -1, memory_order_release);
    return 1;
}

static int first_worker(void* state)
{
    Exchange* exchange = (Exchange*)state;
    const CNA_Vector2 left = {1.0f, 2.0f}, right = {3.0f, 4.0f};
    if (cna_vector2_add(left, right, NULL) != CNA_RESULT_INVALID_ARGUMENT ||
        !read_error(&exchange->first_before)) return abort_exchange(exchange);
    atomic_store_explicit(&exchange->phase, 1, memory_order_release);
    if (!await_phase(exchange, 2) || !read_error(&exchange->first_after)) return abort_exchange(exchange);
    /* Write again after the second thread's error, then let that thread re-query its own. */
    if (cna_vector2_add(left, right, NULL) != CNA_RESULT_INVALID_ARGUMENT) return abort_exchange(exchange);
    atomic_store_explicit(&exchange->phase, 3, memory_order_release);
    return 0;
}

static int second_worker(void* state)
{
    Exchange* exchange = (Exchange*)state;
    if (!await_phase(exchange, 1)) return abort_exchange(exchange);
    if (cna_math_clamp_float(0.5f, 0.0f, 1.0f, NULL) != CNA_RESULT_INVALID_ARGUMENT ||
        !read_error(&exchange->second_before)) return abort_exchange(exchange);
    atomic_store_explicit(&exchange->phase, 2, memory_order_release);
    if (!await_phase(exchange, 3) || !read_error(&exchange->second_after)) return abort_exchange(exchange);
    return 0;
}

static int native_detail_worker(void* state)
{
    Exchange* exchange = (Exchange*)state;
    if (cna_math_clamp_float(0.5f, 0.0f, 1.0f, NULL) != CNA_RESULT_INVALID_ARGUMENT ||
        !read_error(&exchange->first_before)) return abort_exchange(exchange);
    atomic_store_explicit(&exchange->phase, 1, memory_order_release);
    if (!await_phase(exchange, 2) || !read_error(&exchange->first_after)) return abort_exchange(exchange);
    return 0;
}

static void check_canonical_isolation(CnaDemo* demo)
{
    CNA_GameCreateInfo info = {0};
    CNA_Handle game = CNA_INVALID_HANDLE;
    Exchange exchange = {0};
    ErrorSnapshot owner_before, owner_after;
    thrd_t thread;
    int worker_result = 1;
    atomic_init(&exchange.phase, 0);
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(demo, cna_game_create(&info, &game), "create an owner-thread game for a canonical native exception")) return;
    /* MasterVolume rejects 2.0 in native code before audio initialization or mutation.
       The worker uses no Game handle, and the owner never pumps: each keeps its own error. */
    if (!cna_demo_check(demo, cna_sound_effect_set_master_volume(game, 2.0f) == CNA_RESULT_INVALID_ARGUMENT &&
                        read_error(&owner_before) && owner_before.info.category == CNA_ERROR_CATEGORY_ARGUMENT &&
                        strcmp(owner_before.exception_type, "System.ArgumentOutOfRangeException") == 0 &&
                        strcmp(owner_before.parameter, "value") == 0,
                        "the owner stores a canonical exception type and parameter name")) goto cleanup;
    if (!cna_demo_check(demo, thrd_create(&thread, native_detail_worker, &exchange) == thrd_success,
                        "start a C-validation worker while canonical owner details are live")) goto cleanup;
    const int ready = await_phase(&exchange, 1);
    cna_demo_check(demo, ready && read_error(&owner_after) && same_error(&owner_before, &owner_after),
                   "worker validation does not clear the owner's canonical details or message");
    if (ready) {
        cna_demo_check(demo, cna_sound_effect_set_master_volume(game, 2.0f) == CNA_RESULT_INVALID_ARGUMENT &&
                       read_error(&owner_after) && same_error(&owner_before, &owner_after),
                       "the owner writes canonical details again before the worker re-queries");
        atomic_store_explicit(&exchange.phase, 2, memory_order_release);
    } else {
        (void)abort_exchange(&exchange);
    }
    const int joined = thrd_join(thread, &worker_result);
    cna_demo_check(demo, ready && joined == thrd_success && worker_result == 0,
                   "the canonical-detail interleaving finishes within bounded waits");
    if (ready && joined == thrd_success && worker_result == 0) {
        cna_demo_check(demo, exchange.first_before.exception_type[0] == '\0' &&
                       exchange.first_before.parameter[0] == '\0' &&
                       same_error(&exchange.first_before, &exchange.first_after),
                       "owner canonical writes do not populate the worker's empty native detail fields");
        cna_demo_check(demo, read_error(&owner_after) && same_error(&owner_before, &owner_after),
                       "worker queries leave the owner's complete canonical diagnostic intact");
    }
cleanup:
    cna_demo_check_result(demo, cna_game_destroy(game), "destroy the canonical-error fixture on its owner thread");
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    Exchange exchange;
    ErrorSnapshot caller_before, caller_after;
    thrd_t first, second;
    int first_result = 1, second_result = 1;
    cna_demo_init(&demo, "c_api.core.thread_local_errors", argc, argv, 1);
    memset(&exchange, 0, sizeof(exchange));
    atomic_init(&exchange.phase, 0);

    if (!cna_demo_check(&demo, cna_point_init(NULL) == CNA_RESULT_INVALID_ARGUMENT && read_error(&caller_before),
                        "the calling thread stores its own point-output error")) return cna_demo_finish(&demo);
    if (!cna_demo_check(&demo, thrd_create(&first, first_worker, &exchange) == thrd_success,
                        "the first diagnostic worker starts")) return cna_demo_finish(&demo);
    if (!cna_demo_check(&demo, thrd_create(&second, second_worker, &exchange) == thrd_success,
                        "the second diagnostic worker starts")) {
        (void)abort_exchange(&exchange);
        (void)thrd_join(first, &first_result);
        return cna_demo_finish(&demo);
    }
    const int first_joined = thrd_join(first, &first_result);
    const int second_joined = thrd_join(second, &second_result);
    cna_demo_check(&demo, first_joined == thrd_success && second_joined == thrd_success &&
                   first_result == 0 && second_result == 0, "both workers finish the bounded interleaving");
    if (first_joined != thrd_success || second_joined != thrd_success || first_result != 0 || second_result != 0)
        return cna_demo_finish(&demo);

    printf("Caller: %s\nFirst: %s\nSecond: %s\n", caller_before.message,
           exchange.first_before.message, exchange.second_before.message);
    cna_demo_check(&demo, strcmp(exchange.first_before.message, exchange.second_before.message) != 0 &&
                   strcmp(caller_before.message, exchange.first_before.message) != 0 &&
                   strcmp(caller_before.message, exchange.second_before.message) != 0,
                   "the three deliberately different failures have distinguishable diagnostics");
    cna_demo_check(&demo, exchange.first_after.info.result == CNA_RESULT_INVALID_ARGUMENT &&
                   exchange.first_after.info.category == CNA_ERROR_CATEGORY_ARGUMENT &&
                   same_error(&exchange.first_before, &exchange.first_after),
                   "the first worker retains its result, category and message after the second writes");
    cna_demo_check(&demo, exchange.second_after.info.result == CNA_RESULT_INVALID_ARGUMENT &&
                   exchange.second_after.info.category == CNA_ERROR_CATEGORY_ARGUMENT &&
                   same_error(&exchange.second_before, &exchange.second_after),
                   "the second worker retains its diagnostic after the first writes again");
    cna_demo_check(&demo, read_error(&caller_after) && same_error(&caller_before, &caller_after),
                   "both workers leave the calling thread's diagnostic unchanged");
    check_canonical_isolation(&demo);
    return cna_demo_finish(&demo);
}
