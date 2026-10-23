/* SPDX-License-Identifier: MS-PL */
/* Caller-owned XACT Cue text, variables, positioning, transport and lifetime through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static void count_disposal(void* context)
{
    ++*(int*)context;
}

static int asset_path(CNA_Handle game, const char* suffix, char* destination, size_t capacity)
{
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    const size_t suffix_bytes = strlen(suffix);
    if (cna_title_location_get_path_size(game, &bytes) != CNA_RESULT_SUCCESS ||
        bytes + suffix_bytes + 1U > capacity ||
        cna_title_location_copy_path(game, destination, bytes, &copied) != CNA_RESULT_SUCCESS ||
        copied != bytes) {
        return 0;
    }
    memcpy(destination + bytes, suffix, suffix_bytes + 1U);
    return 1;
}

static void cue_info_init(CNA_CueInfo* info)
{
    memset(info, 0, sizeof(*info));
    info->struct_size = (uint32_t)sizeof(*info);
    info->struct_version = UINT32_C(1);
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GameCreateInfo game_info;
    CNA_AudioCapabilities capabilities;
    CNA_AudioListener listener;
    CNA_AudioEmitter emitter;
    CNA_CueInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle engine = CNA_INVALID_HANDLE;
    CNA_Handle tones = CNA_INVALID_HANDLE;
    CNA_Handle sounds = CNA_INVALID_HANDLE;
    CNA_Handle first = CNA_INVALID_HANDLE;
    CNA_Handle second = CNA_INVALID_HANDLE;
    CNA_Handle release_waves = CNA_INVALID_HANDLE;
    CNA_Handle release_sounds = CNA_INVALID_HANDLE;
    CNA_Handle release = CNA_INVALID_HANDLE;
    CNA_AudioEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    float value = -1.0F;
    float other = -1.0F;
    int disposals = 0;
    char tones_path[2048];
    char release_path[2048];
    char name[64];
    char before[64];

    cna_demo_init(&demo, "c_api.audio.xact_cue", argc, argv, 1);
    memset(&game_info, 0, sizeof(game_info));
    game_info.struct_size = (uint32_t)sizeof(game_info);
    game_info.struct_version = UINT32_C(1);
    game_info.is_fixed_time_step = CNA_TRUE;
    game_info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(&demo, cna_game_create(&game_info, &game),
                               "create the XACT Cue owner game")) {
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

    if (!cna_demo_check(&demo,
            asset_path(game, "/assets/xact/tones.xwb", tones_path, sizeof(tones_path)) &&
            asset_path(game, "/assets/xact/release.xwb", release_path, sizeof(release_path)),
            "resolve both WaveBank assets independently of the working directory")) {
        goto cleanup;
    }

    if (!cna_demo_check_result(
            &demo,
            cna_audio_engine_create(game, view("assets/xact/demo.xgs"), &engine),
            "open the generated XACT settings") ||
        !cna_demo_check_result(
            &demo,
            cna_wave_bank_create(engine, view(tones_path), &tones),
            "open the generated tone wave bank") ||
        !cna_demo_check_result(
            &demo,
            cna_sound_bank_create(engine, view("assets/xact/tones.xsb"), &sounds),
            "open the generated tone sound bank") ||
        !cna_demo_check_result(
            &demo, cna_sound_bank_get_cue(sounds, view("LowTone"), &first),
            "prepare the first caller-owned LowTone Cue") ||
        !cna_demo_check_result(
            &demo, cna_sound_bank_get_cue(sounds, view("LowTone"), &second),
            "prepare an independent second LowTone Cue")) {
        goto cleanup;
    }

    memset(name, 0xA5, sizeof(name));
    memset(&info, 0, sizeof(info));
    cna_demo_check(
        &demo,
        cna_cue_get_name_size(first, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof("LowTone") - 1U &&
            cna_cue_copy_name(first, name, bytes, &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(name, "LowTone", (size_t)bytes) == 0 &&
            (unsigned char)name[bytes] == 0xA5U,
        "the authored Cue name uses exact unterminated size and copy semantics");
    memcpy(before, name, sizeof(name));
    cna_demo_check(
        &demo,
        cna_cue_copy_name(first, name, bytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == bytes && memcmp(name, before, sizeof(name)) == 0,
        "a short Cue-name destination is rejected atomically");

    cna_demo_check(
        &demo,
        cna_cue_get_variable(first, view("Intensity"), &value) == CNA_RESULT_SUCCESS &&
            value == 50.0F &&
            cna_cue_get_variable(second, view("Intensity"), &other) == CNA_RESULT_SUCCESS &&
            other == 50.0F,
        "each prepared Cue starts with its own authored Intensity value");
    cna_demo_check(
        &demo,
        cna_cue_set_variable(first, view("Intensity"), 80.0F) == CNA_RESULT_SUCCESS &&
            cna_cue_get_variable(first, view("Intensity"), &value) == CNA_RESULT_SUCCESS &&
            value == 80.0F &&
            cna_cue_get_variable(second, view("Intensity"), &other) == CNA_RESULT_SUCCESS &&
            other == 50.0F,
        "setting one Cue variable leaves the same variable on another Cue unchanged");
    cna_demo_check(
        &demo,
        cna_cue_set_variable(second, view("Intensity"), 250.0F) == CNA_RESULT_SUCCESS &&
            cna_cue_get_variable(second, view("Intensity"), &other) == CNA_RESULT_SUCCESS &&
            other == 100.0F &&
            cna_cue_get_variable(first, view("SpeedOfSound"), &value) ==
                CNA_RESULT_INVALID_STATE,
        "Cue variable writes clamp to their authored range and reject an engine-global name");

    memset(&listener, 0, sizeof(listener));
    memset(&emitter, 0, sizeof(emitter));
    cna_demo_check(
        &demo,
        cna_audio_listener_init(&listener) == CNA_RESULT_SUCCESS &&
            cna_audio_emitter_init(&emitter) == CNA_RESULT_SUCCESS,
        "initialize the public listener and emitter values for Cue positioning");
    emitter.position.z = -5.0F;
    emitter.forward.z = 1.0F;
    cna_demo_check(
        &demo,
        cna_cue_apply_3d(first, &listener, &emitter) == CNA_RESULT_SUCCESS &&
            cna_cue_get_variable(first, view("Distance"), &value) == CNA_RESULT_SUCCESS &&
            value == 5.0F &&
            cna_cue_get_variable(first, view("OrientationAngle"), &value) == CNA_RESULT_SUCCESS &&
            value == 0.0F &&
            cna_cue_get_variable(first, view("DopplerPitchScalar"), &value) ==
                CNA_RESULT_SUCCESS &&
            value == 1.0F,
        "Apply3D publishes exact distance, facing angle and neutral Doppler values before Play");

    cna_demo_check(
        &demo,
        cna_cue_play(first) == CNA_RESULT_SUCCESS &&
            cna_audio_engine_update(engine) == CNA_RESULT_SUCCESS &&
            cna_cue_pause(first) == CNA_RESULT_SUCCESS &&
            cna_cue_get_info(first, &info) == CNA_RESULT_INVALID_ARGUMENT,
        "a Cue starts and pauses while a caller must initialize its snapshot descriptor");
    cue_info_init(&info);
    cna_demo_check(
        &demo,
        cna_cue_get_info(first, &info) == CNA_RESULT_SUCCESS &&
            info.is_playing == CNA_TRUE && info.is_paused == CNA_TRUE &&
            info.is_stopped == CNA_FALSE,
        "the paused Cue snapshot keeps Playing and Paused as independent predicates");
    emitter.position.z = -10.0F;
    cna_demo_check(
        &demo,
        cna_cue_apply_3d(first, &listener, &emitter) == CNA_RESULT_SUCCESS &&
            cna_cue_get_variable(first, view("Distance"), &value) == CNA_RESULT_SUCCESS &&
            value == 10.0F &&
            cna_cue_resume(first) == CNA_RESULT_SUCCESS &&
            cna_audio_engine_update(engine) == CNA_RESULT_SUCCESS,
        "a paused Cue accepts a new position and Resume continues it");
    cue_info_init(&info);
    cna_demo_check(
        &demo,
        cna_cue_get_info(first, &info) == CNA_RESULT_SUCCESS &&
            info.is_playing == CNA_TRUE && info.is_paused == CNA_FALSE &&
            cna_cue_stop(first, UINT32_C(99)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_cue_stop(first, CNA_AUDIO_STOP_OPTIONS_IMMEDIATE) == CNA_RESULT_SUCCESS &&
            cna_cue_get_info(first, &info) == CNA_RESULT_SUCCESS &&
            info.is_stopped == CNA_TRUE && info.is_playing == CNA_FALSE,
        "Resume clears Paused and Immediate Stop validates its option before stopping");

    cna_demo_check_result(
        &demo,
        cna_cue_subscribe_disposing_ext(first, count_disposal, &disposals, &registration),
        "subscribe to the caller-owned Cue disposal event");
    cna_demo_check(
        &demo,
        cna_cue_destroy(first) == CNA_RESULT_SUCCESS && disposals == 1 &&
            cna_cue_destroy(first) == CNA_RESULT_INVALID_HANDLE,
        "Cue destruction raises one event and invalidates its owned handle");
    first = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_audio_unsubscribe_ext(registration) == CNA_RESULT_SUCCESS &&
            cna_audio_unsubscribe_ext(registration) == CNA_RESULT_INVALID_HANDLE,
        "the detached Cue registration releases exactly once after its sender");
    registration = CNA_INVALID_HANDLE;
    cna_demo_check_result(&demo, cna_cue_destroy(second),
                          "release the independently configured second Cue");
    second = CNA_INVALID_HANDLE;

    if (!cna_demo_check_result(
            &demo,
            cna_wave_bank_create(engine, view(release_path), &release_waves),
            "open the authored-release wave bank") ||
        !cna_demo_check_result(
            &demo,
            cna_sound_bank_create(engine, view("assets/xact/release.xsb"), &release_sounds),
            "open the authored-release sound bank") ||
        !cna_demo_check_result(
            &demo,
            cna_sound_bank_get_cue(release_sounds, view("ReleaseTone"), &release),
            "prepare the authored-release Cue")) {
        goto cleanup;
    }
    cue_info_init(&info);
    cna_demo_check(
        &demo,
        cna_cue_play(release) == CNA_RESULT_SUCCESS &&
            cna_audio_engine_update(engine) == CNA_RESULT_SUCCESS &&
            cna_cue_stop(release, CNA_AUDIO_STOP_OPTIONS_AS_AUTHORED) == CNA_RESULT_SUCCESS &&
            cna_cue_get_info(release, &info) == CNA_RESULT_SUCCESS &&
            info.is_stopping == CNA_TRUE && info.is_playing == CNA_FALSE &&
            info.is_stopped == CNA_FALSE,
        "AsAuthored Stop enters the generated Cue's live release phase");
    cna_demo_check(
        &demo,
        cna_cue_stop(release, CNA_AUDIO_STOP_OPTIONS_IMMEDIATE) == CNA_RESULT_SUCCESS &&
            cna_cue_get_info(release, &info) == CNA_RESULT_SUCCESS &&
            info.is_stopping == CNA_FALSE && info.is_stopped == CNA_TRUE,
        "Immediate Stop interrupts the same authored release without waiting");

cleanup:
    if (release != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_cue_destroy(release),
                              "release the authored-release Cue");
    }
    if (release_sounds != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_sound_bank_destroy(release_sounds),
                              "destroy the authored-release sound bank");
    }
    if (release_waves != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_wave_bank_destroy(release_waves),
                              "destroy the authored-release wave bank");
    }
    if (first != CNA_INVALID_HANDLE) {
        (void)cna_cue_destroy(first);
    }
    if (second != CNA_INVALID_HANDLE) {
        (void)cna_cue_destroy(second);
    }
    if (registration != CNA_INVALID_HANDLE) {
        (void)cna_audio_unsubscribe_ext(registration);
    }
    if (sounds != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_sound_bank_destroy(sounds),
                              "destroy the tone sound bank");
    }
    if (tones != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_wave_bank_destroy(tones),
                              "destroy the tone wave bank");
    }
    if (engine != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_audio_engine_destroy(engine),
                              "destroy the Cue engine after its children");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the XACT Cue owner game");
    return cna_demo_finish(&demo);
}
