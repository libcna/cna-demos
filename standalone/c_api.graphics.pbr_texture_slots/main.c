/* SPDX-License-Identifier: MS-PL */
/*
 * PbrEffect from C: all seven retained Texture2D slots, UV selectors, transforms and sRGB flags.
 * Two actual same-device textures make per-slot ownership observable through clone destruction.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define PBR_SLOT_COUNT (CNA_PBR_TEXTURE_MAXIMUM + UINT32_C(1))

typedef struct PbrTextureState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle effect;
    CNA_EffectHandle clone;
    CNA_Handle core_texture;
    CNA_Handle specular_texture;
} PbrTextureState;

static int transform_is(
    const CNA_TextureTransformEXT* value,
    float offset_x,
    float offset_y,
    float scale_x,
    float scale_y,
    float rotation)
{
    return value->struct_size == (uint32_t)sizeof(*value) &&
        value->struct_version == UINT32_C(1) &&
        value->offset.x == offset_x && value->offset.y == offset_y &&
        value->scale.x == scale_x && value->scale.y == scale_y &&
        value->rotation == rotation;
}

static CNA_TextureTransformEXT authored_transform(uint32_t slot)
{
    CNA_TextureTransformEXT value;
    (void)cna_texture_transform_ext_init(&value);
    value.offset.x = (float)slot + 0.125F;
    value.offset.y = -(float)slot - 0.25F;
    value.scale.x = (float)slot + 1.5F;
    value.scale.y = (float)slot + 2.5F;
    value.rotation = (float)slot + 0.75F;
    return value;
}

static int is_color_slot(uint32_t slot)
{
    return slot == CNA_PBR_TEXTURE_BASE_COLOR || slot == CNA_PBR_TEXTURE_EMISSIVE ||
        slot == CNA_PBR_TEXTURE_SPECULAR_COLOR_EXT;
}

static void inspect_transform_value(PbrTextureState* state)
{
    CNA_TextureTransformEXT identity;
    CNA_TextureTransformEXT same;
    CNA_Bool equal = CNA_FALSE;

    cna_demo_check(
        state->demo,
        cna_texture_transform_ext_init(&identity) == CNA_RESULT_SUCCESS &&
            transform_is(&identity, 0.0F, 0.0F, 1.0F, 1.0F, 0.0F),
        "the texture-transform initializer supplies valid identity headers and values");
    cna_demo_check(
        state->demo,
        cna_texture_transform_ext_init(&same) == CNA_RESULT_SUCCESS &&
            cna_texture_transform_ext_equals(&identity, &same, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE,
        "two independently initialized texture transforms compare equal field by field");
    same.offset.x = 0.5F;
    cna_demo_check(
        state->demo,
        cna_texture_transform_ext_equals(&identity, &same, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_FALSE,
        "texture-transform equality observes an authored offset difference");
}

static void inspect_slot_defaults(PbrTextureState* state)
{
    CNA_TextureTransformEXT transform;
    CNA_Bool has_texture = CNA_TRUE;
    CNA_Bool srgb = CNA_FALSE;
    CNA_Handle texture = UINT64_MAX;
    int32_t coordinate_set = -1;
    uint32_t slot;
    int all_empty = 1;
    int all_uv_zero = 1;
    int all_identity = 1;
    int srgb_contract = 1;

    for (slot = 0U; slot < PBR_SLOT_COUNT; ++slot) {
        has_texture = CNA_TRUE;
        texture = UINT64_MAX;
        all_empty = all_empty &&
            cna_pbr_effect_get_texture(
                state->effect, slot, &has_texture, &texture) == CNA_RESULT_SUCCESS &&
            has_texture == CNA_FALSE && texture == CNA_INVALID_HANDLE;
        coordinate_set = -1;
        all_uv_zero = all_uv_zero &&
            cna_pbr_effect_get_texture_coordinate_set_ext(
                state->effect, slot, &coordinate_set) == CNA_RESULT_SUCCESS &&
            coordinate_set == 0;
        (void)cna_texture_transform_ext_init(&transform);
        all_identity = all_identity &&
            cna_pbr_effect_get_texture_transform_ext(
                state->effect, slot, &transform) == CNA_RESULT_SUCCESS &&
            transform_is(&transform, 0.0F, 0.0F, 1.0F, 1.0F, 0.0F);
        srgb = CNA_FALSE;
        if (is_color_slot(slot)) {
            srgb_contract = srgb_contract &&
                cna_pbr_effect_get_texture_is_srgb_ext(
                    state->effect, slot, &srgb) == CNA_RESULT_SUCCESS && srgb == CNA_TRUE;
        } else {
            srgb_contract = srgb_contract &&
                cna_pbr_effect_get_texture_is_srgb_ext(
                    state->effect, slot, &srgb) == CNA_RESULT_INVALID_ARGUMENT &&
                srgb == CNA_FALSE;
        }
    }
    cna_demo_check(state->demo, PBR_SLOT_COUNT == 7U && all_empty,
                   "all seven PBR texture slots start independently empty");
    cna_demo_check(state->demo, all_uv_zero,
                   "all seven slots select packed UV channel zero by default");
    cna_demo_check(state->demo, all_identity,
                   "all seven slots start with independent identity texture transforms");
    cna_demo_check(state->demo, srgb_contract,
                   "only base-color, emissive and specular-color slots expose default-on sRGB state");
}

static void inspect_slot_values(PbrTextureState* state)
{
    CNA_TextureTransformEXT expected;
    CNA_TextureTransformEXT actual;
    CNA_TextureTransformEXT malformed = {0};
    CNA_Bool equal = CNA_FALSE;
    CNA_Bool srgb = CNA_TRUE;
    int32_t coordinate_set = -1;
    uint32_t slot;
    int selectors_match = 1;
    int transforms_match = 1;
    int srgb_match = 1;

    for (slot = 0U; slot < PBR_SLOT_COUNT; ++slot) {
        const int32_t selected = (int32_t)(slot & UINT32_C(1));
        expected = authored_transform(slot);
        selectors_match = selectors_match &&
            cna_pbr_effect_set_texture_coordinate_set_ext(
                state->effect, slot, selected) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_texture_coordinate_set_ext(
                state->effect, slot, &coordinate_set) == CNA_RESULT_SUCCESS &&
            coordinate_set == selected;
        transforms_match = transforms_match &&
            cna_pbr_effect_set_texture_transform_ext(
                state->effect, slot, &expected) == CNA_RESULT_SUCCESS;
        (void)cna_texture_transform_ext_init(&actual);
        transforms_match = transforms_match &&
            cna_pbr_effect_get_texture_transform_ext(
                state->effect, slot, &actual) == CNA_RESULT_SUCCESS &&
            cna_texture_transform_ext_equals(&expected, &actual, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE;
        if (is_color_slot(slot)) {
            srgb_match = srgb_match &&
                cna_pbr_effect_set_texture_is_srgb_ext(
                    state->effect, slot, CNA_FALSE) == CNA_RESULT_SUCCESS &&
                cna_pbr_effect_get_texture_is_srgb_ext(
                    state->effect, slot, &srgb) == CNA_RESULT_SUCCESS && srgb == CNA_FALSE;
        } else {
            srgb_match = srgb_match &&
                cna_pbr_effect_set_texture_is_srgb_ext(
                    state->effect, slot, CNA_FALSE) == CNA_RESULT_INVALID_ARGUMENT;
        }
    }
    cna_demo_check(state->demo, selectors_match,
                   "all seven slots retain their independently selected UV channel");
    cna_demo_check(state->demo, transforms_match,
                   "all seven slots copy and return distinct scale-rotate-translate transforms");
    cna_demo_check(state->demo, srgb_match,
                   "the three color slots toggle sRGB independently while data slots refuse it");

    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_texture_coordinate_set_ext(
            state->effect, CNA_PBR_TEXTURE_MAXIMUM + 1U, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_set_texture_coordinate_set_ext(
                state->effect, CNA_PBR_TEXTURE_BASE_COLOR, -1) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_set_texture_coordinate_set_ext(
                state->effect, CNA_PBR_TEXTURE_BASE_COLOR, 2) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_get_texture_coordinate_set_ext(
                state->effect, CNA_PBR_TEXTURE_BASE_COLOR, &coordinate_set) == CNA_RESULT_SUCCESS &&
            coordinate_set == 0,
        "undefined slots and UV channels are refused without changing the selected channel");
    expected = authored_transform(CNA_PBR_TEXTURE_BASE_COLOR);
    (void)cna_texture_transform_ext_init(&actual);
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_texture_transform_ext(
            state->effect, CNA_PBR_TEXTURE_BASE_COLOR, &malformed) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_set_texture_transform_ext(
                state->effect, CNA_PBR_TEXTURE_MAXIMUM + 1U, &expected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_get_texture_transform_ext(
                state->effect, CNA_PBR_TEXTURE_BASE_COLOR, &actual) == CNA_RESULT_SUCCESS &&
            cna_texture_transform_ext_equals(&actual, &expected, &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE,
        "malformed transforms and undefined slots are refused atomically");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_texture_is_srgb_ext(
            state->effect, CNA_PBR_TEXTURE_BASE_COLOR, UINT32_C(2)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_get_texture_is_srgb_ext(
                state->effect, CNA_PBR_TEXTURE_BASE_COLOR, &srgb) == CNA_RESULT_SUCCESS &&
            srgb == CNA_FALSE,
        "a non-canonical sRGB Boolean is refused without changing the color slot");
}

static CNA_Result create_and_assign_textures(PbrTextureState* state, CNA_Handle device)
{
    CNA_Texture2DCreateInfo info;
    const CNA_Color pixel = {1U, 2U, 3U, 4U};
    CNA_Handle cpu_texture = CNA_INVALID_HANDLE;
    CNA_Handle returned = CNA_INVALID_HANDLE;
    CNA_Bool present = CNA_TRUE;
    CNA_Result result;
    uint32_t slot;
    int all_assigned = 1;

    result = cna_texture2d_create_cpu_only_rgba8(
        1U, 1U, CNA_SURFACE_FORMAT_COLOR, &pixel, 1U, &cpu_texture);
    if (!cna_demo_check_result(state->demo, result, "create an ownerless CPU Texture2D")) {
        return result;
    }
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_texture(
            state->effect, CNA_PBR_TEXTURE_BASE_COLOR, cpu_texture) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_get_texture(
                state->effect, CNA_PBR_TEXTURE_BASE_COLOR, &present, &returned) ==
                CNA_RESULT_SUCCESS && present == CNA_FALSE && returned == CNA_INVALID_HANDLE &&
            cna_texture2d_destroy(cpu_texture) == CNA_RESULT_SUCCESS,
        "the same-device rule refuses an ownerless texture and preserves the empty slot");

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.width = 1U;
    info.height = 1U;
    info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(device, &info, &state->core_texture);
    if (!cna_demo_check_result(state->demo, result, "create the shared core PBR texture")) {
        return result;
    }
    result = cna_texture2d_create(device, &info, &state->specular_texture);
    if (!cna_demo_check_result(state->demo, result, "create the shared specular PBR texture")) {
        return result;
    }
    for (slot = 0U; slot < PBR_SLOT_COUNT; ++slot) {
        const CNA_Handle expected = slot <= CNA_PBR_TEXTURE_OCCLUSION
            ? state->core_texture : state->specular_texture;
        all_assigned = all_assigned &&
            cna_pbr_effect_set_texture(state->effect, slot, expected) == CNA_RESULT_SUCCESS &&
            cna_pbr_effect_get_texture(state->effect, slot, &present, &returned) ==
                CNA_RESULT_SUCCESS && present == CNA_TRUE && returned == expected;
    }
    cna_demo_check(state->demo, all_assigned,
                   "all seven slots retain and return the exact assigned same-device handle");
    cna_demo_check(
        state->demo,
        cna_texture2d_destroy(state->core_texture) == CNA_RESULT_INVALID_STATE &&
            cna_texture2d_destroy(state->specular_texture) == CNA_RESULT_INVALID_STATE,
        "both textures refuse destruction while their PBR slots retain them");
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_texture(
            state->effect, CNA_PBR_TEXTURE_MAXIMUM + 1U, state->core_texture) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_pbr_effect_get_texture(
                state->effect, CNA_PBR_TEXTURE_MAXIMUM + 1U, &present, &returned) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "the first undefined texture-slot identity is refused by both accessors");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result inspect_clone_ownership(PbrTextureState* state)
{
    CNA_TextureTransformEXT expected;
    CNA_TextureTransformEXT actual;
    CNA_Bool present = CNA_FALSE;
    CNA_Bool srgb = CNA_TRUE;
    CNA_Handle returned = CNA_INVALID_HANDLE;
    CNA_Result result;
    uint32_t slot;
    int all_retained = 1;
    int metadata_retained = 1;

    result = cna_effect_clone(state->effect, &state->clone);
    if (!cna_demo_check_result(state->demo, result, "clone the complete PBR texture-slot state")) {
        return result;
    }
    cna_demo_check(
        state->demo, cna_effect_destroy(state->effect) == CNA_RESULT_SUCCESS,
        "destroy the source while its clone retains both textures and all slot metadata");
    state->effect = CNA_INVALID_HANDLE;
    for (slot = 0U; slot < PBR_SLOT_COUNT; ++slot) {
        const CNA_Handle expected_texture = slot <= CNA_PBR_TEXTURE_OCCLUSION
            ? state->core_texture : state->specular_texture;
        int32_t coordinate_set = -1;
        all_retained = all_retained &&
            cna_pbr_effect_get_texture(state->clone, slot, &present, &returned) ==
                CNA_RESULT_SUCCESS && present == CNA_TRUE && returned == expected_texture;
        expected = authored_transform(slot);
        (void)cna_texture_transform_ext_init(&actual);
        metadata_retained = metadata_retained &&
            cna_pbr_effect_get_texture_coordinate_set_ext(
                state->clone, slot, &coordinate_set) == CNA_RESULT_SUCCESS &&
            coordinate_set == (int32_t)(slot & UINT32_C(1)) &&
            cna_pbr_effect_get_texture_transform_ext(
                state->clone, slot, &actual) == CNA_RESULT_SUCCESS &&
            memcmp(&actual, &expected, sizeof(actual)) == 0;
        if (is_color_slot(slot)) {
            metadata_retained = metadata_retained &&
                cna_pbr_effect_get_texture_is_srgb_ext(
                    state->clone, slot, &srgb) == CNA_RESULT_SUCCESS && srgb == CNA_FALSE;
        }
    }
    cna_demo_check(state->demo, all_retained,
                   "the clone retains exact handles in all seven slots after source destruction");
    cna_demo_check(state->demo, metadata_retained,
                   "the clone retains every slot's UV channel, transform and meaningful sRGB flag");

    for (slot = 0U; slot < CNA_PBR_TEXTURE_OCCLUSION; ++slot) {
        (void)cna_pbr_effect_set_texture(state->clone, slot, CNA_INVALID_HANDLE);
    }
    cna_demo_check(
        state->demo,
        cna_texture2d_destroy(state->core_texture) == CNA_RESULT_INVALID_STATE &&
            cna_pbr_effect_set_texture(
                state->clone, CNA_PBR_TEXTURE_OCCLUSION, CNA_INVALID_HANDLE) == CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->core_texture) == CNA_RESULT_SUCCESS,
        "the core texture releases only after the clone's fifth and final retaining slot clears");
    state->core_texture = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        cna_pbr_effect_set_texture(
            state->clone, CNA_PBR_TEXTURE_SPECULAR_EXT, CNA_INVALID_HANDLE) == CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->specular_texture) == CNA_RESULT_INVALID_STATE &&
            cna_pbr_effect_set_texture(
                state->clone, CNA_PBR_TEXTURE_SPECULAR_COLOR_EXT, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->specular_texture) == CNA_RESULT_SUCCESS,
        "the specular texture releases only after both extension slots clear");
    state->specular_texture = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo, cna_effect_apply(state->clone) == CNA_RESULT_SUCCESS,
        "the metadata-rich clone applies after all optional textures are released");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    PbrTextureState* state = (PbrTextureState*)context;
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
    inspect_transform_value(state);
    inspect_slot_defaults(state);
    inspect_slot_values(state);
    result = create_and_assign_textures(state, device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    return inspect_clone_ownership(state);
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

static void cleanup(PbrTextureState* state)
{
    if (state->clone != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->clone);
    }
    if (state->effect != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->effect);
    }
    if (state->specular_texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(state->specular_texture);
    }
    if (state->core_texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(state->core_texture);
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    PbrTextureState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.pbr_texture_slots", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&create_info, 0, sizeof(create_info));
    state.demo = &demo;
    state.effect = CNA_INVALID_HANDLE;
    state.clone = CNA_INVALID_HANDLE;
    state.core_texture = CNA_INVALID_HANDLE;
    state.specular_texture = CNA_INVALID_HANDLE;
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
        &demo, cna_demo_use_hidef(game, &manager), "create the graphics device for PBR textures");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the PBR texture-slot audit");
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
