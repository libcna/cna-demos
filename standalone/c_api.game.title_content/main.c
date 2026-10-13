/* SPDX-License-Identifier: MS-PL */
/* TitleLocation ownership and whole-file TitleContainer reading through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView view(const char* text, uint64_t bytes)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = bytes;
    return result;
}

int main(int argc, char** argv)
{
    static const char AssetSuffix[] = "/assets";
    static const char AssetName[] = "textures/checker.png";
    static const char MissingName[] = "textures/cna-demos-missing-title-file.bin";
    static const uint8_t PngSignature[8] = {
        UINT8_C(0x89), UINT8_C(0x50), UINT8_C(0x4e), UINT8_C(0x47),
        UINT8_C(0x0d), UINT8_C(0x0a), UINT8_C(0x1a), UINT8_C(0x0a)
    };
    CnaDemo demo;
    CNA_GameCreateInfo info = {0};
    CNA_Handle game = CNA_INVALID_HANDLE;
    uint64_t original_bytes = UINT64_MAX;
    uint64_t copied_bytes = UINT64_MAX;
    uint64_t asset_root_bytes = 0U;
    uint64_t file_bytes = UINT64_MAX;
    char original_path[1024];
    char asset_root[1032];
    char copied_path[1032];
    char before_path[1032];
    uint8_t short_data[32];
    uint8_t before_data[32];
    uint8_t* file_data = 0;
    int root_changed = 0;

    cna_demo_init(&demo, "c_api.game.title_content", argc, argv, 1);
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.target_elapsed_time_ticks = INT64_C(166667);

    if (!cna_demo_check_result(
            &demo, cna_game_create(&info, &game), "create the title-content owner game")) {
        return cna_demo_finish(&demo);
    }

    if (!cna_demo_check(
            &demo,
            cna_title_location_get_path_size(game, &original_bytes) == CNA_RESULT_SUCCESS &&
                original_bytes > 0U && original_bytes < sizeof(original_path) &&
                cna_title_location_copy_path(
                    game, original_path, original_bytes, &copied_bytes) == CNA_RESULT_SUCCESS &&
                copied_bytes == original_bytes,
            "TitleLocation exposes its exact unterminated executable-directory path")) {
        goto cleanup;
    }

    memset(copied_path, 0xA5, sizeof(copied_path));
    memcpy(before_path, copied_path, sizeof(copied_path));
    cna_demo_check(
        &demo,
        cna_title_location_copy_path(
            game, copied_path, original_bytes - 1U, &copied_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL && copied_bytes == original_bytes &&
            memcmp(copied_path, before_path, sizeof(copied_path)) == 0,
        "a short TitleLocation destination is refused without a partial path");

    if (!cna_demo_check(
            &demo,
            original_bytes + sizeof(AssetSuffix) - 1U < sizeof(asset_root),
            "the deployed asset-root path fits the bounded demo buffer")) {
        goto cleanup;
    }
    memcpy(asset_root, original_path, (size_t)original_bytes);
    memcpy(asset_root + original_bytes, AssetSuffix, sizeof(AssetSuffix) - 1U);
    asset_root_bytes = original_bytes + sizeof(AssetSuffix) - 1U;

    if (!cna_demo_check(
            &demo,
            cna_title_location_set_path_ext(
                game, view(asset_root, asset_root_bytes)) == CNA_RESULT_SUCCESS,
            "TitleLocation accepts the deployed assets directory as its process root")) {
        goto cleanup;
    }
    root_changed = 1;

    memset(copied_path, 0xA5, sizeof(copied_path));
    cna_demo_check(
        &demo,
        cna_title_location_get_path_size(game, &copied_bytes) == CNA_RESULT_SUCCESS &&
            copied_bytes == asset_root_bytes &&
            cna_title_location_copy_path(
                game, copied_path, asset_root_bytes, &copied_bytes) == CNA_RESULT_SUCCESS &&
            memcmp(copied_path, asset_root, (size_t)asset_root_bytes) == 0 &&
            (unsigned char)copied_path[asset_root_bytes] == 0xA5U,
        "the process-wide root reads back as the exact authored bytes");

    memset(short_data, 0xA5, sizeof(short_data));
    memcpy(before_data, short_data, sizeof(short_data));
    cna_demo_check(
        &demo,
        cna_title_container_read_ext(
            game, view(AssetName, sizeof(AssetName) - 1U), 0, 0U, &file_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL && file_bytes == 256U &&
            cna_title_container_read_ext(
                game, view(AssetName, sizeof(AssetName) - 1U), short_data,
                sizeof(short_data), &copied_bytes) == CNA_RESULT_BUFFER_TOO_SMALL &&
            copied_bytes == file_bytes && memcmp(short_data, before_data, sizeof(short_data)) == 0,
        "title-file sizing and a short destination report 256 bytes without a partial read");

    file_data = (uint8_t*)malloc((size_t)file_bytes);
    cna_demo_check(
        &demo,
        file_data != 0 &&
            cna_title_container_read_ext(
                game, view(AssetName, sizeof(AssetName) - 1U), file_data,
                file_bytes, &copied_bytes) == CNA_RESULT_SUCCESS &&
            copied_bytes == file_bytes && file_bytes == 256U &&
            memcmp(file_data, PngSignature, sizeof(PngSignature)) == 0,
        "TitleContainer returns the complete deployed checker PNG into caller storage");

    copied_bytes = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_title_container_read_ext(
            game, view(MissingName, sizeof(MissingName) - 1U), short_data,
            sizeof(short_data), &copied_bytes) == CNA_RESULT_IO,
        "a missing title file is reported as an I/O failure rather than empty content");

cleanup:
    free(file_data);
    if (root_changed) {
        cna_demo_check(
            &demo,
            cna_title_location_set_path_ext(
                game, view(original_path, original_bytes)) == CNA_RESULT_SUCCESS &&
                cna_title_location_get_path_size(game, &copied_bytes) == CNA_RESULT_SUCCESS &&
                copied_bytes == original_bytes,
            "the original process-wide TitleLocation root is restored");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the title-content owner game");
    return cna_demo_finish(&demo);
}
