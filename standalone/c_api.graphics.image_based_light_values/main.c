/* SPDX-License-Identifier: MS-PL */
/*
 * ImageBasedLightEXT from C: canonical defaults, versioned validation, resource completeness and
 * handle-kind safety. Nothing is drawn and no lighting-output claim is made.
 */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ImageBasedLightState {
    CnaDemo* demo;
    int loads;
} ImageBasedLightState;

static int light_is_valid(const CNA_ImageBasedLightEXT* light, CNA_Bool expected)
{
    CNA_Bool valid = expected == CNA_TRUE ? CNA_FALSE : CNA_TRUE;
    return cna_image_based_light_ext_is_valid(light, &valid) == CNA_RESULT_SUCCESS &&
           valid == expected;
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    ImageBasedLightState* state = (ImageBasedLightState*)context;
    const CNA_TextureCubeCreateInfo cube_info = {
        sizeof(CNA_TextureCubeCreateInfo), UINT32_C(1), 2U,
        CNA_FALSE, {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR, 0U
    };
    const CNA_Texture2DCreateInfo texture_info = {
        sizeof(CNA_Texture2DCreateInfo), UINT32_C(1), 2U, 2U,
        CNA_FALSE, {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR
    };
    CNA_ImageBasedLightEXT light;
    CNA_ImageBasedLightEXT malformed;
    CNA_ImageBasedLightEXT wrong_kind;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle irradiance = CNA_INVALID_HANDLE;
    CNA_Handle prefiltered = CNA_INVALID_HANDLE;
    CNA_Handle brdf = CNA_INVALID_HANDLE;
    CNA_Bool valid = CNA_TRUE;
    CNA_Bool available = UINT32_C(7);
    CNA_Result result;

    (void)game_time;
    (void)out_error;
    ++state->loads;

    cna_demo_check(
        state->demo,
        cna_graphics_ext_is_available(&available) == CNA_RESULT_SUCCESS &&
            (available == CNA_FALSE || available == CNA_TRUE) &&
            cna_graphics_ext_is_available(0) == CNA_RESULT_INVALID_ARGUMENT,
        "extension availability is a canonical Boolean and rejects a null output");

    memset(&light, 0xA5, sizeof(light));
    cna_demo_check(
        state->demo,
        cna_image_based_light_ext_init(&light) == CNA_RESULT_SUCCESS &&
            light.struct_size == sizeof(light) && light.struct_version == UINT32_C(1) &&
            light.irradiance == CNA_INVALID_HANDLE &&
            light.prefiltered_specular == CNA_INVALID_HANDLE &&
            light.brdf_lut == CNA_INVALID_HANDLE && light.prefiltered_mip_count == 1 &&
            light.intensity == 1.0F,
        "ImageBasedLight initialization overwrites storage with complete canonical defaults");
    cna_demo_check(
        state->demo,
        light_is_valid(&light, CNA_FALSE) &&
            cna_image_based_light_ext_init(0) == CNA_RESULT_INVALID_ARGUMENT,
        "the texture-free default is incomplete and initialization rejects a null destination");

    malformed = light;
    malformed.struct_size = (uint32_t)sizeof(malformed) - 1U;
    valid = UINT32_C(7);
    cna_demo_check(
        state->demo,
        cna_image_based_light_ext_is_valid(&malformed, &valid) ==
                CNA_RESULT_INVALID_ARGUMENT && valid == UINT32_C(7),
        "a short versioned structure is refused without changing the caller output");
    malformed = light;
    malformed.struct_version = 0U;
    cna_demo_check(
        state->demo,
        cna_image_based_light_ext_is_valid(&malformed, &valid) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_image_based_light_ext_is_valid(0, &valid) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_image_based_light_ext_is_valid(&light, 0) == CNA_RESULT_INVALID_ARGUMENT,
        "version zero and null input/output pointers are distinct argument refusals");

    light.prefiltered_mip_count = 0;
    cna_demo_check(state->demo, light_is_valid(&light, CNA_FALSE),
                   "a nonpositive prefiltered mip count keeps the bundle invalid");
    light.prefiltered_mip_count = 1;
    light.intensity = -0.01F;
    cna_demo_check(state->demo, light_is_valid(&light, CNA_FALSE),
                   "a negative intensity keeps the bundle invalid");
    light.intensity = NAN;
    cna_demo_check(state->demo, light_is_valid(&light, CNA_FALSE),
                   "a NaN intensity cannot pass the nonnegative validity predicate");
    light.intensity = 0.0F;

    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "get the device for image-based-light resources")) {
        return CNA_RESULT_SUCCESS;
    }
    result = cna_texturecube_create(device, &cube_info, &irradiance);
    if (!cna_demo_check_result(state->demo, result, "create the irradiance cube")) {
        return CNA_RESULT_SUCCESS;
    }
    result = cna_texturecube_create(device, &cube_info, &prefiltered);
    if (!cna_demo_check_result(state->demo, result, "create the prefiltered-specular cube")) {
        (void)cna_texturecube_destroy(irradiance);
        return CNA_RESULT_SUCCESS;
    }
    result = cna_texture2d_create(device, &texture_info, &brdf);
    if (!cna_demo_check_result(state->demo, result, "create the BRDF lookup texture")) {
        (void)cna_texturecube_destroy(prefiltered);
        (void)cna_texturecube_destroy(irradiance);
        return CNA_RESULT_SUCCESS;
    }

    light.irradiance = irradiance;
    cna_demo_check(state->demo, light_is_valid(&light, CNA_FALSE),
                   "one live irradiance cube is still an incomplete split-sum bundle");
    light.prefiltered_specular = prefiltered;
    cna_demo_check(state->demo, light_is_valid(&light, CNA_FALSE),
                   "two live cubes remain incomplete without the BRDF lookup texture");
    light.brdf_lut = brdf;
    cna_demo_check(state->demo, light_is_valid(&light, CNA_TRUE),
                   "two live cubes and one live 2D lookup texture form a valid zero-intensity bundle");

    cna_demo_check_result(state->demo, cna_texture2d_destroy(brdf),
                          "destroy the BRDF lookup texture");
    cna_demo_check(state->demo, light_is_valid(&light, CNA_FALSE),
                   "a stale texture handle invalidates the formerly complete bundle");

    wrong_kind = light;
    wrong_kind.irradiance = game;
    wrong_kind.prefiltered_specular = game;
    wrong_kind.brdf_lut = game;
    cna_demo_check(
        state->demo, light_is_valid(&wrong_kind, CNA_FALSE),
        "live non-texture handles are rejected instead of masquerading as an image-based light");

    cna_demo_check(
        state->demo,
        cna_texturecube_destroy(prefiltered) == CNA_RESULT_SUCCESS &&
            cna_texturecube_destroy(irradiance) == CNA_RESULT_SUCCESS,
        "both caller-owned cube textures are destroyed independently");
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    ImageBasedLightState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.graphics.image_based_light_values", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    if (cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager),
                              "create a HiDef graphics device")) {
        cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                              "run the image-based-light validation callback");
    }
    cna_demo_check(&demo, state.loads == 1,
                   "the image-based-light callback ran exactly once");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "cna_graphics_device_manager_destroy");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
