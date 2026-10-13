/* SPDX-License-Identifier: MS-PL */
/* GraphicsDeviceInformation and every GraphicsDeviceManager preference through the public C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct State {
    CnaDemo* demo;
    CNA_GraphicsDeviceManagerHandle manager;
    int validated;
} State;

static CNA_Result validate_preferences(
    CNA_Handle game,
    const CNA_GameTime* game_time,
    void* context,
    CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    CNA_GraphicsDeviceInformation information;
    CNA_GraphicsDeviceInformation clone;
    CNA_GraphicsDeviceManagerHandle second = CNA_INVALID_HANDLE;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_GraphicsProfile profile = UINT32_MAX;
    CNA_SurfaceFormat surface = UINT32_MAX;
    CNA_DepthFormat depth = UINT32_MAX;
    CNA_DisplayOrientation orientations = UINT32_MAX;
    CNA_PresentationMode mode = UINT32_MAX;
    CNA_Bool flag = UINT8_C(99);
    CNA_Bool original = CNA_FALSE;
    int32_t value = INT32_MIN;
    (void)game_time;
    (void)out_error;

    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;

    memset(&information, 0xA5, sizeof(information));
    cna_demo_check(
        state->demo,
        cna_graphics_device_information_init(&information) == CNA_RESULT_SUCCESS &&
            information.struct_size == (uint32_t)sizeof(information) &&
            information.struct_version == UINT32_C(1) &&
            information.presentation_parameters.struct_version == UINT32_C(1) &&
            information.adapter_index < 0,
        "device-information initialization produces a versioned configuration with no adapter");
    information.graphics_profile = CNA_GRAPHICS_PROFILE_HI_DEF;
    information.presentation_parameters.back_buffer_width = 1024;
    information.presentation_parameters.back_buffer_height = 576;
    memset(&clone, 0, sizeof(clone));
    cna_demo_check(
        state->demo,
        cna_graphics_device_information_clone(&information, &clone) == CNA_RESULT_SUCCESS &&
            clone.adapter_index == information.adapter_index &&
            clone.graphics_profile == CNA_GRAPHICS_PROFILE_HI_DEF &&
            clone.presentation_parameters.back_buffer_width == 1024 &&
            clone.presentation_parameters.back_buffer_height == 576,
        "cloning copies the adapter, profile and nested presentation value");
    clone.presentation_parameters.back_buffer_width = 320;
    cna_demo_check(
        state->demo,
        information.presentation_parameters.back_buffer_width == 1024 &&
            cna_graphics_device_information_clone(&clone, &clone) == CNA_RESULT_SUCCESS &&
            clone.presentation_parameters.back_buffer_width == 320,
        "the cloned value is independent and supports a validated self-copy");
    clone = information;
    clone.presentation_parameters.is_full_screen = (CNA_Bool)2;
    cna_demo_check(
        state->demo,
        cna_graphics_device_information_clone(&clone, &information) == CNA_RESULT_INVALID_ARGUMENT &&
            information.presentation_parameters.back_buffer_width == 1024,
        "invalid nested presentation data is rejected without changing the destination");

    if (!cna_demo_check_result(
            state->demo, cna_graphics_device_manager_create(game, &state->manager),
            "create and register the graphics device manager")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_create(game, &second) == CNA_RESULT_INVALID_ARGUMENT &&
            second == CNA_INVALID_HANDLE,
        "one game refuses a second graphics device manager");
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_get_graphics_device(state->manager, &device) ==
                CNA_RESULT_SUCCESS && device != CNA_INVALID_HANDLE,
        "inside the game callback the manager borrows the initialized game device");

    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_get_graphics_profile(state->manager, &profile) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_set_graphics_profile(
                state->manager, profile == CNA_GRAPHICS_PROFILE_HI_DEF ? CNA_GRAPHICS_PROFILE_REACH
                                                                      : CNA_GRAPHICS_PROFILE_HI_DEF) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_get_graphics_profile(state->manager, &profile) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_set_graphics_profile(state->manager, UINT32_C(99)) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "the requested profile round-trips while an undefined profile is refused atomically");
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_get_is_full_screen(state->manager, &original) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_set_is_full_screen(
                state->manager, original == CNA_TRUE ? CNA_FALSE : CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_get_is_full_screen(state->manager, &flag) ==
                CNA_RESULT_SUCCESS && flag != original &&
            cna_graphics_device_manager_set_is_full_screen(state->manager, original) ==
                CNA_RESULT_SUCCESS,
        "the full-screen preference changes and is restored before application");
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_get_prefer_multi_sampling(state->manager, &original) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_set_prefer_multi_sampling(
                state->manager, original == CNA_TRUE ? CNA_FALSE : CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_get_prefer_multi_sampling(state->manager, &flag) ==
                CNA_RESULT_SUCCESS && flag != original &&
            cna_graphics_device_manager_set_prefer_multi_sampling(state->manager, original) ==
                CNA_RESULT_SUCCESS,
        "the multisampling preference round-trips independently of device support");
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_get_preferred_back_buffer_format(state->manager, &surface) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_set_preferred_back_buffer_format(
                state->manager, CNA_SURFACE_FORMAT_COLOR) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_get_preferred_back_buffer_format(state->manager, &surface) ==
                CNA_RESULT_SUCCESS && surface == CNA_SURFACE_FORMAT_COLOR &&
            cna_graphics_device_manager_set_preferred_back_buffer_format(state->manager, UINT32_C(9999)) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "the preferred color format accepts Color and rejects an undefined identity");
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_get_preferred_depth_stencil_format(state->manager, &depth) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_set_preferred_depth_stencil_format(
                state->manager, CNA_DEPTH_FORMAT_DEPTH24_STENCIL8) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_get_preferred_depth_stencil_format(state->manager, &depth) ==
                CNA_RESULT_SUCCESS && depth == CNA_DEPTH_FORMAT_DEPTH24_STENCIL8 &&
            cna_graphics_device_manager_set_preferred_depth_stencil_format(state->manager, UINT32_C(99)) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "the preferred depth format round-trips and rejects an undefined identity");
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_set_preferred_back_buffer_width(state->manager, -7) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_get_preferred_back_buffer_width(state->manager, &value) ==
                CNA_RESULT_SUCCESS && value == -7 &&
            cna_graphics_device_manager_set_preferred_back_buffer_width(state->manager, 640) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_set_preferred_back_buffer_height(state->manager, 360) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_get_preferred_back_buffer_height(state->manager, &value) ==
                CNA_RESULT_SUCCESS && value == 360,
        "width records even a non-presentable request while height and the final width remain caller choices");
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_get_synchronize_with_vertical_retrace(state->manager, &original) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_set_synchronize_with_vertical_retrace(
                state->manager, original == CNA_TRUE ? CNA_FALSE : CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_get_synchronize_with_vertical_retrace(state->manager, &flag) ==
                CNA_RESULT_SUCCESS && flag != original &&
            cna_graphics_device_manager_set_synchronize_with_vertical_retrace(state->manager, original) ==
                CNA_RESULT_SUCCESS,
        "vertical-retrace synchronization changes and returns to its original preference");
    orientations = CNA_DISPLAY_ORIENTATION_LANDSCAPE_LEFT | CNA_DISPLAY_ORIENTATION_PORTRAIT;
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_set_supported_orientations(state->manager, orientations) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_get_supported_orientations(state->manager, &orientations) ==
                CNA_RESULT_SUCCESS &&
            orientations == (CNA_DISPLAY_ORIENTATION_LANDSCAPE_LEFT | CNA_DISPLAY_ORIENTATION_PORTRAIT),
        "supported orientations preserve a bit-set rather than forcing one identity");
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_get_preferred_presentation_mode_ext(state->manager, &mode) ==
                CNA_RESULT_SUCCESS && mode <= CNA_PRESENTATION_MODE_MAXIMUM &&
            cna_graphics_device_manager_set_preferred_presentation_mode_ext(
                state->manager, CNA_PRESENTATION_MODE_STRETCH) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_get_preferred_presentation_mode_ext(state->manager, &mode) ==
                CNA_RESULT_SUCCESS && mode == CNA_PRESENTATION_MODE_STRETCH &&
            cna_graphics_device_manager_set_preferred_presentation_mode_ext(
                state->manager, CNA_PRESENTATION_MODE_MAXIMUM + 1U) == CNA_RESULT_INVALID_ARGUMENT,
        "presentation mode round-trips Stretch and atomically rejects an undefined mode");
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    State state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.game.graphics_preferences", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.manager = CNA_INVALID_HANDLE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = validate_preferences;
    callbacks.context = &state;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.callbacks = &callbacks;

    if (cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the preference game")) {
        cna_demo_check_result(&demo, cna_game_run_one_frame(game), "run the preference audit callback");
        cna_demo_check(&demo, state.validated == 1, "the one-shot preference audit ran once");
    }
    if (state.manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(state.manager),
                              "release the graphics device manager");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the preference game");
    }
    return cna_demo_finish(&demo);
}
