/* SPDX-License-Identifier: MS-PL */
/*
 * Vibration from C: inspect the real capability without actuating it, then prove exact requests
 * through CNA's public in-memory backend and restore the platform backend before exit.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct VibrationState {
    CnaDemo* demo;
    int validated;
} VibrationState;

static CNA_StringView text_view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static void real_capability_contract(CNA_Handle game, CnaDemo* demo)
{
    CNA_VibrationTestLog log;
    CNA_Bool supported = UINT8_C(9);
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    char* name = NULL;
    int ok;

    ok = cna_vibrate_controller_get_is_supported_ext(game, &supported) == CNA_RESULT_SUCCESS &&
        (supported == CNA_FALSE || supported == CNA_TRUE) &&
        cna_vibrate_controller_get_device_name_size_ext(game, &bytes) == CNA_RESULT_SUCCESS;
    if (ok) {
        name = (char*)malloc((size_t)bytes + 1U);
        if (name != NULL) {
            name[bytes] = '\0';
        }
        ok = name != NULL &&
            cna_vibrate_controller_copy_device_name_ext(game, name, bytes + 1U, &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == bytes && strlen(name) == (size_t)bytes;
    }
    ok = ok && cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_INVALID_STATE;
    cna_demo_check(
        demo, ok,
        "the platform capability and exact device name are readable without installing or actuating a backend");
    if (name != NULL) {
        printf("  platform vibration: supported %u, device '%s'\n", (unsigned)supported, name);
    }
    free(name);
}

static void test_backend_contract(CNA_Handle game, CnaDemo* demo)
{
    static const char device_name[] = "CNA vibration probe";
    CNA_VibrationTestLog log;
    CNA_Bool supported = CNA_FALSE;
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    unsigned char short_buffer[4];
    char name[64];

    cna_demo_check(
        demo,
        cna_vibrate_controller_set_test_backend_ext(
            game, CNA_TRUE, CNA_TRUE, text_view(device_name)) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_is_supported_ext(game, &supported) == CNA_RESULT_SUCCESS &&
            supported == CNA_TRUE,
        "the public test backend publishes its authored supported capability");

    memset(name, 0, sizeof(name));
    memset(short_buffer, 0x5a, sizeof(short_buffer));
    cna_demo_check(
        demo,
        cna_vibrate_controller_get_device_name_size_ext(game, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == (uint64_t)strlen(device_name) &&
            cna_vibrate_controller_copy_device_name_ext(
                game, name, sizeof(name), &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && strcmp(name, device_name) == 0 &&
            cna_vibrate_controller_copy_device_name_ext(
                game, (char*)short_buffer, sizeof(short_buffer), &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == bytes && short_buffer[0] == 0x5aU &&
            short_buffer[sizeof(short_buffer) - 1U] == 0x5aU,
        "device-name size/copy is exact and a short destination remains untouched");

    memset(&log, 0x7f, sizeof(log));
    cna_demo_check(
        demo,
        cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.struct_size == (uint32_t)sizeof(log) && log.struct_version == UINT32_C(1) &&
            log.start_calls == UINT32_C(0) && log.stop_calls == UINT32_C(0) &&
            log.left_right_calls == UINT32_C(0) && log.reserved == UINT32_C(0) &&
            log.reserved_float == 0.0F,
        "a new backend exposes a versioned zero-request log");

    cna_demo_check(
        demo,
        cna_vibrate_controller_start(game, INT64_C(10000000)) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.start_calls == UINT32_C(1) &&
            log.last_duration_ticks == INT64_C(10000000) && log.last_intensity == 1.0F,
        "plain Start reaches the backend for one second at full intensity");

    cna_demo_check(
        demo,
        cna_vibrate_controller_start(game, INT64_C(0)) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_start(game, INT64_C(50000000)) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_start(game, INT64_C(-1)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_vibrate_controller_start(game, INT64_C(50000001)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.start_calls == UINT32_C(3) &&
            log.last_duration_ticks == INT64_C(50000000) && log.last_intensity == 1.0F,
        "zero and five seconds are inclusive while adjacent durations are refused before dispatch");

    cna_demo_check(
        demo,
        cna_vibrate_controller_start_with_intensity_ext(
            game, INT64_C(1000000), -2.0F) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.last_intensity == 0.0F &&
            cna_vibrate_controller_start_with_intensity_ext(
                game, INT64_C(2000000), 4.0F) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.last_intensity == 1.0F &&
            cna_vibrate_controller_start_with_intensity_ext(
                game, INT64_C(3000000), NAN) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.start_calls == UINT32_C(6) && log.last_duration_ticks == INT64_C(3000000) &&
            log.last_intensity == 0.0F,
        "single-motor intensity clamps below zero, above one and NaN before dispatch");

    cna_demo_check(
        demo,
        cna_vibrate_controller_start_with_intensity_ext(
            game, INT64_C(50000001), 0.5F) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.start_calls == UINT32_C(6) && log.last_duration_ticks == INT64_C(3000000),
        "the intensity overload refuses an excessive duration without changing the log");

    cna_demo_check(
        demo,
        cna_vibrate_controller_start_left_right_ext(
            game, 0.25F, 0.75F, INT64_C(4000000)) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.left_right_calls == UINT32_C(1) && log.last_large_motor == 0.25F &&
            log.last_small_motor == 0.75F && log.last_duration_ticks == INT64_C(4000000),
        "the two-motor route preserves distinct large and small strengths");

    cna_demo_check(
        demo,
        cna_vibrate_controller_start_left_right_ext(
            game, 5.0F, -5.0F, INT64_C(5000000)) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.last_large_motor == 1.0F && log.last_small_motor == 0.0F &&
            cna_vibrate_controller_start_left_right_ext(
                game, NAN, NAN, INT64_C(6000000)) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.left_right_calls == UINT32_C(3) && log.last_large_motor == 0.0F &&
            log.last_small_motor == 0.0F && log.last_duration_ticks == INT64_C(6000000),
        "each motor clamps independently and NaN becomes zero before dispatch");

    cna_demo_check(
        demo,
        cna_vibrate_controller_start_left_right_ext(
            game, 0.5F, 0.5F, INT64_C(-1)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_vibrate_controller_start_left_right_ext(
                game, 0.5F, 0.5F, INT64_C(50000001)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.left_right_calls == UINT32_C(3) && log.last_duration_ticks == INT64_C(6000000),
        "the two-motor overload refuses both out-of-range durations before dispatch");

    cna_demo_check(
        demo,
        cna_vibrate_controller_stop(game) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_stop(game) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.stop_calls == UINT32_C(2),
        "Stop is safe both after a request and when already idle");

    cna_demo_check(
        demo,
        cna_vibrate_controller_set_test_backend_ext(
            game, CNA_TRUE, CNA_FALSE, text_view("silent backend")) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_is_supported_ext(game, &supported) == CNA_RESULT_SUCCESS &&
            supported == CNA_FALSE &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_SUCCESS &&
            log.start_calls == UINT32_C(0) && log.stop_calls == UINT32_C(0) &&
            log.left_right_calls == UINT32_C(0),
        "replacing the test backend atomically publishes new capability and resets its log");

    cna_demo_check(
        demo,
        cna_vibrate_controller_set_test_backend_ext(
            game, CNA_FALSE, CNA_FALSE, text_view("")) == CNA_RESULT_SUCCESS &&
            cna_vibrate_controller_get_test_log_ext(game, &log) == CNA_RESULT_INVALID_STATE,
        "removing the test backend restores the platform backend and releases its log");
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    VibrationState* state = (VibrationState*)context;
    (void)game_time;
    (void)out_error;
    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    real_capability_contract(game, state->demo);
    test_backend_contract(game, state->demo);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    VibrationState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.devices.vibration", argc, argv, 0);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                          "run one frame for owner-thread vibration requests");
    cna_demo_check(&demo, state.validated == 1,
                   "the update callback completed the vibration audit once");
    (void)cna_vibrate_controller_set_test_backend_ext(
        game, CNA_FALSE, CNA_FALSE, text_view(""));
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
