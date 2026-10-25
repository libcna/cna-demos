/* SPDX-License-Identifier: MS-PL */
/* Host microphone availability and the common indexed upper bound through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static void on_buffer_ready(void* context)
{
    ++*(int*)context;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GameCreateInfo game_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_AudioEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    CNA_MicrophoneState state = CNA_MICROPHONE_STATE_MAXIMUM + UINT32_C(1);
    CNA_Bool available = UINT8_C(7);
    CNA_Bool headset = UINT8_C(7);
    uint8_t captured[19200];
    char name[256];
    uint64_t count = UINT64_C(99);
    uint64_t default_index = UINT64_MAX;
    uint64_t bytes = UINT64_C(99);
    int32_t sample_rate = -1;
    int32_t sample_bytes = -1;
    int64_t duration = INT64_C(-1);
    int ready_calls = 0;
    int ready_calls_before_bounds;

    cna_demo_init(&demo, "c_api.audio.microphone_availability", argc, argv, 1);
    memset(&game_info, 0, sizeof(game_info));
    game_info.struct_size = (uint32_t)sizeof(game_info);
    game_info.struct_version = UINT32_C(1);
    game_info.is_fixed_time_step = CNA_FALSE;
    game_info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(
            &demo, cna_game_create(&game_info, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }

    cna_demo_check(
        &demo,
        cna_microphone_get_count(game, &count) == CNA_RESULT_SUCCESS,
        "read the host microphone-count snapshot");
    printf("  host microphones: %llu\n", (unsigned long long)count);
    cna_demo_check(
        &demo,
        cna_microphone_get_default_index_ext(game, &default_index, &available) ==
                CNA_RESULT_SUCCESS &&
            ((count == 0U && available == CNA_FALSE && default_index == UINT64_MAX) ||
             (count > 0U && available == CNA_TRUE && default_index < count)),
        "default availability is separate from its index value");
    cna_demo_check_result(
        &demo, cna_microphone_check_all_buffers_ext(game),
        "pump all currently available capture buffers");

    if (count > 0U && available == CNA_TRUE) {
        char untouched[sizeof(name)];
        uint64_t name_bytes = UINT64_C(0);
        uint64_t copied = UINT64_C(0);

        memset(name, 0xA5, sizeof(name));
        cna_demo_check(
            &demo,
            cna_microphone_get_name_size_at(game, default_index, &name_bytes) ==
                    CNA_RESULT_SUCCESS &&
                name_bytes > 0U && name_bytes < sizeof(name) &&
                cna_microphone_copy_name_at(
                    game, default_index, name, name_bytes, &copied) == CNA_RESULT_SUCCESS &&
                copied == name_bytes && (unsigned char)name[name_bytes] == 0xA5U,
            "the default microphone exposes an exact nonterminated UTF-8 name");
        memset(name, 0xA5, sizeof(name));
        memset(untouched, 0xA5, sizeof(untouched));
        copied = UINT64_C(0);
        cna_demo_check(
            &demo,
            cna_microphone_copy_name_at(
                game, default_index, name, name_bytes - 1U, &copied) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                copied == name_bytes && memcmp(name, untouched, sizeof(name)) == 0,
            "a short microphone-name buffer is refused atomically");
        cna_demo_check(
            &demo,
            cna_microphone_get_buffer_duration_ticks_at(
                game, default_index, &duration) == CNA_RESULT_SUCCESS &&
                duration >= INT64_C(1000000) &&
                cna_microphone_get_is_headset_at(
                    game, default_index, &headset) == CNA_RESULT_SUCCESS &&
                (headset == CNA_FALSE || headset == CNA_TRUE) &&
                cna_microphone_get_sample_rate_at(
                    game, default_index, &sample_rate) == CNA_RESULT_SUCCESS &&
                sample_rate > 0 &&
                cna_microphone_get_state_at(game, default_index, &state) == CNA_RESULT_SUCCESS &&
                (state == CNA_MICROPHONE_STATE_STOPPED || state == CNA_MICROPHONE_STATE_STARTED),
            "read duration, headset, sample-rate and state metadata without opening capture");
        cna_demo_check(
            &demo,
            cna_microphone_get_sample_size_in_bytes_at(
                game, default_index, INT64_C(1000000), &sample_bytes) == CNA_RESULT_SUCCESS &&
                sample_bytes > 0 && (uint64_t)sample_bytes <= sizeof(captured) &&
                cna_microphone_get_sample_duration_ticks_at(
                    game, default_index, sample_bytes, &duration) == CNA_RESULT_SUCCESS &&
                duration == INT64_C(1000000),
            "sample-size and duration conversion round-trips 100 ms without opening capture");
    }

    memset(name, 0xA5, sizeof(name));
    bytes = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_microphone_get_name_size_at(game, count, &bytes) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_microphone_copy_name_at(game, count, name, sizeof(name), &bytes) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "the index at Count is refused by both microphone-name routes");
    cna_demo_check(
        &demo,
        cna_microphone_get_buffer_duration_ticks_at(game, count, &duration) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_microphone_set_buffer_duration_ticks_at(
                game, count, INT64_C(1000000)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_microphone_get_is_headset_at(game, count, &headset) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_microphone_get_sample_rate_at(game, count, &sample_rate) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_microphone_get_state_at(game, count, &state) == CNA_RESULT_INVALID_ARGUMENT,
        "the index at Count is refused by capture metadata and state routes");
    bytes = UINT64_C(99);
    memset(captured, 0xA5, sizeof(captured));
    cna_demo_check(
        &demo,
        cna_microphone_start_at(game, count) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_microphone_stop_at(game, count) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_microphone_get_data_at(game, count, captured, sizeof(captured), &bytes) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "the index at Count is refused by capture transport and data routes");
    ready_calls_before_bounds = ready_calls;
    cna_demo_check(
        &demo,
        cna_microphone_get_sample_duration_ticks_at(
            game, count, 1600, &duration) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_microphone_get_sample_size_in_bytes_at(
                game, count, INT64_C(1000000), &sample_bytes) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_microphone_subscribe_buffer_ready_at(
                game, count, on_buffer_ready, &ready_calls, &registration) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            registration == CNA_INVALID_HANDLE && ready_calls == ready_calls_before_bounds,
        "the index at Count is refused by conversion and BufferReady routes");

    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
