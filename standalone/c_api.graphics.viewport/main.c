/* SPDX-License-Identifier: MS-PL */
/*
 * Viewport values from C: constructors, derived properties, canonical text, projection math and
 * an exact live-device set/get/restore cycle. The default value check also preserves XNA's value-
 * type zero initialization: only the parameterized constructors install a 0..1 depth range.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ViewportDemo {
    CnaDemo* demo;
    int loads;
} ViewportDemo;

static int near_float(float left, float right)
{
    return fabsf(left - right) < 1e-4f;
}

static int viewport_equal(CNA_Viewport left, CNA_Viewport right)
{
    return left.x == right.x && left.y == right.y && left.width == right.width &&
        left.height == right.height && left.min_depth == right.min_depth &&
        left.max_depth == right.max_depth;
}

static int rectangle_equal(CNA_Rectangle left, CNA_Rectangle right)
{
    return left.x == right.x && left.y == right.y && left.width == right.width &&
        left.height == right.height;
}

static void check_value_routes(CnaDemo* demo)
{
    static const char ExpectedText[] =
        "{X:1 Y:2 Width:3 Height:4 MinDepth:0.25 MaxDepth:0.75}";
    CNA_Viewport empty = {7, 7, 7, 7, 7.0f, 7.0f};
    CNA_Viewport bounds_view = {0, 0, 0, 0, 0.0f, 0.0f};
    CNA_Viewport rectangle_view = {0, 0, 0, 0, 0.0f, 0.0f};
    CNA_Viewport properties = {5, 6, 320, 240, 0.25f, 0.75f};
    CNA_Viewport zero_width = {0, 0, 0, 240, 0.0f, 1.0f};
    CNA_Rectangle source_bounds = {10, 20, 320, 240};
    CNA_Rectangle derived = {0, 0, 0, 0};
    CNA_Matrix identity;
    CNA_Matrix projection;
    CNA_Matrix view;
    CNA_Vector3 projected = {-1.0f, -1.0f, -1.0f};
    CNA_Vector3 unprojected = {-1.0f, -1.0f, -1.0f};
    float aspect = -1.0f;
    char text[sizeof(ExpectedText)];
    char short_text[4] = {'q', 'q', 'q', 'q'};
    uint64_t required = 0U;
    uint64_t copied = 0U;

    cna_demo_check(
        demo,
        cna_viewport_init(&empty) == CNA_RESULT_SUCCESS &&
            viewport_equal(empty, (CNA_Viewport){0, 0, 0, 0, 0.0f, 0.0f}),
        "the default Viewport value initializes all six fields to zero");
    cna_demo_check(
        demo,
        cna_viewport_init_bounds(3, -4, 800, 600, &bounds_view) == CNA_RESULT_SUCCESS &&
            viewport_equal(bounds_view, (CNA_Viewport){3, -4, 800, 600, 0.0f, 1.0f}) &&
            cna_viewport_init_from_rectangle(source_bounds, &rectangle_view) ==
                CNA_RESULT_SUCCESS &&
            viewport_equal(rectangle_view, (CNA_Viewport){10, 20, 320, 240, 0.0f, 1.0f}),
        "both parameterized constructors copy their bounds and install depth zero through one");
    cna_demo_check(
        demo,
        cna_viewport_get_aspect_ratio(properties, &aspect) == CNA_RESULT_SUCCESS &&
            near_float(aspect, 4.0f / 3.0f) &&
            cna_viewport_get_aspect_ratio(zero_width, &aspect) == CNA_RESULT_SUCCESS &&
            aspect == 0.0f,
        "aspect ratio is width over height and zero for an empty dimension");
    cna_demo_check(
        demo,
        cna_viewport_get_bounds(properties, &derived) == CNA_RESULT_SUCCESS &&
            rectangle_equal(derived, (CNA_Rectangle){5, 6, 320, 240}) &&
            cna_viewport_get_title_safe_area(properties, &derived) == CNA_RESULT_SUCCESS &&
            rectangle_equal(derived, (CNA_Rectangle){5, 6, 320, 240}) &&
            cna_viewport_set_bounds(&properties, (CNA_Rectangle){1, 2, 3, 4}) ==
                CNA_RESULT_SUCCESS &&
            viewport_equal(properties, (CNA_Viewport){1, 2, 3, 4, 0.25f, 0.75f}),
        "bounds and desktop safe area agree, while replacing bounds preserves the depth range");

    cna_demo_check(
        demo, cna_matrix_get_identity(&identity) == CNA_RESULT_SUCCESS,
        "create the identity matrices used by the exact clip-space controls");
    properties = (CNA_Viewport){10, 20, 320, 240, 0.25f, 0.75f};
    cna_demo_check(
        demo,
        cna_viewport_project(
            properties, (CNA_Vector3){0.0f, 0.0f, 0.0f}, identity, identity, identity,
            &projected) == CNA_RESULT_SUCCESS &&
            near_float(projected.x, 170.0f) && near_float(projected.y, 140.0f) &&
            near_float(projected.z, 0.25f) &&
            cna_viewport_unproject(
                properties, projected, identity, identity, identity, &unprojected) ==
                CNA_RESULT_SUCCESS &&
            near_float(unprojected.x, 0.0f) && near_float(unprojected.y, 0.0f) &&
            near_float(unprojected.z, 0.0f),
        "identity projection maps clip origin to viewport centre and unprojects it exactly");

    properties = (CNA_Viewport){0, 0, 800, 600, 0.0f, 1.0f};
    cna_demo_check(
        demo,
        cna_matrix_create_perspective_field_of_view(
            CNA_MATH_PI_OVER_4, 800.0f / 600.0f, 1.0f, 100.0f, &projection) ==
                CNA_RESULT_SUCCESS &&
            cna_matrix_create_look_at(
                (CNA_Vector3){0.0f, 0.0f, 10.0f}, (CNA_Vector3){0.0f, 0.0f, 0.0f},
                (CNA_Vector3){0.0f, 1.0f, 0.0f}, &view) == CNA_RESULT_SUCCESS &&
            cna_viewport_project(
                properties, (CNA_Vector3){1.5f, -2.0f, 3.0f}, projection, view, identity,
                &projected) == CNA_RESULT_SUCCESS &&
            cna_viewport_unproject(
                properties, projected, projection, view, identity, &unprojected) ==
                CNA_RESULT_SUCCESS &&
            near_float(unprojected.x, 1.5f) && near_float(unprojected.y, -2.0f) &&
            near_float(unprojected.z, 3.0f),
        "perspective projection and unprojection round-trip a world-space point");

    properties = (CNA_Viewport){1, 2, 3, 4, 0.25f, 0.75f};
    memset(text, 'z', sizeof(text));
    cna_demo_check(
        demo,
        cna_viewport_get_string_size(properties, &required) == CNA_RESULT_SUCCESS &&
            required == sizeof(ExpectedText) - 1U &&
            cna_viewport_copy_string(properties, short_text, sizeof(short_text), &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == required && short_text[0] == 'q' && short_text[1] == 'q' &&
            short_text[2] == 'q' && short_text[3] == 'q' &&
            cna_viewport_copy_string(properties, text, required, &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == required && memcmp(text, ExpectedText, (size_t)required) == 0 &&
            text[required] == 'z',
        "canonical Viewport text copies exactly and a short buffer remains untouched");
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    ViewportDemo* state = (ViewportDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Viewport original;
    CNA_Viewport requested = {4, 6, 40, 30, 0.25f, 0.75f};
    CNA_Viewport observed = {0, 0, 0, 0, 0.0f, 0.0f};
    CNA_Viewport invalid;
    CNA_Result borrow_result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    borrow_result = cna_game_get_graphics_device(game, &device);
    cna_demo_check(
        state->demo, borrow_result == CNA_RESULT_SUCCESS,
        "borrow the live graphics device for this callback");
    cna_demo_check(
        state->demo,
        borrow_result == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_viewport(device, &original) == CNA_RESULT_SUCCESS &&
            original.width > 0 && original.height > 0 && isfinite(original.min_depth) &&
            isfinite(original.max_depth),
        "read a complete finite viewport from the live device");
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_viewport(device, requested) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_viewport(device, &observed) == CNA_RESULT_SUCCESS &&
            viewport_equal(observed, requested),
        "apply and read back every field of a distinctive live viewport");

    invalid = requested;
    invalid.max_depth = INFINITY;
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_viewport(device, invalid) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_device_get_viewport(device, &observed) == CNA_RESULT_SUCCESS &&
            viewport_equal(observed, requested),
        "a non-finite depth value is refused before changing the live viewport");
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_viewport(device, original) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_viewport(device, &observed) == CNA_RESULT_SUCCESS &&
            viewport_equal(observed, original),
        "restore and read back the exact original viewport before leaving the callback");
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
    ViewportDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.viewport", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    check_value_routes(&demo);

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
        "create the graphics device whose live viewport is audited");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through load and the immediate exit request");
        cna_demo_check(&demo, state.loads == 1,
                       "the live viewport audit ran in exactly one load callback");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
