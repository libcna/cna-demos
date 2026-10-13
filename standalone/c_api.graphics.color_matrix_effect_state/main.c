/* SPDX-License-Identifier: MS-PL */
/*
 * ColorMatrixEffect from C: finite matrix/offset state, the Rec. 709 grayscale preset, reset and
 * type-preserving clone lifetime. The demo deliberately performs no SpriteBatch draw.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ColorMatrixState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle effect;
    CNA_EffectHandle clone;
} ColorMatrixState;

static CNA_ColorMatrix4x4 identity_matrix(void)
{
    CNA_ColorMatrix4x4 value;
    memset(&value, 0, sizeof(value));
    value.values[0] = 1.0F;
    value.values[5] = 1.0F;
    value.values[10] = 1.0F;
    value.values[15] = 1.0F;
    return value;
}

static CNA_ColorMatrix4x4 authored_matrix(void)
{
    CNA_ColorMatrix4x4 value = {{
        0.5F, 0.1F, 0.2F, 0.3F,
        -0.4F, 1.5F, 0.6F, 0.7F,
        0.8F, 0.9F, -1.0F, 1.1F,
        1.2F, 1.3F, 1.4F, 0.25F,
    }};
    return value;
}

static int matrix_is(CNA_ColorMatrix4x4 value, CNA_ColorMatrix4x4 expected)
{
    return memcmp(&value, &expected, sizeof(value)) == 0;
}

static int vector_is(CNA_Vector4 value, float x, float y, float z, float w)
{
    return value.x == x && value.y == y && value.z == z && value.w == w;
}

static void inspect_defaults(ColorMatrixState* state)
{
    CNA_ColorMatrix4x4 matrix = {{0.0F}};
    CNA_Vector4 offset = {9.0F, 9.0F, 9.0F, 9.0F};

    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_get_matrix(state->effect, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, identity_matrix()),
        "ColorMatrixEffect starts with the row-major identity transform");
    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_get_offset(state->effect, &offset) == CNA_RESULT_SUCCESS &&
            vector_is(offset, 0.0F, 0.0F, 0.0F, 0.0F),
        "the default post-transform RGBA offset is zero");
}

static void inspect_authored_and_invalid_state(ColorMatrixState* state)
{
    const CNA_ColorMatrix4x4 authored = authored_matrix();
    const CNA_Vector4 authored_offset = {-0.25F, 0.5F, 1.25F, -2.0F};
    CNA_ColorMatrix4x4 invalid;
    CNA_ColorMatrix4x4 matrix = {{0.0F}};
    CNA_Vector4 offset = {0.0F, 0.0F, 0.0F, 0.0F};

    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_set_matrix(state->effect, authored) == CNA_RESULT_SUCCESS &&
            cna_color_matrix_effect_get_matrix(state->effect, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, authored),
        "all sixteen finite row-major matrix elements round-trip exactly");
    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_set_offset(state->effect, authored_offset) == CNA_RESULT_SUCCESS &&
            cna_color_matrix_effect_get_offset(state->effect, &offset) == CNA_RESULT_SUCCESS &&
            vector_is(offset, -0.25F, 0.5F, 1.25F, -2.0F),
        "the finite RGBA offset preserves unclamped authored values");

    invalid = authored;
    invalid.values[6] = NAN;
    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_set_matrix(state->effect, invalid) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_color_matrix_effect_get_matrix(state->effect, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, authored),
        "a NaN matrix element is refused atomically");
    invalid = authored;
    invalid.values[15] = INFINITY;
    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_set_matrix(state->effect, invalid) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_color_matrix_effect_get_matrix(state->effect, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, authored),
        "an infinite matrix element is refused without changing the transform");
    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_set_offset(
            state->effect, (CNA_Vector4){-0.25F, NAN, 1.25F, -2.0F}) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_color_matrix_effect_set_offset(
                state->effect, (CNA_Vector4){-0.25F, 0.5F, 1.25F, -INFINITY}) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_color_matrix_effect_get_offset(state->effect, &offset) == CNA_RESULT_SUCCESS &&
            vector_is(offset, -0.25F, 0.5F, 1.25F, -2.0F),
        "non-finite offsets are refused atomically while the prior RGBA value survives");
}

static void inspect_presets(ColorMatrixState* state)
{
    const CNA_ColorMatrix4x4 grayscale = {{
        0.2126F, 0.7152F, 0.0722F, 0.0F,
        0.2126F, 0.7152F, 0.0722F, 0.0F,
        0.2126F, 0.7152F, 0.0722F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F,
    }};
    CNA_ColorMatrix4x4 matrix = {{0.0F}};
    CNA_Vector4 offset = {9.0F, 9.0F, 9.0F, 9.0F};

    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_set_grayscale(state->effect) == CNA_RESULT_SUCCESS &&
            cna_color_matrix_effect_get_matrix(state->effect, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, grayscale),
        "the grayscale preset installs three exact Rec. 709 RGB rows and preserves alpha");
    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_get_offset(state->effect, &offset) == CNA_RESULT_SUCCESS &&
            vector_is(offset, 0.0F, 0.0F, 0.0F, 0.0F),
        "the grayscale preset also clears the previous authored offset");
    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_reset(state->effect) == CNA_RESULT_SUCCESS &&
            cna_color_matrix_effect_get_matrix(state->effect, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, identity_matrix()) &&
            cna_color_matrix_effect_get_offset(state->effect, &offset) == CNA_RESULT_SUCCESS &&
            vector_is(offset, 0.0F, 0.0F, 0.0F, 0.0F),
        "reset restores the identity matrix and zero offset together");
}

static CNA_Result inspect_clone(ColorMatrixState* state)
{
    const CNA_ColorMatrix4x4 authored = authored_matrix();
    CNA_ColorMatrix4x4 matrix = {{0.0F}};
    CNA_Vector4 offset = {0.0F, 0.0F, 0.0F, 0.0F};
    CNA_Result result;

    result = cna_color_matrix_effect_set_matrix(state->effect, authored);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_color_matrix_effect_set_offset(
            state->effect, (CNA_Vector4){0.125F, -0.25F, 0.5F, 0.75F});
    }
    if (!cna_demo_check_result(state->demo, result, "restore authored state before cloning")) {
        return result;
    }
    result = cna_effect_clone(state->effect, &state->clone);
    if (!cna_demo_check_result(state->demo, result, "clone the configured ColorMatrixEffect")) {
        return result;
    }
    cna_demo_check(
        state->demo,
        cna_effect_destroy(state->effect) == CNA_RESULT_SUCCESS,
        "destroy the source after creating an independent type-preserving clone");
    state->effect = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        cna_color_matrix_effect_get_matrix(state->clone, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, authored) &&
            cna_color_matrix_effect_get_offset(state->clone, &offset) == CNA_RESULT_SUCCESS &&
            vector_is(offset, 0.125F, -0.25F, 0.5F, 0.75F),
        "the clone retains every matrix and offset element after source destruction");
    cna_demo_check(
        state->demo, cna_effect_apply(state->clone) == CNA_RESULT_SUCCESS,
        "the retained ColorMatrixEffect clone applies without drawing");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    ColorMatrixState* state = (ColorMatrixState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_color_matrix_effect_create(device, &state->effect);
    if (!cna_demo_check_result(state->demo, result, "create ColorMatrixEffect")) {
        return result;
    }
    inspect_defaults(state);
    inspect_authored_and_invalid_state(state);
    inspect_presets(state);
    return inspect_clone(state);
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

static void cleanup(ColorMatrixState* state)
{
    if (state->clone != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->clone);
    }
    if (state->effect != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->effect);
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    ColorMatrixState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.color_matrix_effect_state", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&create_info, 0, sizeof(create_info));
    state.demo = &demo;
    state.effect = CNA_INVALID_HANDLE;
    state.clone = CNA_INVALID_HANDLE;
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.context = &state;
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
        "create the graphics device for ColorMatrixEffect");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the ColorMatrixEffect state audit");
    }
    cna_demo_check(&demo, state.loads == 1, "LoadContent runs exactly once");

    cleanup(&state);
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
