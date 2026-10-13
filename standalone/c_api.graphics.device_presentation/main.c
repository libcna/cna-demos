/* SPDX-License-Identifier: MS-PL */
/*
 * Applied graphics-device presentation state from C. A callback-scoped device changes its
 * presentation interval, proves the changed value through a fresh read, and restores every
 * caller-visible original field before the callback returns. The same loan supplies the current
 * display mode and the explicit stable-ABI refusal for a device-owned native-window answer.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct DevicePresentationState {
    CnaDemo* demo;
    int loads;
} DevicePresentationState;

static int parameters_are_equal(const CNA_PresentationParameters* left,
                                const CNA_PresentationParameters* right)
{
    return left->struct_size == right->struct_size &&
        left->struct_version == right->struct_version &&
        left->back_buffer_format == right->back_buffer_format &&
        left->back_buffer_width == right->back_buffer_width &&
        left->back_buffer_height == right->back_buffer_height &&
        left->depth_stencil_format == right->depth_stencil_format &&
        left->multi_sample_count == right->multi_sample_count &&
        left->presentation_interval == right->presentation_interval &&
        left->display_orientation == right->display_orientation &&
        left->render_target_usage == right->render_target_usage &&
        left->is_full_screen == right->is_full_screen &&
        left->headless_ext == right->headless_ext &&
        left->reserved[0] == right->reserved[0] && left->reserved[1] == right->reserved[1];
}

static int parameters_are_complete(const CNA_PresentationParameters* parameters)
{
    return parameters->struct_size == sizeof(*parameters) &&
        parameters->struct_version == UINT32_C(1) && parameters->back_buffer_width > 0 &&
        parameters->back_buffer_height > 0 &&
        parameters->back_buffer_format <= CNA_SURFACE_FORMAT_USHORT_EXT &&
        parameters->depth_stencil_format <= CNA_DEPTH_FORMAT_DEPTH24_STENCIL8 &&
        parameters->multi_sample_count >= 0 &&
        parameters->presentation_interval <= CNA_PRESENT_INTERVAL_IMMEDIATE &&
        (parameters->display_orientation & ~UINT32_C(7)) == 0U &&
        parameters->render_target_usage <= CNA_RENDER_TARGET_USAGE_PLATFORM_CONTENTS &&
        parameters->is_full_screen <= CNA_TRUE &&
        parameters->headless_ext <= CNA_TRUE && parameters->reserved[0] == 0U &&
        parameters->reserved[1] == 0U;
}

static int mode_is_coherent(const CNA_DisplayMode* mode)
{
    return mode->struct_size == sizeof(*mode) && mode->struct_version == UINT32_C(1) &&
        mode->width > 0 && mode->height > 0 && mode->format <= CNA_SURFACE_FORMAT_USHORT_EXT &&
        fabsf(mode->aspect_ratio - (float)mode->width / (float)mode->height) < 1e-5f;
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    DevicePresentationState* state = (DevicePresentationState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_PresentationParameters original;
    CNA_PresentationParameters changed;
    CNA_PresentationParameters observed;
    CNA_PresentationParameters restored;
    CNA_DisplayMode mode;
    CNA_NativeHandleValue window = UINT64_C(91);
    CNA_Result borrow_result;
    CNA_Result get_result;
    CNA_Result set_result;
    CNA_Result observe_result;
    CNA_Result restore_result;
    CNA_Result verify_result;
    int original_read;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    borrow_result = cna_game_get_graphics_device(game, &graphics_device);
    cna_demo_check(state->demo, borrow_result == CNA_RESULT_SUCCESS,
                   "borrow the live graphics device for this callback");

    memset(&original, 0, sizeof(original));
    original.struct_size = (uint32_t)sizeof(original);
    original.struct_version = UINT32_C(1);
    get_result = borrow_result == CNA_RESULT_SUCCESS
        ? cna_graphics_device_get_presentation_parameters(graphics_device, &original)
        : CNA_RESULT_INVALID_STATE;
    original_read = cna_demo_check(
        state->demo,
        get_result == CNA_RESULT_SUCCESS && parameters_are_complete(&original),
        "read complete applied presentation parameters from the live device");

    changed = original;
    changed.presentation_interval = original.presentation_interval == CNA_PRESENT_INTERVAL_IMMEDIATE
        ? CNA_PRESENT_INTERVAL_ONE
        : CNA_PRESENT_INTERVAL_IMMEDIATE;
    set_result = original_read
        ? cna_graphics_device_set_presentation_parameters(graphics_device, &changed)
        : CNA_RESULT_INVALID_STATE;
    memset(&observed, 0, sizeof(observed));
    observed.struct_size = (uint32_t)sizeof(observed);
    observed.struct_version = UINT32_C(1);
    observe_result = set_result == CNA_RESULT_SUCCESS
        ? cna_graphics_device_get_presentation_parameters(graphics_device, &observed)
        : CNA_RESULT_INVALID_STATE;
    cna_demo_check(
        state->demo,
        set_result == CNA_RESULT_SUCCESS && observe_result == CNA_RESULT_SUCCESS &&
            parameters_are_equal(&changed, &observed),
        "a changed presentation interval is applied and every other public field is preserved");

    restore_result = original_read
        ? cna_graphics_device_set_presentation_parameters(graphics_device, &original)
        : CNA_RESULT_INVALID_STATE;
    memset(&restored, 0, sizeof(restored));
    restored.struct_size = (uint32_t)sizeof(restored);
    restored.struct_version = UINT32_C(1);
    verify_result = restore_result == CNA_RESULT_SUCCESS
        ? cna_graphics_device_get_presentation_parameters(graphics_device, &restored)
        : CNA_RESULT_INVALID_STATE;
    cna_demo_check(
        state->demo,
        restore_result == CNA_RESULT_SUCCESS && verify_result == CNA_RESULT_SUCCESS &&
            parameters_are_equal(&original, &restored),
        "the exact original presentation parameters are restored before the callback returns");

    memset(&mode, 0, sizeof(mode));
    mode.struct_size = (uint32_t)sizeof(mode);
    mode.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        borrow_result == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_display_mode(graphics_device, &mode) == CNA_RESULT_SUCCESS &&
            mode_is_coherent(&mode),
        "the active device exposes a complete coherent current display mode");
    if (mode.width > 0 && mode.height > 0) {
        printf("  device mode: %dx%d format %u, aspect %.3f\n", mode.width, mode.height,
               (unsigned)mode.format, (double)mode.aspect_ratio);
    }

    cna_demo_check(
        state->demo,
        borrow_result == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_native_window_handle(graphics_device, &window) ==
                CNA_RESULT_NOT_SUPPORTED &&
            window == UINT64_C(0),
        "the device-level stable ABI withholds a native window handle and zeroes the output");
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
    DevicePresentationState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.device_presentation", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
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

    manager_ready = cna_demo_check_result(
        &demo, cna_demo_use_hidef(game, &manager),
        "create the graphics device whose applied presentation state is audited");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through load and the immediate exit request");
        cna_demo_check(&demo, state.loads == 1,
                       "the presentation audit ran in exactly one load callback");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
