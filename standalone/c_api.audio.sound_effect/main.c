/* SPDX-License-Identifier: MS-PL */
/*
 * Sound from C: a SoundEffect made from PCM samples the program computes, a SoundEffectInstance
 * played from it, and the instance's state followed from the update callback until the sound ends.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed,
 * 77 skipped (no audio device).
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define SAMPLE_RATE 22050U
#define TONE_SAMPLES (SAMPLE_RATE / 2U) /* half a second */

typedef struct AudioState {
    CnaDemo* demo;
    CNA_Handle sound;    /* owned SoundEffect */
    CNA_Handle instance; /* owned SoundEffectInstance */
    int playing_seen;
    int stopped_at_update;
    int updates;
} AudioState;

/* A 440 Hz sine with a short fade in and out, as signed 16-bit little-endian PCM. */
static void make_tone(uint8_t* bytes)
{
    const double pi = 3.14159265358979323846;
    unsigned i;
    for (i = 0; i < TONE_SAMPLES; ++i) {
        const double t = (double)i / SAMPLE_RATE;
        const unsigned edge = i < TONE_SAMPLES - i ? i : TONE_SAMPLES - i;
        const double envelope = edge < 1000U ? edge / 1000.0 : 1.0;
        const int16_t sample = (int16_t)(0.3 * envelope * sin(2.0 * pi * 440.0 * t) * 32767.0);
        bytes[2U * i] = (uint8_t)((uint16_t)sample & 0xFFU);
        bytes[2U * i + 1U] = (uint8_t)((uint16_t)sample >> 8);
    }
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    AudioState* state = (AudioState*)context;
    CNA_SoundEffectInstanceInfo info;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->updates;
    /* The instance's properties come back as one versioned snapshot. */
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    result = cna_sound_effect_instance_get_info(state->instance, &info);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    if (info.state == CNA_SOUND_STATE_PLAYING) {
        state->playing_seen = 1;
    } else if (info.state == CNA_SOUND_STATE_STOPPED && state->stopped_at_update == 0) {
        /* Not looped: the instance stops by itself at the end of the samples. */
        state->stopped_at_update = state->updates;
        printf("stopped after %d updates (%.2f s)\n", state->updates, state->updates / 60.0);
        return cna_game_request_exit(game);
    }
    if (state->demo->frames > 0 && state->updates >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    static uint8_t pcm[TONE_SAMPLES * 2U];
    CnaDemo demo;
    AudioState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_AudioCapabilities capabilities;
    CNA_SoundEffectCreateInfo sound_info;
    CNA_SoundEffectInstanceInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    int64_t duration_ticks = 0;

    cna_demo_init(&demo, "c_api.audio.sound_effect", argc, argv, 120);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.sound = CNA_INVALID_HANDLE;
    state.instance = CNA_INVALID_HANDLE;

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

    /* Ask whether sound can be played at all instead of assuming an audio device. */
    memset(&capabilities, 0, sizeof(capabilities));
    capabilities.struct_size = (uint32_t)sizeof(capabilities);
    capabilities.struct_version = UINT32_C(1);
    if (cna_audio_get_capabilities(game, &capabilities) != CNA_RESULT_SUCCESS ||
        capabilities.is_playback_available != CNA_TRUE) {
        (void)cna_game_destroy(game);
        return cna_demo_skip("no audio playback device");
    }

    /* The samples are copied during the call; the effect belongs to the game. */
    make_tone(pcm);
    memset(&sound_info, 0, sizeof(sound_info));
    sound_info.struct_size = (uint32_t)sizeof(sound_info);
    sound_info.struct_version = UINT32_C(1);
    sound_info.sample_rate = SAMPLE_RATE;
    sound_info.channels = CNA_AUDIO_CHANNELS_MONO;
    if (cna_demo_check_result(&demo, cna_sound_effect_create_pcm16(game, &sound_info, pcm, sizeof(pcm), &state.sound),
                              "cna_sound_effect_create_pcm16") &&
        cna_demo_check_result(&demo, cna_sound_effect_get_duration_ticks(state.sound, &duration_ticks),
                              "cna_sound_effect_get_duration_ticks")) {
        printf("duration %.3f s\n", (double)duration_ticks / 1e7);
        cna_demo_check(&demo, duration_ticks == INT64_C(5000000), "11025 samples at 22050 Hz last 0.5 s");

        /* An instance is one controllable playback of the effect. */
        if (cna_demo_check_result(&demo, cna_sound_effect_create_instance(state.sound, &state.instance),
                                  "cna_sound_effect_create_instance")) {
            cna_demo_check_result(&demo, cna_sound_effect_instance_set_volume(state.instance, 0.8f),
                                  "cna_sound_effect_instance_set_volume");
            cna_demo_check_result(&demo, cna_sound_effect_instance_play(state.instance),
                                  "cna_sound_effect_instance_play");
            memset(&info, 0, sizeof(info));
            info.struct_size = (uint32_t)sizeof(info);
            info.struct_version = UINT32_C(1);
            if (cna_demo_check_result(&demo, cna_sound_effect_instance_get_info(state.instance, &info),
                                      "cna_sound_effect_instance_get_info")) {
                cna_demo_check(&demo, info.state == CNA_SOUND_STATE_PLAYING && info.volume == 0.8f,
                               "the instance plays at the volume set");
            }

            cna_demo_check_result(&demo, cna_game_run(game), "cna_game_run");
            cna_demo_check(&demo, state.playing_seen, "the update callback sees it playing");
            cna_demo_check(&demo, state.stopped_at_update > 0, "it stops by itself at the end of the samples");
            /* Children first: the instance, then the effect, then the game. */
            cna_demo_check_result(&demo, cna_sound_effect_instance_destroy(state.instance),
                                  "cna_sound_effect_instance_destroy");
        }
        cna_demo_check_result(&demo, cna_sound_effect_destroy(state.sound), "cna_sound_effect_destroy");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
