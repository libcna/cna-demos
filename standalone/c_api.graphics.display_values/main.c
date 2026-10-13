/* SPDX-License-Identifier: MS-PL */
/*
 * Display values from C: DisplayMode computes its aspect ratio and compares its defining fields;
 * PresentationParameters supplies XNA-compatible defaults, clones its native XNA fields and
 * derives zero-origin back-buffer bounds. No game or graphics device is needed.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int nearly_equal(float left, float right)
{
    return fabsf(left - right) < 1e-6f;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_DisplayMode full_hd;
    CNA_DisplayMode same;
    CNA_DisplayMode zero_height;
    CNA_Bool equal = CNA_FALSE;
    CNA_PresentationParameters defaults;
    CNA_PresentationParameters requested;
    CNA_PresentationParameters clone;
    CNA_Rectangle bounds;

    cna_demo_init(&demo, "c_api.graphics.display_values", argc, argv, 0);

    cna_demo_check(
        &demo,
        cna_display_mode_init(1920, 1080, CNA_SURFACE_FORMAT_COLOR, &full_hd) ==
                CNA_RESULT_SUCCESS &&
            full_hd.struct_size == sizeof(full_hd) && full_hd.struct_version == UINT32_C(1) &&
            full_hd.width == 1920 && full_hd.height == 1080 &&
            nearly_equal(full_hd.aspect_ratio, 16.0f / 9.0f) &&
            full_hd.format == CNA_SURFACE_FORMAT_COLOR,
        "DisplayMode initializes every field and computes a 16:9 aspect ratio");

    cna_demo_check(
        &demo,
        cna_display_mode_init(640, 0, CNA_SURFACE_FORMAT_COLOR, &zero_height) ==
                CNA_RESULT_SUCCESS &&
            zero_height.width == 640 && zero_height.height == 0 &&
            zero_height.aspect_ratio == 0.0f,
        "a zero-height DisplayMode has the documented zero aspect ratio");

    same = full_hd;
    same.aspect_ratio = -123.0f;
    cna_demo_check(
        &demo,
        cna_display_mode_equals(&full_hd, &same, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE,
        "DisplayMode equality uses width, height and format rather than a cached aspect ratio");

    same.width = 1280;
    cna_demo_check(
        &demo,
        cna_display_mode_equals(&full_hd, &same, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "DisplayMode equality detects a defining-field difference");

    cna_demo_check(
        &demo,
        cna_presentation_parameters_init(&defaults) == CNA_RESULT_SUCCESS &&
            defaults.struct_size == sizeof(defaults) &&
            defaults.struct_version == UINT32_C(1) &&
            defaults.back_buffer_format == CNA_SURFACE_FORMAT_COLOR &&
            defaults.back_buffer_width == 800 && defaults.back_buffer_height == 480 &&
            defaults.depth_stencil_format == CNA_DEPTH_FORMAT_NONE &&
            defaults.multi_sample_count == 0 &&
            defaults.presentation_interval == CNA_PRESENT_INTERVAL_DEFAULT &&
            defaults.display_orientation == CNA_DISPLAY_ORIENTATION_DEFAULT &&
            defaults.render_target_usage == CNA_RENDER_TARGET_USAGE_DISCARD_CONTENTS &&
            defaults.is_full_screen == CNA_FALSE && defaults.headless_ext == CNA_FALSE &&
            defaults.reserved[0] == 0U && defaults.reserved[1] == 0U,
        "PresentationParameters initializes the complete XNA-compatible default value");

    requested = defaults;
    requested.back_buffer_width = 1024;
    requested.back_buffer_height = 576;
    requested.depth_stencil_format = CNA_DEPTH_FORMAT_DEPTH24_STENCIL8;
    requested.multi_sample_count = 4;
    requested.presentation_interval = CNA_PRESENT_INTERVAL_IMMEDIATE;
    requested.display_orientation =
        CNA_DISPLAY_ORIENTATION_LANDSCAPE_LEFT | CNA_DISPLAY_ORIENTATION_PORTRAIT;
    requested.render_target_usage = CNA_RENDER_TARGET_USAGE_PRESERVE_CONTENTS;
    requested.is_full_screen = CNA_TRUE;
    requested.headless_ext = CNA_TRUE;
    memset(&clone, 0xA5, sizeof(clone));
    cna_demo_check(
        &demo,
        cna_presentation_parameters_clone(&requested, &clone) == CNA_RESULT_SUCCESS &&
            clone.struct_size == requested.struct_size &&
            clone.struct_version == requested.struct_version &&
            clone.back_buffer_format == requested.back_buffer_format &&
            clone.back_buffer_width == requested.back_buffer_width &&
            clone.back_buffer_height == requested.back_buffer_height &&
            clone.depth_stencil_format == requested.depth_stencil_format &&
            clone.multi_sample_count == requested.multi_sample_count &&
            clone.presentation_interval == requested.presentation_interval &&
            clone.display_orientation == requested.display_orientation &&
            clone.render_target_usage == requested.render_target_usage &&
            clone.is_full_screen == requested.is_full_screen &&
            clone.headless_ext == CNA_FALSE && clone.reserved[0] == 0U &&
            clone.reserved[1] == 0U,
        "the native clone preserves XNA fields and resets its non-XNA headless extension");

    cna_demo_check(
        &demo,
        cna_presentation_parameters_get_bounds(&clone, &bounds) == CNA_RESULT_SUCCESS &&
            bounds.x == 0 && bounds.y == 0 && bounds.width == 1024 && bounds.height == 576,
        "PresentationParameters bounds use the requested back-buffer dimensions at origin zero");

    clone.reserved[1] = UINT8_C(1);
    bounds = (CNA_Rectangle){-7, -7, -7, -7};
    cna_demo_check(
        &demo,
        cna_presentation_parameters_get_bounds(&clone, &bounds) == CNA_RESULT_INVALID_ARGUMENT &&
            bounds.x == -7 && bounds.y == -7 && bounds.width == -7 && bounds.height == -7,
        "an invalid reserved byte is refused before the bounds output changes");

    return cna_demo_finish(&demo);
}
