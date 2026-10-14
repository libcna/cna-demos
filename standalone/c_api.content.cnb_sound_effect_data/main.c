/* SPDX-License-Identifier: MS-PL */
/* CNB audio identities, neutral SoundEffect samples and complete document codec through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static int bytes_are(const uint8_t* bytes, size_t count, uint8_t expected)
{
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != expected) {
            return 0;
        }
    }
    return 1;
}

static int format_name_is(CNA_CnbAudioFormat format, const char* expected)
{
    char text[48];
    char short_text[48];
    char short_before[48];
    uint64_t size = UINT64_C(0);
    uint64_t copied = UINT64_C(99);
    const uint64_t expected_size = (uint64_t)strlen(expected);

    memset(text, 0, sizeof(text));
    memset(short_text, 0xA5, sizeof(short_text));
    memcpy(short_before, short_text, sizeof(short_text));
    return cna_cnb_get_audio_format_name_size(format, &size) == CNA_RESULT_SUCCESS &&
           size == expected_size &&
           cna_cnb_copy_audio_format_name(
               format, short_text, expected_size - UINT64_C(1), &copied) ==
               CNA_RESULT_BUFFER_TOO_SMALL &&
           copied == expected_size && memcmp(short_text, short_before, sizeof(short_text)) == 0 &&
           cna_cnb_copy_audio_format_name(
               format, text, sizeof(text), &copied) == CNA_RESULT_SUCCESS &&
           copied == expected_size && memcmp(text, expected, (size_t)expected_size) == 0;
}

static int sound_is(
    CNA_CnbSoundEffectDataHandle sound, const uint8_t* expected, size_t expected_size)
{
    CNA_CnbSoundEffectInfo info;
    uint8_t samples[32];
    uint8_t short_samples[32];
    uint8_t short_before[32];
    uint64_t copied = UINT64_C(0);

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_SOUND_EFFECT_INFO_STRUCT_VERSION;
    memset(short_samples, 0xA5, sizeof(short_samples));
    memcpy(short_before, short_samples, sizeof(short_samples));
    return cna_cnb_sound_effect_data_get_info(sound, &info) == CNA_RESULT_SUCCESS &&
           info.format == CNA_CNB_AUDIO_FORMAT_PCM8 && info.sample_rate == UINT32_C(48000) &&
           info.channels == UINT32_C(2) && info.frame_count == UINT32_C(8) &&
           info.loop_start == UINT32_C(2) && info.loop_length == UINT32_C(4) &&
           cna_cnb_sound_effect_data_copy_samples(
               sound, NULL, UINT64_C(0), &copied) == CNA_RESULT_BUFFER_TOO_SMALL &&
           copied == (uint64_t)expected_size &&
           cna_cnb_sound_effect_data_copy_samples(
               sound, short_samples, (uint64_t)expected_size - UINT64_C(1), &copied) ==
               CNA_RESULT_BUFFER_TOO_SMALL &&
           copied == (uint64_t)expected_size &&
           memcmp(short_samples, short_before, sizeof(short_samples)) == 0 &&
           cna_cnb_sound_effect_data_copy_samples(
               sound, samples, sizeof(samples), &copied) == CNA_RESULT_SUCCESS &&
           copied == (uint64_t)expected_size && memcmp(samples, expected, expected_size) == 0;
}

static int release_sound(CNA_CnbSoundEffectDataHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_sound_effect_data_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_document(CNA_CnbDocumentHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_document_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_CnbSoundEffectDataHandle sound = CNA_INVALID_HANDLE;
    CNA_CnbSoundEffectDataHandle decoded = CNA_INVALID_HANDLE;
    CNA_CnbSoundEffectDataHandle broken = CNA_INVALID_HANDLE;
    CNA_CnbSoundEffectDataHandle reserved = CNA_INVALID_HANDLE;
    CNA_CnbSoundEffectDataHandle stale_sound = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle document = CNA_INVALID_HANDLE;
    CNA_CnbSoundEffectInfo info;
    uint8_t source[16];
    uint8_t expected[16];
    uint8_t image[65536];
    uint8_t second_image[65536];
    uint8_t short_image[65536];
    uint32_t bytes_per_frame = UINT32_MAX;
    uint32_t asset_type = UINT32_C(0);
    uint32_t schema = UINT32_C(0);
    uint64_t image_size = UINT64_C(0);
    uint64_t second_size = UINT64_C(0);
    size_t byte_index;
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_sound_effect_data", argc, argv, 1);
    cna_demo_check(
        &demo,
        CNA_CNB_AUDIO_FORMAT_UNKNOWN == UINT32_C(0) &&
            CNA_CNB_AUDIO_FORMAT_PCM16 == UINT32_C(1) &&
            CNA_CNB_AUDIO_FORMAT_PCM8 == UINT32_C(2) &&
            CNA_CNB_AUDIO_FORMAT_PCM_FLOAT32 == UINT32_C(3) &&
            CNA_CNB_AUDIO_FORMAT_ADPCM == UINT32_C(4) &&
            CNA_CNB_AUDIO_FORMAT_VORBIS == UINT32_C(5) &&
            CNA_CNB_SOUND_EFFECT_SCHEMA_VERSION == UINT32_C(2) &&
            CNA_CNB_SOUND_EFFECT_HEADER_STRIDE == UINT32_C(28),
        "audio format identities and the current SoundEffect schema stay fixed");
    cna_demo_check(
        &demo,
        cna_cnb_audio_frame_bytes(
            CNA_CNB_AUDIO_FORMAT_PCM16, UINT32_C(1), &bytes_per_frame) == CNA_RESULT_SUCCESS &&
            bytes_per_frame == UINT32_C(2) &&
            cna_cnb_audio_frame_bytes(
                CNA_CNB_AUDIO_FORMAT_PCM16, UINT32_C(2), &bytes_per_frame) ==
                CNA_RESULT_SUCCESS &&
            bytes_per_frame == UINT32_C(4) &&
            cna_cnb_audio_frame_bytes(
                CNA_CNB_AUDIO_FORMAT_PCM8, UINT32_C(2), &bytes_per_frame) ==
                CNA_RESULT_SUCCESS &&
            bytes_per_frame == UINT32_C(2) &&
            cna_cnb_audio_frame_bytes(
                CNA_CNB_AUDIO_FORMAT_PCM_FLOAT32, UINT32_C(2), &bytes_per_frame) ==
                CNA_RESULT_SUCCESS &&
            bytes_per_frame == UINT32_C(8) &&
            cna_cnb_audio_frame_bytes(
                CNA_CNB_AUDIO_FORMAT_VORBIS, UINT32_C(2), &bytes_per_frame) ==
                CNA_RESULT_SUCCESS &&
            bytes_per_frame == UINT32_C(0) &&
            cna_cnb_audio_frame_bytes(UINT32_C(99), UINT32_C(2), &bytes_per_frame) ==
                CNA_RESULT_SUCCESS &&
            bytes_per_frame == UINT32_C(0),
        "fixed-width formats scale by channels while packet and unknown formats report zero");
    cna_demo_check(
        &demo,
        format_name_is(CNA_CNB_AUDIO_FORMAT_UNKNOWN, "Unknown") &&
            format_name_is(CNA_CNB_AUDIO_FORMAT_PCM16, "Pcm16") &&
            format_name_is(CNA_CNB_AUDIO_FORMAT_PCM8, "Pcm8") &&
            format_name_is(CNA_CNB_AUDIO_FORMAT_PCM_FLOAT32, "PcmFloat32") &&
            format_name_is(CNA_CNB_AUDIO_FORMAT_ADPCM, "Adpcm") &&
            format_name_is(CNA_CNB_AUDIO_FORMAT_VORBIS, "Vorbis") &&
            format_name_is(UINT32_C(99), "unknown audio format 99"),
        "every named and unknown format has an exact atomic diagnostic name");

    for (byte_index = 0U; byte_index < sizeof(source); ++byte_index) {
        source[byte_index] = (uint8_t)(byte_index * 11U + 3U);
    }
    memcpy(expected, source, sizeof(expected));
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_SOUND_EFFECT_INFO_STRUCT_VERSION;
    info.format = CNA_CNB_AUDIO_FORMAT_PCM8;
    info.sample_rate = UINT32_C(48000);
    info.channels = UINT32_C(2);
    info.frame_count = UINT32_C(8);
    info.loop_start = UINT32_C(2);
    info.loop_length = UINT32_C(4);
    if (!cna_demo_check_result(
            &demo,
            cna_cnb_sound_effect_data_create(&info, source, sizeof(source), &sound),
            "create a copied stereo PCM8 description with an interior loop")) {
        goto cleanup;
    }
    memset(source, 0xEE, sizeof(source));
    cna_demo_check(
        &demo, sound_is(sound, expected, sizeof(expected)),
        "shape and samples remain exact after the caller changes its source bytes");

    image_size = UINT64_C(0);
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_sound_effect(
                sound, text_view("Sounds/Pulse"), NULL, UINT64_C(0), &image_size) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                image_size > UINT64_C(0) && image_size <= sizeof(image),
            "size the complete schema-two SoundEffect document")) {
        goto cleanup;
    }
    memset(short_image, 0xA5, sizeof(short_image));
    second_size = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_cnb_encode_sound_effect(
            sound, text_view("Sounds/Pulse"), short_image, image_size - UINT64_C(1),
            &second_size) == CNA_RESULT_BUFFER_TOO_SMALL &&
            second_size == image_size && bytes_are(short_image, sizeof(short_image), 0xA5U),
        "a short document destination is sized without a partial write");
    cna_demo_check(
        &demo,
        cna_cnb_encode_sound_effect(
            sound, text_view("Sounds/Pulse"), image, sizeof(image), &second_size) ==
                CNA_RESULT_SUCCESS &&
            second_size == image_size &&
            cna_cnb_encode_sound_effect(
                sound, text_view("Sounds/Pulse"), second_image, sizeof(second_image),
                &second_size) == CNA_RESULT_SUCCESS &&
            second_size == image_size && memcmp(image, second_image, (size_t)image_size) == 0,
        "complete SoundEffect encoding is exact and repeatable");
    if (!cna_demo_check_result(
            &demo,
            cna_cnb_document_parse(image, image_size, text_view("sound-memory"), NULL, &document),
            "parse the complete SoundEffect document")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_document_get_asset_type_id(document, &asset_type) == CNA_RESULT_SUCCESS &&
            asset_type == CNA_CNB_ASSET_TYPE_SOUND_EFFECT &&
            cna_cnb_document_get_asset_schema_version(document, &schema) == CNA_RESULT_SUCCESS &&
            schema == CNA_CNB_SOUND_EFFECT_SCHEMA_VERSION,
        "the document records the SoundEffect asset identity and schema two");
    if (!cna_demo_check_result(
            &demo, cna_cnb_decode_sound_effect(document, &decoded),
            "decode an independent neutral SoundEffect from the document")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo, sound_is(decoded, expected, sizeof(expected)),
        "decoding restores the exact PCM8 shape, loop and sample bytes");

    stale_sound = sound;
    cna_demo_check_result(
        &demo, cna_cnb_sound_effect_data_destroy(sound),
        "release the original neutral SoundEffect description");
    sound = CNA_INVALID_HANDLE;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_SOUND_EFFECT_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_sound_effect_data_get_info(stale_sound, &info) == CNA_RESULT_INVALID_HANDLE &&
            sound_is(decoded, expected, sizeof(expected)),
        "the stale original is rejected while the decoded sound remains independent");

    info.format = CNA_CNB_AUDIO_FORMAT_PCM8;
    info.sample_rate = UINT32_C(48000);
    info.channels = UINT32_C(2);
    info.frame_count = UINT32_C(8);
    info.loop_start = UINT32_C(2);
    info.loop_length = UINT32_C(4);
    cna_demo_check(
        &demo,
        cna_cnb_sound_effect_data_create(
            &info, expected, sizeof(expected) - 1U, &broken) == CNA_RESULT_SUCCESS &&
            cna_cnb_encode_sound_effect(
                broken, text_view(""), image, sizeof(image), &second_size) == CNA_RESULT_IO,
        "encoding rejects a stored payload shorter than its declared frame shape");
    info.format = CNA_CNB_AUDIO_FORMAT_VORBIS;
    info.frame_count = UINT32_C(0);
    info.loop_start = UINT32_C(0);
    info.loop_length = UINT32_C(0);
    cna_demo_check(
        &demo,
        cna_cnb_sound_effect_data_create(
            &info, NULL, UINT64_C(0), &reserved) == CNA_RESULT_SUCCESS &&
            cna_cnb_encode_sound_effect(
                reserved, text_view(""), image, sizeof(image), &second_size) == CNA_RESULT_IO,
        "a named packet-format identity remains inspectable but has no document codec");

cleanup:
    releases_ok &= release_sound(&reserved);
    releases_ok &= release_sound(&broken);
    releases_ok &= release_sound(&decoded);
    releases_ok &= release_sound(&sound);
    releases_ok &= release_document(&document);
    cna_demo_check(&demo, releases_ok, "release every remaining sound and document handle");
    return cna_demo_finish(&demo);
}
