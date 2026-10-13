/* SPDX-License-Identifier: MS-PL */
/*
 * Graphics-device clear overloads from C. An explicit Depth24Stencil8 back buffer lets the
 * options overload exercise all three planes; RGBA8 readback proves the color result after each
 * accepted call and proves refused or empty masks preserve the last established color.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ClearState {
    CnaDemo* demo;
    int frames;
} ClearState;

static int color_equal(CNA_Color left, CNA_Color right)
{
    return left.r == right.r && left.g == right.g && left.b == right.b &&
        left.a == right.a;
}

static int backbuffer_has_color(
    CnaDemo* demo, CNA_Handle device, CNA_BackBufferInfo info, CNA_Color expected,
    const char* message)
{
    const uint64_t pixel_count = (uint64_t)info.width * (uint64_t)info.height;
    CNA_Color* pixels = (CNA_Color*)malloc((size_t)pixel_count * sizeof(CNA_Color));
    uint64_t copied = 0U;
    CNA_Result result;
    int matches = 0;
    if (pixels == NULL) {
        return cna_demo_check(demo, 0, message);
    }
    memset(pixels, 0x5A, (size_t)pixel_count * sizeof(CNA_Color));
    result = cna_graphics_device_get_backbuffer_data_rgba8(
        device, pixels, pixel_count, &copied);
    if (result == CNA_RESULT_SUCCESS && copied == pixel_count && pixel_count != 0U) {
        const uint64_t last = pixel_count - 1U;
        const uint64_t center = ((uint64_t)info.height / 2U) * info.width + info.width / 2U;
        matches = color_equal(pixels[0], expected) &&
            color_equal(pixels[info.width - 1U], expected) &&
            color_equal(pixels[last - info.width + 1U], expected) &&
            color_equal(pixels[last], expected) && color_equal(pixels[center], expected);
        printf("  sampled clear color: %u,%u,%u,%u across %ux%u\n",
               (unsigned)pixels[center].r, (unsigned)pixels[center].g,
               (unsigned)pixels[center].b, (unsigned)pixels[center].a,
               (unsigned)info.width, (unsigned)info.height);
    } else if (result != CNA_RESULT_SUCCESS) {
        cna_demo_print_result_diagnostic("read the cleared back buffer", result);
    }
    free(pixels);
    return cna_demo_check(demo, result == CNA_RESULT_SUCCESS && copied == pixel_count && matches,
                          message);
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    ClearState* state = (ClearState*)context;
    (void)game_time;
    (void)out_error;
    if (state->demo->frames > 0 && state->frames >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    ClearState* state = (ClearState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_BackBufferInfo backbuffer = {
        sizeof(CNA_BackBufferInfo), UINT32_C(1), 0U, 0U, 0U, 0U};
    CNA_PresentationParameters presentation;
    const CNA_Color red = {255U, 0U, 0U, 255U};
    const CNA_Color green = {0U, 255U, 0U, 255U};
    const CNA_Color blue = {0U, 0U, 255U, 255U};
    const CNA_Color yellow = {255U, 255U, 0U, 255U};
    CNA_Result result;
    (void)game_time;
    (void)out_error;

    ++state->frames;
    if (state->frames != 1) {
        return CNA_RESULT_SUCCESS;
    }

    memset(&presentation, 0, sizeof(presentation));
    presentation.struct_size = (uint32_t)sizeof(presentation);
    presentation.struct_version = UINT32_C(1);
    result = cna_game_get_graphics_device(game, &device);
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_backbuffer_info(device, &backbuffer) == CNA_RESULT_SUCCESS &&
            backbuffer.width > 0U && backbuffer.height > 0U &&
            backbuffer.format == CNA_SURFACE_FORMAT_COLOR &&
            cna_graphics_device_get_presentation_parameters(device, &presentation) ==
                CNA_RESULT_SUCCESS &&
            presentation.depth_stencil_format == CNA_DEPTH_FORMAT_DEPTH24_STENCIL8,
        "borrow a Color back buffer with the requested depth and stencil planes");
    if (result != CNA_RESULT_SUCCESS || backbuffer.width == 0U || backbuffer.height == 0U) {
        return CNA_RESULT_SUCCESS;
    }

    cna_demo_check_result(
        state->demo, cna_graphics_device_clear_rgba(device, 1.0F, 0.0F, 0.0F, 1.0F),
        "clear the color buffer through four normalized RGBA channels");
    backbuffer_has_color(
        state->demo, device, backbuffer, red,
        "RGBA clear writes the exact red color across representative back-buffer pixels");

    cna_demo_check(
        state->demo,
        cna_graphics_device_clear_rgba(device, NAN, 0.0F, 0.0F, 1.0F) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_device_clear_rgba(device, 0.0F, 0.0F, INFINITY, 1.0F) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "RGBA clear rejects non-finite channels");
    backbuffer_has_color(
        state->demo, device, backbuffer, red,
        "rejected RGBA clears leave the established color untouched");

    cna_demo_check_result(
        state->demo, cna_graphics_device_clear_color_depth(device, green, 0.25F),
        "clear the Color target and its depth buffer together");
    backbuffer_has_color(
        state->demo, device, backbuffer, green,
        "color/depth clear writes the exact supplied Color value");
    cna_demo_check(
        state->demo,
        cna_graphics_device_clear_color_depth(device, yellow, NAN) ==
            CNA_RESULT_INVALID_ARGUMENT,
        "color/depth clear rejects a non-finite depth before clearing");
    backbuffer_has_color(
        state->demo, device, backbuffer, green,
        "the rejected depth value preserves the established color buffer");

    cna_demo_check_result(
        state->demo,
        cna_graphics_device_clear_options(
            device,
            CNA_CLEAR_OPTION_TARGET | CNA_CLEAR_OPTION_DEPTH_BUFFER |
                CNA_CLEAR_OPTION_STENCIL,
            blue, 0.75F, 19),
        "clear the selected color, depth and stencil planes with one bit mask");
    backbuffer_has_color(
        state->demo, device, backbuffer, blue,
        "the all-plane options clear writes the exact selected target color");

    cna_demo_check_result(
        state->demo, cna_graphics_device_clear_options(device, 0U, yellow, 0.5F, 7),
        "an empty clear-options mask is an accepted no-op");
    backbuffer_has_color(
        state->demo, device, backbuffer, blue,
        "the empty options mask leaves the color buffer untouched");
    cna_demo_check(
        state->demo,
        cna_graphics_device_clear_options(device, UINT32_C(8), yellow, 0.5F, 7) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_device_clear_options(
                device, CNA_CLEAR_OPTION_TARGET, yellow, INFINITY, 7) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "clear options reject unknown bits and non-finite depth");
    backbuffer_has_color(
        state->demo, device, backbuffer, blue,
        "rejected options clears preserve the last complete clear result");
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    ClearState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_Result setup;

    cna_demo_init(&demo, "c_api.graphics.device_clears", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
    callbacks.draw = draw;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_FALSE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }
    setup = cna_graphics_device_manager_create(game, &manager);
    if (setup == CNA_RESULT_SUCCESS) {
        setup = cna_graphics_device_manager_set_graphics_profile(
            manager, CNA_GRAPHICS_PROFILE_HI_DEF);
    }
    if (setup == CNA_RESULT_SUCCESS) {
        setup = cna_graphics_device_manager_set_preferred_depth_stencil_format(
            manager, CNA_DEPTH_FORMAT_DEPTH24_STENCIL8);
    }
    if (setup == CNA_RESULT_SUCCESS) {
        setup = cna_graphics_device_manager_apply_changes(manager);
    }
    if (cna_demo_check_result(
            &demo, setup, "create a HiDef device with a depth/stencil back buffer")) {
        cna_demo_check_result(
            &demo, cna_game_run(game), "run the one-frame clear/readback sequence");
        cna_demo_check(
            &demo, state.frames == 1, "the clear audit ran in exactly one draw callback");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
