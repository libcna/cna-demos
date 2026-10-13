/* SPDX-License-Identifier: MS-PL */
/* Content path/key resolution, device association and manifest inspection through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct State {
    CnaDemo* demo;
    CNA_Handle manager;
    int validated;
} State;

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static int ends_with_path(const char* text, size_t length, const char* suffix)
{
    size_t suffix_length = strlen(suffix);
    size_t i;
    if (suffix_length > length) {
        return 0;
    }
    text += length - suffix_length;
    for (i = 0; i < suffix_length; ++i) {
        char actual = text[i] == '\\' ? '/' : text[i];
        if (actual != suffix[i]) {
            return 0;
        }
    }
    return 1;
}

static CNA_Result inspect_content(
    CNA_Handle game,
    const CNA_GameTime* game_time,
    void* context,
    CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    CNA_ContentManagerCreateInfo create_info;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle associated = CNA_INVALID_HANDLE;
    CNA_Bool has_provider = CNA_TRUE;
    uint64_t bytes = 0U;
    uint64_t copied = 0U;
    uint64_t entries = 0U;
    uint64_t usages = 0U;
    uint64_t i;
    int saw_native = 0;
    int saw_xnb = 0;
    char text[1024];
    char before[1024];
    (void)game_time;
    (void)out_error;

    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "borrow the game graphics device")) {
        return CNA_RESULT_SUCCESS;
    }
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.root_directory = view("assets");
    if (!cna_demo_check_result(
            state->demo, cna_content_manager_create(device, &create_info, &state->manager),
            "create a content manager rooted at deployed assets")) {
        return CNA_RESULT_SUCCESS;
    }

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_content_manager_get_asset_path_size(
            state->manager, view("textures/checker.png"), &bytes) == CNA_RESULT_SUCCESS &&
            bytes < sizeof(text) &&
            cna_content_manager_copy_asset_path(
                state->manager, view("textures/checker.png"), text, bytes, &copied) ==
                CNA_RESULT_SUCCESS && copied == bytes &&
            ends_with_path(text, (size_t)bytes, "/assets/textures/checker.png") &&
            (unsigned char)text[bytes] == 0xA5U,
        "asset path size and copy return exact unterminated bytes under the content root");
    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_content_manager_copy_asset_path(
            state->manager, view("textures/checker.png"), text, bytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL && copied == bytes &&
            memcmp(text, before, sizeof(text)) == 0,
        "a short asset-path destination is reported without a partial write");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_content_manager_get_normalized_key_size(
            state->manager, view("Textures\\CHECKER.PNG"), &bytes) == CNA_RESULT_SUCCESS &&
            cna_content_manager_copy_normalized_key(
                state->manager, view("Textures\\CHECKER.PNG"), text, bytes, &copied) ==
                CNA_RESULT_SUCCESS && copied == bytes && bytes == strlen("textures/checker.png") &&
            memcmp(text, "textures/checker.png", (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "normalized cache keys fold case and path separators into exact bytes");
    cna_demo_check(
        state->demo,
        cna_content_manager_register_builtin_loaders(state->manager) == CNA_RESULT_SUCCESS &&
            cna_content_manager_register_builtin_loaders(state->manager) == CNA_RESULT_SUCCESS,
        "registering built-in content loaders is idempotent");
    cna_demo_check(
        state->demo,
        cna_content_manager_get_has_service_provider(state->manager, &has_provider) ==
                CNA_RESULT_SUCCESS && has_provider == CNA_FALSE &&
            cna_content_manager_get_graphics_device(state->manager, &associated) ==
                CNA_RESULT_SUCCESS && associated == device &&
            cna_content_manager_set_graphics_device(state->manager, device) == CNA_RESULT_SUCCESS &&
            cna_content_manager_set_graphics_device(state->manager, CNA_INVALID_HANDLE) ==
                CNA_RESULT_INVALID_HANDLE,
        "a C manager has no native service provider and retains its same-game graphics device");

    cna_demo_check_result(
        state->demo, cna_content_manager_refresh_content_manifest(state->manager),
        "refresh the point-in-time content manifest");
    if (!cna_demo_check(
            state->demo,
            cna_content_manager_get_manifest_entry_count(state->manager, &entries) ==
                    CNA_RESULT_SUCCESS && entries > 0U,
            "the deployed asset tree produces at least one manifest entry")) {
        return CNA_RESULT_SUCCESS;
    }
    for (i = 0U; i < entries; ++i) {
        CNA_ContentManifestEntryInfo entry;
        memset(&entry, 0, sizeof(entry));
        entry.struct_size = (uint32_t)sizeof(entry);
        entry.struct_version = UINT32_C(1);
        memset(text, 0, sizeof(text));
        if (cna_content_manager_get_manifest_entry(state->manager, i, &entry) != CNA_RESULT_SUCCESS ||
            cna_content_manager_copy_manifest_relative_path(
                state->manager, i, text, sizeof(text) - 1U, &bytes) != CNA_RESULT_SUCCESS ||
            bytes == 0U || bytes >= sizeof(text) ||
            (entry.has_xnb != CNA_FALSE && entry.has_xnb != CNA_TRUE) ||
            (entry.has_cnj != CNA_FALSE && entry.has_cnj != CNA_TRUE)) {
            continue;
        }
        if (!saw_native && entry.native_extension_count > 0U) {
            memset(text, 0, sizeof(text));
            if (cna_content_manager_copy_manifest_native_extension(
                    state->manager, i, 0U, text, sizeof(text) - 1U, &bytes) == CNA_RESULT_SUCCESS &&
                bytes > 1U && text[0] == '.') {
                saw_native = 1;
            }
        }
        if (!saw_xnb && entry.xnb_reader_name_count > 0U) {
            memset(text, 0, sizeof(text));
            if (cna_content_manager_copy_manifest_xnb_reader_name(
                    state->manager, i, 0U, text, sizeof(text) - 1U, &bytes) == CNA_RESULT_SUCCESS &&
                bytes > 0U) {
                saw_xnb = 1;
            }
        }
    }
    cna_demo_check(
        state->demo, saw_native && saw_xnb,
        "manifest entries expose both a native extension and an XNB reader name");
    {
        CNA_ContentManifestEntryInfo rejected;
        memset(&rejected, 0, sizeof(rejected));
        rejected.struct_size = (uint32_t)sizeof(rejected);
        rejected.struct_version = UINT32_C(1);
        cna_demo_check(
            state->demo,
            cna_content_manager_get_manifest_entry(state->manager, entries, &rejected) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                cna_content_manager_copy_manifest_relative_path(
                    state->manager, entries, text, sizeof(text), &bytes) ==
                    CNA_RESULT_INVALID_ARGUMENT,
            "manifest indices stop exactly at the reported entry count");
    }

    cna_demo_check(
        state->demo,
        cna_content_manager_get_xnb_reader_usage_count(state->manager, &usages) ==
                CNA_RESULT_SUCCESS && usages > 0U,
        "the deployed XNB files produce a non-empty reader-usage summary");
    if (usages > 0U) {
        CNA_ContentReaderUsageInfo usage;
        memset(&usage, 0, sizeof(usage));
        usage.struct_size = (uint32_t)sizeof(usage);
        usage.struct_version = UINT32_C(1);
        memset(text, 0, sizeof(text));
        cna_demo_check(
            state->demo,
            cna_content_manager_get_xnb_reader_usage(state->manager, 0U, &usage) ==
                    CNA_RESULT_SUCCESS && usage.file_count > 0U &&
                (usage.is_registered == CNA_FALSE || usage.is_registered == CNA_TRUE) &&
                cna_content_manager_copy_xnb_reader_usage_name(
                    state->manager, 0U, text, sizeof(text) - 1U, &bytes) == CNA_RESULT_SUCCESS &&
                bytes > 0U,
            "the first sorted reader-usage row has a name, registration state and file count");
    }
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    State state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.content.manager_inspection", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.manager = CNA_INVALID_HANDLE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = inspect_content;
    callbacks.context = &state;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.callbacks = &callbacks;

    if (cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the content-inspection game")) {
        cna_demo_check_result(&demo, cna_game_run_one_frame(game), "run its content inspection callback");
        cna_demo_check(&demo, state.validated == 1, "the content inspection ran once");
    }
    if (state.manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_manager_destroy(state.manager),
                              "destroy the inspected content manager");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the content-inspection game");
    }
    return cna_demo_finish(&demo);
}
