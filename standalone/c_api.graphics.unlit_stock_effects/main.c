/* SPDX-License-Identifier: MS-PL */
/*
 * Two unlit stock effects from C. AlphaTestEffect owns one texture and adds an alpha comparison;
 * DualTextureEffect owns two independently indexed texture layers. Both also implement the shared
 * matrix and fog interfaces. Cloning makes every ownership interval explicit without drawing.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct UnlitEffectState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle alpha_test;
    CNA_EffectHandle alpha_clone;
    CNA_EffectHandle dual_texture;
    CNA_EffectHandle dual_clone;
    CNA_Handle texture_a;
    CNA_Handle texture_b;
} UnlitEffectState;

static int vector_is(CNA_Vector3 value, float x, float y, float z)
{
    return value.x == x && value.y == y && value.z == z;
}

static CNA_Matrix identity_matrix(void)
{
    CNA_Matrix value;
    memset(&value, 0, sizeof(value));
    value.m11 = 1.0F;
    value.m22 = 1.0F;
    value.m33 = 1.0F;
    value.m44 = 1.0F;
    return value;
}

static void inspect_defaults(UnlitEffectState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Vector3 vector = {9.0F, 9.0F, 9.0F};
    CNA_Bool boolean = CNA_TRUE;
    CNA_Bool has_texture = CNA_TRUE;
    CNA_Handle texture = UINT64_MAX;
    CNA_CompareFunction comparison = CNA_COMPARE_ALWAYS;
    int32_t reference = -1;
    float alpha = -1.0F;

    cna_demo_check(
        demo,
        cna_alpha_test_effect_get_diffuse_color(state->alpha_test, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F) &&
            cna_alpha_test_effect_get_alpha(state->alpha_test, &alpha) == CNA_RESULT_SUCCESS &&
            alpha == 1.0F &&
            cna_alpha_test_effect_get_vertex_color_enabled(state->alpha_test, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE &&
            cna_alpha_test_effect_get_alpha_function(state->alpha_test, &comparison) ==
                CNA_RESULT_SUCCESS &&
            comparison == CNA_COMPARE_GREATER &&
            cna_alpha_test_effect_get_reference_alpha(state->alpha_test, &reference) ==
                CNA_RESULT_SUCCESS &&
            reference == 0,
        "AlphaTestEffect starts white and opaque with vertex color off and Greater-than-zero alpha testing");
    cna_demo_check(
        demo,
        cna_alpha_test_effect_get_texture(state->alpha_test, &has_texture, &texture) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_FALSE && texture == CNA_INVALID_HANDLE,
        "AlphaTestEffect starts with its retained texture slot empty");

    cna_demo_check(
        demo,
        cna_dual_texture_effect_get_diffuse_color(state->dual_texture, &vector) ==
                CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F) &&
            cna_dual_texture_effect_get_alpha(state->dual_texture, &alpha) == CNA_RESULT_SUCCESS &&
            alpha == 1.0F &&
            cna_dual_texture_effect_get_vertex_color_enabled(state->dual_texture, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE,
        "DualTextureEffect starts white and opaque with vertex color off");
    cna_demo_check(
        demo,
        cna_dual_texture_effect_get_texture(state->dual_texture, 0U, &has_texture, &texture) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_FALSE && texture == CNA_INVALID_HANDLE &&
            cna_dual_texture_effect_get_texture(state->dual_texture, 1U, &has_texture, &texture) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_FALSE && texture == CNA_INVALID_HANDLE,
        "both independent DualTextureEffect layers start empty");
}

static void inspect_state(UnlitEffectState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Matrix alpha_world = identity_matrix();
    CNA_Matrix dual_world = identity_matrix();
    CNA_Matrix actual = {0};
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool boolean = CNA_FALSE;
    CNA_CompareFunction comparison = CNA_COMPARE_ALWAYS;
    int32_t reference = 0;
    float alpha = 0.0F;

    cna_demo_check(
        demo,
        cna_alpha_test_effect_set_diffuse_color(
            state->alpha_test, (CNA_Vector3){0.2F, 0.4F, 0.6F}) == CNA_RESULT_SUCCESS &&
            cna_alpha_test_effect_set_alpha(state->alpha_test, 1.25F) == CNA_RESULT_SUCCESS &&
            cna_alpha_test_effect_set_vertex_color_enabled(state->alpha_test, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_alpha_test_effect_set_alpha_function(state->alpha_test, CNA_COMPARE_EQUAL) ==
                CNA_RESULT_SUCCESS &&
            cna_alpha_test_effect_set_reference_alpha(state->alpha_test, 300) == CNA_RESULT_SUCCESS &&
            cna_alpha_test_effect_get_diffuse_color(state->alpha_test, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.2F, 0.4F, 0.6F) &&
            cna_alpha_test_effect_get_alpha(state->alpha_test, &alpha) == CNA_RESULT_SUCCESS &&
            alpha == 1.25F &&
            cna_alpha_test_effect_get_vertex_color_enabled(state->alpha_test, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_alpha_test_effect_get_alpha_function(state->alpha_test, &comparison) ==
                CNA_RESULT_SUCCESS &&
            comparison == CNA_COMPARE_EQUAL &&
            cna_alpha_test_effect_get_reference_alpha(state->alpha_test, &reference) ==
                CNA_RESULT_SUCCESS &&
            reference == 300,
        "AlphaTestEffect round-trips its material and deliberately unclamped alpha thresholds");
    cna_demo_check(
        demo,
        cna_alpha_test_effect_set_alpha_function(state->alpha_test, UINT32_C(8)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_alpha_test_effect_set_vertex_color_enabled(state->alpha_test, UINT32_C(2)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_alpha_test_effect_get_alpha_function(state->alpha_test, &comparison) ==
                CNA_RESULT_SUCCESS &&
            comparison == CNA_COMPARE_EQUAL &&
            cna_alpha_test_effect_get_vertex_color_enabled(state->alpha_test, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "undefined comparison and Boolean values leave AlphaTestEffect state unchanged");

    cna_demo_check(
        demo,
        cna_dual_texture_effect_set_diffuse_color(
            state->dual_texture, (CNA_Vector3){0.7F, 0.5F, 0.3F}) == CNA_RESULT_SUCCESS &&
            cna_dual_texture_effect_set_alpha(state->dual_texture, -0.25F) == CNA_RESULT_SUCCESS &&
            cna_dual_texture_effect_set_vertex_color_enabled(state->dual_texture, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_dual_texture_effect_get_diffuse_color(state->dual_texture, &vector) ==
                CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.7F, 0.5F, 0.3F) &&
            cna_dual_texture_effect_get_alpha(state->dual_texture, &alpha) == CNA_RESULT_SUCCESS &&
            alpha == -0.25F &&
            cna_dual_texture_effect_get_vertex_color_enabled(state->dual_texture, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "DualTextureEffect round-trips diffuse, unclamped alpha and vertex-color state");

    alpha_world.m41 = 2.0F;
    dual_world.m42 = -3.0F;
    cna_demo_check(
        demo,
        cna_effect_matrices_set_world(state->alpha_test, alpha_world) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_set_world(state->dual_texture, dual_world) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_get_world(state->alpha_test, &actual) == CNA_RESULT_SUCCESS &&
            memcmp(&actual, &alpha_world, sizeof(actual)) == 0 &&
            cna_effect_matrices_get_world(state->dual_texture, &actual) == CNA_RESULT_SUCCESS &&
            memcmp(&actual, &dual_world, sizeof(actual)) == 0,
        "the two effects expose independent world matrices through IEffectMatrices");

    cna_demo_check(
        demo,
        cna_effect_fog_set_color(state->alpha_test, (CNA_Vector3){0.1F, 0.2F, 0.3F}) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_enabled(state->alpha_test, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_start(state->alpha_test, 2.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_end(state->alpha_test, 6.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_get_color(state->alpha_test, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.1F, 0.2F, 0.3F) &&
            cna_effect_fog_get_enabled(state->alpha_test, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE && cna_effect_fog_get_start(state->alpha_test, &alpha) == CNA_RESULT_SUCCESS &&
            alpha == 2.0F && cna_effect_fog_get_end(state->alpha_test, &alpha) == CNA_RESULT_SUCCESS &&
            alpha == 6.0F,
        "AlphaTestEffect participates in the shared fog contract");
    cna_demo_check(
        demo,
        cna_effect_fog_set_color(state->dual_texture, (CNA_Vector3){0.6F, 0.4F, 0.2F}) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_enabled(state->dual_texture, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_start(state->dual_texture, 7.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_end(state->dual_texture, 11.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_get_color(state->dual_texture, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.6F, 0.4F, 0.2F) &&
            cna_effect_fog_get_enabled(state->dual_texture, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_effect_fog_get_start(state->dual_texture, &alpha) == CNA_RESULT_SUCCESS &&
            alpha == 7.0F && cna_effect_fog_get_end(state->dual_texture, &alpha) == CNA_RESULT_SUCCESS &&
            alpha == 11.0F,
        "DualTextureEffect keeps its own independent shared fog state");

    cna_demo_check(
        demo,
        cna_alpha_test_effect_get_alpha(state->dual_texture, &alpha) == CNA_RESULT_INVALID_HANDLE &&
            cna_dual_texture_effect_get_alpha(state->alpha_test, &alpha) == CNA_RESULT_INVALID_HANDLE,
        "the two concrete APIs reject one another's stock-effect handles");
}

static CNA_Result create_textures(UnlitEffectState* state, CNA_Handle device)
{
    CNA_Texture2DCreateInfo info;
    CNA_Result result;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.width = 1U;
    info.height = 1U;
    info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(device, &info, &state->texture_a);
    if (!cna_demo_check_result(state->demo, result, "create the first same-device texture")) {
        return result;
    }
    result = cna_texture2d_create(device, &info, &state->texture_b);
    cna_demo_check_result(state->demo, result, "create the second same-device texture");
    return result;
}

static void inspect_texture_slots(UnlitEffectState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Bool has_texture = CNA_FALSE;
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Handle refused = UINT64_MAX;

    cna_demo_check(
        demo,
        cna_alpha_test_effect_set_texture(state->alpha_test, state->texture_a) == CNA_RESULT_SUCCESS &&
            cna_alpha_test_effect_get_texture(state->alpha_test, &has_texture, &texture) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && texture == state->texture_a,
        "AlphaTestEffect retains and returns the exact first texture handle");
    cna_demo_check(
        demo,
        cna_dual_texture_effect_set_texture(state->dual_texture, 0U, state->texture_a) ==
                CNA_RESULT_SUCCESS &&
            cna_dual_texture_effect_set_texture(state->dual_texture, 1U, state->texture_b) ==
                CNA_RESULT_SUCCESS &&
            cna_dual_texture_effect_get_texture(
                state->dual_texture, 0U, &has_texture, &texture) == CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && texture == state->texture_a &&
            cna_dual_texture_effect_get_texture(
                state->dual_texture, 1U, &has_texture, &texture) == CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && texture == state->texture_b,
        "DualTextureEffect independently retains and returns both indexed texture layers");
    cna_demo_check(
        demo,
        cna_dual_texture_effect_get_texture(state->dual_texture, 2U, &has_texture, &refused) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            has_texture == CNA_FALSE && refused == CNA_INVALID_HANDLE &&
            cna_dual_texture_effect_set_texture(
                state->dual_texture, 2U, CNA_INVALID_HANDLE) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_dual_texture_effect_get_texture(
                state->dual_texture, 1U, &has_texture, &texture) == CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && texture == state->texture_b,
        "the first invalid layer is refused atomically and leaves the valid second layer intact");
    cna_demo_check(
        demo,
        cna_texture2d_destroy(state->texture_a) == CNA_RESULT_INVALID_STATE &&
            cna_texture2d_destroy(state->texture_b) == CNA_RESULT_INVALID_STATE,
        "neither caller texture can be destroyed while a stock effect retains it");
}

static CNA_Result clone_and_release_sources(UnlitEffectState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Result result = cna_effect_clone(state->alpha_test, &state->alpha_clone);
    if (!cna_demo_check_result(demo, result, "clone AlphaTestEffect with its complete state")) {
        return result;
    }
    result = cna_effect_clone(state->dual_texture, &state->dual_clone);
    if (!cna_demo_check_result(demo, result, "clone DualTextureEffect with both texture layers")) {
        return result;
    }
    cna_demo_check(
        demo,
        cna_effect_destroy(state->alpha_test) == CNA_RESULT_SUCCESS &&
            cna_effect_destroy(state->dual_texture) == CNA_RESULT_SUCCESS,
        "both source handles can be destroyed after their independent clones are created");
    state->alpha_test = CNA_INVALID_HANDLE;
    state->dual_texture = CNA_INVALID_HANDLE;
    return CNA_RESULT_SUCCESS;
}

static void inspect_clones_and_release_textures(UnlitEffectState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool has_texture = CNA_FALSE;
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_CompareFunction comparison = CNA_COMPARE_ALWAYS;
    int32_t reference = 0;
    float alpha = 0.0F;

    cna_demo_check(
        demo,
        cna_alpha_test_effect_get_diffuse_color(state->alpha_clone, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.2F, 0.4F, 0.6F) &&
            cna_alpha_test_effect_get_alpha_function(state->alpha_clone, &comparison) ==
                CNA_RESULT_SUCCESS &&
            comparison == CNA_COMPARE_EQUAL &&
            cna_alpha_test_effect_get_reference_alpha(state->alpha_clone, &reference) ==
                CNA_RESULT_SUCCESS &&
            reference == 300 &&
            cna_alpha_test_effect_get_texture(state->alpha_clone, &has_texture, &texture) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && texture == state->texture_a,
        "the AlphaTestEffect clone preserves material, comparison and retained texture state");
    cna_demo_check(
        demo,
        cna_dual_texture_effect_get_alpha(state->dual_clone, &alpha) == CNA_RESULT_SUCCESS &&
            alpha == -0.25F &&
            cna_dual_texture_effect_get_texture(state->dual_clone, 0U, &has_texture, &texture) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && texture == state->texture_a &&
            cna_dual_texture_effect_get_texture(state->dual_clone, 1U, &has_texture, &texture) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && texture == state->texture_b,
        "the DualTextureEffect clone preserves its material and both retained texture slots");
    cna_demo_check(
        demo,
        cna_alpha_test_effect_set_texture(state->alpha_clone, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->texture_a) == CNA_RESULT_INVALID_STATE &&
            cna_dual_texture_effect_set_texture(state->dual_clone, 0U, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->texture_a) == CNA_RESULT_SUCCESS,
        "the shared first texture releases only after both clone-owned slots are cleared");
    state->texture_a = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo,
        cna_texture2d_destroy(state->texture_b) == CNA_RESULT_INVALID_STATE &&
            cna_dual_texture_effect_set_texture(state->dual_clone, 1U, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->texture_b) == CNA_RESULT_SUCCESS,
        "the independent second texture releases when the second clone slot is cleared");
    state->texture_b = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo,
        cna_effect_apply(state->alpha_clone) == CNA_RESULT_SUCCESS &&
            cna_effect_apply(state->dual_clone) == CNA_RESULT_SUCCESS,
        "both fully configured clones apply after their source and textures are released");
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    UnlitEffectState* state = (UnlitEffectState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_alpha_test_effect_create(device, &state->alpha_test);
    if (!cna_demo_check_result(state->demo, result, "create AlphaTestEffect")) {
        return result;
    }
    result = cna_dual_texture_effect_create(device, &state->dual_texture);
    if (!cna_demo_check_result(state->demo, result, "create DualTextureEffect")) {
        return result;
    }
    inspect_defaults(state);
    inspect_state(state);
    result = create_textures(state, device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    inspect_texture_slots(state);
    result = clone_and_release_sources(state);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    inspect_clones_and_release_textures(state);
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

static void cleanup(UnlitEffectState* state)
{
    if (state->dual_clone != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->dual_clone);
    }
    if (state->alpha_clone != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->alpha_clone);
    }
    if (state->dual_texture != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->dual_texture);
    }
    if (state->alpha_test != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->alpha_test);
    }
    if (state->texture_b != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(state->texture_b);
    }
    if (state->texture_a != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(state->texture_a);
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    UnlitEffectState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.unlit_stock_effects", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&create_info, 0, sizeof(create_info));
    state.demo = &demo;
    state.alpha_test = CNA_INVALID_HANDLE;
    state.alpha_clone = CNA_INVALID_HANDLE;
    state.dual_texture = CNA_INVALID_HANDLE;
    state.dual_clone = CNA_INVALID_HANDLE;
    state.texture_a = CNA_INVALID_HANDLE;
    state.texture_b = CNA_INVALID_HANDLE;
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
        "create the graphics device for both unlit stock effects");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the unlit stock-effect state audit");
        cna_demo_check(&demo, state.loads == 1, "the audit ran in exactly one load callback");
    }
    cleanup(&state);
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager after every effect and texture");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
