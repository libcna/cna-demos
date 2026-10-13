/* SPDX-License-Identifier: MS-PL */
/*
 * PbrEffect from C: core and glTF-extension material state, shared effect interfaces and cloning.
 * Texture slots are kept in their own focused ownership demo; nothing is drawn here.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct PbrMaterialState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle effect;
    CNA_EffectHandle clone;
} PbrMaterialState;

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

static void inspect_defaults(PbrMaterialState* state)
{
    CNA_Vector3 vector = {9.0F, 9.0F, 9.0F};
    CNA_Bool boolean = CNA_TRUE;
    CNA_AlphaModeEXT mode = UINT32_MAX;
    float scalar = -1.0F;

    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_diffuse_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F) &&
            cna_pbr_effect_get_alpha(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F &&
            cna_pbr_effect_get_emissive_factor(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F),
        "PbrEffect starts with a white opaque base factor and no emissive contribution");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_metallic_factor(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F &&
            cna_pbr_effect_get_roughness_factor(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F,
        "the core metallic and roughness factors both default to one");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_ior_ext(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.5F &&
            cna_pbr_effect_get_specular_factor_ext(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F &&
            cna_pbr_effect_get_specular_color_factor_ext(state->effect, &vector) ==
                CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F),
        "the glTF dielectric defaults are IOR 1.5 with full white specular response");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_normal_scale_ext(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F &&
            cna_pbr_effect_get_occlusion_strength_ext(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS && scalar == 1.0F,
        "normal and occlusion map strengths default to their authored values");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_vertex_color_enabled_ext(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS && boolean == CNA_FALSE &&
            cna_pbr_effect_get_encode_output_to_srgb_ext(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS && boolean == CNA_TRUE &&
            cna_pbr_effect_get_double_sided_ext(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE,
        "vertex color and double-sided modes start off while output encoding starts on");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_alpha_mode_ext(state->effect, &mode) == CNA_RESULT_SUCCESS &&
            mode == CNA_ALPHA_MODE_OPAQUE_EXT &&
            cna_pbr_effect_get_alpha_cutoff_ext(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 0.5F,
        "alpha coverage starts opaque with the glTF mask cutoff of one half");
}

static void inspect_material_mutations(PbrMaterialState* state)
{
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    float scalar = 0.0F;

    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_diffuse_color(
            state->effect, (CNA_Vector3){0.1F, 0.2F, 0.3F}) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_alpha(state->effect, -0.25F) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_emissive_factor(
                state->effect, (CNA_Vector3){0.4F, 0.5F, 0.6F}) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_diffuse_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.1F, 0.2F, 0.3F) &&
            cna_pbr_effect_get_alpha(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == -0.25F &&
            cna_pbr_effect_get_emissive_factor(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.4F, 0.5F, 0.6F),
        "base color, emissive factor and deliberately unclamped alpha round-trip exactly");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_metallic_factor(state->effect, -1.0F) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_roughness_factor(state->effect, 2.0F) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_metallic_factor(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == -1.0F &&
            cna_pbr_effect_get_roughness_factor(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 2.0F,
        "metallic and roughness factors retain values outside the rendering range without clamping");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_ior_ext(state->effect, -2.5F) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_specular_factor_ext(state->effect, 3.0F) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_specular_color_factor_ext(
                state->effect, (CNA_Vector3){0.25F, 0.5F, 0.75F}) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_ior_ext(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == -2.5F &&
            cna_pbr_effect_get_specular_factor_ext(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 3.0F &&
            cna_pbr_effect_get_specular_color_factor_ext(state->effect, &vector) ==
                CNA_RESULT_SUCCESS && vector_is(vector, 0.25F, 0.5F, 0.75F),
        "IOR, specular strength and color preserve the extension's unclamped authored factors");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_normal_scale_ext(state->effect, 0.0F) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_occlusion_strength_ext(state->effect, 0.75F) ==
                CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_normal_scale_ext(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 0.0F &&
            cna_pbr_effect_get_occlusion_strength_ext(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS && scalar == 0.75F,
        "normal flattening and partial occlusion strength round-trip independently");
}

static void inspect_coverage_modes(PbrMaterialState* state)
{
    CNA_Bool boolean = CNA_FALSE;
    CNA_AlphaModeEXT mode = UINT32_MAX;
    float cutoff = 0.0F;

    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_alpha_mode_ext(state->effect, CNA_ALPHA_MODE_OPAQUE_EXT) ==
                CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_alpha_mode_ext(state->effect, CNA_ALPHA_MODE_MASK_EXT) ==
                CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_alpha_mode_ext(state->effect, CNA_ALPHA_MODE_BLEND_EXT) ==
                CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_alpha_mode_ext(state->effect, &mode) == CNA_RESULT_SUCCESS &&
            mode == CNA_ALPHA_MODE_BLEND_EXT,
        "all three named alpha modes round-trip and remain distinct");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_alpha_mode_ext(
            state->effect, CNA_ALPHA_MODE_MAXIMUM_EXT + 1U) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_get_alpha_mode_ext(state->effect, &mode) == CNA_RESULT_SUCCESS &&
            mode == CNA_ALPHA_MODE_BLEND_EXT,
        "an undefined alpha mode is refused without changing the active blend mode");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_alpha_cutoff_ext(state->effect, 0.125F) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_vertex_color_enabled_ext(state->effect, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_double_sided_ext(state->effect, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_encode_output_to_srgb_ext(state->effect, CNA_FALSE) ==
                CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_alpha_cutoff_ext(state->effect, &cutoff) == CNA_RESULT_SUCCESS &&
            cutoff == 0.125F &&
            cna_pbr_effect_get_vertex_color_enabled_ext(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS && boolean == CNA_TRUE &&
            cna_pbr_effect_get_double_sided_ext(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_pbr_effect_get_encode_output_to_srgb_ext(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS && boolean == CNA_FALSE,
        "alpha cutoff, vertex color, double-sided and output-encoding switches mutate independently");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_vertex_color_enabled_ext(state->effect, UINT32_C(2)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_set_double_sided_ext(state->effect, UINT32_C(2)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_set_encode_output_to_srgb_ext(state->effect, UINT32_C(2)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_get_vertex_color_enabled_ext(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS && boolean == CNA_TRUE &&
            cna_pbr_effect_get_double_sided_ext(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_pbr_effect_get_encode_output_to_srgb_ext(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS && boolean == CNA_FALSE,
        "non-canonical Booleans are refused without changing any PBR switch");
}

static void inspect_shared_interfaces(PbrMaterialState* state)
{
    CNA_Matrix world = identity_matrix();
    CNA_Matrix actual = {0};
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool boolean = CNA_FALSE;
    float scalar = 0.0F;

    cna_demo_check(
        state->demo,
        cna_effect_matrices_get_world(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            memcmp(&actual, &world, sizeof(actual)) == 0 &&
            cna_effect_fog_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE &&
            cna_effect_lights_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_effect_lights_get_ambient_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F),
        "PbrEffect exposes identity matrices, disabled fog and always-enabled lighting");
    world.m41 = 2.0F;
    world.m42 = -3.0F;
    cna_demo_check(
        state->demo,
        cna_effect_matrices_set_world(state->effect, world) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_enabled(state->effect, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_end(state->effect, 9.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_lights_set_ambient_color(
                state->effect, (CNA_Vector3){0.2F, 0.3F, 0.4F}) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_get_world(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            memcmp(&actual, &world, sizeof(actual)) == 0 &&
            cna_effect_fog_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE && cna_effect_fog_get_end(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS && scalar == 9.0F &&
            cna_effect_lights_get_ambient_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.2F, 0.3F, 0.4F),
        "matrix, fog and ambient-light state round-trip through the shared interfaces");
    cna_demo_check(
        state->demo,
        cna_effect_lights_set_enabled(state->effect, CNA_FALSE) == CNA_RESULT_INVALID_STATE &&
            cna_effect_lights_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_effect_lights_enable_default(state->effect) == CNA_RESULT_SUCCESS &&
            cna_effect_lights_get_ambient_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.05333332F, 0.09882354F, 0.1819608F),
        "required lighting refuses disable and the default-light preset replaces ambient state");
}

static CNA_Result inspect_clone(PbrMaterialState* state)
{
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool boolean = CNA_FALSE;
    CNA_AlphaModeEXT mode = UINT32_MAX;
    float scalar = 0.0F;
    CNA_Result result = cna_effect_clone(state->effect, &state->clone);

    if (!cna_demo_check_result(state->demo, result, "clone the complete PbrEffect material state")) {
        return result;
    }
    cna_demo_check(
        state->demo, cna_effect_destroy(state->effect) == CNA_RESULT_SUCCESS,
        "destroy the source while its type-preserving clone remains independent");
    state->effect = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_diffuse_color(state->clone, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.1F, 0.2F, 0.3F) &&
            cna_pbr_effect_get_roughness_factor(state->clone, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 2.0F &&
            cna_pbr_effect_get_specular_color_factor_ext(state->clone, &vector) ==
                CNA_RESULT_SUCCESS && vector_is(vector, 0.25F, 0.5F, 0.75F),
        "the clone retains core metallic-roughness and dielectric state after source destruction");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_vertex_color_enabled_ext(state->clone, &boolean) ==
                CNA_RESULT_SUCCESS && boolean == CNA_TRUE,
        "the clone retains the enabled vertex-color mode (CNA-DEMOS-ISSUE-070)");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_alpha_mode_ext(state->clone, &mode) == CNA_RESULT_SUCCESS &&
            mode == CNA_ALPHA_MODE_BLEND_EXT,
        "the clone retains the authored alpha mode (CNA-DEMOS-ISSUE-070)");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_alpha_cutoff_ext(state->clone, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 0.125F,
        "the clone retains the authored alpha cutoff (CNA-DEMOS-ISSUE-070)");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_double_sided_ext(state->clone, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "the clone retains double-sided rendering (CNA-DEMOS-ISSUE-070)");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_encode_output_to_srgb_ext(state->clone, &boolean) ==
                CNA_RESULT_SUCCESS && boolean == CNA_FALSE,
        "the clone retains the independent output-encoding switch");
    cna_demo_check(
        state->demo, cna_effect_apply(state->clone) == CNA_RESULT_SUCCESS,
        "the retained PbrEffect clone applies without drawing");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    PbrMaterialState* state = (PbrMaterialState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_pbr_effect_create(device, &state->effect);
    if (!cna_demo_check_result(state->demo, result, "create PbrEffect")) {
        return result;
    }
    inspect_defaults(state);
    inspect_material_mutations(state);
    inspect_coverage_modes(state);
    inspect_shared_interfaces(state);
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

static void cleanup(PbrMaterialState* state)
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
    PbrMaterialState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.pbr_effect_material_state", argc, argv, 1);
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
        &demo, cna_demo_use_hidef(game, &manager), "create the graphics device for PbrEffect");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the PBR material state audit");
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
