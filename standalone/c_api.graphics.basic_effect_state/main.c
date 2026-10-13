/* SPDX-License-Identifier: MS-PL */
/*
 * BasicEffect state from C: exercise the stock effect through its concrete routes and through the
 * shared IEffectMatrices, IEffectFog and IEffectLights adapters. A real same-device Texture2D
 * demonstrates retained ownership, and both standalone and nested DirectionalLight handles make
 * their different lifetimes visible.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct BasicEffectState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle basic;
    CNA_EffectHandle empty;
    CNA_Handle texture;
    CNA_DirectionalLightHandle retained_light;
} BasicEffectState;

static int vector_is(CNA_Vector3 value, float x, float y, float z)
{
    return value.x == x && value.y == y && value.z == z;
}

static int matrix_is(CNA_Matrix value, CNA_Matrix expected)
{
    return memcmp(&value, &expected, sizeof(value)) == 0;
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

static CNA_Result inspect_standalone_light(BasicEffectState* state)
{
    CnaDemo* demo = state->demo;
    CNA_DirectionalLightHandle light = CNA_INVALID_HANDLE;
    CNA_Vector3 vector = {9.0F, 9.0F, 9.0F};
    CNA_Bool enabled = CNA_TRUE;
    CNA_Result result = cna_directional_light_create(&light);
    if (!cna_demo_check_result(demo, result, "create a standalone DirectionalLight")) {
        return result;
    }

    cna_demo_check(
        demo,
        cna_directional_light_get_diffuse_color(light, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F) &&
            cna_directional_light_get_direction(light, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, -1.0F, 0.0F) &&
            cna_directional_light_get_specular_color(light, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F) &&
            cna_directional_light_get_enabled(light, &enabled) == CNA_RESULT_SUCCESS &&
            enabled == CNA_FALSE,
        "a standalone light starts disabled with the XNA white-down-no-specular defaults");

    cna_demo_check(
        demo,
        cna_directional_light_set_diffuse_color(light, (CNA_Vector3){0.2F, 0.3F, 0.4F}) ==
                CNA_RESULT_SUCCESS &&
            cna_directional_light_set_direction(light, (CNA_Vector3){-0.5F, -0.75F, 0.25F}) ==
                CNA_RESULT_SUCCESS &&
            cna_directional_light_set_specular_color(light, (CNA_Vector3){0.6F, 0.7F, 0.8F}) ==
                CNA_RESULT_SUCCESS &&
            cna_directional_light_set_enabled(light, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_directional_light_get_diffuse_color(light, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.2F, 0.3F, 0.4F) &&
            cna_directional_light_get_direction(light, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, -0.5F, -0.75F, 0.25F) &&
            cna_directional_light_get_specular_color(light, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.6F, 0.7F, 0.8F) &&
            cna_directional_light_get_enabled(light, &enabled) == CNA_RESULT_SUCCESS &&
            enabled == CNA_TRUE,
        "standalone diffuse, direction, specular and enabled state round-trip together");

    cna_demo_check(
        demo,
        cna_directional_light_set_enabled(light, UINT32_C(2)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_directional_light_get_enabled(light, &enabled) == CNA_RESULT_SUCCESS &&
            enabled == CNA_TRUE,
        "a non-canonical light Boolean is refused without changing enabled state");

    result = cna_directional_light_destroy(light);
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_directional_light_get_enabled(light, &enabled) == CNA_RESULT_INVALID_HANDLE &&
            cna_directional_light_destroy(light) == CNA_RESULT_INVALID_HANDLE,
        "standalone light destruction is one-shot and makes the handle stale");
    return result;
}

static void inspect_defaults(BasicEffectState* state)
{
    CnaDemo* demo = state->demo;
    const CNA_Matrix identity = identity_matrix();
    CNA_Matrix matrix = {0};
    CNA_Vector3 vector = {9.0F, 9.0F, 9.0F};
    CNA_Bool boolean = CNA_TRUE;
    CNA_Bool has_texture = CNA_TRUE;
    CNA_Handle texture = UINT64_MAX;
    float scalar = -1.0F;
    CNA_DirectionalLightHandle lights[3] = {
        CNA_INVALID_HANDLE, CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    unsigned int index;

    cna_demo_check(
        demo,
        cna_effect_matrices_get_world(state->basic, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, identity) &&
            cna_effect_matrices_get_view(state->basic, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, identity) &&
            cna_effect_matrices_get_projection(state->basic, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, identity),
        "all three shared matrix properties start at identity");

    cna_demo_check(
        demo,
        cna_basic_effect_get_vertex_color_enabled(state->basic, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE &&
            cna_basic_effect_get_prefer_per_pixel_lighting(state->basic, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE &&
            cna_effect_lights_get_enabled(state->basic, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE &&
            cna_basic_effect_get_texture_enabled(state->basic, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE,
        "vertex color, per-pixel lighting, lighting and texture mapping start disabled");

    cna_demo_check(
        demo,
        cna_basic_effect_get_diffuse_color(state->basic, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F) &&
            cna_basic_effect_get_emissive_color(state->basic, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F) &&
            cna_basic_effect_get_specular_color(state->basic, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F) &&
            cna_basic_effect_get_specular_power(state->basic, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 16.0F &&
            cna_basic_effect_get_alpha(state->basic, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F,
        "the default material is white, opaque and non-emissive with power-16 white specular");

    cna_demo_check(
        demo,
        cna_effect_fog_get_color(state->basic, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F) &&
            cna_effect_fog_get_enabled(state->basic, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE &&
            cna_effect_fog_get_start(state->basic, &scalar) == CNA_RESULT_SUCCESS && scalar == 0.0F &&
            cna_effect_fog_get_end(state->basic, &scalar) == CNA_RESULT_SUCCESS && scalar == 1.0F,
        "shared fog starts disabled with black color and the zero-to-one range");

    cna_demo_check(
        demo,
        cna_basic_effect_get_texture(state->basic, &has_texture, &texture) == CNA_RESULT_SUCCESS &&
            has_texture == CNA_FALSE && texture == CNA_INVALID_HANDLE,
        "the retained BasicEffect texture slot starts empty");

    for (index = 0U; index < 3U; ++index) {
        (void)cna_effect_lights_get_directional_light(state->basic, index, &lights[index]);
    }
    cna_demo_check(
        demo,
        lights[0] != CNA_INVALID_HANDLE && lights[1] != CNA_INVALID_HANDLE &&
            lights[2] != CNA_INVALID_HANDLE &&
            cna_directional_light_get_enabled(lights[0], &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_directional_light_get_enabled(lights[1], &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE &&
            cna_directional_light_get_enabled(lights[2], &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE,
        "BasicEffect starts with only its first stable directional-light member enabled");
    for (index = 0U; index < 3U; ++index) {
        if (lights[index] != CNA_INVALID_HANDLE) {
            (void)cna_directional_light_destroy(lights[index]);
        }
    }
}

static void inspect_round_trips(BasicEffectState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Matrix world = identity_matrix();
    CNA_Matrix view = identity_matrix();
    CNA_Matrix projection = identity_matrix();
    CNA_Matrix actual = {0};
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool boolean = CNA_FALSE;
    float scalar = 0.0F;

    world.m41 = 1.25F;
    view.m42 = -2.5F;
    projection.m33 = 0.75F;
    projection.m43 = 3.5F;
    cna_demo_check(
        demo,
        cna_effect_matrices_set_world(state->basic, world) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_set_view(state->basic, view) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_set_projection(state->basic, projection) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_get_world(state->basic, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, world) &&
            cna_effect_matrices_get_view(state->basic, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, view) &&
            cna_effect_matrices_get_projection(state->basic, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, projection),
        "distinct world, view and projection matrices round-trip through IEffectMatrices");

    cna_demo_check(
        demo,
        cna_basic_effect_set_vertex_color_enabled(state->basic, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_basic_effect_set_prefer_per_pixel_lighting(state->basic, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_basic_effect_set_texture_enabled(state->basic, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_basic_effect_get_vertex_color_enabled(state->basic, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_basic_effect_get_prefer_per_pixel_lighting(state->basic, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_basic_effect_get_texture_enabled(state->basic, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "the three independent BasicEffect shader-selection switches round-trip");

    cna_demo_check(
        demo,
        cna_basic_effect_set_diffuse_color(state->basic, (CNA_Vector3){0.2F, 0.4F, 0.6F}) ==
                CNA_RESULT_SUCCESS &&
            cna_basic_effect_set_emissive_color(state->basic, (CNA_Vector3){0.1F, 0.3F, 0.5F}) ==
                CNA_RESULT_SUCCESS &&
            cna_basic_effect_set_specular_color(state->basic, (CNA_Vector3){0.7F, 0.8F, 0.9F}) ==
                CNA_RESULT_SUCCESS &&
            cna_basic_effect_set_specular_power(state->basic, 23.5F) == CNA_RESULT_SUCCESS &&
            cna_basic_effect_set_alpha(state->basic, 0.625F) == CNA_RESULT_SUCCESS &&
            cna_basic_effect_get_diffuse_color(state->basic, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.2F, 0.4F, 0.6F) &&
            cna_basic_effect_get_emissive_color(state->basic, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.1F, 0.3F, 0.5F) &&
            cna_basic_effect_get_specular_color(state->basic, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.7F, 0.8F, 0.9F) &&
            cna_basic_effect_get_specular_power(state->basic, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 23.5F && cna_basic_effect_get_alpha(state->basic, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 0.625F,
        "diffuse, emissive, specular, power and alpha keep their exact independent values");

    cna_demo_check(
        demo,
        cna_effect_fog_set_color(state->basic, (CNA_Vector3){0.25F, 0.5F, 0.75F}) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_enabled(state->basic, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_start(state->basic, 4.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_end(state->basic, 12.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_get_color(state->basic, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.25F, 0.5F, 0.75F) &&
            cna_effect_fog_get_enabled(state->basic, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE && cna_effect_fog_get_start(state->basic, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 4.0F && cna_effect_fog_get_end(state->basic, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 12.0F,
        "fog color, enabled state and start/end range round-trip through IEffectFog");

    cna_demo_check(
        demo,
        cna_effect_lights_set_ambient_color(state->basic, (CNA_Vector3){0.15F, 0.25F, 0.35F}) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_lights_set_enabled(state->basic, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_effect_lights_get_ambient_color(state->basic, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.15F, 0.25F, 0.35F) &&
            cna_effect_lights_get_enabled(state->basic, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "ambient color and the BasicEffect lighting switch round-trip through IEffectLights");
}

static void inspect_nested_lights(BasicEffectState* state)
{
    CnaDemo* demo = state->demo;
    CNA_DirectionalLightHandle first = CNA_INVALID_HANDLE;
    CNA_DirectionalLightHandle same = CNA_INVALID_HANDLE;
    CNA_DirectionalLightHandle second = CNA_INVALID_HANDLE;
    CNA_DirectionalLightHandle third = CNA_INVALID_HANDLE;
    CNA_DirectionalLightHandle refused = UINT64_MAX;
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool enabled = CNA_FALSE;

    cna_demo_check(
        demo,
        cna_effect_lights_get_directional_light(state->basic, 0U, &first) == CNA_RESULT_SUCCESS &&
            cna_directional_light_set_direction(first, (CNA_Vector3){-1.0F, -2.0F, -3.0F}) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_lights_get_directional_light(state->basic, 0U, &same) == CNA_RESULT_SUCCESS &&
            first != same &&
            cna_directional_light_get_direction(same, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, -1.0F, -2.0F, -3.0F),
        "separate handles for light zero are views of the same stable BasicEffect member");

    cna_demo_check(
        demo,
        cna_effect_lights_get_directional_light(state->basic, 3U, &refused) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            refused == CNA_INVALID_HANDLE,
        "the first out-of-range directional-light index is refused without publishing a view");

    cna_demo_check(
        demo,
        cna_effect_lights_enable_default(state->basic) == CNA_RESULT_SUCCESS &&
            cna_effect_lights_get_ambient_color(state->basic, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.05333332F, 0.09882354F, 0.1819608F) &&
            cna_directional_light_get_direction(first, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, -0.5265408F, -0.5735765F, -0.6275069F) &&
            cna_directional_light_get_enabled(first, &enabled) == CNA_RESULT_SUCCESS &&
            enabled == CNA_TRUE,
        "EnableDefaultLighting rewrites an existing member view with the canonical key light");

    (void)cna_effect_lights_get_directional_light(state->basic, 1U, &second);
    (void)cna_effect_lights_get_directional_light(state->basic, 2U, &third);
    cna_demo_check(
        demo,
        second != CNA_INVALID_HANDLE && third != CNA_INVALID_HANDLE &&
            cna_directional_light_get_diffuse_color(second, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.9647059F, 0.7607844F, 0.4078432F) &&
            cna_directional_light_get_enabled(second, &enabled) == CNA_RESULT_SUCCESS &&
            enabled == CNA_TRUE &&
            cna_directional_light_get_diffuse_color(third, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.3231373F, 0.3607844F, 0.3937255F) &&
            cna_directional_light_get_enabled(third, &enabled) == CNA_RESULT_SUCCESS &&
            enabled == CNA_TRUE,
        "the canonical fill and back lights are distinct and both enabled");

    if (third != CNA_INVALID_HANDLE) {
        (void)cna_directional_light_destroy(third);
    }
    if (second != CNA_INVALID_HANDLE) {
        (void)cna_directional_light_destroy(second);
    }
    if (same != CNA_INVALID_HANDLE) {
        (void)cna_directional_light_destroy(same);
    }
    if (first != CNA_INVALID_HANDLE) {
        (void)cna_directional_light_destroy(first);
    }
}

static CNA_Result inspect_texture(BasicEffectState* state, CNA_Handle device)
{
    CnaDemo* demo = state->demo;
    CNA_Texture2DCreateInfo info;
    CNA_Bool has_texture = CNA_FALSE;
    CNA_Handle returned = CNA_INVALID_HANDLE;
    CNA_Result result;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.width = 1U;
    info.height = 1U;
    info.format = CNA_SURFACE_FORMAT_COLOR;

    result = cna_texture2d_create(device, &info, &state->texture);
    if (!cna_demo_check_result(demo, result, "create a same-device Texture2D")) {
        return result;
    }
    cna_demo_check(
        demo,
        cna_basic_effect_set_texture(state->basic, state->texture) == CNA_RESULT_SUCCESS &&
            cna_basic_effect_get_texture(state->basic, &has_texture, &returned) == CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && returned == state->texture,
        "BasicEffect retains and returns the exact assigned same-device texture handle");
    cna_demo_check(
        demo,
        cna_texture2d_destroy(state->texture) == CNA_RESULT_INVALID_STATE &&
            cna_basic_effect_get_texture(state->basic, &has_texture, &returned) == CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && returned == state->texture,
        "the caller cannot destroy a texture while BasicEffect retains it");
    cna_demo_check(
        demo,
        cna_basic_effect_set_texture(state->basic, CNA_INVALID_HANDLE) == CNA_RESULT_SUCCESS &&
            cna_basic_effect_get_texture(state->basic, &has_texture, &returned) == CNA_RESULT_SUCCESS &&
            has_texture == CNA_FALSE && returned == CNA_INVALID_HANDLE &&
            cna_texture2d_destroy(state->texture) == CNA_RESULT_SUCCESS,
        "clearing the slot removes the retained handle and permits texture destruction");
    state->texture = CNA_INVALID_HANDLE;
    return CNA_RESULT_SUCCESS;
}

static void inspect_interface_failures(BasicEffectState* state, CNA_Handle device)
{
    CnaDemo* demo = state->demo;
    CNA_Matrix matrix = identity_matrix();
    CNA_Vector3 vector = {7.0F, 8.0F, 9.0F};
    CNA_DirectionalLightHandle light = UINT64_MAX;

    cna_demo_check_result(
        demo, cna_effect_create_empty(device, &state->empty),
        "create a plain Effect for interface refusal checks");
    cna_demo_check(
        demo,
        cna_effect_matrices_get_world(state->empty, &matrix) == CNA_RESULT_NOT_SUPPORTED &&
            cna_effect_fog_get_color(state->empty, &vector) == CNA_RESULT_NOT_SUPPORTED &&
            cna_effect_lights_get_ambient_color(state->empty, &vector) == CNA_RESULT_NOT_SUPPORTED &&
            cna_effect_lights_get_directional_light(state->empty, 0U, &light) ==
                CNA_RESULT_NOT_SUPPORTED &&
            light == CNA_INVALID_HANDLE,
        "a plain Effect refuses all three stock interfaces without publishing a light view");
    cna_demo_check(
        demo,
        cna_basic_effect_get_diffuse_color(state->empty, &vector) == CNA_RESULT_INVALID_HANDLE,
        "a plain Effect is also rejected by the concrete BasicEffect API");
}

static CNA_Result create_retained_member(BasicEffectState* state, CNA_Handle device)
{
    CNA_EffectHandle owner = CNA_INVALID_HANDLE;
    CNA_Result result = cna_basic_effect_create(device, &owner);
    if (!cna_demo_check_result(
            state->demo, result, "create a second BasicEffect for member-view retention")) {
        return result;
    }
    result = cna_effect_lights_get_directional_light(owner, 2U, &state->retained_light);
    if (!cna_demo_check_result(
            state->demo, result, "retain its third directional-light member view")) {
        (void)cna_effect_destroy(owner);
        return result;
    }
    cna_demo_check(
        state->demo,
        cna_effect_destroy(owner) == CNA_RESULT_SUCCESS,
        "destroying the root BasicEffect handle leaves the owned member view alive");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    BasicEffectState* state = (BasicEffectState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = inspect_standalone_light(state);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_basic_effect_create(device, &state->basic);
    if (!cna_demo_check_result(state->demo, result, "create the BasicEffect")) {
        return result;
    }
    inspect_defaults(state);
    inspect_round_trips(state);
    inspect_nested_lights(state);
    result = inspect_texture(state, device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    inspect_interface_failures(state, device);
    cna_demo_check(
        state->demo, cna_effect_apply(state->basic) == CNA_RESULT_SUCCESS,
        "the complete mutated BasicEffect state applies successfully");
    return create_retained_member(state, device);
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

static void cleanup(BasicEffectState* state)
{
    if (state->retained_light != CNA_INVALID_HANDLE) {
        (void)cna_directional_light_destroy(state->retained_light);
    }
    if (state->texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(state->texture);
    }
    if (state->empty != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->empty);
    }
    if (state->basic != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->basic);
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    BasicEffectState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool enabled = CNA_FALSE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.basic_effect_state", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&create_info, 0, sizeof(create_info));
    state.demo = &demo;
    state.basic = CNA_INVALID_HANDLE;
    state.empty = CNA_INVALID_HANDLE;
    state.texture = CNA_INVALID_HANDLE;
    state.retained_light = CNA_INVALID_HANDLE;
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
        "create the graphics device for BasicEffect and its texture");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the BasicEffect state audit");
        cna_demo_check(&demo, state.loads == 1, "the state audit ran in exactly one load callback");
        cna_demo_check(
            &demo,
            cna_directional_light_set_diffuse_color(
                state.retained_light, (CNA_Vector3){0.11F, 0.22F, 0.33F}) == CNA_RESULT_SUCCESS &&
                cna_directional_light_set_enabled(state.retained_light, CNA_TRUE) == CNA_RESULT_SUCCESS &&
                cna_directional_light_get_diffuse_color(state.retained_light, &vector) ==
                    CNA_RESULT_SUCCESS &&
                vector_is(vector, 0.11F, 0.22F, 0.33F) &&
                cna_directional_light_get_enabled(state.retained_light, &enabled) == CNA_RESULT_SUCCESS &&
                enabled == CNA_TRUE,
            "the nested member view remains mutable after its root handle and callback end");
        cna_demo_check(
            &demo,
            cna_directional_light_destroy(state.retained_light) == CNA_RESULT_SUCCESS,
            "releasing the retained member view ends its ownership interval");
        state.retained_light = CNA_INVALID_HANDLE;
    }
    cleanup(&state);
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager after every effect and light view");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
