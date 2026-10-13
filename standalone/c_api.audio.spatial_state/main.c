/* SPDX-License-Identifier: MS-PL */
/* Spatial-audio POD values, process scales and Apply3D ordering from the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int vector_is(CNA_Vector3 value, float x, float y, float z)
{
    return value.x == x && value.y == y && value.z == z;
}

int main(int argc, char** argv)
{
    static const uint8_t Silence[1600] = {0U};
    CnaDemo demo;
    CNA_AudioEmitter emitter;
    CNA_AudioListener listener;
    CNA_AudioListener listeners[2];
    CNA_AudioCapabilities capabilities;
    CNA_GameCreateInfo game_info;
    CNA_SoundEffectCreateInfo sound_info;
    CNA_SoundEffectInstanceInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle sound = CNA_INVALID_HANDLE;
    CNA_Handle positioned = CNA_INVALID_HANDLE;
    CNA_Handle unaimed = CNA_INVALID_HANDLE;
    float original_distance = 0.0f;
    float original_doppler = 0.0f;
    float original_speed = 0.0f;
    float value = 0.0f;

    cna_demo_init(&demo, "c_api.audio.spatial_state", argc, argv, 1);
    memset(&emitter, 0xA5, sizeof(emitter));
    memset(&listener, 0xA5, sizeof(listener));
    cna_demo_check(
        &demo,
        cna_audio_emitter_init(&emitter) == CNA_RESULT_SUCCESS &&
            emitter.struct_size == sizeof(emitter) &&
            emitter.struct_version == UINT32_C(1) && emitter.doppler_scale == 1.0f &&
            vector_is(emitter.forward, 0.0f, 0.0f, -1.0f) &&
            vector_is(emitter.position, 0.0f, 0.0f, 0.0f) &&
            vector_is(emitter.up, 0.0f, 1.0f, 0.0f) &&
            vector_is(emitter.velocity, 0.0f, 0.0f, 0.0f),
        "the emitter initializer writes canonical orientation, position and motion");
    cna_demo_check(
        &demo,
        cna_audio_listener_init(&listener) == CNA_RESULT_SUCCESS &&
            listener.struct_size == sizeof(listener) &&
            listener.struct_version == UINT32_C(1) &&
            vector_is(listener.forward, 0.0f, 0.0f, -1.0f) &&
            vector_is(listener.position, 0.0f, 0.0f, 0.0f) &&
            vector_is(listener.up, 0.0f, 1.0f, 0.0f) &&
            vector_is(listener.velocity, 0.0f, 0.0f, 0.0f),
        "the listener initializer writes canonical orientation, position and motion");

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
        cna_sound_effect_get_distance_scale(game, &original_distance) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_get_doppler_scale(game, &original_doppler) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_get_speed_of_sound(game, &original_speed) == CNA_RESULT_SUCCESS &&
            original_distance > 0.0f && original_doppler >= 0.0f && original_speed > 0.0f,
        "read the three process spatial settings before changing them");
    cna_demo_check(
        &demo,
        cna_sound_effect_set_distance_scale(game, 4.0f) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_set_doppler_scale(game, 2.0f) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_set_speed_of_sound(game, 300.0f) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_get_distance_scale(game, &value) == CNA_RESULT_SUCCESS &&
            value == 4.0f,
        "set distinguishable process distance, Doppler and sound-speed values");
    cna_demo_check(
        &demo,
        cna_sound_effect_get_doppler_scale(game, &value) == CNA_RESULT_SUCCESS &&
            value == 2.0f &&
            cna_sound_effect_get_speed_of_sound(game, &value) == CNA_RESULT_SUCCESS &&
            value == 300.0f,
        "the authored Doppler scale and speed of sound round-trip exactly");

    memset(&sound_info, 0, sizeof(sound_info));
    sound_info.struct_size = (uint32_t)sizeof(sound_info);
    sound_info.struct_version = UINT32_C(1);
    sound_info.sample_rate = UINT32_C(8000);
    sound_info.channels = CNA_AUDIO_CHANNELS_MONO;
    cna_demo_check(
        &demo,
        cna_sound_effect_create_pcm16(
            game, &sound_info, Silence, sizeof(Silence), &sound) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_create_instance(sound, &positioned) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_create_instance(sound, &unaimed) == CNA_RESULT_SUCCESS,
        "create positioned and deliberately unaimed instances from one effect");

    emitter.position.x = 40.0f;
    emitter.velocity.x = -60.0f;
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_apply_3d(positioned, &listener, &emitter) ==
                CNA_RESULT_SUCCESS &&
            cna_sound_effect_instance_play(positioned) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_instance_apply_3d(positioned, &listener, &emitter) ==
                CNA_RESULT_SUCCESS,
        "an instance positioned before Play accepts spatial updates while playing");
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_set_pan(positioned, 0.5f) == CNA_RESULT_INVALID_STATE,
        "a playing 3D instance refuses direct pan that would conflict with positioning");
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_get_info(positioned, &info) == CNA_RESULT_SUCCESS &&
            info.pan == 0.0f,
        "refused direct pan leaves the instance's public pan unchanged");
    cna_demo_check_result(
        &demo, cna_sound_effect_instance_stop(positioned, CNA_TRUE),
        "stop the positioned instance immediately");

    cna_demo_check(
        &demo,
        cna_sound_effect_instance_play(unaimed) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_instance_apply_3d(unaimed, &listener, &emitter) ==
                CNA_RESULT_INVALID_STATE &&
            cna_sound_effect_instance_apply_3d_multi_ext(
                unaimed, &listener, 1U, &emitter) == CNA_RESULT_INVALID_STATE,
        "an instance first played in 2D refuses both spatial routes until it stops");
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_stop(unaimed, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_instance_apply_3d(unaimed, &listener, &emitter) ==
                CNA_RESULT_SUCCESS,
        "stopping the unaimed voice permits a later spatial choice");

    cna_demo_check(
        &demo,
        cna_audio_listener_init(&listeners[0]) == CNA_RESULT_SUCCESS &&
            cna_audio_listener_init(&listeners[1]) == CNA_RESULT_SUCCESS,
        "initialize both listeners for the multi-listener route");
    listeners[0].position.x = 1000.0f;
    listeners[1].position = emitter.position;
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_apply_3d_multi_ext(
            unaimed, listeners, 2U, &emitter) == CNA_RESULT_SUCCESS,
        "the multi-listener route accepts a second listener closer to the emitter");
    cna_demo_check(
        &demo,
        cna_sound_effect_instance_apply_3d_multi_ext(
            unaimed, listeners, 0U, &emitter) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_sound_effect_instance_apply_3d_multi_ext(
                unaimed, NULL, 1U, &emitter) == CNA_RESULT_INVALID_ARGUMENT,
        "the multi-listener route refuses empty and missing listener arrays");

    cna_demo_check(
        &demo,
        cna_sound_effect_set_distance_scale(game, original_distance) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_set_doppler_scale(game, original_doppler) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_set_speed_of_sound(game, original_speed) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_get_distance_scale(game, &value) == CNA_RESULT_SUCCESS &&
            value == original_distance,
        "restore all three process spatial settings");
    cna_demo_check(
        &demo,
        cna_sound_effect_get_doppler_scale(game, &value) == CNA_RESULT_SUCCESS &&
            value == original_doppler &&
            cna_sound_effect_get_speed_of_sound(game, &value) == CNA_RESULT_SUCCESS &&
            value == original_speed,
        "the restored Doppler scale and speed of sound round-trip");

    cna_demo_check_result(
        &demo, cna_sound_effect_instance_destroy(unaimed),
        "destroy the later-positioned instance");
    cna_demo_check_result(
        &demo, cna_sound_effect_instance_destroy(positioned),
        "destroy the initially positioned instance");
    cna_demo_check_result(&demo, cna_sound_effect_destroy(sound), "destroy the source effect");
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
