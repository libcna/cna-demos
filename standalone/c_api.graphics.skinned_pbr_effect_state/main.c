/* SPDX-License-Identifier: MS-PL */
/*
 * SkinnedPbrEffect from C: shared PBR routes plus the weight and 72-bone palette contract.
 * A same-device texture makes shared ownership observable across a type-preserving clone.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct SkinnedPbrState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle effect;
    CNA_EffectHandle clone;
    CNA_Handle texture;
} SkinnedPbrState;

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

static int matrix_is(CNA_Matrix value, CNA_Matrix expected)
{
    return memcmp(&value, &expected, sizeof(value)) == 0;
}

static void inspect_defaults_and_shared_routes(SkinnedPbrState* state)
{
    const CNA_Matrix identity = identity_matrix();
    CNA_Matrix matrix = {0};
    CNA_Matrix bones[CNA_SKINNED_PBR_EFFECT_MAX_BONES];
    CNA_Vector3 vector = {9.0F, 9.0F, 9.0F};
    CNA_Bool boolean = CNA_TRUE;
    CNA_Bool present = CNA_TRUE;
    CNA_Handle texture = UINT64_MAX;
    int32_t weights = 0;
    float scalar = -1.0F;
    uint64_t count = 0U;
    uint32_t index;
    int all_identity = 1;

    cna_demo_check(
        state->demo,
        CNA_SKINNED_PBR_EFFECT_MAX_BONES == UINT32_C(72) &&
            cna_skinned_pbr_effect_copy_bone_transforms(
                state->effect, CNA_SKINNED_PBR_EFFECT_MAX_BONES, bones,
                CNA_SKINNED_PBR_EFFECT_MAX_BONES, &count) == CNA_RESULT_SUCCESS &&
            count == CNA_SKINNED_PBR_EFFECT_MAX_BONES,
        "a new SkinnedPbrEffect exposes the complete fixed 72-matrix palette");
    for (index = 0U; index < CNA_SKINNED_PBR_EFFECT_MAX_BONES; ++index) {
        all_identity = all_identity && matrix_is(bones[index], identity);
    }
    cna_demo_check(state->demo, all_identity,
                   "all 72 default SkinnedPbrEffect bone transforms are identities");
    cna_demo_check(
        state->demo,
        cna_skinned_pbr_effect_get_weights_per_vertex(state->effect, &weights) ==
                CNA_RESULT_SUCCESS && weights == 4,
        "SkinnedPbrEffect defaults to four blend weights per vertex");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_diffuse_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F) &&
            cna_pbr_effect_get_alpha(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F &&
            cna_pbr_effect_get_metallic_factor(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F &&
            cna_pbr_effect_get_roughness_factor(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F &&
            cna_pbr_effect_get_emissive_factor(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F),
        "the shared PBR route family exposes canonical material defaults on the skinned type");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_vertex_color_enabled_ext(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS && boolean == CNA_FALSE &&
            cna_pbr_effect_get_texture(
                state->effect, CNA_PBR_TEXTURE_BASE_COLOR, &present, &texture) ==
                CNA_RESULT_SUCCESS && present == CNA_FALSE && texture == CNA_INVALID_HANDLE,
        "shared PBR vertex-color and base-texture routes accept SkinnedPbrEffect");
    cna_demo_check(
        state->demo,
        cna_effect_matrices_get_world(state->effect, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, identity) &&
            cna_effect_fog_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE &&
            cna_effect_lights_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "the skinned PBR type exposes identity matrices, disabled fog and required lighting");
}

static void inspect_shared_mutations(SkinnedPbrState* state)
{
    CNA_Matrix world = identity_matrix();
    CNA_Matrix actual = {0};
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool boolean = CNA_FALSE;
    float scalar = 0.0F;

    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_diffuse_color(
            state->effect, (CNA_Vector3){0.1F, 0.2F, 0.3F}) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_alpha(state->effect, -0.5F) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_metallic_factor(state->effect, 0.25F) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_roughness_factor(state->effect, 0.75F) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_set_emissive_factor(
                state->effect, (CNA_Vector3){0.4F, 0.5F, 0.6F}) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_diffuse_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.1F, 0.2F, 0.3F) &&
            cna_pbr_effect_get_alpha(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == -0.5F &&
            cna_pbr_effect_get_metallic_factor(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 0.25F &&
            cna_pbr_effect_get_roughness_factor(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 0.75F &&
            cna_pbr_effect_get_emissive_factor(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.4F, 0.5F, 0.6F),
        "shared PBR material setters round-trip through SkinnedPbrEffect, including unclamped alpha");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_vertex_color_enabled_ext(state->effect, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_vertex_color_enabled_ext(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS && boolean == CNA_TRUE &&
            cna_pbr_effect_set_vertex_color_enabled_ext(state->effect, CNA_FALSE) ==
                CNA_RESULT_SUCCESS,
        "the shared vertex-color switch mutates on SkinnedPbrEffect");
    world.m41 = 3.0F;
    world.m42 = -4.0F;
    cna_demo_check(
        state->demo,
        cna_effect_matrices_set_world(state->effect, world) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_enabled(state->effect, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_get_world(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, world) &&
            cna_effect_fog_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "shared matrix and fog state round-trip on the skinned PBR type");
    cna_demo_check(
        state->demo,
        cna_effect_lights_set_enabled(state->effect, CNA_FALSE) == CNA_RESULT_INVALID_STATE &&
            cna_effect_lights_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "SkinnedPbrEffect refuses to disable its required lighting without mutation");
}

static void inspect_weight_contract(SkinnedPbrState* state)
{
    int32_t weights = 0;
    cna_demo_check(
        state->demo,
        cna_skinned_pbr_effect_set_weights_per_vertex(state->effect, 1) == CNA_RESULT_SUCCESS &&
            cna_skinned_pbr_effect_get_weights_per_vertex(state->effect, &weights) ==
                CNA_RESULT_SUCCESS && weights == 1 &&
            cna_skinned_pbr_effect_set_weights_per_vertex(state->effect, 2) ==
                CNA_RESULT_SUCCESS &&
            cna_skinned_pbr_effect_get_weights_per_vertex(state->effect, &weights) ==
                CNA_RESULT_SUCCESS && weights == 2 &&
            cna_skinned_pbr_effect_set_weights_per_vertex(state->effect, 4) ==
                CNA_RESULT_SUCCESS &&
            cna_skinned_pbr_effect_get_weights_per_vertex(state->effect, &weights) ==
                CNA_RESULT_SUCCESS && weights == 4,
        "weights per vertex accepts and exposes exactly the one, two and four modes");
    cna_demo_check(
        state->demo,
        cna_skinned_pbr_effect_set_weights_per_vertex(state->effect, 0) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_pbr_effect_set_weights_per_vertex(state->effect, 3) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_pbr_effect_get_weights_per_vertex(state->effect, &weights) ==
                CNA_RESULT_SUCCESS && weights == 4,
        "zero and three weights are refused without changing the last valid mode");
}

static void inspect_bone_palette(SkinnedPbrState* state)
{
    CNA_Matrix authored[CNA_SKINNED_PBR_EFFECT_MAX_BONES];
    CNA_Matrix copied[CNA_SKINNED_PBR_EFFECT_MAX_BONES];
    CNA_Matrix too_many[CNA_SKINNED_PBR_EFFECT_MAX_BONES + 1U];
    CNA_Matrix guard = identity_matrix();
    uint64_t count = UINT64_MAX;
    uint32_t index;
    int equal = 1;

    for (index = 0U; index < CNA_SKINNED_PBR_EFFECT_MAX_BONES; ++index) {
        authored[index] = identity_matrix();
        authored[index].m41 = (float)index + 0.5F;
        authored[index].m42 = -(float)index - 0.75F;
        too_many[index] = authored[index];
    }
    too_many[CNA_SKINNED_PBR_EFFECT_MAX_BONES] = identity_matrix();
    cna_demo_check(
        state->demo,
        cna_skinned_pbr_effect_set_bone_transforms(
            state->effect, authored, CNA_SKINNED_PBR_EFFECT_MAX_BONES) == CNA_RESULT_SUCCESS,
        "the setter accepts and copies the exact maximum 72-bone palette");
    authored[0].m41 = 999.0F;
    memset(copied, 0, sizeof(copied));
    cna_demo_check(
        state->demo,
        cna_skinned_pbr_effect_copy_bone_transforms(
            state->effect, CNA_SKINNED_PBR_EFFECT_MAX_BONES, copied,
            CNA_SKINNED_PBR_EFFECT_MAX_BONES, &count) == CNA_RESULT_SUCCESS &&
            count == CNA_SKINNED_PBR_EFFECT_MAX_BONES && copied[0].m41 == 0.5F &&
            copied[CNA_SKINNED_PBR_EFFECT_MAX_BONES - 1U].m41 == 71.5F,
        "the effect owns its input copy and returns the complete palette through the final slot");
    for (index = 0U; index < CNA_SKINNED_PBR_EFFECT_MAX_BONES; ++index) {
        CNA_Matrix expected = identity_matrix();
        expected.m41 = (float)index + 0.5F;
        expected.m42 = -(float)index - 0.75F;
        equal = equal && matrix_is(copied[index], expected);
    }
    cna_demo_check(state->demo, equal,
                   "every authored matrix survives the maximum-size round trip exactly");

    copied[0] = guard;
    count = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_skinned_pbr_effect_copy_bone_transforms(
            state->effect, 2U, copied, 1U, &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            count == 2U && matrix_is(copied[0], guard),
        "a short destination reports the required count without a partial palette write");
    cna_demo_check(
        state->demo,
        cna_skinned_pbr_effect_set_bone_transforms(state->effect, NULL, 0U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_pbr_effect_set_bone_transforms(
                state->effect, too_many, CNA_SKINNED_PBR_EFFECT_MAX_BONES + 1U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_pbr_effect_copy_bone_transforms(
                state->effect, 0U, copied, CNA_SKINNED_PBR_EFFECT_MAX_BONES, &count) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_pbr_effect_copy_bone_transforms(
                state->effect, CNA_SKINNED_PBR_EFFECT_MAX_BONES + 1U, copied,
                CNA_SKINNED_PBR_EFFECT_MAX_BONES, &count) == CNA_RESULT_INVALID_ARGUMENT,
        "empty and over-maximum palette requests are refused at both boundaries");
    count = 0U;
    cna_demo_check(
        state->demo,
        cna_skinned_pbr_effect_copy_bone_transforms(
            state->effect, 2U, copied, 2U, &count) == CNA_RESULT_SUCCESS &&
            count == 2U && copied[0].m41 == 0.5F && copied[1].m41 == 1.5F,
        "invalid palette requests preserve the previously authored bone state");
}

static CNA_Result inspect_texture_and_clone(SkinnedPbrState* state, CNA_Handle device)
{
    CNA_Texture2DCreateInfo info;
    CNA_Matrix bones[2];
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool present = CNA_FALSE;
    CNA_Handle returned = CNA_INVALID_HANDLE;
    uint64_t count = 0U;
    CNA_Result result;

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.width = 1U;
    info.height = 1U;
    info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(device, &info, &state->texture);
    if (!cna_demo_check_result(state->demo, result, "create the same-device skinned PBR texture")) {
        return result;
    }
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_texture(
            state->effect, CNA_PBR_TEXTURE_BASE_COLOR, state->texture) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_texture(
                state->effect, CNA_PBR_TEXTURE_BASE_COLOR, &present, &returned) ==
                CNA_RESULT_SUCCESS && present == CNA_TRUE && returned == state->texture &&
            cna_texture2d_destroy(state->texture) == CNA_RESULT_INVALID_STATE,
        "the shared base-color slot retains the exact texture on SkinnedPbrEffect");
    result = cna_effect_clone(state->effect, &state->clone);
    if (!cna_demo_check_result(state->demo, result, "clone the complete SkinnedPbrEffect state")) {
        return result;
    }
    cna_demo_check(
        state->demo, cna_effect_destroy(state->effect) == CNA_RESULT_SUCCESS,
        "destroy the source while its clone retains material, texture and palette");
    state->effect = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        cna_pbr_effect_get_diffuse_color(state->clone, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.1F, 0.2F, 0.3F) &&
            cna_skinned_pbr_effect_copy_bone_transforms(
                state->clone, 2U, bones, 2U, &count) == CNA_RESULT_SUCCESS && count == 2U &&
            bones[0].m41 == 0.5F && bones[0].m42 == -0.75F &&
            bones[1].m41 == 1.5F && bones[1].m42 == -1.75F &&
            cna_pbr_effect_get_texture(
                state->clone, CNA_PBR_TEXTURE_BASE_COLOR, &present, &returned) ==
                CNA_RESULT_SUCCESS && present == CNA_TRUE && returned == state->texture,
        "the type-preserving clone keeps shared material, copied bones and texture ownership");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_texture(
            state->clone, CNA_PBR_TEXTURE_BASE_COLOR, CNA_INVALID_HANDLE) == CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->texture) == CNA_RESULT_SUCCESS,
        "clearing the clone's final texture slot releases the retained resource");
    state->texture = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo, cna_effect_apply(state->clone) == CNA_RESULT_SUCCESS,
        "the retained SkinnedPbrEffect clone applies without drawing");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    SkinnedPbrState* state = (SkinnedPbrState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_skinned_pbr_effect_create(device, &state->effect);
    if (!cna_demo_check_result(state->demo, result, "create SkinnedPbrEffect")) {
        return result;
    }
    inspect_defaults_and_shared_routes(state);
    inspect_shared_mutations(state);
    inspect_weight_contract(state);
    inspect_bone_palette(state);
    return inspect_texture_and_clone(state, device);
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

static void cleanup(SkinnedPbrState* state)
{
    if (state->clone != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->clone);
    }
    if (state->effect != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->effect);
    }
    if (state->texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(state->texture);
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    SkinnedPbrState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.skinned_pbr_effect_state", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&create_info, 0, sizeof(create_info));
    state.demo = &demo;
    state.effect = CNA_INVALID_HANDLE;
    state.clone = CNA_INVALID_HANDLE;
    state.texture = CNA_INVALID_HANDLE;
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
        "create the graphics device for SkinnedPbrEffect");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the SkinnedPbrEffect state audit");
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
