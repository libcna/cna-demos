/* SPDX-License-Identifier: MS-PL */
/*
 * Graphics-adapter discovery from C: a game handle can enumerate the system adapters before its
 * first frame or graphics-device loan. The demo copies exact UTF-8 metadata and display-mode
 * arrays, exercises both format-filter forms and checks atomic short-buffer/index refusal.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int all_bytes_are(const void* data, size_t count, unsigned char value)
{
    const unsigned char* bytes = (const unsigned char*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != value) {
            return 0;
        }
    }
    return 1;
}

static int mode_is_coherent(const CNA_DisplayMode* mode)
{
    return mode->struct_size == sizeof(*mode) && mode->struct_version == UINT32_C(1) &&
        mode->width > 0 && mode->height > 0 && mode->format <= CNA_SURFACE_FORMAT_USHORT_EXT &&
        fabsf(mode->aspect_ratio - (float)mode->width / (float)mode->height) < 1e-5f;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_GraphicsAdapterInfo info;
    CNA_GraphicsAdapterInfo refused_info;
    CNA_DisplayMode current;
    CNA_DisplayMode refused_mode;
    CNA_DisplayMode* modes = NULL;
    CNA_DisplayMode* filtered = NULL;
    CNA_DisplayMode* short_modes = NULL;
    uint64_t adapter_count = UINT64_C(0);
    uint64_t mode_count = UINT64_C(0);
    uint64_t filtered_count = UINT64_C(0);
    uint64_t reported = UINT64_C(0);
    char* description = NULL;
    char* device_name = NULL;
    char* short_text = NULL;
    int ok;
    uint64_t index;

    cna_demo_init(&demo, "c_api.graphics.adapter_snapshot", argc, argv, 0);
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }

    cna_demo_check(
        &demo,
        cna_graphics_adapter_get_count(game, &adapter_count) == CNA_RESULT_SUCCESS &&
            adapter_count > UINT64_C(0) && adapter_count <= UINT32_MAX,
        "the game handle enumerates at least one system adapter before its first frame");
    printf("  adapters: %llu\n", (unsigned long long)adapter_count);

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        adapter_count > UINT64_C(0) &&
            cna_graphics_adapter_get_info(game, 0U, &info) == CNA_RESULT_SUCCESS &&
            info.struct_size == sizeof(info) && info.struct_version == UINT32_C(1) &&
            info.adapter_index == 0U && info.is_default_adapter == CNA_TRUE &&
            info.is_wide_screen <= CNA_TRUE && info.use_null_device <= CNA_TRUE &&
            info.use_reference_device <= CNA_TRUE && info.description_byte_length > UINT64_C(0) &&
            info.device_name_byte_length > UINT64_C(0),
        "adapter zero is a complete default-adapter metadata snapshot with nonempty names");

    description = (char*)malloc((size_t)info.description_byte_length + 1U);
    device_name = (char*)malloc((size_t)info.device_name_byte_length + 1U);
    if (description == NULL || device_name == NULL) {
        cna_demo_check(&demo, 0, "allocate adapter metadata buffers");
        goto cleanup;
    }
    memset(description, '#', (size_t)info.description_byte_length + 1U);
    reported = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_graphics_adapter_copy_description(
            game, 0U, description, info.description_byte_length, &reported) == CNA_RESULT_SUCCESS &&
            reported == info.description_byte_length &&
            description[info.description_byte_length] == '#',
        "the description copies exactly its reported unterminated UTF-8 bytes");
    description[info.description_byte_length] = '\0';

    short_text = (char*)malloc((size_t)info.description_byte_length + 1U);
    if (short_text == NULL) {
        cna_demo_check(&demo, 0, "allocate the short description buffer");
        goto cleanup;
    }
    memset(short_text, 0x5A, (size_t)info.description_byte_length + 1U);
    reported = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_graphics_adapter_copy_description(
            game, 0U, short_text, info.description_byte_length - 1U, &reported) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            reported == info.description_byte_length &&
            all_bytes_are(short_text, (size_t)info.description_byte_length + 1U, 0x5A),
        "a short description buffer reports the exact requirement without a partial write");
    free(short_text);
    short_text = NULL;

    memset(device_name, '#', (size_t)info.device_name_byte_length + 1U);
    reported = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_graphics_adapter_copy_device_name(
            game, 0U, device_name, info.device_name_byte_length, &reported) == CNA_RESULT_SUCCESS &&
            reported == info.device_name_byte_length &&
            device_name[info.device_name_byte_length] == '#',
        "the device name copies exactly its reported unterminated UTF-8 bytes");
    device_name[info.device_name_byte_length] = '\0';

    short_text = (char*)malloc((size_t)info.device_name_byte_length + 1U);
    if (short_text == NULL) {
        cna_demo_check(&demo, 0, "allocate the short device-name buffer");
        goto cleanup;
    }
    memset(short_text, 0x6B, (size_t)info.device_name_byte_length + 1U);
    reported = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_graphics_adapter_copy_device_name(
            game, 0U, short_text, info.device_name_byte_length - 1U, &reported) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            reported == info.device_name_byte_length &&
            all_bytes_are(short_text, (size_t)info.device_name_byte_length + 1U, 0x6B),
        "a short device-name buffer reports the exact requirement without a partial write");
    free(short_text);
    short_text = NULL;
    printf("  default: '%s' (%s)\n", description, device_name);

    current = (CNA_DisplayMode){sizeof(CNA_DisplayMode), UINT32_C(1), 0, 0, 0.0f, 0U};
    cna_demo_check(
        &demo,
        cna_graphics_adapter_get_current_display_mode(game, 0U, &current) ==
                CNA_RESULT_SUCCESS &&
            mode_is_coherent(&current) &&
            info.is_wide_screen ==
                (CNA_Bool)((int64_t)current.width * INT64_C(5) >
                                   (int64_t)current.height * INT64_C(8)
                               ? CNA_TRUE
                               : CNA_FALSE),
        "the current display mode is coherent with the adapter's widescreen flag");
    printf("  current: %dx%d format %u, aspect %.3f\n", current.width, current.height,
           (unsigned)current.format, (double)current.aspect_ratio);

    if (!cna_demo_check(
            &demo,
            cna_graphics_adapter_get_display_mode_count(
                game, 0U, CNA_FALSE, CNA_SURFACE_FORMAT_COLOR, &mode_count) ==
                    CNA_RESULT_SUCCESS &&
                mode_count > UINT64_C(0),
            "the unfiltered supported-mode collection is nonempty")) {
        goto cleanup;
    }
    short_modes = (CNA_DisplayMode*)malloc((size_t)(mode_count + 1U) * sizeof(*short_modes));
    modes = (CNA_DisplayMode*)malloc((size_t)(mode_count + 1U) * sizeof(*modes));
    if (short_modes == NULL || modes == NULL) {
        cna_demo_check(&demo, 0, "allocate supported-mode buffers");
        goto cleanup;
    }
    memset(short_modes, 0x4C, (size_t)(mode_count + 1U) * sizeof(*short_modes));
    reported = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_graphics_adapter_copy_display_modes(
            game, 0U, CNA_FALSE, CNA_SURFACE_FORMAT_COLOR, short_modes, mode_count - 1U,
            &reported) == CNA_RESULT_BUFFER_TOO_SMALL &&
            reported == mode_count &&
            all_bytes_are(
                short_modes, (size_t)(mode_count + 1U) * sizeof(*short_modes), 0x4C),
        "a short mode array reports the element count without a partial write");

    memset(modes, 0x3D, (size_t)(mode_count + 1U) * sizeof(*modes));
    reported = UINT64_C(0);
    ok = cna_graphics_adapter_copy_display_modes(
             game, 0U, CNA_FALSE, CNA_SURFACE_FORMAT_COLOR, modes, mode_count, &reported) ==
            CNA_RESULT_SUCCESS &&
        reported == mode_count &&
        all_bytes_are(&modes[mode_count], sizeof(modes[mode_count]), 0x3D);
    for (index = UINT64_C(0); ok && index < mode_count; ++index) {
        ok = mode_is_coherent(&modes[index]);
    }
    cna_demo_check(
        &demo, ok,
        "the exact mode array contains complete coherent values and preserves its guard element");

    cna_demo_check(
        &demo,
        cna_graphics_adapter_get_display_mode_count(
            game, 0U, CNA_TRUE, CNA_SURFACE_FORMAT_COLOR, &filtered_count) ==
                CNA_RESULT_SUCCESS &&
            filtered_count <= mode_count,
        "the Color-filtered count is bounded by the complete mode count");
    if (filtered_count > UINT64_C(0)) {
        filtered = (CNA_DisplayMode*)malloc((size_t)filtered_count * sizeof(*filtered));
    }
    if (filtered_count > UINT64_C(0) && filtered == NULL) {
        cna_demo_check(&demo, 0, "allocate the filtered mode buffer");
        goto cleanup;
    }
    reported = UINT64_C(99);
    ok = cna_graphics_adapter_copy_display_modes(
             game, 0U, CNA_TRUE, CNA_SURFACE_FORMAT_COLOR, filtered, filtered_count, &reported) ==
            CNA_RESULT_SUCCESS &&
        reported == filtered_count;
    for (index = UINT64_C(0); ok && index < filtered_count; ++index) {
        ok = mode_is_coherent(&filtered[index]) &&
            filtered[index].format == CNA_SURFACE_FORMAT_COLOR;
    }
    cna_demo_check(
        &demo, ok,
        "the filtered copy contains exactly the Color display modes, including an empty result");
    printf("  supported modes: %llu total, %llu Color\n", (unsigned long long)mode_count,
           (unsigned long long)filtered_count);

    memset(&refused_info, 0x2A, sizeof(refused_info));
    refused_info.struct_size = (uint32_t)sizeof(refused_info);
    refused_info.struct_version = UINT32_C(1);
    memset(&refused_mode, 0x2B, sizeof(refused_mode));
    refused_mode.struct_size = (uint32_t)sizeof(refused_mode);
    refused_mode.struct_version = UINT32_C(1);
    memset(short_modes, 0x2C, sizeof(*short_modes));
    reported = UINT64_C(77);
    ok = cna_graphics_adapter_get_info(game, (uint32_t)adapter_count, &refused_info) ==
            CNA_RESULT_INVALID_ARGUMENT &&
        cna_graphics_adapter_copy_description(
            game, (uint32_t)adapter_count, NULL, 0U, &reported) == CNA_RESULT_INVALID_ARGUMENT &&
        reported == UINT64_C(77) &&
        cna_graphics_adapter_copy_device_name(
            game, (uint32_t)adapter_count, NULL, 0U, &reported) == CNA_RESULT_INVALID_ARGUMENT &&
        reported == UINT64_C(77) &&
        cna_graphics_adapter_get_current_display_mode(
            game, (uint32_t)adapter_count, &refused_mode) == CNA_RESULT_INVALID_ARGUMENT &&
        cna_graphics_adapter_get_display_mode_count(
            game, (uint32_t)adapter_count, CNA_FALSE, CNA_SURFACE_FORMAT_COLOR, &reported) ==
            CNA_RESULT_INVALID_ARGUMENT &&
        reported == UINT64_C(77) &&
        cna_graphics_adapter_copy_display_modes(
            game, (uint32_t)adapter_count, CNA_FALSE, CNA_SURFACE_FORMAT_COLOR, short_modes, 1U,
            &reported) == CNA_RESULT_INVALID_ARGUMENT &&
        reported == UINT64_C(77);
    cna_demo_check(
        &demo,
        ok && refused_info.adapter_index == UINT32_C(0x2A2A2A2A) &&
            refused_mode.width == (int32_t)UINT32_C(0x2B2B2B2B) &&
            all_bytes_are(short_modes, sizeof(*short_modes), 0x2C),
        "the index equal to adapter count is refused by every snapshot route before output changes");

cleanup:
    free(short_text);
    free(filtered);
    free(short_modes);
    free(modes);
    free(device_name);
    free(description);
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
