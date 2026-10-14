/* SPDX-License-Identifier: MS-PL */
#define _POSIX_C_SOURCE 200809L
/* Direct image, DDS cube and WAV import/decode front ends through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(__linux__)
#include <unistd.h>
#endif

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

#if defined(__linux__)
typedef struct ImportFixture {
    char root[256];
    char image[288];
    char dds[288];
    char wav[288];
    int ready;
} ImportFixture;

/* A 1x1 24-bit BMP whose one BGR pixel is RGB (10, 20, 30). */
static const uint8_t FixtureBmp[58] = {
    0x42U, 0x4DU, 0x3AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x36U, 0x00U, 0x00U, 0x00U, 0x28U, 0x00U,
    0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U,
    0x00U, 0x00U, 0x01U, 0x00U, 0x18U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x04U, 0x00U, 0x00U, 0x00U, 0x13U, 0x0BU,
    0x00U, 0x00U, 0x13U, 0x0BU, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x1EU, 0x14U,
    0x0AU, 0x00U};

static void push_u16(uint8_t* data, size_t* offset, uint32_t value)
{
    data[(*offset)++] = (uint8_t)(value & UINT32_C(0xff));
    data[(*offset)++] = (uint8_t)((value >> 8U) & UINT32_C(0xff));
}

static void push_u32(uint8_t* data, size_t* offset, uint32_t value)
{
    push_u16(data, offset, value & UINT32_C(0xffff));
    push_u16(data, offset, (value >> 16U) & UINT32_C(0xffff));
}

static void push_tag(uint8_t* data, size_t* offset, const char tag[4])
{
    size_t index;
    for (index = 0U; index < 4U; ++index) {
        data[(*offset)++] = (uint8_t)tag[index];
    }
}

/* Six zero DXT1 blocks: one 4x4 block for each face of a complete cube. */
static size_t build_cube_dds(uint8_t data[176])
{
    size_t offset = 0U;
    int index;
    push_tag(data, &offset, "DDS ");
    push_u32(data, &offset, UINT32_C(124));
    push_u32(data, &offset, UINT32_C(0x6));
    push_u32(data, &offset, UINT32_C(4));
    push_u32(data, &offset, UINT32_C(4));
    push_u32(data, &offset, UINT32_C(0));
    push_u32(data, &offset, UINT32_C(0));
    push_u32(data, &offset, UINT32_C(1));
    for (index = 0; index < 11; ++index) {
        push_u32(data, &offset, UINT32_C(0));
    }
    push_u32(data, &offset, UINT32_C(32));
    push_u32(data, &offset, UINT32_C(0x4));
    push_tag(data, &offset, "DXT1");
    for (index = 0; index < 5; ++index) {
        push_u32(data, &offset, UINT32_C(0));
    }
    push_u32(data, &offset, UINT32_C(0x1000));
    push_u32(data, &offset, UINT32_C(0xfe00));
    push_u32(data, &offset, UINT32_C(0));
    push_u32(data, &offset, UINT32_C(0));
    push_u32(data, &offset, UINT32_C(0));
    memset(data + offset, 0, 48U);
    return offset + 48U;
}

/* Four mono unsigned PCM8 frames and one inclusive RIFF loop [1, 2]. */
static size_t build_looped_wav(uint8_t data[116])
{
    static const uint8_t Samples[4] = {0U, 64U, 128U, 255U};
    size_t offset = 0U;
    push_tag(data, &offset, "RIFF");
    push_u32(data, &offset, UINT32_C(108));
    push_tag(data, &offset, "WAVE");
    push_tag(data, &offset, "fmt ");
    push_u32(data, &offset, UINT32_C(16));
    push_u16(data, &offset, UINT32_C(1));
    push_u16(data, &offset, UINT32_C(1));
    push_u32(data, &offset, UINT32_C(8000));
    push_u32(data, &offset, UINT32_C(8000));
    push_u16(data, &offset, UINT32_C(1));
    push_u16(data, &offset, UINT32_C(8));
    push_tag(data, &offset, "data");
    push_u32(data, &offset, UINT32_C(4));
    memcpy(data + offset, Samples, sizeof(Samples));
    offset += sizeof(Samples);
    push_tag(data, &offset, "smpl");
    push_u32(data, &offset, UINT32_C(60));
    memset(data + offset, 0, 60U);
    data[offset + 28U] = 1U; /* one loop entry */
    data[offset + 44U] = 1U; /* entry start */
    data[offset + 48U] = 2U; /* entry inclusive end */
    return offset + 60U;
}

static int write_bytes(const char* path, const void* bytes, size_t byte_count)
{
    FILE* file = fopen(path, "wb");
    int ok;
    if (file == NULL) {
        return 0;
    }
    ok = fwrite(bytes, 1U, byte_count, file) == byte_count;
    return fclose(file) == 0 && ok;
}

static int prepare_fixture(
    ImportFixture* fixture, const uint8_t* dds, size_t dds_size,
    const uint8_t* wav, size_t wav_size)
{
    char template_path[] = "/tmp/cna-demos-cnb-importers-XXXXXX";
    char* root;
    memset(fixture, 0, sizeof(*fixture));
    root = mkdtemp(template_path);
    if (root == NULL || strlen(root) >= sizeof(fixture->root)) {
        return 0;
    }
    memcpy(fixture->root, root, strlen(root) + 1U);
    fixture->ready = 1;
    if (snprintf(fixture->image, sizeof(fixture->image), "%s/pixel.bmp", root) < 0 ||
        snprintf(fixture->dds, sizeof(fixture->dds), "%s/cube.dds", root) < 0 ||
        snprintf(fixture->wav, sizeof(fixture->wav), "%s/loop.wav", root) < 0) {
        return 0;
    }
    return write_bytes(fixture->image, FixtureBmp, sizeof(FixtureBmp)) &&
           write_bytes(fixture->dds, dds, dds_size) &&
           write_bytes(fixture->wav, wav, wav_size);
}

static int cleanup_fixture(ImportFixture* fixture)
{
    int ok = 1;
    if (!fixture->ready) {
        return 1;
    }
    if (unlink(fixture->image) != 0 || unlink(fixture->dds) != 0 ||
        unlink(fixture->wav) != 0 || rmdir(fixture->root) != 0) {
        ok = 0;
    }
    fixture->ready = 0;
    return ok;
}

static int release_texture(CNA_CnbTextureDataHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_texture_data_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
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
#endif

int main(int argc, char** argv)
{
    CnaDemo demo;

    cna_demo_init(&demo, "c_api.content.cnb_importers", argc, argv, 1);
#if !defined(__linux__)
    return cna_demo_skip("the private filesystem fixture is currently implemented for Linux");
#else
    ImportFixture fixture;
    CNA_CnbTextureDataHandle image = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle keyed = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle unmatched = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle cube_memory = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle cube_file = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle rejected_texture = CNA_INVALID_HANDLE;
    CNA_CnbSoundEffectDataHandle sound_memory = CNA_INVALID_HANDLE;
    CNA_CnbSoundEffectDataHandle sound_file = CNA_INVALID_HANDLE;
    CNA_CnbSoundEffectDataHandle rejected_sound = CNA_INVALID_HANDLE;
    CNA_CnbTextureInfo texture_info;
    CNA_CnbSoundEffectInfo sound_info;
    CNA_CnbImageImportOptions options;
    uint8_t dds[176];
    uint8_t non_cube_dds[176];
    uint8_t wav[116];
    uint8_t compressed_wav[116];
    uint8_t pixel[4];
    uint8_t memory_level[64];
    uint8_t file_level[64];
    uint8_t samples[8];
    uint8_t file_samples[8];
    CNA_CnbTextureFormat format = CNA_CNB_TEXTURE_FORMAT_UNKNOWN;
    uint64_t produced = UINT64_C(0);
    uint64_t level_count = UINT64_C(0);
    uint64_t face;
    int faces_ok = 1;
    int releases_ok = 1;
    char missing[288];

    memset(&fixture, 0, sizeof(fixture));
    if (!cna_demo_check(
            &demo,
            build_cube_dds(dds) == sizeof(dds) &&
                build_looped_wav(wav) == sizeof(wav) &&
                prepare_fixture(&fixture, dds, sizeof(dds), wav, sizeof(wav)),
            "create private BMP, DXT1 cube and looping PCM8 WAV fixtures")) {
        goto cleanup;
    }

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_import_image_as_texture2d(text_view(fixture.image), NULL, &image),
            "import the image with explicit default behavior")) {
        goto cleanup;
    }
    memset(&texture_info, 0, sizeof(texture_info));
    texture_info.struct_size = (uint32_t)sizeof(texture_info);
    texture_info.struct_version = CNA_CNB_TEXTURE_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_get_info(image, &texture_info) == CNA_RESULT_SUCCESS &&
            texture_info.width == UINT32_C(1) && texture_info.height == UINT32_C(1) &&
            texture_info.depth == UINT32_C(1) && texture_info.face_count == UINT32_C(1) &&
            texture_info.mip_count == UINT32_C(1) &&
            texture_info.representation_count == UINT32_C(1) &&
            cna_cnb_texture_data_get_representation_format(image, UINT64_C(0), &format) ==
                CNA_RESULT_SUCCESS &&
            format == CNA_CNB_TEXTURE_FORMAT_RGBA8 &&
            cna_cnb_texture_data_copy_level(
                image, UINT64_C(0), UINT64_C(0), pixel, sizeof(pixel), &produced) ==
                CNA_RESULT_SUCCESS &&
            produced == sizeof(pixel) && pixel[0] == 10U && pixel[1] == 20U &&
            pixel[2] == 30U && pixel[3] == 255U,
        "the image importer exposes its exact one-pixel RGBA8 description");

    memset(&options, 0, sizeof(options));
    options.struct_size = (uint32_t)sizeof(options);
    options.struct_version = CNA_CNB_IMAGE_IMPORT_OPTIONS_STRUCT_VERSION;
    options.color_key[0] = 10U;
    options.color_key[1] = 20U;
    options.color_key[2] = 30U;
    options.has_color_key = CNA_TRUE;
    if (cna_demo_check_result(
            &demo,
            cna_cnb_import_image_as_texture2d(text_view(fixture.image), &options, &keyed),
            "import the same image with a matching color key")) {
        cna_demo_check(
            &demo,
            cna_cnb_texture_data_copy_level(
                keyed, UINT64_C(0), UINT64_C(0), pixel, sizeof(pixel), &produced) ==
                    CNA_RESULT_SUCCESS &&
                produced == sizeof(pixel) && pixel[0] == 10U && pixel[1] == 20U &&
                pixel[2] == 30U && pixel[3] == 0U,
            "a matching color key keeps RGB and clears only alpha");
    }
    options.color_key[0] = 200U;
    cna_demo_check(
        &demo,
        cna_cnb_import_image_as_texture2d(
            text_view(fixture.image), &options, &unmatched) == CNA_RESULT_SUCCESS &&
            cna_cnb_texture_data_copy_level(
                unmatched, UINT64_C(0), UINT64_C(0), pixel, sizeof(pixel), &produced) ==
                CNA_RESULT_SUCCESS &&
            pixel[3] == 255U,
        "a nonmatching color key leaves the source alpha opaque");
    {
        CNA_CnbImageImportOptions bad = options;
        bad.struct_version = CNA_CNB_IMAGE_IMPORT_OPTIONS_STRUCT_VERSION + UINT32_C(1);
        rejected_texture = UINT64_MAX;
        cna_demo_check(
            &demo,
            cna_cnb_import_image_as_texture2d(
                text_view(fixture.image), &bad, &rejected_texture) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                rejected_texture == CNA_INVALID_HANDLE &&
                ((bad = options), (bad.has_color_key = (CNA_Bool)9),
                 cna_cnb_import_image_as_texture2d(
                     text_view(fixture.image), &bad, &rejected_texture)) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                rejected_texture == CNA_INVALID_HANDLE,
            "unknown option versions and noncanonical color-key booleans are refused");
    }
    if (snprintf(missing, sizeof(missing), "%s/missing.bmp", fixture.root) < 0) {
        cna_demo_check(&demo, 0, "form the missing image path");
        goto cleanup;
    }
    rejected_texture = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_import_image_as_texture2d(text_view(missing), NULL, &rejected_texture) ==
                CNA_RESULT_IO &&
            rejected_texture == CNA_INVALID_HANDLE,
        "a missing image is an IO failure without a partial texture");

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_decode_dds_as_texture_cube(
                dds, sizeof(dds), text_view("memory-cube.dds"), &cube_memory),
            "decode the complete cube DDS from caller-owned memory")) {
        goto cleanup;
    }
    memset(&texture_info, 0, sizeof(texture_info));
    texture_info.struct_size = (uint32_t)sizeof(texture_info);
    texture_info.struct_version = CNA_CNB_TEXTURE_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_get_info(cube_memory, &texture_info) == CNA_RESULT_SUCCESS &&
            texture_info.width == UINT32_C(4) && texture_info.height == UINT32_C(4) &&
            texture_info.depth == UINT32_C(1) &&
            texture_info.face_count == CNA_CNB_TEXTURE_CUBE_FACE_COUNT &&
            texture_info.mip_count == UINT32_C(1) &&
            texture_info.representation_count == UINT32_C(1) &&
            cna_cnb_texture_data_get_representation_format(
                cube_memory, UINT64_C(0), &format) == CNA_RESULT_SUCCESS &&
            format == CNA_CNB_TEXTURE_FORMAT_RGBA8 && format != CNA_CNB_TEXTURE_FORMAT_BC1 &&
            cna_cnb_texture_data_get_level_count(
                cube_memory, UINT64_C(0), &level_count) == CNA_RESULT_SUCCESS &&
            level_count == CNA_CNB_TEXTURE_CUBE_FACE_COUNT,
        "the DXT1 source becomes one portable RGBA8 level for each cube face");
    memset(dds, 0xA5, sizeof(dds));
    for (face = UINT64_C(0); face < CNA_CNB_TEXTURE_CUBE_FACE_COUNT; ++face) {
        size_t index;
        produced = UINT64_C(0);
        if (cna_cnb_texture_data_copy_level(
                cube_memory, UINT64_C(0), face, memory_level, sizeof(memory_level), &produced) !=
                CNA_RESULT_SUCCESS ||
            produced != sizeof(memory_level)) {
            faces_ok = 0;
            break;
        }
        for (index = 0U; index < sizeof(memory_level); index += 4U) {
            if (memory_level[index] != 0U || memory_level[index + 1U] != 0U ||
                memory_level[index + 2U] != 0U || memory_level[index + 3U] != 255U) {
                faces_ok = 0;
            }
        }
    }
    cna_demo_check(
        &demo, faces_ok,
        "all six decompressed faces remain owned and contain sixteen opaque black texels");

    if (cna_demo_check_result(
            &demo, cna_cnb_import_dds_as_texture_cube(text_view(fixture.dds), &cube_file),
            "import the identical DDS through its filesystem front end")) {
        faces_ok = 1;
        for (face = UINT64_C(0); face < CNA_CNB_TEXTURE_CUBE_FACE_COUNT; ++face) {
            uint64_t memory_size = UINT64_C(0);
            uint64_t file_size = UINT64_C(0);
            if (cna_cnb_texture_data_copy_level(
                    cube_memory, UINT64_C(0), face, memory_level, sizeof(memory_level),
                    &memory_size) != CNA_RESULT_SUCCESS ||
                cna_cnb_texture_data_copy_level(
                    cube_file, UINT64_C(0), face, file_level, sizeof(file_level), &file_size) !=
                    CNA_RESULT_SUCCESS ||
                memory_size != file_size || memory_size != sizeof(memory_level) ||
                memcmp(memory_level, file_level, sizeof(memory_level)) != 0) {
                faces_ok = 0;
            }
        }
        cna_demo_check(
            &demo, faces_ok,
            "filesystem and memory DDS front ends produce byte-identical face payloads");
    }
    build_cube_dds(non_cube_dds);
    memset(non_cube_dds + 112U, 0, 4U);
    rejected_texture = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_decode_dds_as_texture_cube(
            non_cube_dds, sizeof(non_cube_dds), text_view("not-a-cube.dds"),
            &rejected_texture) != CNA_RESULT_SUCCESS &&
            rejected_texture == CNA_INVALID_HANDLE &&
            ((rejected_texture = UINT64_MAX),
             cna_cnb_decode_dds_as_texture_cube(
                 non_cube_dds, UINT64_C(8), text_view("truncated.dds"),
                 &rejected_texture)) != CNA_RESULT_SUCCESS &&
            rejected_texture == CNA_INVALID_HANDLE,
        "non-cube and truncated DDS inputs fail without partial textures");
    if (snprintf(missing, sizeof(missing), "%s/missing.dds", fixture.root) < 0) {
        cna_demo_check(&demo, 0, "form the missing DDS path");
        goto cleanup;
    }
    rejected_texture = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_import_dds_as_texture_cube(text_view(missing), &rejected_texture) ==
                CNA_RESULT_IO &&
            rejected_texture == CNA_INVALID_HANDLE,
        "a missing DDS is an IO failure without a partial texture");

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_decode_wav_as_sound_effect(
                wav, sizeof(wav), text_view("memory-loop.wav"), &sound_memory),
            "decode the looping PCM8 WAV from caller-owned memory")) {
        goto cleanup;
    }
    memset(wav, 0xA5, sizeof(wav));
    memset(&sound_info, 0, sizeof(sound_info));
    sound_info.struct_size = (uint32_t)sizeof(sound_info);
    sound_info.struct_version = CNA_CNB_SOUND_EFFECT_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_sound_effect_data_get_info(sound_memory, &sound_info) == CNA_RESULT_SUCCESS &&
            sound_info.format == CNA_CNB_AUDIO_FORMAT_PCM8 &&
            sound_info.sample_rate == UINT32_C(8000) && sound_info.channels == UINT32_C(1) &&
            sound_info.frame_count == UINT32_C(4) && sound_info.loop_start == UINT32_C(1) &&
            sound_info.loop_length == UINT32_C(2),
        "the WAV keeps its eight-bit width, exact rate, shape and inclusive RIFF loop");
    produced = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_cnb_sound_effect_data_copy_samples(
            sound_memory, samples, sizeof(samples), &produced) == CNA_RESULT_SUCCESS &&
            produced == UINT64_C(4) && samples[0] == 0U && samples[1] == 64U &&
            samples[2] == 128U && samples[3] == 255U,
        "the decoder owns the four headerless samples after caller memory changes");
    if (cna_demo_check_result(
            &demo, cna_cnb_import_wav_as_sound_effect(text_view(fixture.wav), &sound_file),
            "import the identical WAV through its filesystem front end")) {
        CNA_CnbSoundEffectInfo file_info;
        uint64_t file_size = UINT64_C(0);
        memset(&file_info, 0, sizeof(file_info));
        file_info.struct_size = (uint32_t)sizeof(file_info);
        file_info.struct_version = CNA_CNB_SOUND_EFFECT_INFO_STRUCT_VERSION;
        cna_demo_check(
            &demo,
            cna_cnb_sound_effect_data_get_info(sound_file, &file_info) == CNA_RESULT_SUCCESS &&
                memcmp(&sound_info, &file_info, sizeof(sound_info)) == 0 &&
                cna_cnb_sound_effect_data_copy_samples(
                    sound_file, file_samples, sizeof(file_samples), &file_size) ==
                    CNA_RESULT_SUCCESS &&
                file_size == produced && memcmp(samples, file_samples, (size_t)produced) == 0,
            "filesystem and memory WAV front ends produce identical descriptions and samples");
    }
    build_looped_wav(compressed_wav);
    compressed_wav[20] = 2U;
    rejected_sound = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_decode_wav_as_sound_effect(
            compressed_wav, sizeof(compressed_wav), text_view("compressed.wav"),
            &rejected_sound) == CNA_RESULT_IO &&
            rejected_sound == CNA_INVALID_HANDLE &&
            ((rejected_sound = UINT64_MAX),
             cna_cnb_decode_wav_as_sound_effect(
                 compressed_wav, UINT64_C(20), text_view("truncated.wav"),
                 &rejected_sound)) == CNA_RESULT_IO &&
            rejected_sound == CNA_INVALID_HANDLE,
        "compressed and truncated WAV inputs fail atomically as IO errors");
    if (snprintf(missing, sizeof(missing), "%s/missing.wav", fixture.root) < 0) {
        cna_demo_check(&demo, 0, "form the missing WAV path");
        goto cleanup;
    }
    rejected_sound = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_import_wav_as_sound_effect(text_view(missing), &rejected_sound) ==
                CNA_RESULT_IO &&
            rejected_sound == CNA_INVALID_HANDLE,
        "a missing WAV is an IO failure without a partial sound");

cleanup:
    releases_ok &= release_sound(&rejected_sound);
    releases_ok &= release_sound(&sound_file);
    releases_ok &= release_sound(&sound_memory);
    releases_ok &= release_texture(&rejected_texture);
    releases_ok &= release_texture(&cube_file);
    releases_ok &= release_texture(&cube_memory);
    releases_ok &= release_texture(&unmatched);
    releases_ok &= release_texture(&keyed);
    releases_ok &= release_texture(&image);
    releases_ok &= cleanup_fixture(&fixture);
    cna_demo_check(
        &demo, releases_ok,
        "release every imported data object and remove the private source files");
    return cna_demo_finish(&demo);
#endif
}
