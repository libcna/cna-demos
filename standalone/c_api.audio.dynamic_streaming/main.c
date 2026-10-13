/* SPDX-License-Identifier: MS-PL */
/* Caller-fed integer and float audio queues through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct BufferNeededState {
    int calls;
} BufferNeededState;

static void on_buffer_needed(void* context)
{
    ++((BufferNeededState*)context)->calls;
}

int main(int argc, char** argv)
{
    uint8_t pcm[1604];
    float samples[404];
    CnaDemo demo;
    CNA_AudioCapabilities capabilities;
    CNA_GameCreateInfo game_info;
    CNA_SoundEffectInstanceInfo info;
    BufferNeededState needed = {0};
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle integer_stream = CNA_INVALID_HANDLE;
    CNA_Handle float_stream = CNA_INVALID_HANDLE;
    CNA_AudioEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    int32_t pending = -1;
    int32_t bytes = -1;
    int64_t ticks = INT64_C(-1);

    cna_demo_init(&demo, "c_api.audio.dynamic_streaming", argc, argv, 1);
    memset(pcm, 0x31, sizeof(pcm));
    memset(samples, 0, sizeof(samples));
    memset(&game_info, 0, sizeof(game_info));
    game_info.struct_size = (uint32_t)sizeof(game_info);
    game_info.struct_version = UINT32_C(1);
    game_info.is_fixed_time_step = CNA_FALSE;
    game_info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(
            &demo, cna_game_create(&game_info, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }
    memset(&capabilities, 0, sizeof(capabilities));
    capabilities.struct_size = (uint32_t)sizeof(capabilities);
    capabilities.struct_version = UINT32_C(1);
    if (cna_audio_get_capabilities(game, &capabilities) != CNA_RESULT_SUCCESS ||
        capabilities.is_playback_available != CNA_TRUE) {
        (void)cna_game_destroy(game);
        return cna_demo_skip("no audio playback device");
    }

    cna_demo_check(
        &demo,
        cna_dynamic_sound_effect_instance_create(
            game, 7999, CNA_AUDIO_CHANNELS_MONO, &integer_stream) == CNA_RESULT_INVALID_ARGUMENT &&
            integer_stream == CNA_INVALID_HANDLE &&
            cna_dynamic_sound_effect_instance_create(
                game, 8000, CNA_AUDIO_CHANNELS_MONO, &integer_stream) == CNA_RESULT_SUCCESS,
        "create an 8 kHz mono stream after rejecting an out-of-range sample rate");
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_get_info(integer_stream, &info) == CNA_RESULT_SUCCESS &&
            info.state == CNA_SOUND_STATE_STOPPED && info.is_looped == CNA_FALSE &&
            info.volume == 1.0f && info.pitch == 0.0f && info.pan == 0.0f,
        "the dynamic handle exposes the canonical stopped sound-instance snapshot");
    cna_demo_check(
        &demo,
        cna_dynamic_sound_effect_instance_get_sample_duration_ticks(
            integer_stream, 1600, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(1000000) &&
            cna_dynamic_sound_effect_instance_get_sample_size_in_bytes(
                integer_stream, ticks, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == 1600,
        "instance sample conversion round-trips one tenth of a second exactly");
    cna_demo_check(
        &demo,
        cna_dynamic_sound_effect_instance_get_pending_buffer_count(
            integer_stream, &pending) == CNA_RESULT_SUCCESS && pending == 0,
        "a new stream has no pending caller buffers");
    cna_demo_check(
        &demo,
        cna_dynamic_sound_effect_instance_submit_buffer(
            integer_stream, pcm, sizeof(pcm), 2, 800) == CNA_RESULT_SUCCESS &&
            cna_dynamic_sound_effect_instance_submit_buffer(
                integer_stream, pcm, sizeof(pcm), 802, 800) == CNA_RESULT_SUCCESS &&
            cna_dynamic_sound_effect_instance_get_pending_buffer_count(
                integer_stream, &pending) == CNA_RESULT_SUCCESS && pending == 2,
        "two aligned PCM16 source windows become two pending buffers");
    memset(pcm, 0xD4, sizeof(pcm));
    cna_demo_check(
        &demo,
        cna_dynamic_sound_effect_instance_queue_initial_buffers_ext(integer_stream) ==
                CNA_RESULT_SUCCESS &&
            cna_dynamic_sound_effect_instance_get_pending_buffer_count(
                integer_stream, &pending) == CNA_RESULT_SUCCESS && pending == 2,
        "staged buffers remain owned after caller storage reuse and an initial queue request");
    cna_demo_check(
        &demo,
        cna_dynamic_sound_effect_instance_submit_buffer(
            integer_stream, pcm, sizeof(pcm), 3, 800) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_dynamic_sound_effect_instance_submit_buffer(
                integer_stream, pcm, sizeof(pcm), 2, 799) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_dynamic_sound_effect_instance_get_pending_buffer_count(
                integer_stream, &pending) == CNA_RESULT_SUCCESS && pending == 2,
        "misaligned PCM16 offset and length are refused without changing the queue");
    cna_demo_check_result(
        &demo,
        cna_dynamic_sound_effect_instance_subscribe_buffer_needed(
            integer_stream, on_buffer_needed, &needed, &registration),
        "subscribe to BufferNeeded");
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_play(integer_stream) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_instance_get_info(integer_stream, &info) == CNA_RESULT_SUCCESS &&
            info.state == CNA_SOUND_STATE_PLAYING,
        "Play starts the dynamic instance through the shared transport API");
    cna_demo_check(
        &demo,
        cna_dynamic_sound_effect_instance_submit_float_buffer_ext(
            integer_stream, samples, sizeof(samples) / sizeof(samples[0]), 2, 400) ==
                CNA_RESULT_INVALID_STATE,
        "a live integer stream refuses a float-format transition");
    cna_demo_check(
        &demo,
        cna_dynamic_sound_effect_instance_clear_buffers_ext(integer_stream) == CNA_RESULT_SUCCESS &&
            cna_dynamic_sound_effect_instance_get_pending_buffer_count(
                integer_stream, &pending) == CNA_RESULT_SUCCESS && pending == 0,
        "clearing a live stream removes queued and submitted buffers");
    cna_demo_check(
        &demo,
        cna_dynamic_sound_effect_instance_update_ext(integer_stream) == CNA_RESULT_SUCCESS &&
            needed.calls == 3,
        "an explicit empty-queue update raises the three low-buffer requests");
    cna_demo_check(
        &demo,
        cna_audio_unsubscribe_ext(registration) == CNA_RESULT_SUCCESS &&
            cna_dynamic_sound_effect_instance_update_ext(integer_stream) == CNA_RESULT_SUCCESS &&
            needed.calls == 3 &&
            cna_audio_unsubscribe_ext(registration) == CNA_RESULT_INVALID_HANDLE,
        "released BufferNeeded registration ignores later updates and expires once");
    registration = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_stop(integer_stream, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_instance_get_info(integer_stream, &info) == CNA_RESULT_SUCCESS &&
            info.state == CNA_SOUND_STATE_STOPPED,
        "immediate Stop returns the integer stream to its empty stopped state");

    cna_demo_check(
        &demo,
        cna_dynamic_sound_effect_instance_create(
            game, 8000, CNA_AUDIO_CHANNELS_MONO, &float_stream) == CNA_RESULT_SUCCESS &&
            cna_dynamic_sound_effect_instance_submit_float_buffer_ext(
                float_stream, samples, sizeof(samples) / sizeof(samples[0]), 2, 400) ==
                CNA_RESULT_SUCCESS &&
            cna_dynamic_sound_effect_instance_get_pending_buffer_count(
                float_stream, &pending) == CNA_RESULT_SUCCESS && pending == 1,
        "a second instance copies one float-sample source window");
    memset(samples, 0xA5, sizeof(samples));
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_play(float_stream) == CNA_RESULT_SUCCESS &&
            cna_dynamic_sound_effect_instance_submit_buffer(
                float_stream, pcm, sizeof(pcm), 2, 800) == CNA_RESULT_INVALID_STATE,
        "a live float stream symmetrically refuses integer input");
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_stop(float_stream, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_dynamic_sound_effect_instance_submit_buffer(
                float_stream, pcm, sizeof(pcm), 2, 800) == CNA_RESULT_SUCCESS &&
            cna_dynamic_sound_effect_instance_get_pending_buffer_count(
                float_stream, &pending) == CNA_RESULT_SUCCESS && pending == 1,
        "after Stop the same instance may switch back to integer input");

    cna_demo_check_result(
        &demo, cna_sound_effect_instance_destroy(float_stream), "destroy the format-switch stream");
    cna_demo_check_result(
        &demo, cna_sound_effect_instance_destroy(integer_stream), "destroy the integer stream");
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
