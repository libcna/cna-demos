/* SPDX-License-Identifier: MS-PL */
/*
 * Graphics-adapter capability and limitation queries from C. The adapter preference mutation is
 * deliberately made before device creation and both original flags are restored immediately.
 * Adapter refresh needs a callback-scoped borrowed device, so one short game run demonstrates its
 * explicit NOT_SUPPORTED contract without invalidating the active device's adapter reference.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct AdapterCapabilityState {
    CnaDemo* demo;
    int loads;
    CNA_Result borrow_result;
    CNA_Result refresh_result;
} AdapterCapabilityState;

static int selection_is_exact(const CNA_GraphicsFormatSelection* selection)
{
    return selection->struct_size == sizeof(*selection) &&
        selection->struct_version == UINT32_C(1) && selection->exact_match == CNA_TRUE &&
        selection->reserved[0] == 0U && selection->reserved[1] == 0U &&
        selection->reserved[2] == 0U && selection->format == CNA_SURFACE_FORMAT_COLOR &&
        selection->depth_format == CNA_DEPTH_FORMAT_NONE && selection->multi_sample_count == 0;
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    AdapterCapabilityState* state = (AdapterCapabilityState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    (void)game_time;
    (void)out_error;
    ++state->loads;
    state->borrow_result = cna_game_get_graphics_device(game, &graphics_device);
    state->refresh_result = state->borrow_result == CNA_RESULT_SUCCESS
        ? cna_graphics_adapters_refresh(graphics_device)
        : CNA_RESULT_INVALID_STATE;
    cna_demo_check(
        state->demo,
        state->borrow_result == CNA_RESULT_SUCCESS &&
            state->refresh_result == CNA_RESULT_NOT_SUPPORTED,
        "a callback-scoped device reports adapter-cache refresh as explicitly unsupported");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    AdapterCapabilityState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_GraphicsAdapterInfo original;
    CNA_GraphicsAdapterInfo changed;
    CNA_GraphicsAdapterInfo restored;
    CNA_GraphicsFormatSelection selection;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_Bool reach_supported = CNA_FALSE;
    CNA_Bool alternate_null;
    CNA_NativeHandleValue monitor = UINT64_C(73);
    CNA_Result result;
    int manager_ready = 0;
    int original_read;

    cna_demo_init(&demo, "c_api.graphics.adapter_capabilities", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.borrow_result = CNA_RESULT_INVALID_STATE;
    state.refresh_result = CNA_RESULT_INVALID_STATE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_FALSE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }

    memset(&original, 0, sizeof(original));
    original.struct_size = (uint32_t)sizeof(original);
    original.struct_version = UINT32_C(1);
    original_read = cna_demo_check(
        &demo,
        cna_graphics_adapter_get_info(game, 0U, &original) == CNA_RESULT_SUCCESS &&
            original.use_null_device <= CNA_TRUE && original.use_reference_device <= CNA_TRUE,
        "read both original device-preference flags before changing the default adapter");

    alternate_null = original.use_null_device == CNA_TRUE ? CNA_FALSE : CNA_TRUE;
    result = original_read
        ? cna_graphics_adapter_set_device_preferences(
              game, 0U, alternate_null, original.use_reference_device)
        : CNA_RESULT_INVALID_STATE;
    cna_demo_check(&demo, result == CNA_RESULT_SUCCESS,
                   "set one preference while preserving the other flag exactly");
    memset(&changed, 0, sizeof(changed));
    changed.struct_size = (uint32_t)sizeof(changed);
    changed.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_graphics_adapter_get_info(game, 0U, &changed) == CNA_RESULT_SUCCESS &&
            changed.use_null_device == alternate_null &&
            changed.use_reference_device == original.use_reference_device,
        "the adapter snapshot exposes the requested pair of preference flags");

    result = original_read
        ? cna_graphics_adapter_set_device_preferences(
              game, 0U, original.use_null_device, original.use_reference_device)
        : CNA_RESULT_INVALID_STATE;
    cna_demo_check(&demo, result == CNA_RESULT_SUCCESS,
                   "restore both original preference flags before creating a device");
    memset(&restored, 0, sizeof(restored));
    restored.struct_size = (uint32_t)sizeof(restored);
    restored.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_graphics_adapter_get_info(game, 0U, &restored) == CNA_RESULT_SUCCESS &&
            restored.use_null_device == original.use_null_device &&
            restored.use_reference_device == original.use_reference_device,
        "a fresh snapshot confirms exact preference restoration");

    cna_demo_check(
        &demo,
        cna_graphics_adapter_is_profile_supported(
            game, 0U, CNA_GRAPHICS_PROFILE_REACH, &reach_supported) == CNA_RESULT_SUCCESS &&
            reach_supported == CNA_TRUE,
        "the default adapter supports the baseline Reach graphics profile");

    memset(&selection, 0xA5, sizeof(selection));
    selection.struct_size = (uint32_t)sizeof(selection);
    selection.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_graphics_adapter_query_render_target_format(
            game, 0U, CNA_GRAPHICS_PROFILE_REACH, CNA_SURFACE_FORMAT_COLOR,
            CNA_DEPTH_FORMAT_NONE, 0, &selection) == CNA_RESULT_SUCCESS &&
            selection_is_exact(&selection),
        "Reach accepts the baseline Color render target exactly and returns a complete selection");

    memset(&selection, 0x5A, sizeof(selection));
    selection.struct_size = (uint32_t)sizeof(selection);
    selection.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_graphics_adapter_query_backbuffer_format(
            game, 0U, CNA_GRAPHICS_PROFILE_REACH, CNA_SURFACE_FORMAT_COLOR,
            CNA_DEPTH_FORMAT_NONE, 0, &selection) == CNA_RESULT_SUCCESS &&
            selection_is_exact(&selection),
        "Reach accepts the baseline Color backbuffer exactly and returns a complete selection");

    cna_demo_check(
        &demo,
        cna_graphics_adapter_get_native_monitor_handle(game, 0U, &monitor) ==
                CNA_RESULT_NOT_SUPPORTED &&
            monitor == UINT64_C(0),
        "the stable C ABI explicitly withholds native monitor handles and zeroes the output");

    manager_ready = cna_demo_check_result(
        &demo, cna_demo_use_hidef(game, &manager),
        "create the graphics device used for the callback-scoped refresh contract");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through load and the immediate exit request");
        cna_demo_check(&demo, state.loads == 1, "the adapter refresh check ran in exactly one load callback");
    }

    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    printf("  original preferences: null=%u reference=%u\n",
           (unsigned)original.use_null_device, (unsigned)original.use_reference_device);
    return cna_demo_finish(&demo);
}
