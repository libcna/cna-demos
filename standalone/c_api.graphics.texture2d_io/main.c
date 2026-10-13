/* SPDX-License-Identifier: MS-PL */
/*
 * Texture2D data and image I/O from C: standalone textures expose their CPU storage, typed
 * rectangle transfers preserve caller array windows, PNG/JPEG encoding is atomic, and a saved
 * PNG loads again. A one-frame game then repeats the creation paths that require a live graphics
 * device, including RGBA upload and encoded-memory decoding.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static const char SavedPath[] = "/tmp/cna-demos-texture2d-io.png";

typedef struct TextureIoState {
    CnaDemo* demo;
    const char* asset_path;
    const uint8_t* encoded_png;
    uint64_t encoded_size;
    int loads;
} TextureIoState;

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static CNA_Texture2DTransfer transfer(int32_t level, uint64_t start, uint64_t count)
{
    CNA_Texture2DTransfer result;
    memset(&result, 0, sizeof(result));
    result.struct_size = (uint32_t)sizeof(result);
    result.struct_version = UINT32_C(1);
    result.level = level;
    result.start_index = start;
    result.element_count = count;
    return result;
}

static int bytes_are(const void* data, size_t count, unsigned char expected)
{
    const unsigned char* bytes = (const unsigned char*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != expected) {
            return 0;
        }
    }
    return 1;
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    TextureIoState* state = (TextureIoState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle file_texture = CNA_INVALID_HANDLE;
    CNA_Handle rgba_texture = CNA_INVALID_HANDLE;
    CNA_Handle decoded = CNA_INVALID_HANDLE;
    CNA_Handle cropped = CNA_INVALID_HANDLE;
    CNA_Texture2DInfo info;
    CNA_TextureInfo common;
    CNA_Texture2DStorageInfo storage;
    CNA_Texture2DDecodeInfo decode;
    CNA_Color initial[4] = {
        {1U, 2U, 3U, 255U}, {4U, 5U, 6U, 255U},
        {7U, 8U, 9U, 255U}, {10U, 11U, 12U, 255U}
    };
    const uint8_t replacement[16] = {
        21U, 22U, 23U, 255U, 31U, 32U, 33U, 255U,
        41U, 42U, 43U, 255U, 51U, 52U, 53U, 255U
    };
    CNA_Color readback[4];
    CNA_Texture2DTransfer full = transfer(0, 0U, 4U);
    uint64_t required = 0U;
    CNA_Result result;
    (void)game_time;
    (void)out_error;

    ++state->loads;
    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "get the live graphics device for Texture2D I/O")) {
        return CNA_RESULT_SUCCESS;
    }

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    memset(&storage, 0, sizeof(storage));
    storage.struct_size = (uint32_t)sizeof(storage);
    storage.struct_version = UINT32_C(1);
    result = cna_texture2d_create_from_file_with_device(device, view(state->asset_path), &file_texture);
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_SUCCESS && file_texture != CNA_INVALID_HANDLE &&
            cna_texture2d_get_info(file_texture, &info) == CNA_RESULT_SUCCESS &&
            info.width == 64U && info.height == 64U && info.level_count == 1U &&
            info.format == CNA_SURFACE_FORMAT_COLOR &&
            cna_texture2d_get_storage_info(file_texture, &storage) == CNA_RESULT_SUCCESS &&
            storage.has_renderer == CNA_TRUE && bytes_are(storage.reserved, sizeof(storage.reserved), 0U),
        "file creation with a device publishes a 64x64 renderer-backed Color texture");

    result = cna_texture2d_create_from_rgba8(device, 2U, 2U, initial, 3U, &rgba_texture);
    cna_demo_check(state->demo,
                   result == CNA_RESULT_INVALID_ARGUMENT && rgba_texture == CNA_INVALID_HANDLE,
                   "RGBA creation refuses a pixel count that does not match its dimensions");
    cna_demo_check_result(state->demo,
                          cna_texture2d_create_from_rgba8(device, 2U, 2U, initial, 4U, &rgba_texture),
                          "create and upload a 2x2 device Texture2D in one call");
    if (rgba_texture != CNA_INVALID_HANDLE) {
        memset(&common, 0, sizeof(common));
        common.struct_size = (uint32_t)sizeof(common);
        common.struct_version = UINT32_C(1);
        memset(&storage, 0, sizeof(storage));
        storage.struct_size = (uint32_t)sizeof(storage);
        storage.struct_version = UINT32_C(1);
        cna_demo_check(
            state->demo,
            cna_texture_get_info(rgba_texture, &common) == CNA_RESULT_SUCCESS &&
                common.level_count == 1U && common.format == CNA_SURFACE_FORMAT_COLOR &&
                cna_texture2d_get_storage_info(rgba_texture, &storage) == CNA_RESULT_SUCCESS &&
                storage.has_renderer == CNA_TRUE,
            "common and storage information describe the uploaded device texture");
        memset(readback, 0, sizeof(readback));
        cna_demo_check(
            state->demo,
            cna_texture2d_set_data_rgba8_bytes(rgba_texture, replacement, 3U) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                cna_texture2d_set_data_rgba8_bytes(rgba_texture, replacement, 4U) ==
                    CNA_RESULT_SUCCESS &&
                cna_texture2d_get_data(rgba_texture, CNA_TEXTURE_DATA_COLOR, &full, readback, 4U,
                                       &required) == CNA_RESULT_SUCCESS &&
                required == 4U && memcmp(readback, replacement, sizeof(replacement)) == 0,
            "raw RGBA bytes require the exact pixel count and replace every device texel");
    }

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        cna_texture2d_create_from_encoded_memory(device, state->encoded_png, state->encoded_size,
                                                 NULL, &decoded) == CNA_RESULT_SUCCESS &&
            cna_texture2d_get_info(decoded, &info) == CNA_RESULT_SUCCESS &&
            info.width == 8U && info.height == 4U,
        "encoded-memory creation preserves the PNG dimensions without decode options");

    memset(&decode, 0, sizeof(decode));
    decode.struct_size = (uint32_t)sizeof(decode);
    decode.struct_version = UINT32_C(1);
    decode.width = 2U;
    decode.height = 3U;
    decode.zoom = CNA_TRUE;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        cna_texture2d_create_from_encoded_memory(device, state->encoded_png, state->encoded_size,
                                                 &decode, &cropped) == CNA_RESULT_SUCCESS &&
            cna_texture2d_get_info(cropped, &info) == CNA_RESULT_SUCCESS &&
            info.width == 2U && info.height == 3U,
        "zoom decoding covers and crops the encoded image to the requested dimensions");

    {
        const CNA_Result cropped_release = cropped != CNA_INVALID_HANDLE
            ? cna_texture2d_destroy(cropped) : CNA_RESULT_SUCCESS;
        const CNA_Result decoded_release = decoded != CNA_INVALID_HANDLE
            ? cna_texture2d_destroy(decoded) : CNA_RESULT_SUCCESS;
        const CNA_Result rgba_release = rgba_texture != CNA_INVALID_HANDLE
            ? cna_texture2d_destroy(rgba_texture) : CNA_RESULT_SUCCESS;
        const CNA_Result file_release = file_texture != CNA_INVALID_HANDLE
            ? cna_texture2d_destroy(file_texture) : CNA_RESULT_SUCCESS;
        cna_demo_check(state->demo,
                       cropped_release == CNA_RESULT_SUCCESS && decoded_release == CNA_RESULT_SUCCESS &&
                           rgba_release == CNA_RESULT_SUCCESS && file_release == CNA_RESULT_SUCCESS,
                       "release every device-owned Texture2D before leaving the callback");
    }
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    TextureIoState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle standalone = CNA_INVALID_HANDLE;
    CNA_Handle file_texture = CNA_INVALID_HANDLE;
    CNA_Handle saved_texture = CNA_INVALID_HANDLE;
    CNA_TextureInfo common;
    CNA_Texture2DInfo info;
    CNA_Texture2DStorageInfo storage;
    char asset_path[1024];
    uint64_t path_length = 0U;
    uint8_t* raw_pixels = NULL;
    uint8_t* png = NULL;
    uint8_t* jpeg = NULL;
    uint64_t png_size = 0U;
    uint64_t jpeg_size = 0U;
    uint64_t copied = 0U;
    CNA_Result result;

    cna_demo_init(&demo, "c_api.graphics.texture2d_io", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
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

    result = cna_title_location_copy_path(game, asset_path, sizeof(asset_path) - 64U, &path_length);
    if (result == CNA_RESULT_SUCCESS) {
        path_length += (uint64_t)snprintf(asset_path + path_length,
                                          sizeof(asset_path) - (size_t)path_length,
                                          "/assets/textures/checker.png");
    }
    state.asset_path = asset_path;
    cna_demo_check_result(&demo, result, "locate the checker PNG fixture");

    memset(&common, 0, sizeof(common));
    common.struct_size = (uint32_t)sizeof(common);
    common.struct_version = UINT32_C(1);
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    memset(&storage, 0xA5, sizeof(storage));
    storage.struct_size = (uint32_t)sizeof(storage);
    storage.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_texture2d_create_standalone(&standalone) == CNA_RESULT_SUCCESS &&
            cna_texture_get_info(standalone, &common) == CNA_RESULT_SUCCESS &&
            common.level_count == 1U && common.format == CNA_SURFACE_FORMAT_COLOR &&
            cna_texture2d_get_info(standalone, &info) == CNA_RESULT_SUCCESS &&
            info.width == 0U && info.height == 0U &&
            cna_texture2d_get_storage_info(standalone, &storage) == CNA_RESULT_SUCCESS &&
            storage.has_renderer == CNA_FALSE && storage.has_cpu_shadow == CNA_FALSE &&
            bytes_are(storage.reserved, sizeof(storage.reserved), 0U),
        "a standalone default texture has canonical Color identity and no backing storage");
    if (standalone != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_texture2d_destroy(standalone),
                              "destroy the standalone default texture");
        standalone = CNA_INVALID_HANDLE;
    }

    result = cna_texture2d_create_from_file(view(asset_path), &file_texture);
    cna_demo_check_result(&demo, result, "decode the checker PNG into a standalone texture");
    if (file_texture != CNA_INVALID_HANDLE) {
        memset(&common, 0, sizeof(common));
        common.struct_size = (uint32_t)sizeof(common);
        common.struct_version = UINT32_C(1);
        memset(&storage, 0, sizeof(storage));
        storage.struct_size = (uint32_t)sizeof(storage);
        storage.struct_version = UINT32_C(1);
        cna_demo_check(
            &demo,
            cna_texture_get_info(file_texture, &common) == CNA_RESULT_SUCCESS &&
                common.level_count == 1U && common.format == CNA_SURFACE_FORMAT_COLOR &&
                cna_texture2d_get_storage_info(file_texture, &storage) == CNA_RESULT_SUCCESS &&
                storage.has_renderer == CNA_FALSE && storage.has_cpu_shadow == CNA_TRUE,
            "the standalone file texture keeps authoritative CPU pixels without a renderer");

        raw_pixels = (uint8_t*)malloc(64U * 64U * 4U);
        if (raw_pixels != NULL) {
            uint64_t index;
            for (index = 0U; index < 64U * 64U; ++index) {
                raw_pixels[index * 4U] = 17U;
                raw_pixels[index * 4U + 1U] = 34U;
                raw_pixels[index * 4U + 2U] = 51U;
                raw_pixels[index * 4U + 3U] = 255U;
            }
        }
        cna_demo_check(
            &demo,
            raw_pixels != NULL &&
                cna_texture2d_set_data_rgba8_bytes(file_texture, raw_pixels, 64U * 64U - 1U) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                cna_texture2d_set_data_rgba8_bytes(file_texture, raw_pixels, 64U * 64U) ==
                    CNA_RESULT_SUCCESS,
            "the standalone file texture accepts only an exact full RGBA byte upload");

        {
            CNA_Texture2DTransfer region = transfer(0, 1U, 4U);
            CNA_Color patch[6] = {
                {9U, 9U, 9U, 9U},
                {201U, 1U, 2U, 255U}, {3U, 202U, 4U, 255U},
                {5U, 6U, 203U, 255U}, {204U, 205U, 7U, 255U},
                {8U, 8U, 8U, 8U}
            };
            CNA_Color output[6];
            CNA_Color before[6];
            uint64_t required = 0U;
            region.has_rectangle = CNA_TRUE;
            region.rectangle = (CNA_Rectangle){7, 9, 2, 2};
            memset(output, 0x5A, sizeof(output));
            memcpy(before, output, sizeof(before));
            cna_demo_check(
                &demo,
                cna_texture2d_set_data(file_texture, CNA_TEXTURE_DATA_COLOR, &region, patch, 6U) ==
                        CNA_RESULT_SUCCESS &&
                    cna_texture2d_get_data(file_texture, CNA_TEXTURE_DATA_COLOR, &region, output, 4U,
                                           &required) == CNA_RESULT_BUFFER_TOO_SMALL &&
                    required == 4U && memcmp(output, before, sizeof(output)) == 0,
                "a typed rectangle upload uses its caller window and short readback is atomic");
            cna_demo_check(
                &demo,
                cna_texture2d_get_data(file_texture, CNA_TEXTURE_DATA_COLOR, &region, output, 6U,
                                       &required) == CNA_RESULT_SUCCESS &&
                    required == 4U && memcmp(&output[1], &patch[1], 4U * sizeof(CNA_Color)) == 0 &&
                    memcmp(&output[0], &before[0], sizeof(CNA_Color)) == 0 &&
                    memcmp(&output[5], &before[5], sizeof(CNA_Color)) == 0,
                "typed rectangle readback writes only the selected caller-array window");
        }

        cna_demo_check(
            &demo,
            cna_texture2d_get_encoded_byte_count(file_texture, CNA_TEXTURE_IMAGE_FORMAT_PNG, 8U, 4U,
                                                 &png_size) == CNA_RESULT_SUCCESS &&
                png_size >= 8U &&
                cna_texture2d_get_encoded_byte_count(file_texture, CNA_TEXTURE_IMAGE_FORMAT_JPEG, 8U, 4U,
                                                     &jpeg_size) == CNA_RESULT_SUCCESS &&
                jpeg_size >= 2U,
            "PNG and JPEG encoders report nonempty resized output sizes");
        png = (uint8_t*)malloc((size_t)png_size);
        jpeg = (uint8_t*)malloc((size_t)jpeg_size);
        if (png != NULL) {
            memset(png, 0xA5, (size_t)png_size);
        }
        cna_demo_check(
            &demo,
            png != NULL && jpeg != NULL &&
                cna_texture2d_copy_encoded(file_texture, CNA_TEXTURE_IMAGE_FORMAT_PNG, 8U, 4U, png,
                                           png_size - 1U, &copied) == CNA_RESULT_BUFFER_TOO_SMALL &&
                copied == png_size && bytes_are(png, (size_t)png_size, 0xA5U) &&
                cna_texture2d_copy_encoded(file_texture, CNA_TEXTURE_IMAGE_FORMAT_PNG, 8U, 4U, png,
                                           png_size, &copied) == CNA_RESULT_SUCCESS &&
                copied == png_size &&
                memcmp(png, (uint8_t[8]){0x89U, 'P', 'N', 'G', 0x0dU, 0x0aU, 0x1aU, 0x0aU}, 8U) == 0 &&
                cna_texture2d_copy_encoded(file_texture, CNA_TEXTURE_IMAGE_FORMAT_JPEG, 8U, 4U, jpeg,
                                           jpeg_size, &copied) == CNA_RESULT_SUCCESS &&
                copied == jpeg_size && jpeg[0] == 0xFFU && jpeg[1] == 0xD8U,
            "encoded copies are atomic and carry the exact PNG and JPEG signatures");

        (void)remove(SavedPath);
        cna_demo_check_result(&demo,
                              cna_texture2d_save_file(file_texture, CNA_TEXTURE_IMAGE_FORMAT_PNG,
                                                      view(SavedPath)),
                              "save the standalone texture as a PNG file");
        memset(&info, 0, sizeof(info));
        info.struct_size = (uint32_t)sizeof(info);
        info.struct_version = UINT32_C(1);
        cna_demo_check(
            &demo,
            cna_texture2d_create_from_file(view(SavedPath), &saved_texture) == CNA_RESULT_SUCCESS &&
                cna_texture2d_get_info(saved_texture, &info) == CNA_RESULT_SUCCESS &&
                info.width == 64U && info.height == 64U && info.format == CNA_SURFACE_FORMAT_COLOR,
            "the saved PNG decodes again with the original dimensions and format");
        if (saved_texture != CNA_INVALID_HANDLE) {
            cna_demo_check_result(&demo, cna_texture2d_destroy(saved_texture),
                                  "destroy the PNG round-trip texture");
            saved_texture = CNA_INVALID_HANDLE;
        }
        (void)remove(SavedPath);
        cna_demo_check_result(&demo, cna_texture2d_destroy(file_texture),
                              "destroy the standalone file texture");
        file_texture = CNA_INVALID_HANDLE;
    }

    state.encoded_png = png;
    state.encoded_size = png_size;
    cna_demo_check_result(&demo, cna_game_run_one_frame(game), "run the device-backed I/O callback");
    cna_demo_check(&demo, state.loads == 1, "the Texture2D I/O callback ran exactly once");

    if (saved_texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(saved_texture);
    }
    if (file_texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(file_texture);
    }
    if (standalone != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(standalone);
    }
    (void)remove(SavedPath);
    free(jpeg);
    free(png);
    free(raw_pixels);
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
