/* SPDX-License-Identifier: MS-PL */
/* CRT, color-depth and ASCII post-processing through the public CNA C graphics extension API. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct RetroEffectsState {
    CnaDemo* demo;
    int loads;
} RetroEffectsState;

static int near_float(float left, float right)
{
    return fabsf(left - right) <= 0.0001F;
}

static void check_crt_effect(RetroEffectsState* state, CNA_Handle device)
{
    CNA_EffectHandle effect = CNA_INVALID_HANDLE;
    CNA_Result created = cna_crt_effect_create(device, &effect);
    float value = -1.0F;
    CNA_CRTMaskType mask = UINT32_MAX;

    cna_demo_check(
        state->demo,
        (created == CNA_RESULT_SUCCESS && effect != CNA_INVALID_HANDLE) ||
            (created == CNA_RESULT_NOT_SUPPORTED && effect == CNA_INVALID_HANDLE),
        "CRT creation succeeds or explicitly reports missing shader-effect support");
    if (created != CNA_RESULT_SUCCESS) {
        return;
    }

    cna_demo_check(
        state->demo,
        cna_crt_effect_get_scanline_intensity(effect, &value) == CNA_RESULT_SUCCESS &&
            near_float(value, 0.3F) &&
            cna_crt_effect_get_curvature(effect, &value) == CNA_RESULT_SUCCESS &&
            near_float(value, 0.08F) &&
            cna_crt_effect_get_vignette_intensity(effect, &value) == CNA_RESULT_SUCCESS &&
            near_float(value, 0.25F) &&
            cna_crt_effect_get_mask_intensity(effect, &value) == CNA_RESULT_SUCCESS &&
            near_float(value, 0.35F) &&
            cna_crt_effect_get_mask_type(effect, &mask) == CNA_RESULT_SUCCESS &&
            mask == CNA_CRT_MASK_TYPE_APERTURE_GRILLE,
        "CRT defaults describe scanlines, curvature, vignette and an aperture-grille mask");

    cna_demo_check(
        state->demo,
        cna_crt_effect_set_scanline_intensity(effect, 0.75F) == CNA_RESULT_SUCCESS &&
            cna_crt_effect_set_curvature(effect, 5.0F) == CNA_RESULT_SUCCESS &&
            cna_crt_effect_set_vignette_intensity(effect, -3.0F) == CNA_RESULT_SUCCESS &&
            cna_crt_effect_set_mask_intensity(effect, 0.5F) == CNA_RESULT_SUCCESS &&
            cna_crt_effect_set_mask_type(effect, CNA_CRT_MASK_TYPE_SHADOW_MASK) ==
                CNA_RESULT_SUCCESS &&
            cna_crt_effect_get_scanline_intensity(effect, &value) == CNA_RESULT_SUCCESS &&
            near_float(value, 0.75F) &&
            cna_crt_effect_get_curvature(effect, &value) == CNA_RESULT_SUCCESS &&
            near_float(value, 1.0F) &&
            cna_crt_effect_get_vignette_intensity(effect, &value) == CNA_RESULT_SUCCESS &&
            near_float(value, 0.0F) &&
            cna_crt_effect_get_mask_intensity(effect, &value) == CNA_RESULT_SUCCESS &&
            near_float(value, 0.5F) &&
            cna_crt_effect_get_mask_type(effect, &mask) == CNA_RESULT_SUCCESS &&
            mask == CNA_CRT_MASK_TYPE_SHADOW_MASK,
        "CRT scalar setters clamp to zero through one while the mask mode changes independently");

    cna_demo_check(
        state->demo,
        cna_crt_effect_set_scanline_intensity(effect, NAN) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_crt_effect_set_curvature(effect, INFINITY) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_crt_effect_set_vignette_intensity(effect, NAN) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_crt_effect_set_mask_intensity(effect, -INFINITY) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_crt_effect_set_mask_type(effect, UINT32_C(9)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_crt_effect_get_scanline_intensity(effect, &value) == CNA_RESULT_SUCCESS &&
            near_float(value, 0.75F) &&
            cna_crt_effect_get_mask_type(effect, &mask) == CNA_RESULT_SUCCESS &&
            mask == CNA_CRT_MASK_TYPE_SHADOW_MASK,
        "nonfinite CRT scalars and an unknown mask are refused without changing retained state");

    cna_demo_check(
        state->demo,
        cna_crt_effect_get_scanline_intensity(effect, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_crt_effect_get_curvature(effect, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_crt_effect_get_vignette_intensity(effect, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_crt_effect_get_mask_intensity(effect, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_crt_effect_get_mask_type(effect, 0) == CNA_RESULT_INVALID_ARGUMENT,
        "every CRT getter rejects a null output");
    cna_demo_check_result(state->demo, cna_effect_destroy(effect),
                          "destroy the ordinary Effect handle backing CRT state");
}

static void check_depth_effect(RetroEffectsState* state, CNA_Handle device)
{
    CNA_EffectHandle effect = CNA_INVALID_HANDLE;
    CNA_Result created = cna_depth_effect_create(device, &effect);
    CNA_DepthEffectMode mode = UINT32_MAX;
    CNA_DitherMode dither = UINT32_MAX;
    CNA_DepthEffectMode candidate_mode;
    CNA_DitherMode candidate_dither;
    int modes_ok = 1;
    int dithers_ok = 1;

    cna_demo_check(
        state->demo,
        (created == CNA_RESULT_SUCCESS && effect != CNA_INVALID_HANDLE) ||
            (created == CNA_RESULT_NOT_SUPPORTED && effect == CNA_INVALID_HANDLE),
        "depth-effect creation succeeds or explicitly reports missing shader-effect support");
    if (created != CNA_RESULT_SUCCESS) {
        return;
    }

    cna_demo_check(
        state->demo,
        cna_depth_effect_get_mode(effect, &mode) == CNA_RESULT_SUCCESS &&
            mode == CNA_DEPTH_EFFECT_MODE_COLOR_16_BIT &&
            cna_depth_effect_get_dither_mode(effect, &dither) == CNA_RESULT_SUCCESS &&
            dither == CNA_DITHER_MODE_NONE,
        "depth reduction starts at RGB565 without ordered dithering");

    for (candidate_mode = CNA_DEPTH_EFFECT_MODE_COLOR_16_BIT;
         candidate_mode <= CNA_DEPTH_EFFECT_MODE_PALETTE_16; ++candidate_mode) {
        modes_ok = modes_ok &&
            cna_depth_effect_set_mode(effect, candidate_mode) == CNA_RESULT_SUCCESS &&
            cna_depth_effect_get_mode(effect, &mode) == CNA_RESULT_SUCCESS &&
            mode == candidate_mode;
    }
    cna_demo_check(state->demo, modes_ok,
                   "all seven RGB, grayscale and fixed-palette depth modes round-trip");

    for (candidate_dither = CNA_DITHER_MODE_NONE;
         candidate_dither <= CNA_DITHER_MODE_BAYER_8X8; ++candidate_dither) {
        dithers_ok = dithers_ok &&
            cna_depth_effect_set_dither_mode(effect, candidate_dither) == CNA_RESULT_SUCCESS &&
            cna_depth_effect_get_dither_mode(effect, &dither) == CNA_RESULT_SUCCESS &&
            dither == candidate_dither;
    }
    cna_demo_check(state->demo, dithers_ok,
                   "none, Bayer4x4 and Bayer8x8 dither modes round-trip distinctly");

    cna_demo_check(
        state->demo,
        cna_depth_effect_set_mode(effect, UINT32_C(9)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_depth_effect_set_dither_mode(effect, UINT32_C(9)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_depth_effect_get_mode(effect, &mode) == CNA_RESULT_SUCCESS &&
            mode == CNA_DEPTH_EFFECT_MODE_PALETTE_16 &&
            cna_depth_effect_get_dither_mode(effect, &dither) == CNA_RESULT_SUCCESS &&
            dither == CNA_DITHER_MODE_BAYER_8X8,
        "unknown depth and dither modes are refused without changing the last valid pair");
    cna_demo_check(
        state->demo,
        cna_depth_effect_get_mode(effect, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_depth_effect_get_dither_mode(effect, 0) == CNA_RESULT_INVALID_ARGUMENT,
        "both depth-effect getters reject a null output");
    cna_demo_check_result(state->demo, cna_effect_destroy(effect),
                          "destroy the ordinary Effect handle backing depth state");
}

static void check_ascii_effect(RetroEffectsState* state, CNA_Handle device)
{
    const CNA_Texture2DCreateInfo texture_info = {
        sizeof(CNA_Texture2DCreateInfo), UINT32_C(1), 16U, 16U,
        CNA_FALSE, {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR
    };
    CNA_AsciiPostProcessEffectHandle effect = CNA_INVALID_HANDLE;
    CNA_Handle source = CNA_INVALID_HANDLE;
    CNA_Color pixels[256];
    CNA_AsciiQuantizeMode mode = UINT32_MAX;
    CNA_Rectangle destination = {0, 0, 32, 16};
    int32_t width = -1;
    int32_t height = -1;
    int32_t columns = -1;
    int32_t rows = -1;
    uint32_t index;

    if (!cna_demo_check_result(
            state->demo,
            cna_ascii_post_process_effect_create(device, &effect),
            "create the renderer-independent ASCII post-process")) {
        return;
    }
    cna_demo_check(
        state->demo,
        cna_ascii_post_process_effect_get_cell_size(effect, &width, &height) ==
                CNA_RESULT_SUCCESS && width == 8 && height == 8 &&
            cna_ascii_post_process_effect_get_quantize_mode(effect, &mode) ==
                CNA_RESULT_SUCCESS &&
            (mode == CNA_ASCII_QUANTIZE_MODE_BLACK_WHITE ||
             mode == CNA_ASCII_QUANTIZE_MODE_COLOR) &&
            cna_ascii_post_process_effect_get_last_grid_dimensions(
                effect, &columns, &rows) == CNA_RESULT_SUCCESS &&
            columns == 0 && rows == 0,
        "ASCII defaults use 8x8 cells and report no grid before the first draw");

    cna_demo_check(
        state->demo,
        cna_ascii_post_process_effect_set_cell_size(effect, 4, 2) == CNA_RESULT_SUCCESS &&
            cna_ascii_post_process_effect_set_quantize_mode(
                effect, CNA_ASCII_QUANTIZE_MODE_COLOR) == CNA_RESULT_SUCCESS &&
            cna_ascii_post_process_effect_get_cell_size(effect, &width, &height) ==
                CNA_RESULT_SUCCESS && width == 4 && height == 2 &&
            cna_ascii_post_process_effect_get_quantize_mode(effect, &mode) ==
                CNA_RESULT_SUCCESS && mode == CNA_ASCII_QUANTIZE_MODE_COLOR,
        "ASCII cell dimensions and colored-glyph quantization mutate independently");

    cna_demo_check(
        state->demo,
        cna_ascii_post_process_effect_set_cell_size(effect, 0, 2) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_ascii_post_process_effect_set_cell_size(effect, 4, -1) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_ascii_post_process_effect_set_quantize_mode(effect, UINT32_C(9)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_ascii_post_process_effect_get_cell_size(effect, &width, &height) ==
                CNA_RESULT_SUCCESS && width == 4 && height == 2 &&
            cna_ascii_post_process_effect_get_quantize_mode(effect, &mode) ==
                CNA_RESULT_SUCCESS && mode == CNA_ASCII_QUANTIZE_MODE_COLOR,
        "invalid ASCII dimensions and mode are refused without changing retained state");
    cna_demo_check(
        state->demo,
        cna_ascii_post_process_effect_get_cell_size(effect, 0, &height) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_ascii_post_process_effect_get_cell_size(effect, &width, 0) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_ascii_post_process_effect_get_quantize_mode(effect, 0) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_ascii_post_process_effect_get_last_grid_dimensions(effect, 0, &rows) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_ascii_post_process_effect_get_last_grid_dimensions(effect, &columns, 0) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "ASCII state and grid getters reject every null output position");

    if (!cna_demo_check_result(state->demo, cna_texture2d_create(device, &texture_info, &source),
                               "create the authored ASCII source texture")) {
        (void)cna_ascii_post_process_effect_destroy(effect);
        return;
    }
    for (index = 0U; index < 256U; ++index) {
        const uint8_t level = (uint8_t)index;
        pixels[index] = (CNA_Color){level, (uint8_t)(255U - level),
                                    (uint8_t)((index * 17U) & 0xffU), 255U};
    }
    cna_demo_check_result(state->demo, cna_texture2d_set_data_rgba8(source, pixels, 256U),
                          "upload a deterministic color/luminance field for ASCII quantization");
    cna_demo_check(
        state->demo,
        cna_ascii_post_process_effect_draw(effect, source, &destination) == CNA_RESULT_SUCCESS &&
            cna_ascii_post_process_effect_get_last_grid_dimensions(
                effect, &columns, &rows) == CNA_RESULT_SUCCESS &&
            columns == 4 && rows == 8,
        "explicit-destination drawing quantizes the 16x16 source into the exact 4x8 glyph grid");

    cna_demo_check(
        state->demo,
        cna_ascii_post_process_effect_set_cell_size(effect, 8, 8) == CNA_RESULT_SUCCESS &&
            cna_ascii_post_process_effect_set_quantize_mode(
                effect, CNA_ASCII_QUANTIZE_MODE_BLACK_WHITE) == CNA_RESULT_SUCCESS &&
            cna_ascii_post_process_effect_draw(effect, source, 0) == CNA_RESULT_SUCCESS &&
            cna_ascii_post_process_effect_get_last_grid_dimensions(
                effect, &columns, &rows) == CNA_RESULT_SUCCESS &&
            columns == 2 && rows == 2,
        "whole-viewport monochrome drawing re-quantizes the source into an exact 2x2 grid");

    cna_demo_check_result(state->demo, cna_texture2d_destroy(source),
                          "destroy the caller-owned ASCII source texture");
    cna_demo_check(
        state->demo,
        cna_ascii_post_process_effect_draw(effect, source, 0) == CNA_RESULT_INVALID_HANDLE,
        "ASCII drawing rejects the stale source texture handle");
    cna_demo_check(
        state->demo,
        cna_ascii_post_process_effect_destroy(effect) == CNA_RESULT_SUCCESS &&
            cna_ascii_post_process_effect_destroy(effect) == CNA_RESULT_INVALID_HANDLE,
        "ASCII destruction releases its owned handle exactly once");
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    RetroEffectsState* state = (RetroEffectsState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    (void)game_time;
    (void)out_error;
    ++state->loads;
    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "get the device for retro graphics effects")) {
        return CNA_RESULT_SUCCESS;
    }
    check_crt_effect(state, device);
    check_depth_effect(state, device);
    check_ascii_effect(state, device);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    RetroEffectsState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.graphics.retro_effects", argc, argv, 1);
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
    if (cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager),
                              "create a HiDef graphics device")) {
        cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                              "run the retro-effect validation callback");
    }
    cna_demo_check(&demo, state.loads == 1, "the retro-effect callback ran exactly once");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "cna_graphics_device_manager_destroy");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
