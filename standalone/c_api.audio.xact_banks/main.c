/* SPDX-License-Identifier: MS-PL */
/* XACT wave/sound banks, categories and parent-child lifetime through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
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
    CNA_CueInfo music_info;
    CNA_CueInfo sfx_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle engine = CNA_INVALID_HANDLE;
    CNA_Handle resident = CNA_INVALID_HANDLE;
    CNA_Handle streaming = CNA_INVALID_HANDLE;
    CNA_Handle sound_bank = CNA_INVALID_HANDLE;
    CNA_Handle music = CNA_INVALID_HANDLE;
    CNA_Handle music_again = CNA_INVALID_HANDLE;
    CNA_Handle sfx = CNA_INVALID_HANDLE;
    CNA_Handle music_cue = CNA_INVALID_HANDLE;
    CNA_Handle sfx_cue = CNA_INVALID_HANDLE;
    CNA_Handle rejected = UINT64_C(99);
    CNA_AudioEventRegistrationHandle wave_registration = CNA_INVALID_HANDLE;
    CNA_AudioEventRegistrationHandle sound_registration = CNA_INVALID_HANDLE;
    CNA_Bool flag = UINT8_C(77);
    CNA_Bool equal = CNA_FALSE;
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    int32_t hash = 0;
    int32_t same_hash = 1;
    int wave_disposals = 0;
    int sound_disposals = 0;
    char streaming_path[2048];
    char resident_path[2048];
    char invalid_bank_path[2048];
    char name[64];
    char before[64];

    cna_demo_init(&demo, "c_api.audio.xact_banks", argc, argv, 1);
    memset(&game_info, 0, sizeof(game_info));
    game_info.struct_size = (uint32_t)sizeof(game_info);
    game_info.struct_version = UINT32_C(1);
    game_info.is_fixed_time_step = CNA_TRUE;
    game_info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(&demo, cna_game_create(&game_info, &game),
                               "create the XACT bank owner game")) {
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
    if (!cna_demo_check_result(
            &demo,
            cna_audio_engine_create(game, view("assets/xact/demo.xgs"), &engine),
            "open the generated XACT settings")) {
        (void)cna_game_destroy(game);
        return cna_demo_finish(&demo);
    }

    if (!cna_demo_check(&demo,
            asset_path(game, "/assets/xact/tones.xwb", resident_path, sizeof(resident_path)) &&
            asset_path(game, "/assets/xact/demo.xgs", invalid_bank_path, sizeof(invalid_bank_path)),
            "resolve the resident WaveBank asset independently of the working directory")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_wave_bank_create(engine, view("assets/xact/definitely-missing.xwb"), &rejected) ==
                CNA_RESULT_IO &&
            rejected == CNA_INVALID_HANDLE,
        "a missing resident bank is an atomic I/O failure");
    rejected = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_wave_bank_create_streaming(
            engine, view("assets/xact/definitely-missing.xwb"), 0, 16, &rejected) ==
                CNA_RESULT_IO && rejected == CNA_INVALID_HANDLE,
        "a missing streaming bank is an atomic I/O failure");
    if (rejected != CNA_INVALID_HANDLE) (void)cna_wave_bank_destroy(rejected);
    rejected = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_wave_bank_create(engine, view(invalid_bank_path), &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT && rejected == CNA_INVALID_HANDLE,
        "a non-XWB resident file is refused before publishing a handle");
    if (rejected != CNA_INVALID_HANDLE) (void)cna_wave_bank_destroy(rejected);
    rejected = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_wave_bank_create_streaming(engine, view(invalid_bank_path), 0, 16, &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT && rejected == CNA_INVALID_HANDLE,
        "a non-XWB streaming file is refused before publishing a handle");
    if (rejected != CNA_INVALID_HANDLE) (void)cna_wave_bank_destroy(rejected);
    if (!cna_demo_check_result(
            &demo,
            cna_wave_bank_create(engine, view(resident_path), &resident),
            "open the generated resident tone bank")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_wave_bank_get_is_disposed(resident, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_wave_bank_get_is_prepared(resident, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_wave_bank_get_is_in_use(resident, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "the resident wave bank is live, prepared and initially idle");
    cna_demo_check_result(
        &demo,
        cna_wave_bank_subscribe_disposing_ext(
            resident, count_disposal, &wave_disposals, &wave_registration),
        "subscribe to resident-bank disposal");

    cna_demo_check(
        &demo,
        asset_path(game, "/assets/xact/music.xwb", streaming_path, sizeof(streaming_path)) &&
            cna_wave_bank_create_streaming(
                engine, view(streaming_path), 0, 16, &streaming) == CNA_RESULT_SUCCESS &&
            cna_wave_bank_get_is_prepared(streaming, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_wave_bank_get_is_in_use(streaming, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_wave_bank_destroy(streaming) == CNA_RESULT_SUCCESS,
        "the authored streaming bank resolves by absolute path and is prepared but idle");
    streaming = CNA_INVALID_HANDLE;

    cna_demo_check(
        &demo,
        cna_sound_bank_create(engine, view("assets/xact/definitely-missing.xsb"), &rejected) ==
                CNA_RESULT_IO &&
            rejected == CNA_INVALID_HANDLE,
        "a missing sound bank is an atomic I/O failure");
    if (!cna_demo_check_result(
            &demo,
            cna_sound_bank_create(engine, view("assets/xact/tones.xsb"), &sound_bank),
            "open the generated sound bank")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_sound_bank_get_is_disposed(sound_bank, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_sound_bank_get_is_in_use(sound_bank, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "the sound bank is live and initially idle");
    cna_demo_check_result(
        &demo,
        cna_sound_bank_subscribe_disposing_ext(
            sound_bank, count_disposal, &sound_disposals, &sound_registration),
        "subscribe to sound-bank disposal");

    cna_demo_check(
        &demo,
        cna_audio_engine_get_category(engine, view("Music"), &music) == CNA_RESULT_SUCCESS &&
            cna_audio_engine_get_category(engine, view("Music"), &music_again) ==
                CNA_RESULT_SUCCESS &&
            cna_audio_engine_get_category(engine, view("SFX"), &sfx) == CNA_RESULT_SUCCESS &&
            cna_audio_engine_get_category(engine, view("Speech"), &rejected) ==
                CNA_RESULT_INVALID_STATE &&
            rejected == CNA_INVALID_HANDLE,
        "category lookup returns independent owned values and refuses an unknown name");
    memset(name, 0xA5, sizeof(name));
    cna_demo_check(
        &demo,
        cna_audio_category_get_name_size(music, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof("Music") - 1U &&
            cna_audio_category_copy_name(music, name, bytes, &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(name, "Music", (size_t)bytes) == 0 &&
            (unsigned char)name[bytes] == 0xA5U,
        "a category name uses exact unterminated size and copy semantics");
    memcpy(before, name, sizeof(name));
    cna_demo_check(
        &demo,
        cna_audio_category_copy_name(music, name, bytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == bytes && memcmp(name, before, sizeof(name)) == 0,
        "a short category-name destination is rejected atomically");
    cna_demo_check(
        &demo,
        cna_audio_category_equals(music, music_again, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE &&
            cna_audio_category_get_hash_code(music, &hash) == CNA_RESULT_SUCCESS &&
            cna_audio_category_get_hash_code(music_again, &same_hash) == CNA_RESULT_SUCCESS &&
            hash == same_hash &&
            cna_audio_category_equals(music, sfx, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "same-name category values compare equal with one hash while Music and SFX differ");

    cna_demo_check(
        &demo,
        cna_sound_bank_get_cue(sound_bank, view("LowTone"), &music_cue) == CNA_RESULT_SUCCESS &&
            cna_sound_bank_get_cue(sound_bank, view("HighTone"), &sfx_cue) == CNA_RESULT_SUCCESS &&
            cna_sound_bank_get_cue(sound_bank, view("NoSuchCue"), &rejected) ==
                CNA_RESULT_INVALID_STATE &&
            rejected == CNA_INVALID_HANDLE,
        "the sound bank prepares independent caller-owned Music and SFX cues by name");
    cue_info_init(&music_info);
    cue_info_init(&sfx_info);
    cna_demo_check(
        &demo,
        cna_cue_get_info(music_cue, &music_info) == CNA_RESULT_SUCCESS &&
            music_info.is_prepared == CNA_TRUE && music_info.is_playing == CNA_FALSE &&
            cna_cue_get_info(sfx_cue, &sfx_info) == CNA_RESULT_SUCCESS &&
            sfx_info.is_prepared == CNA_TRUE && sfx_info.is_playing == CNA_FALSE,
        "both newly prepared cues expose the same coherent prepared state");
    cna_demo_check(
        &demo,
        cna_cue_play(music_cue) == CNA_RESULT_SUCCESS &&
            cna_cue_play(sfx_cue) == CNA_RESULT_SUCCESS &&
            cna_audio_engine_update(engine) == CNA_RESULT_SUCCESS &&
            cna_sound_bank_get_is_in_use(sound_bank, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_wave_bank_get_is_in_use(resident, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "playing both cues puts their sound and resident wave banks in use");
    cna_demo_check(
        &demo,
        cna_audio_category_set_volume(music, 0.25F) == CNA_RESULT_SUCCESS &&
            cna_audio_category_set_volume(music, NAN) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_audio_category_set_volume(music, 1.0F) == CNA_RESULT_SUCCESS,
        "category volume accepts finite gain, rejects NaN and is restored");
    cna_demo_check(
        &demo,
        cna_audio_category_pause(sfx) == CNA_RESULT_SUCCESS &&
            cna_cue_get_info(sfx_cue, &sfx_info) == CNA_RESULT_SUCCESS &&
            sfx_info.is_paused == CNA_TRUE && sfx_info.is_playing == CNA_TRUE &&
            cna_cue_get_info(music_cue, &music_info) == CNA_RESULT_SUCCESS &&
            music_info.is_paused == CNA_FALSE && music_info.is_playing == CNA_TRUE &&
            cna_wave_bank_get_is_in_use(resident, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "pausing SFX leaves Music playing and a paused consumer keeps the wave bank in use");
    cna_demo_check(
        &demo,
        cna_audio_category_resume(sfx) == CNA_RESULT_SUCCESS &&
            cna_cue_get_info(sfx_cue, &sfx_info) == CNA_RESULT_SUCCESS &&
            sfx_info.is_paused == CNA_FALSE && sfx_info.is_playing == CNA_TRUE,
        "resuming SFX returns only its paused cue to active playback");
    cna_demo_check(
        &demo,
        cna_audio_category_stop(music, CNA_AUDIO_STOP_OPTIONS_IMMEDIATE) == CNA_RESULT_SUCCESS &&
            cna_cue_get_info(music_cue, &music_info) == CNA_RESULT_SUCCESS &&
            music_info.is_stopped == CNA_TRUE &&
            cna_cue_get_info(sfx_cue, &sfx_info) == CNA_RESULT_SUCCESS &&
            sfx_info.is_playing == CNA_TRUE &&
            cna_audio_category_stop(sfx, UINT32_C(99)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_audio_category_stop(sfx, CNA_AUDIO_STOP_OPTIONS_IMMEDIATE) ==
                CNA_RESULT_SUCCESS &&
            cna_audio_engine_update(engine) == CNA_RESULT_SUCCESS &&
            cna_sound_bank_get_is_in_use(sound_bank, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_wave_bank_get_is_in_use(resident, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "category Stop is selective, validates its option and makes both banks idle at the end");

    cna_demo_check(
        &demo,
        cna_sound_bank_play_cue(sound_bank, view("Beep")) == CNA_RESULT_SUCCESS &&
            cna_audio_engine_update(engine) == CNA_RESULT_SUCCESS &&
            cna_sound_bank_get_is_in_use(sound_bank, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_audio_category_stop(sfx, CNA_AUDIO_STOP_OPTIONS_IMMEDIATE) ==
                CNA_RESULT_SUCCESS &&
            cna_audio_engine_update(engine) == CNA_RESULT_SUCCESS &&
            cna_sound_bank_get_is_in_use(sound_bank, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "fire-and-forget playback is bank-owned and category Stop retires it through Update");
    memset(&listener, 0, sizeof(listener));
    memset(&emitter, 0, sizeof(emitter));
    cna_demo_check(
        &demo,
        cna_audio_listener_init(&listener) == CNA_RESULT_SUCCESS &&
            cna_audio_emitter_init(&emitter) == CNA_RESULT_SUCCESS &&
            (emitter.position.x = 3.0F) == 3.0F &&
            cna_sound_bank_play_cue_3d(
                sound_bank, view("Beep"), &listener, &emitter) == CNA_RESULT_SUCCESS &&
            cna_audio_engine_update(engine) == CNA_RESULT_SUCCESS &&
            cna_sound_bank_get_is_in_use(sound_bank, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_audio_category_stop(sfx, CNA_AUDIO_STOP_OPTIONS_IMMEDIATE) ==
                CNA_RESULT_SUCCESS &&
            cna_audio_engine_update(engine) == CNA_RESULT_SUCCESS,
        "the bank positions a fire-and-forget cue from validated listener/emitter values");

    cna_demo_check(
        &demo,
        cna_audio_engine_destroy(engine) == CNA_RESULT_INVALID_STATE &&
            cna_sound_bank_destroy(sound_bank) == CNA_RESULT_INVALID_STATE,
        "engine and sound-bank parents refuse destruction while C child handles remain");
    cna_demo_check(
        &demo,
        cna_cue_destroy(music_cue) == CNA_RESULT_SUCCESS &&
            cna_cue_destroy(sfx_cue) == CNA_RESULT_SUCCESS,
        "both stopped caller-owned Cue handles release before their bank");
    music_cue = CNA_INVALID_HANDLE;
    sfx_cue = CNA_INVALID_HANDLE;

cleanup:
    if (music_cue != CNA_INVALID_HANDLE) {
        (void)cna_cue_destroy(music_cue);
    }
    if (sfx_cue != CNA_INVALID_HANDLE) {
        (void)cna_cue_destroy(sfx_cue);
    }
    if (music != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_audio_category_destroy(music),
                              "release the first Music category value");
    }
    if (music_again != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_audio_category_destroy(music_again),
                              "release the equal Music category value independently");
    }
    if (sfx != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo,
            cna_audio_category_destroy(sfx) == CNA_RESULT_SUCCESS &&
                cna_audio_category_destroy(sfx) == CNA_RESULT_INVALID_HANDLE,
            "the final category handle releases exactly once");
    }
    if (sound_bank != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo,
            cna_sound_bank_destroy(sound_bank) == CNA_RESULT_SUCCESS && sound_disposals == 1,
            "sound-bank destruction raises its subscribed event exactly once");
        sound_bank = CNA_INVALID_HANDLE;
    }
    if (sound_registration != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_audio_unsubscribe_ext(sound_registration),
                              "release the detached sound-bank registration");
    }
    if (streaming != CNA_INVALID_HANDLE) {
        (void)cna_wave_bank_destroy(streaming);
    }
    if (resident != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo,
            cna_wave_bank_destroy(resident) == CNA_RESULT_SUCCESS && wave_disposals == 1,
            "resident-bank destruction raises its subscribed event exactly once");
        resident = CNA_INVALID_HANDLE;
    }
    if (wave_registration != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_audio_unsubscribe_ext(wave_registration),
                              "release the detached wave-bank registration");
    }
    if (engine != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_audio_engine_destroy(engine),
                              "destroy the engine after every C child");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the XACT bank owner game");
    return cna_demo_finish(&demo);
}
