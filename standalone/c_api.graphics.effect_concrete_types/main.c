/* SPDX-License-Identifier: MS-PL */
/*
 * Concrete Effect adapters in C: EffectMaterial keeps parameter textures alive after the caller
 * releases its handle, while SpriteEffect exposes the exact stock identity used by SpriteBatch.
 * Their different canonical Clone results are explicit: material inherits base Effect.Clone,
 * whereas SpriteEffect preserves its concrete type.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ConcreteEffectState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle source;
    CNA_EffectHandle material;
    CNA_EffectHandle material_clone;
    CNA_Handle texture;
    CNA_EffectHandle sprite;
    CNA_EffectHandle sprite_clone;
} ConcreteEffectState;

static int bytes_are(const void* data, size_t count, unsigned char value)
{
    const unsigned char* bytes = (const unsigned char*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != value) {
            return 0;
        }
    }
    return 1;
}

static int effect_type_is(CNA_EffectHandle effect, const char* expected)
{
    char destination[64];
    const uint64_t expected_count = (uint64_t)strlen(expected);
    uint64_t count = UINT64_MAX;
    memset(destination, 0x5A, sizeof(destination));
    return cna_effect_get_type_name_byte_count(effect, &count) == CNA_RESULT_SUCCESS &&
        count == expected_count &&
        cna_effect_copy_type_name(effect, destination, expected_count - UINT64_C(1), &count) ==
            CNA_RESULT_BUFFER_TOO_SMALL &&
        count == expected_count && bytes_are(destination, sizeof(destination), 0x5A) &&
        cna_effect_copy_type_name(effect, destination, expected_count, &count) ==
            CNA_RESULT_SUCCESS &&
        count == expected_count &&
        memcmp(destination, expected, (size_t)expected_count) == 0 &&
        (unsigned char)destination[expected_count] == 0x5A;
}

static CNA_Result inspect_material(ConcreteEffectState* state, CNA_Handle device)
{
    static const char effect_name[] = "Microsoft.Xna.Framework.Graphics.Effect";
    static const char material_name[] = "Microsoft.Xna.Framework.Graphics.EffectMaterial";
    CnaDemo* demo = state->demo;
    CNA_Handle owner = CNA_INVALID_HANDLE;
    CNA_Bool exact_sprite = CNA_TRUE;
    CNA_Bool has_renderer = CNA_TRUE;
    CNA_Color pixel = {17U, 34U, 51U, 255U};
    uint64_t retained = UINT64_MAX;
    CNA_Result result;

    result = cna_effect_create_empty(device, &state->source);
    if (!cna_demo_check_result(demo, result, "create the source Effect for an EffectMaterial")) {
        return result;
    }
    result = cna_effect_material_create(state->source, &state->material);
    if (!cna_demo_check_result(demo, result, "clone the source into an EffectMaterial")) {
        return result;
    }
    cna_demo_check(
        demo,
        effect_type_is(state->material, material_name) &&
            cna_effect_get_graphics_device(state->material, &owner) == CNA_RESULT_SUCCESS &&
            owner == device &&
            cna_effect_has_renderer(state->material, &has_renderer) == CNA_RESULT_SUCCESS &&
            has_renderer == CNA_FALSE &&
            cna_effect_is_exact_stock_sprite_effect(state->material, &exact_sprite) ==
                CNA_RESULT_SUCCESS &&
            exact_sprite == CNA_FALSE && cna_effect_apply(state->material) == CNA_RESULT_SUCCESS,
        "EffectMaterial preserves the owner and exposes its non-renderer, non-sprite concrete identity");

    retained = UINT64_MAX;
    cna_demo_check(
        demo,
        cna_effect_material_get_retained_parameter_texture_count_ext(
            state->source, &retained) == CNA_RESULT_INVALID_ARGUMENT &&
            retained == UINT64_MAX &&
            cna_effect_material_retain_parameter_texture_ext(
                state->source, CNA_EFFECT_TEXTURE_2D, CNA_INVALID_HANDLE) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "EffectMaterial-only routes reject the ordinary source Effect without changing output");

    retained = UINT64_MAX;
    cna_demo_check(
        demo,
        cna_effect_material_get_retained_parameter_texture_count_ext(
            state->material, &retained) == CNA_RESULT_SUCCESS &&
            retained == UINT64_C(0) &&
            cna_effect_material_retain_parameter_texture_ext(
                state->material, CNA_EFFECT_TEXTURE_2D, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_material_get_retained_parameter_texture_count_ext(
                state->material, &retained) == CNA_RESULT_SUCCESS &&
            retained == UINT64_C(0),
        "a new material retains nothing and deliberately ignores the native null texture");
    cna_demo_check(
        demo,
        cna_effect_material_retain_parameter_texture_ext(
            state->material, UINT32_C(4), CNA_INVALID_HANDLE) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_effect_material_get_retained_parameter_texture_count_ext(
                state->material, &retained) == CNA_RESULT_SUCCESS &&
            retained == UINT64_C(0),
        "an undefined texture family is refused before changing the material's ownership list");

    result = cna_texture2d_create_cpu_only_rgba8(
        UINT32_C(1), UINT32_C(1), CNA_SURFACE_FORMAT_COLOR, &pixel, UINT64_C(1), &state->texture);
    if (!cna_demo_check_result(
            demo, result, "create a caller-owned Texture2D for material retention")) {
        return result;
    }
    cna_demo_check(
        demo,
        cna_effect_material_retain_parameter_texture_ext(
            state->material, CNA_EFFECT_TEXTURE_2D, state->texture) == CNA_RESULT_SUCCESS &&
            cna_effect_material_retain_parameter_texture_ext(
                state->material, CNA_EFFECT_TEXTURE_BASE, state->texture) == CNA_RESULT_SUCCESS &&
            cna_effect_material_get_retained_parameter_texture_count_ext(
                state->material, &retained) == CNA_RESULT_SUCCESS &&
            retained == UINT64_C(2),
        "typed and base retention append two independent owning entries for one texture");
    result = cna_texture2d_destroy(state->texture);
    if (result == CNA_RESULT_SUCCESS) {
        state->texture = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_material_get_retained_parameter_texture_count_ext(
                state->material, &retained) == CNA_RESULT_SUCCESS &&
            retained == UINT64_C(2),
        "caller handle destruction succeeds while both material-owned texture entries remain");

    result = cna_effect_clone(state->material, &state->material_clone);
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS && effect_type_is(state->material_clone, effect_name) &&
            cna_effect_apply(state->material_clone) == CNA_RESULT_SUCCESS,
        "canonical EffectMaterial Clone is an independent base Effect, not another material");
    retained = UINT64_C(77);
    cna_demo_check(
        demo,
        cna_effect_material_get_retained_parameter_texture_count_ext(
            state->material_clone, &retained) == CNA_RESULT_INVALID_ARGUMENT &&
            retained == UINT64_C(77),
        "the inherited base clone is observably outside the EffectMaterial-only ownership API");

    result = cna_effect_destroy(state->source);
    if (result == CNA_RESULT_SUCCESS) {
        state->source = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS && cna_effect_apply(state->material) == CNA_RESULT_SUCCESS &&
            cna_effect_apply(state->material_clone) == CNA_RESULT_SUCCESS &&
            cna_effect_material_get_retained_parameter_texture_count_ext(
                state->material, &retained) == CNA_RESULT_SUCCESS &&
            retained == UINT64_C(2),
        "material and inherited clone remain independent after their original source is destroyed");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result inspect_sprite(ConcreteEffectState* state, CNA_Handle device)
{
    static const char sprite_name[] = "Microsoft.Xna.Framework.Graphics.SpriteEffect";
    CnaDemo* demo = state->demo;
    CNA_Handle owner = CNA_INVALID_HANDLE;
    CNA_Bool exact_sprite = CNA_FALSE;
    CNA_Bool has_renderer = CNA_TRUE;
    CNA_Bool disposed = CNA_FALSE;
    CNA_Result result;

    result = cna_sprite_effect_create(device, &state->sprite);
    if (!cna_demo_check_result(demo, result, "create the stock SpriteEffect")) {
        return result;
    }
    cna_demo_check(
        demo,
        effect_type_is(state->sprite, sprite_name) &&
            cna_effect_get_graphics_device(state->sprite, &owner) == CNA_RESULT_SUCCESS &&
            owner == device &&
            cna_effect_is_exact_stock_sprite_effect(state->sprite, &exact_sprite) ==
                CNA_RESULT_SUCCESS &&
            exact_sprite == CNA_TRUE &&
            cna_effect_has_renderer(state->sprite, &has_renderer) == CNA_RESULT_SUCCESS &&
            has_renderer == CNA_FALSE && cna_effect_apply(state->sprite) == CNA_RESULT_SUCCESS,
        "SpriteEffect exposes the exact stock identity while using the renderer's internal sprite path");

    result = cna_effect_clone(state->sprite, &state->sprite_clone);
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS && effect_type_is(state->sprite_clone, sprite_name) &&
            cna_effect_is_exact_stock_sprite_effect(state->sprite_clone, &exact_sprite) ==
                CNA_RESULT_SUCCESS &&
            exact_sprite == CNA_TRUE &&
            cna_effect_get_graphics_device(state->sprite_clone, &owner) == CNA_RESULT_SUCCESS &&
            owner == device && cna_effect_apply(state->sprite_clone) == CNA_RESULT_SUCCESS,
        "SpriteEffect Clone preserves the exact concrete stock type and borrowed owner");

    result = cna_effect_dispose(state->sprite);
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_graphics_resource_get_is_disposed(state->sprite, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE && cna_effect_apply(state->sprite) == CNA_RESULT_INVALID_STATE &&
            cna_effect_apply(state->sprite_clone) == CNA_RESULT_SUCCESS,
        "disposing the original SpriteEffect leaves its independently cloned stock effect usable");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    ConcreteEffectState* state = (ConcreteEffectState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = inspect_material(state, device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    return inspect_sprite(state, device);
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

static void cleanup(ConcreteEffectState* state)
{
    if (state->sprite_clone != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->sprite_clone);
    }
    if (state->sprite != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->sprite);
    }
    if (state->texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(state->texture);
    }
    if (state->material_clone != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->material_clone);
    }
    if (state->material != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->material);
    }
    if (state->source != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->source);
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    ConcreteEffectState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.effect_concrete_types", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&create_info, 0, sizeof(create_info));
    state.demo = &demo;
    state.source = CNA_INVALID_HANDLE;
    state.material = CNA_INVALID_HANDLE;
    state.material_clone = CNA_INVALID_HANDLE;
    state.texture = CNA_INVALID_HANDLE;
    state.sprite = CNA_INVALID_HANDLE;
    state.sprite_clone = CNA_INVALID_HANDLE;
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

    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }
    manager_ready = cna_demo_check_result(
        &demo, cna_demo_use_hidef(game, &manager),
        "create the graphics device for both concrete Effect adapters");
    if (manager_ready) {
        cna_demo_check_result(
            &demo, cna_game_run(game), "run the one-callback concrete Effect audit");
        cna_demo_check(
            &demo, state.loads == 1,
            "the concrete Effect audit ran in exactly one load callback");
    }
    cleanup(&state);
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager after every concrete Effect");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
