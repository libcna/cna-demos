/* SPDX-License-Identifier: MS-PL */
/*
 * EnvironmentMapEffect from C: material, cube-map blend controls, IEffectMatrices, IEffectFog and
 * the always-enabled IEffectLights implementation. A real same-device Texture2D and TextureCube
 * make the two distinct ownership slots observable across cloning and source destruction.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct EnvironmentState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle effect;
    CNA_EffectHandle clone;
    CNA_Handle diffuse_texture;
    CNA_Handle environment_cube;
} EnvironmentState;

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

static void inspect_defaults(EnvironmentState* state)
{
    CnaDemo* demo = state->demo;
    const CNA_Matrix identity = identity_matrix();
    CNA_Matrix matrix = {0};
    CNA_Vector3 vector = {9.0F, 9.0F, 9.0F};
    CNA_Bool boolean = CNA_FALSE;
    CNA_Bool has_texture = CNA_TRUE;
    CNA_Handle texture = UINT64_MAX;
    CNA_DirectionalLightHandle lights[3] = {
        CNA_INVALID_HANDLE, CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    float scalar = -1.0F;
    unsigned int index;

    cna_demo_check(
        demo,
        cna_environment_map_effect_get_diffuse_color(state->effect, &vector) ==
                CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F) &&
            cna_environment_map_effect_get_emissive_color(state->effect, &vector) ==
                CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F) &&
            cna_environment_map_effect_get_alpha(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F,
        "EnvironmentMapEffect starts with the canonical white opaque non-emissive material");
    cna_demo_check(
        demo,
        cna_environment_map_effect_get_amount(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F &&
            cna_environment_map_effect_get_specular(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F) &&
            cna_environment_map_effect_get_fresnel_factor(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS &&
            scalar == 1.0F,
        "environment amount and Fresnel start at one while its specular tint starts black");
    cna_demo_check(
        demo,
        cna_environment_map_effect_get_texture(state->effect, &has_texture, &texture) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_FALSE && texture == CNA_INVALID_HANDLE &&
            cna_environment_map_effect_get_environment_map(
                state->effect, &has_texture, &texture) == CNA_RESULT_SUCCESS &&
            has_texture == CNA_FALSE && texture == CNA_INVALID_HANDLE,
        "the diffuse and cube-map ownership slots start independently empty");
    cna_demo_check(
        demo,
        cna_effect_matrices_get_world(state->effect, &matrix) == CNA_RESULT_SUCCESS &&
            memcmp(&matrix, &identity, sizeof(matrix)) == 0 &&
            cna_effect_fog_get_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F) &&
            cna_effect_fog_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE && cna_effect_fog_get_start(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS &&
            scalar == 0.0F && cna_effect_fog_get_end(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F,
        "the shared matrix and fog interfaces expose their canonical defaults");
    cna_demo_check(
        demo,
        cna_effect_lights_get_ambient_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F) &&
            cna_effect_lights_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "EnvironmentMapEffect starts with zero ambient light but lighting permanently enabled");

    for (index = 0U; index < 3U; ++index) {
        (void)cna_effect_lights_get_directional_light(state->effect, index, &lights[index]);
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
        "only the first EnvironmentMapEffect directional light starts enabled");
    for (index = 0U; index < 3U; ++index) {
        if (lights[index] != CNA_INVALID_HANDLE) {
            (void)cna_directional_light_destroy(lights[index]);
        }
    }
}

static void inspect_state(EnvironmentState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Matrix world = identity_matrix();
    CNA_Matrix actual = {0};
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool boolean = CNA_FALSE;
    CNA_DirectionalLightHandle lights[3] = {
        CNA_INVALID_HANDLE, CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    float scalar = 0.0F;
    unsigned int index;

    cna_demo_check(
        demo,
        cna_environment_map_effect_set_diffuse_color(
            state->effect, (CNA_Vector3){0.2F, 0.4F, 0.6F}) == CNA_RESULT_SUCCESS &&
            cna_environment_map_effect_set_emissive_color(
                state->effect, (CNA_Vector3){0.1F, 0.3F, 0.5F}) == CNA_RESULT_SUCCESS &&
            cna_environment_map_effect_set_alpha(state->effect, -0.5F) == CNA_RESULT_SUCCESS &&
            cna_environment_map_effect_get_diffuse_color(state->effect, &vector) ==
                CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.2F, 0.4F, 0.6F) &&
            cna_environment_map_effect_get_emissive_color(state->effect, &vector) ==
                CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.1F, 0.3F, 0.5F) &&
            cna_environment_map_effect_get_alpha(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == -0.5F,
        "diffuse, emissive and deliberately unclamped alpha state round-trip exactly");
    cna_demo_check(
        demo,
        cna_environment_map_effect_set_amount(state->effect, 1.5F) == CNA_RESULT_SUCCESS &&
            cna_environment_map_effect_set_specular(
                state->effect, (CNA_Vector3){0.7F, 0.8F, 0.9F}) == CNA_RESULT_SUCCESS &&
            cna_environment_map_effect_set_fresnel_factor(state->effect, -0.25F) ==
                CNA_RESULT_SUCCESS &&
            cna_environment_map_effect_get_amount(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.5F &&
            cna_environment_map_effect_get_specular(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.7F, 0.8F, 0.9F) &&
            cna_environment_map_effect_get_fresnel_factor(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS &&
            scalar == -0.25F,
        "environment amount, specular tint and Fresnel factor retain unclamped authored values");

    world.m41 = 2.0F;
    world.m42 = -3.0F;
    cna_demo_check(
        demo,
        cna_effect_matrices_set_world(state->effect, world) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_get_world(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            memcmp(&actual, &world, sizeof(actual)) == 0 &&
            cna_effect_fog_set_color(state->effect, (CNA_Vector3){0.25F, 0.5F, 0.75F}) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_enabled(state->effect, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_start(state->effect, 4.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_end(state->effect, 12.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_get_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.25F, 0.5F, 0.75F) &&
            cna_effect_fog_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE && cna_effect_fog_get_start(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS &&
            scalar == 4.0F && cna_effect_fog_get_end(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 12.0F,
        "world and fog state round-trip through the shared interfaces");

    cna_demo_check(
        demo,
        cna_effect_lights_set_ambient_color(
            state->effect, (CNA_Vector3){0.15F, 0.25F, 0.35F}) == CNA_RESULT_SUCCESS &&
            cna_effect_lights_set_enabled(state->effect, CNA_FALSE) == CNA_RESULT_INVALID_STATE &&
            cna_effect_lights_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_effect_lights_set_enabled(state->effect, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_effect_lights_get_ambient_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.15F, 0.25F, 0.35F),
        "ambient color is mutable while EnvironmentMapEffect refuses to disable required lighting");

    cna_demo_check_result(
        demo, cna_effect_lights_enable_default(state->effect),
        "apply the canonical three-light preset to EnvironmentMapEffect");
    for (index = 0U; index < 3U; ++index) {
        (void)cna_effect_lights_get_directional_light(state->effect, index, &lights[index]);
    }
    cna_demo_check(
        demo,
        cna_effect_lights_get_ambient_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.05333332F, 0.09882354F, 0.1819608F) &&
            lights[0] != CNA_INVALID_HANDLE && lights[1] != CNA_INVALID_HANDLE &&
            lights[2] != CNA_INVALID_HANDLE &&
            cna_directional_light_get_enabled(lights[0], &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_directional_light_get_enabled(lights[1], &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_directional_light_get_enabled(lights[2], &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "the default preset replaces ambient state and enables all three directional lights");
    for (index = 0U; index < 3U; ++index) {
        if (lights[index] != CNA_INVALID_HANDLE) {
            (void)cna_directional_light_destroy(lights[index]);
        }
    }
}

static CNA_Result create_textures(EnvironmentState* state, CNA_Handle device)
{
    CNA_Texture2DCreateInfo texture_info;
    CNA_TextureCubeCreateInfo cube_info;
    CNA_Result result;
    memset(&texture_info, 0, sizeof(texture_info));
    texture_info.struct_size = (uint32_t)sizeof(texture_info);
    texture_info.struct_version = UINT32_C(1);
    texture_info.width = 1U;
    texture_info.height = 1U;
    texture_info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(device, &texture_info, &state->diffuse_texture);
    if (!cna_demo_check_result(state->demo, result, "create the diffuse Texture2D")) {
        return result;
    }

    memset(&cube_info, 0, sizeof(cube_info));
    cube_info.struct_size = (uint32_t)sizeof(cube_info);
    cube_info.struct_version = UINT32_C(1);
    cube_info.size = 1U;
    cube_info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texturecube_create(device, &cube_info, &state->environment_cube);
    cna_demo_check_result(state->demo, result, "create the environment TextureCube");
    return result;
}

static void inspect_texture_ownership(EnvironmentState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Bool present = CNA_FALSE;
    CNA_Handle texture = CNA_INVALID_HANDLE;

    cna_demo_check(
        demo,
        cna_environment_map_effect_set_texture(state->effect, state->diffuse_texture) ==
                CNA_RESULT_SUCCESS &&
            cna_environment_map_effect_set_environment_map(
                state->effect, state->environment_cube) == CNA_RESULT_SUCCESS &&
            cna_environment_map_effect_get_texture(state->effect, &present, &texture) ==
                CNA_RESULT_SUCCESS &&
            present == CNA_TRUE && texture == state->diffuse_texture &&
            cna_environment_map_effect_get_environment_map(state->effect, &present, &texture) ==
                CNA_RESULT_SUCCESS &&
            present == CNA_TRUE && texture == state->environment_cube,
        "the effect retains and returns exact handles for its distinct 2D and cube slots");
    cna_demo_check(
        demo,
        cna_texture2d_destroy(state->diffuse_texture) == CNA_RESULT_INVALID_STATE &&
            cna_texturecube_destroy(state->environment_cube) == CNA_RESULT_INVALID_STATE,
        "neither texture can be destroyed while EnvironmentMapEffect retains it");
}

static CNA_Result clone_and_release_source(EnvironmentState* state)
{
    CNA_Result result = cna_effect_clone(state->effect, &state->clone);
    if (!cna_demo_check_result(state->demo, result, "clone the complete EnvironmentMapEffect")) {
        return result;
    }
    cna_demo_check(
        state->demo,
        cna_effect_destroy(state->effect) == CNA_RESULT_SUCCESS,
        "destroy the source effect while its independent clone retains both textures");
    state->effect = CNA_INVALID_HANDLE;
    return CNA_RESULT_SUCCESS;
}

static void inspect_clone_and_release(EnvironmentState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool present = CNA_FALSE;
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Bool lighting = CNA_FALSE;
    float scalar = 0.0F;

    cna_demo_check(
        demo,
        cna_environment_map_effect_get_diffuse_color(state->clone, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.2F, 0.4F, 0.6F) &&
            cna_environment_map_effect_get_amount(state->clone, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.5F &&
            cna_environment_map_effect_get_fresnel_factor(state->clone, &scalar) ==
                CNA_RESULT_SUCCESS &&
            scalar == -0.25F &&
            cna_effect_lights_get_enabled(state->clone, &lighting) == CNA_RESULT_SUCCESS &&
            lighting == CNA_TRUE,
        "the type-preserving clone retains material, environment and required-lighting state");
    cna_demo_check(
        demo,
        cna_environment_map_effect_get_texture(state->clone, &present, &texture) ==
                CNA_RESULT_SUCCESS &&
            present == CNA_TRUE && texture == state->diffuse_texture &&
            cna_environment_map_effect_get_environment_map(state->clone, &present, &texture) ==
                CNA_RESULT_SUCCESS &&
            present == CNA_TRUE && texture == state->environment_cube,
        "the clone retains the exact two texture handles after source destruction");
    cna_demo_check(
        demo,
        cna_environment_map_effect_set_texture(state->clone, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->diffuse_texture) == CNA_RESULT_SUCCESS,
        "clearing the clone's diffuse slot releases the Texture2D independently");
    state->diffuse_texture = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo,
        cna_environment_map_effect_set_environment_map(state->clone, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_texturecube_destroy(state->environment_cube) == CNA_RESULT_SUCCESS,
        "clearing the clone's environment slot releases the TextureCube independently");
    state->environment_cube = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo, cna_effect_apply(state->clone) == CNA_RESULT_SUCCESS,
        "the configured clone still applies after both optional textures are cleared");
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    EnvironmentState* state = (EnvironmentState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_environment_map_effect_create(device, &state->effect);
    if (!cna_demo_check_result(state->demo, result, "create EnvironmentMapEffect")) {
        return result;
    }
    inspect_defaults(state);
    inspect_state(state);
    result = create_textures(state, device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    inspect_texture_ownership(state);
    result = clone_and_release_source(state);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    inspect_clone_and_release(state);
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

static void cleanup(EnvironmentState* state)
{
    if (state->clone != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->clone);
    }
    if (state->effect != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->effect);
    }
    if (state->environment_cube != CNA_INVALID_HANDLE) {
        (void)cna_texturecube_destroy(state->environment_cube);
    }
    if (state->diffuse_texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(state->diffuse_texture);
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    EnvironmentState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.environment_map_effect_state", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&create_info, 0, sizeof(create_info));
    state.demo = &demo;
    state.effect = CNA_INVALID_HANDLE;
    state.clone = CNA_INVALID_HANDLE;
    state.diffuse_texture = CNA_INVALID_HANDLE;
    state.environment_cube = CNA_INVALID_HANDLE;
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
        "create the graphics device for EnvironmentMapEffect and both textures");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the EnvironmentMapEffect state audit");
        cna_demo_check(&demo, state.loads == 1, "the audit ran in exactly one load callback");
    }
    cleanup(&state);
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager after the effect and both textures");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
