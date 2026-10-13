/* SPDX-License-Identifier: MS-PL */
/* SoundEffectInstance mixing properties and playback transitions from the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define SAMPLE_RATE UINT32_C(8000)
#define PCM_BYTES 1600U

typedef struct InstanceState {
    CnaDemo* demo;
    CNA_Handle instance;
    int updates;
    int stopped;
} InstanceState;

static CNA_Result read_info(CNA_Handle instance, CNA_SoundEffectInstanceInfo* out_info)
{
    memset(out_info, 0xA5, sizeof(*out_info));
    out_info->struct_size = (uint32_t)sizeof(*out_info);
    out_info->struct_version = UINT32_C(1);
    return cna_sound_effect_instance_get_info(instance, out_info);
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    InstanceState* state = (InstanceState*)context;
    CNA_SoundEffectInstanceInfo info;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->updates;
    result = read_info(state->instance, &info);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    if (info.state == CNA_SOUND_STATE_STOPPED) {
        state->stopped = 1;
        return cna_game_request_exit(game);
    }
    if (state->demo->frames > 0 && state->updates >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    static uint8_t pcm[PCM_BYTES];
    CnaDemo demo;
    InstanceState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo game_info;
    CNA_AudioCapabilities capabilities;
    CNA_SoundEffectCreateInfo sound_info;
    CNA_SoundEffectInstanceInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle sound = CNA_INVALID_HANDLE;
    CNA_Bool disposed = UINT8_C(77);

    cna_demo_init(&demo, "c_api.audio.sound_effect_instance_state", argc, argv, 120);
    memset(pcm, 0, sizeof(pcm));
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.instance = CNA_INVALID_HANDLE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
    callbacks.context = &state;
    memset(&game_info, 0, sizeof(game_info));
    game_info.struct_size = (uint32_t)sizeof(game_info);
    game_info.struct_version = UINT32_C(1);
    game_info.is_fixed_time_step = CNA_TRUE;
    game_info.target_elapsed_time_ticks = INT64_C(166667);
    game_info.callbacks = &callbacks;
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

    memset(&sound_info, 0, sizeof(sound_info));
    sound_info.struct_size = (uint32_t)sizeof(sound_info);
    sound_info.struct_version = UINT32_C(1);
    sound_info.sample_rate = SAMPLE_RATE;
    sound_info.channels = CNA_AUDIO_CHANNELS_MONO;
    cna_demo_check_result(
        &demo,
        cna_sound_effect_create_pcm16(game, &sound_info, pcm, sizeof(pcm), &sound),
        "create a 100 ms mono SoundEffect");
    cna_demo_check_result(
        &demo, cna_sound_effect_create_instance(sound, &state.instance),
        "create one controllable SoundEffectInstance");

    cna_demo_check(
        &demo,
        read_info(state.instance, &info) == CNA_RESULT_SUCCESS &&
            info.struct_size == sizeof(info) && info.struct_version == UINT32_C(1) &&
            info.state == CNA_SOUND_STATE_STOPPED && info.is_looped == CNA_FALSE &&
            info.volume == 1.0f && info.pitch == 0.0f && info.pan == 0.0f &&
            info.reserved0[0] == 0U && info.reserved0[1] == 0U &&
            info.reserved0[2] == 0U && info.reserved1 == 0U,
        "a fresh instance reports canonical stopped defaults and zero reserved output");

    cna_demo_check(
        &demo,
        cna_sound_effect_instance_set_volume(state.instance, 0.4f) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_instance_set_pitch(state.instance, 2.0f) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_sound_effect_instance_set_pitch(state.instance, 1.0f) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_instance_set_pan(state.instance, -0.25f) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_instance_set_is_looped(state.instance, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            read_info(state.instance, &info) == CNA_RESULT_SUCCESS &&
            info.volume == 0.4f && info.pitch == 1.0f && info.pan == -0.25f &&
            info.is_looped == CNA_TRUE,
        "mixing properties round-trip while pitch is range-checked before Play");
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_set_volume(state.instance, NAN) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_sound_effect_instance_set_pitch(state.instance, NAN) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_sound_effect_instance_set_pan(state.instance, 1.01f) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            read_info(state.instance, &info) == CNA_RESULT_SUCCESS &&
            info.volume == 0.4f && info.pitch == 1.0f && info.pan == -0.25f &&
            info.is_looped == CNA_TRUE,
        "invalid mixing values preserve every configured property");
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_get_is_disposed(state.instance, &disposed) ==
                CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE,
        "the owned instance reports that it is live before playback");

    cna_demo_check(
        &demo,
        cna_sound_effect_instance_play(state.instance) == CNA_RESULT_SUCCESS &&
            read_info(state.instance, &info) == CNA_RESULT_SUCCESS &&
            info.state == CNA_SOUND_STATE_PLAYING,
        "Play enters the playing state");
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_pause(state.instance) == CNA_RESULT_SUCCESS &&
            read_info(state.instance, &info) == CNA_RESULT_SUCCESS &&
            info.state == CNA_SOUND_STATE_PAUSED,
        "Pause preserves the voice in the paused state");
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_resume(state.instance) == CNA_RESULT_SUCCESS &&
            read_info(state.instance, &info) == CNA_RESULT_SUCCESS &&
            info.state == CNA_SOUND_STATE_PLAYING,
        "Resume returns the paused voice to playing");
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_set_is_looped(state.instance, CNA_FALSE) ==
                CNA_RESULT_INVALID_STATE &&
            read_info(state.instance, &info) == CNA_RESULT_SUCCESS &&
            info.is_looped == CNA_TRUE,
        "looping cannot be changed after playback begins");
    cna_demo_check_result(
        &demo, cna_sound_effect_instance_stop(state.instance, CNA_FALSE),
        "request the authored release instead of cutting off the loop");
    cna_demo_check_result(
        &demo, cna_game_run(game), "run until the released voice reaches Stopped");
    cna_demo_check(
        &demo, state.stopped && state.updates > 0 &&
            read_info(state.instance, &info) == CNA_RESULT_SUCCESS &&
            info.state == CNA_SOUND_STATE_STOPPED,
        "the released looping voice stops within the bounded game run");

    cna_demo_check(
        &demo,
        cna_sound_effect_instance_play(state.instance) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_instance_stop(state.instance, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            read_info(state.instance, &info) == CNA_RESULT_SUCCESS &&
            info.state == CNA_SOUND_STATE_STOPPED,
        "replay followed by immediate Stop cuts the voice off synchronously");

    cna_demo_check_result(
        &demo, cna_sound_effect_instance_destroy(state.instance),
        "destroy the controlled instance");
    cna_demo_check(
        &demo,
        read_info(state.instance, &info) == CNA_RESULT_INVALID_HANDLE,
        "the released instance handle refuses another state snapshot");
    cna_demo_check_result(&demo, cna_sound_effect_destroy(sound), "destroy the source effect");
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
