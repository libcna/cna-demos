/* SPDX-License-Identifier: MS-PL */
/*
 * SkinnedEffect from C: its material and shader switches, required lighting, matrix/fog adapters,
 * same-device texture ownership and the exact 72-bone palette boundary. Bone arrays are copied in
 * and out atomically, then preserved by a type-specific clone after source destruction.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct SkinnedState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle effect;
    CNA_EffectHandle clone;
    CNA_Handle texture;
} SkinnedState;

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

static void inspect_defaults(SkinnedState* state)
{
    CnaDemo* demo = state->demo;
    const CNA_Matrix identity = identity_matrix();
    CNA_Matrix matrix = {0};
    CNA_Matrix bones[CNA_SKINNED_EFFECT_MAX_BONES];
    CNA_Vector3 vector = {9.0F, 9.0F, 9.0F};
    CNA_Bool boolean = CNA_TRUE;
    CNA_Handle texture = UINT64_MAX;
    int32_t weights = 0;
    float scalar = -1.0F;
    uint64_t count = 0U;
    uint32_t index;
    int all_identity = 1;

    cna_demo_check(
        demo,
        CNA_SKINNED_EFFECT_MAX_BONES == UINT32_C(72) &&
            cna_skinned_effect_copy_bone_transforms(
                state->effect, CNA_SKINNED_EFFECT_MAX_BONES, bones,
                CNA_SKINNED_EFFECT_MAX_BONES, &count) == CNA_RESULT_SUCCESS &&
            count == CNA_SKINNED_EFFECT_MAX_BONES,
        "a new SkinnedEffect exposes the complete fixed 72-matrix palette");
    for (index = 0U; index < CNA_SKINNED_EFFECT_MAX_BONES; ++index) {
        all_identity = all_identity && matrix_is(bones[index], identity);
    }
    cna_demo_check(demo, all_identity, "all 72 default bone transforms are independent identities");

    cna_demo_check(
        demo,
        cna_skinned_effect_get_diffuse_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F) &&
            cna_skinned_effect_get_emissive_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F) &&
            cna_skinned_effect_get_specular_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 1.0F, 1.0F, 1.0F) &&
            cna_skinned_effect_get_specular_power(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 16.0F && cna_skinned_effect_get_alpha(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS &&
            scalar == 1.0F,
        "the skinned material starts white, opaque and non-emissive with power-16 specular");
    cna_demo_check(
        demo,
        cna_skinned_effect_get_prefer_per_pixel_lighting(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE &&
            cna_skinned_effect_get_vertex_color_enabled(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE &&
            cna_skinned_effect_get_weights_per_vertex(state->effect, &weights) == CNA_RESULT_SUCCESS &&
            weights == 4 && cna_skinned_effect_get_texture(state->effect, &boolean, &texture) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE && texture == CNA_INVALID_HANDLE,
        "per-pixel and vertex-color modes start off, four weights are active and texture is empty");
    cna_demo_check(
        demo,
        cna_effect_matrices_get_world(state->effect, &matrix) == CNA_RESULT_SUCCESS &&
            matrix_is(matrix, identity) &&
            cna_effect_fog_get_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F) &&
            cna_effect_fog_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_FALSE && cna_effect_fog_get_start(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS &&
            scalar == 0.0F && cna_effect_fog_get_end(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 1.0F,
        "SkinnedEffect exposes identity world and canonical disabled-fog defaults");
    cna_demo_check(
        demo,
        cna_effect_lights_get_ambient_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.0F, 0.0F, 0.0F) &&
            cna_effect_lights_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "SkinnedEffect starts with required lighting enabled and zero ambient color");
}

static void inspect_material_and_interfaces(SkinnedState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Matrix world = identity_matrix();
    CNA_Matrix actual = {0};
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    CNA_Bool boolean = CNA_FALSE;
    float scalar = 0.0F;

    cna_demo_check(
        demo,
        cna_skinned_effect_set_diffuse_color(
            state->effect, (CNA_Vector3){0.1F, 0.2F, 0.3F}) == CNA_RESULT_SUCCESS &&
            cna_skinned_effect_set_emissive_color(
                state->effect, (CNA_Vector3){0.4F, 0.5F, 0.6F}) == CNA_RESULT_SUCCESS &&
            cna_skinned_effect_set_specular_color(
                state->effect, (CNA_Vector3){0.7F, 0.8F, 0.9F}) == CNA_RESULT_SUCCESS &&
            cna_skinned_effect_set_specular_power(state->effect, 64.0F) == CNA_RESULT_SUCCESS &&
            cna_skinned_effect_set_alpha(state->effect, -0.75F) == CNA_RESULT_SUCCESS &&
            cna_skinned_effect_get_diffuse_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.1F, 0.2F, 0.3F) &&
            cna_skinned_effect_get_emissive_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.4F, 0.5F, 0.6F) &&
            cna_skinned_effect_get_specular_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.7F, 0.8F, 0.9F) &&
            cna_skinned_effect_get_specular_power(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 64.0F && cna_skinned_effect_get_alpha(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS &&
            scalar == -0.75F,
        "all skinned material channels round-trip, including unclamped alpha and specular power");
    cna_demo_check(
        demo,
        cna_skinned_effect_set_prefer_per_pixel_lighting(state->effect, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_skinned_effect_set_vertex_color_enabled(state->effect, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_skinned_effect_get_prefer_per_pixel_lighting(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_skinned_effect_get_vertex_color_enabled(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "per-pixel lighting and the CNA vertex-color extension enable independently");
    cna_demo_check(
        demo,
        cna_skinned_effect_set_prefer_per_pixel_lighting(state->effect, UINT32_C(2)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_effect_set_vertex_color_enabled(state->effect, UINT32_C(2)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_effect_get_prefer_per_pixel_lighting(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE &&
            cna_skinned_effect_get_vertex_color_enabled(state->effect, &boolean) ==
                CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE,
        "non-canonical Booleans are refused without changing either shader switch");

    world.m41 = 3.0F;
    world.m42 = 4.0F;
    cna_demo_check(
        demo,
        cna_effect_matrices_set_world(state->effect, world) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_get_world(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, world) &&
            cna_effect_fog_set_color(state->effect, (CNA_Vector3){0.25F, 0.5F, 0.75F}) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_enabled(state->effect, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_start(state->effect, 5.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_set_end(state->effect, 15.0F) == CNA_RESULT_SUCCESS &&
            cna_effect_fog_get_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.25F, 0.5F, 0.75F) &&
            cna_effect_fog_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE && cna_effect_fog_get_start(state->effect, &scalar) ==
                CNA_RESULT_SUCCESS &&
            scalar == 5.0F && cna_effect_fog_get_end(state->effect, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 15.0F,
        "world and fog state round-trip through the shared skinned-effect interfaces");
    cna_demo_check(
        demo,
        cna_effect_lights_set_ambient_color(
            state->effect, (CNA_Vector3){0.2F, 0.3F, 0.4F}) == CNA_RESULT_SUCCESS &&
            cna_effect_lights_set_enabled(state->effect, CNA_FALSE) == CNA_RESULT_INVALID_STATE &&
            cna_effect_lights_get_enabled(state->effect, &boolean) == CNA_RESULT_SUCCESS &&
            boolean == CNA_TRUE && cna_effect_lights_enable_default(state->effect) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_lights_get_ambient_color(state->effect, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.05333332F, 0.09882354F, 0.1819608F),
        "required lighting refuses disable and the default preset replaces the authored ambient color");
}

static void inspect_weight_contract(SkinnedState* state)
{
    CnaDemo* demo = state->demo;
    int32_t weights = 0;
    cna_demo_check(
        demo,
        cna_skinned_effect_set_weights_per_vertex(state->effect, 1) == CNA_RESULT_SUCCESS &&
            cna_skinned_effect_get_weights_per_vertex(state->effect, &weights) == CNA_RESULT_SUCCESS &&
            weights == 1 && cna_skinned_effect_set_weights_per_vertex(state->effect, 2) ==
                CNA_RESULT_SUCCESS &&
            cna_skinned_effect_get_weights_per_vertex(state->effect, &weights) == CNA_RESULT_SUCCESS &&
            weights == 2 && cna_skinned_effect_set_weights_per_vertex(state->effect, 4) ==
                CNA_RESULT_SUCCESS &&
            cna_skinned_effect_get_weights_per_vertex(state->effect, &weights) == CNA_RESULT_SUCCESS &&
            weights == 4,
        "weights per vertex accepts and exposes exactly the canonical one, two and four modes");
    cna_demo_check(
        demo,
        cna_skinned_effect_set_weights_per_vertex(state->effect, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_effect_set_weights_per_vertex(state->effect, 3) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_effect_get_weights_per_vertex(state->effect, &weights) == CNA_RESULT_SUCCESS &&
            weights == 4,
        "zero and three weights are refused without changing the last valid four-weight mode");
}

static void inspect_bone_palette(SkinnedState* state)
{
    CnaDemo* demo = state->demo;
    CNA_Matrix authored[CNA_SKINNED_EFFECT_MAX_BONES];
    CNA_Matrix copied[CNA_SKINNED_EFFECT_MAX_BONES];
    CNA_Matrix too_many[CNA_SKINNED_EFFECT_MAX_BONES + 1U];
    CNA_Matrix guard = identity_matrix();
    uint64_t count = UINT64_MAX;
    uint32_t index;
    int equal = 1;

    for (index = 0U; index < CNA_SKINNED_EFFECT_MAX_BONES; ++index) {
        authored[index] = identity_matrix();
        authored[index].m41 = (float)index + 0.25F;
        authored[index].m42 = -(float)index - 0.5F;
        too_many[index] = authored[index];
    }
    too_many[CNA_SKINNED_EFFECT_MAX_BONES] = identity_matrix();
    cna_demo_check(
        demo,
        cna_skinned_effect_set_bone_transforms(
            state->effect, authored, CNA_SKINNED_EFFECT_MAX_BONES) == CNA_RESULT_SUCCESS,
        "the setter accepts and copies the exact maximum 72-bone palette");
    memset(copied, 0, sizeof(copied));
    cna_demo_check(
        demo,
        cna_skinned_effect_copy_bone_transforms(
            state->effect, CNA_SKINNED_EFFECT_MAX_BONES, copied,
            CNA_SKINNED_EFFECT_MAX_BONES, &count) == CNA_RESULT_SUCCESS &&
            count == CNA_SKINNED_EFFECT_MAX_BONES,
        "the getter returns the requested complete 72-bone palette");
    for (index = 0U; index < CNA_SKINNED_EFFECT_MAX_BONES; ++index) {
        equal = equal && matrix_is(copied[index], authored[index]);
    }
    cna_demo_check(
        demo, equal,
        "every copied bone retains its distinct translation through the maximum palette slot");

    authored[0].m41 = 999.0F;
    count = UINT64_MAX;
    cna_demo_check(
        demo,
        cna_skinned_effect_copy_bone_transforms(state->effect, 1U, copied, 1U, &count) ==
                CNA_RESULT_SUCCESS &&
            count == 1U && copied[0].m41 == 0.25F,
        "overwriting the caller array after Set does not change the copied native palette");
    cna_demo_check(
        demo,
        cna_skinned_effect_copy_bone_transforms(state->effect, 2U, &guard, 1U, &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == 2U && matrix_is(guard, identity_matrix()),
        "a short bone destination reports the requirement without a partial write");

    count = UINT64_C(77);
    cna_demo_check(
        demo,
        cna_skinned_effect_set_bone_transforms(state->effect, NULL, 0U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_effect_set_bone_transforms(
                state->effect, too_many, CNA_SKINNED_EFFECT_MAX_BONES + 1U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_effect_copy_bone_transforms(
                state->effect, 0U, copied, CNA_SKINNED_EFFECT_MAX_BONES, &count) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            count == UINT64_C(77) &&
            cna_skinned_effect_copy_bone_transforms(
                state->effect, CNA_SKINNED_EFFECT_MAX_BONES + 1U, copied,
                CNA_SKINNED_EFFECT_MAX_BONES, &count) == CNA_RESULT_INVALID_ARGUMENT &&
            count == UINT64_C(77),
        "empty and 73-bone set/copy requests are refused before changing outputs or state");
}

static CNA_Result inspect_texture_and_clone(SkinnedState* state, CNA_Handle device)
{
    CnaDemo* demo = state->demo;
    const CNA_Color pixel = {1U, 2U, 3U, 4U};
    CNA_Texture2DCreateInfo info;
    CNA_Handle cpu_texture = CNA_INVALID_HANDLE;
    CNA_Handle returned = CNA_INVALID_HANDLE;
    CNA_Bool has_texture = CNA_FALSE;
    CNA_Matrix bones[2];
    CNA_Vector3 vector = {0.0F, 0.0F, 0.0F};
    uint64_t count = 0U;
    CNA_Result result;

    result = cna_texture2d_create_cpu_only_rgba8(
        1U, 1U, CNA_SURFACE_FORMAT_COLOR, &pixel, 1U, &cpu_texture);
    if (!cna_demo_check_result(demo, result, "create an ownerless CPU Texture2D")) {
        return result;
    }
    cna_demo_check(
        demo,
        cna_skinned_effect_set_texture(state->effect, cpu_texture) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_effect_get_texture(state->effect, &has_texture, &returned) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_FALSE && returned == CNA_INVALID_HANDLE &&
            cna_texture2d_destroy(cpu_texture) == CNA_RESULT_SUCCESS,
        "the same-device rule refuses an ownerless texture and preserves the empty slot");

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.width = 1U;
    info.height = 1U;
    info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(device, &info, &state->texture);
    if (!cna_demo_check_result(demo, result, "create the same-device skinned texture")) {
        return result;
    }
    cna_demo_check(
        demo,
        cna_skinned_effect_set_texture(state->effect, state->texture) == CNA_RESULT_SUCCESS &&
            cna_skinned_effect_get_texture(state->effect, &has_texture, &returned) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && returned == state->texture &&
            cna_texture2d_destroy(state->texture) == CNA_RESULT_INVALID_STATE,
        "SkinnedEffect retains the exact same-device texture and blocks caller destruction");

    result = cna_effect_clone(state->effect, &state->clone);
    if (!cna_demo_check_result(demo, result, "clone the complete SkinnedEffect state")) {
        return result;
    }
    cna_demo_check(
        demo,
        cna_effect_destroy(state->effect) == CNA_RESULT_SUCCESS,
        "destroy the source effect while its clone retains the texture and palette");
    state->effect = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo,
        cna_skinned_effect_get_specular_color(state->clone, &vector) == CNA_RESULT_SUCCESS &&
            vector_is(vector, 0.7F, 0.8F, 0.9F) &&
            cna_skinned_effect_copy_bone_transforms(state->clone, 2U, bones, 2U, &count) ==
                CNA_RESULT_SUCCESS &&
            count == 2U && bones[0].m41 == 0.25F && bones[0].m42 == -0.5F &&
            bones[1].m41 == 1.25F && bones[1].m42 == -1.5F &&
            cna_skinned_effect_get_texture(state->clone, &has_texture, &returned) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && returned == state->texture,
        "the type-preserving clone keeps material, copied bones and exact texture ownership");
    cna_demo_check(
        demo,
        cna_skinned_effect_set_texture(state->clone, CNA_INVALID_HANDLE) == CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->texture) == CNA_RESULT_SUCCESS,
        "clearing the clone's final texture slot releases the retained resource");
    state->texture = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo, cna_effect_apply(state->clone) == CNA_RESULT_SUCCESS,
        "the cloned skinned state applies after its optional texture is cleared");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    SkinnedState* state = (SkinnedState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_skinned_effect_create(device, &state->effect);
    if (!cna_demo_check_result(state->demo, result, "create SkinnedEffect")) {
        return result;
    }
    inspect_defaults(state);
    inspect_material_and_interfaces(state);
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

static void cleanup(SkinnedState* state)
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
    SkinnedState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.skinned_effect_state", argc, argv, 1);
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
        "create the graphics device for SkinnedEffect and its texture");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the SkinnedEffect state audit");
        cna_demo_check(&demo, state.loads == 1, "the audit ran in exactly one load callback");
    }
    cleanup(&state);
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager after the effect and texture");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
