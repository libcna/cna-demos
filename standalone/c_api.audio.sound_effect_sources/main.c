/* SPDX-License-Identifier: MS-PL */
/* SoundEffect creation sources, owned state and one-shot playback from the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define SAMPLE_RATE UINT32_C(8000)
#define PCM_BYTES 1600U

static const char WavPath[] = "/tmp/cna-demos-sound-effect-sources.wav";

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static int bytes_are(const void* data, size_t count, uint8_t expected)
{
    const uint8_t* bytes = (const uint8_t*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != expected) {
            return 0;
        }
    }
    return 1;
}

static void put_u16_le(uint8_t* destination, uint16_t value)
{
    destination[0] = (uint8_t)(value & UINT16_C(0xFF));
    destination[1] = (uint8_t)(value >> 8U);
}

static void put_u32_le(uint8_t* destination, uint32_t value)
{
    destination[0] = (uint8_t)(value & UINT32_C(0xFF));
    destination[1] = (uint8_t)((value >> 8U) & UINT32_C(0xFF));
    destination[2] = (uint8_t)((value >> 16U) & UINT32_C(0xFF));
    destination[3] = (uint8_t)((value >> 24U) & UINT32_C(0xFF));
}

static void make_wav(uint8_t* wav, const uint8_t* pcm)
{
    memcpy(wav, "RIFF", 4U);
    put_u32_le(wav + 4U, 36U + PCM_BYTES);
    memcpy(wav + 8U, "WAVEfmt ", 8U);
    put_u32_le(wav + 16U, 16U);
    put_u16_le(wav + 20U, 1U);
    put_u16_le(wav + 22U, 1U);
    put_u32_le(wav + 24U, SAMPLE_RATE);
    put_u32_le(wav + 28U, SAMPLE_RATE * 2U);
    put_u16_le(wav + 32U, 2U);
    put_u16_le(wav + 34U, 16U);
    memcpy(wav + 36U, "data", 4U);
    put_u32_le(wav + 40U, PCM_BYTES);
    memcpy(wav + 44U, pcm, PCM_BYTES);
}

static int write_file(const char* path, const uint8_t* bytes, size_t count)
{
    FILE* file = fopen(path, "wb");
    size_t written;
    int close_result;
    int success;
    if (file == NULL) {
        return 0;
    }
    written = fwrite(bytes, 1U, count, file);
    close_result = fclose(file);
    success = written == count && close_result == 0;
    if (!success) {
        (void)remove(path);
    }
    return success;
}

int main(int argc, char** argv)
{
    uint8_t pcm[PCM_BYTES];
    uint8_t wav[44U + PCM_BYTES];
    CnaDemo demo;
    CNA_GameCreateInfo game_info;
    CNA_AudioCapabilities capabilities;
    CNA_SoundEffectCreateInfo sound_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle ranged = CNA_INVALID_HANDLE;
    CNA_Handle encoded = CNA_INVALID_HANDLE;
    CNA_Handle asset = CNA_INVALID_HANDLE;
    int64_t ticks = -1;
    int32_t sample_bytes = -1;
    float original_master_volume = -1.0f;
    float master_volume = -1.0f;
    CNA_Bool disposed = UINT8_C(77);
    CNA_Bool played = CNA_FALSE;
    uint64_t count = UINT64_MAX;
    char name[16];
    char short_name[4];
    int file_ready;

    cna_demo_init(&demo, "c_api.audio.sound_effect_sources", argc, argv, 1);
    memset(pcm, 0, sizeof(pcm));
    make_wav(wav, pcm);
    file_ready = write_file(WavPath, wav, sizeof(wav));
    cna_demo_check(&demo, file_ready, "write the generated 100 ms mono WAV to a private file");
    if (!file_ready) {
        return cna_demo_finish(&demo);
    }

    cna_demo_check(
        &demo,
        cna_sound_effect_get_sample_duration_ticks(
            (int32_t)PCM_BYTES, (int32_t)SAMPLE_RATE, CNA_AUDIO_CHANNELS_MONO, &ticks) ==
                CNA_RESULT_SUCCESS &&
            ticks == INT64_C(1000000) &&
            cna_sound_effect_get_sample_size_in_bytes(
                ticks, (int32_t)SAMPLE_RATE, CNA_AUDIO_CHANNELS_MONO, &sample_bytes) ==
                CNA_RESULT_SUCCESS &&
            sample_bytes == (int32_t)PCM_BYTES,
        "the static PCM helpers convert 1600 mono bytes to 100 ms and back exactly");

    memset(&game_info, 0, sizeof(game_info));
    game_info.struct_size = (uint32_t)sizeof(game_info);
    game_info.struct_version = UINT32_C(1);
    game_info.is_fixed_time_step = CNA_FALSE;
    game_info.target_elapsed_time_ticks = INT64_C(166667);
    game_info.window_title = view("C SoundEffect sources");
    if (!cna_demo_check_result(
            &demo, cna_game_create(&game_info, &game), "create the owner game")) {
        (void)remove(WavPath);
        return cna_demo_finish(&demo);
    }

    memset(&capabilities, 0, sizeof(capabilities));
    capabilities.struct_size = (uint32_t)sizeof(capabilities);
    capabilities.struct_version = UINT32_C(1);
    if (cna_audio_get_capabilities(game, &capabilities) != CNA_RESULT_SUCCESS ||
        capabilities.is_playback_available != CNA_TRUE) {
        (void)cna_game_destroy(game);
        (void)remove(WavPath);
        return cna_demo_skip("no audio playback device");
    }

    memset(&sound_info, 0, sizeof(sound_info));
    sound_info.struct_size = (uint32_t)sizeof(sound_info);
    sound_info.struct_version = UINT32_C(1);
    sound_info.sample_rate = SAMPLE_RATE;
    sound_info.channels = CNA_AUDIO_CHANNELS_MONO;
    cna_demo_check_result(
        &demo,
        cna_sound_effect_create_pcm16_range_ext(
            game, &sound_info, pcm, sizeof(pcm), 400, 800, 50, 100, &ranged),
        "create a 50 ms effect from a copied middle PCM range with an authored loop region");
    memset(pcm, 0x7F, sizeof(pcm));
    ticks = -1;
    cna_demo_check(
        &demo,
        ranged != CNA_INVALID_HANDLE &&
            cna_sound_effect_get_duration_ticks(ranged, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(500000),
        "the ranged effect retains its exact duration after caller PCM storage is overwritten");

    cna_demo_check_result(
        &demo,
        cna_sound_effect_create_from_encoded_ext(game, wav, sizeof(wav), &encoded),
        "decode an owned SoundEffect from generated WAV bytes");
    memset(wav, 0xA5, sizeof(wav));
    ticks = -1;
    cna_demo_check(
        &demo,
        encoded != CNA_INVALID_HANDLE &&
            cna_sound_effect_get_duration_ticks(encoded, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(1000000),
        "the encoded-memory effect retains its duration after caller bytes are overwritten");

    if (file_ready) {
        cna_demo_check_result(
            &demo,
            cna_sound_effect_create_from_asset_ext(game, view(WavPath), &asset),
            "decode an owned SoundEffect from the private WAV file");
    }
    cna_demo_check(&demo, remove(WavPath) == 0, "remove the private WAV after asset creation");
    ticks = -1;
    cna_demo_check(
        &demo,
        asset != CNA_INVALID_HANDLE &&
            cna_sound_effect_get_duration_ticks(asset, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(1000000),
        "the file-created effect remains usable after its source file is removed");

    memset(name, 0x5A, sizeof(name));
    cna_demo_check(
        &demo,
        ranged != CNA_INVALID_HANDLE &&
            cna_sound_effect_set_name(ranged, view("range tone")) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_get_name_size(ranged, &count) == CNA_RESULT_SUCCESS && count == 10U &&
            cna_sound_effect_copy_name(ranged, name, count, &count) == CNA_RESULT_SUCCESS &&
            count == 10U && memcmp(name, "range tone", 10U) == 0 &&
            (uint8_t)name[10] == UINT8_C(0x5A),
        "the effect owns an exact UTF-8 name copied without a terminator");
    memset(short_name, 0x5A, sizeof(short_name));
    count = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_sound_effect_copy_name(
            ranged, short_name, sizeof(short_name), &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            count == 10U && bytes_are(short_name, sizeof(short_name), UINT8_C(0x5A)),
        "a short name destination reports the byte count without partial output");

    cna_demo_check(
        &demo,
        cna_sound_effect_get_is_disposed(ranged, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE,
        "an owned live SoundEffect reports that it has not been disposed");

    cna_demo_check(
        &demo,
        cna_sound_effect_get_master_volume(game, &original_master_volume) ==
                CNA_RESULT_SUCCESS &&
            cna_sound_effect_set_master_volume(game, 0.375f) == CNA_RESULT_SUCCESS &&
            cna_sound_effect_get_master_volume(game, &master_volume) == CNA_RESULT_SUCCESS &&
            master_volume == 0.375f,
        "the process master volume round-trips a distinguishable value");
    cna_demo_check_result(
        &demo, cna_sound_effect_set_master_volume(game, original_master_volume),
        "restore the process master volume");

    cna_demo_check(
        &demo,
        cna_sound_effect_play(ranged, &played) == CNA_RESULT_SUCCESS && played == CNA_TRUE,
        "the parameterless one-shot route starts the ranged effect");
    played = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_sound_effect_play_with_settings(ranged, 0.5f, 99.0f, -0.25f, &played) ==
                CNA_RESULT_SUCCESS &&
            played == CNA_TRUE,
        "the configured one-shot route accepts volume and pan while clamping extreme pitch");
    played = UINT8_C(77);
    cna_demo_check(
        &demo,
        cna_sound_effect_play_with_settings(ranged, 0.5f, 0.0f, 1.01f, &played) ==
            CNA_RESULT_INVALID_ARGUMENT,
        "the configured one-shot route refuses pan outside its closed interval");

    if (asset != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_sound_effect_destroy(asset), "destroy the file-created effect");
    }
    if (encoded != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_sound_effect_destroy(encoded), "destroy the memory-decoded effect");
    }
    if (ranged != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_sound_effect_destroy(ranged), "destroy the ranged PCM effect");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
