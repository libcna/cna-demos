// SPDX-License-Identifier: MS-PL
// ISSUE-011: independently owned C sound/cube handles survive their content manager.
// Run with the demos asset root and a real supported renderer/dummy SDL audio.
#include <CNA/C/cna.h>
#include "CnaTestReport.h"
#include <stdio.h>
#include <string.h>

#ifndef CNA_DEMOS_ASSET_ROOT
#error "CNA_DEMOS_ASSET_ROOT must name the existing demos assets"
#endif

typedef struct State {
    CNA_Handle manager;
    CNA_Handle cube;
    CNA_Handle sound;
    CNA_Handle instance;
    CNA_Handle fresh_instance;
    CNA_GraphicsRendererType expected_renderer;
    int validated;
} State;

static CNA_StringView view(const char* text)
{
    const CNA_StringView result = {text, (uint64_t)strlen(text)};
    return result;
}

static int check_cube(CNA_Handle cube)
{
    /* These exact six colors are authored in tools/dds_assets.py. */
    static const CNA_Color expected[] = {
        {255U, 0U, 0U, 255U}, {0U, 255U, 0U, 255U}, {0U, 0U, 255U, 255U},
        {255U, 255U, 0U, 255U}, {255U, 0U, 255U, 255U}, {0U, 255U, 255U, 255U}
    };
    CNA_TextureCubeInfo info = {0};
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    if (!CNA_TEST_STAGE(cna_texturecube_get_info(cube, &info) == CNA_RESULT_SUCCESS) ||
        !CNA_TEST_STAGE(info.size == 4U && info.level_count == 1U &&
                        info.format == CNA_SURFACE_FORMAT_COLOR)) return 0;
    for (uint32_t face = 0U; face < 6U; ++face) {
        CNA_TextureCubeTransfer transfer = {0};
        CNA_Color pixels[16] = {{0}};
        uint64_t required = UINT64_MAX;
        transfer.struct_size = (uint32_t)sizeof(transfer);
        transfer.struct_version = UINT32_C(1);
        transfer.face = face;
        transfer.element_count = UINT64_C(16);
        if (!CNA_TEST_STAGE(cna_texturecube_get_data(cube, &transfer, pixels, 16U,
                                                    &required) == CNA_RESULT_SUCCESS) ||
            !CNA_TEST_STAGE(required == 16U)) return 0;
        for (unsigned int pixel = 0U; pixel < 16U; ++pixel) {
            if (!CNA_TEST_STAGE(pixels[pixel].r == expected[face].r &&
                                pixels[pixel].g == expected[face].g &&
                                pixels[pixel].b == expected[face].b &&
                                pixels[pixel].a == expected[face].a)) return 0;
        }
    }
    return 1;
}

static int check_playing(CNA_Handle instance)
{
    CNA_SoundEffectInstanceInfo info = {0};
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    return CNA_TEST_STAGE(cna_sound_effect_instance_get_info(instance, &info) ==
                          CNA_RESULT_SUCCESS) &&
        CNA_TEST_STAGE(info.state == CNA_SOUND_STATE_PLAYING && info.is_looped == CNA_TRUE);
}

static int start_loop(CNA_Handle instance)
{
    return CNA_TEST_STAGE(cna_sound_effect_instance_set_is_looped(instance, CNA_TRUE) ==
                          CNA_RESULT_SUCCESS) &&
        CNA_TEST_STAGE(cna_sound_effect_instance_play(instance) == CNA_RESULT_SUCCESS) &&
        check_playing(instance);
}

static int check_handles(State* state, const char* boundary)
{
    int64_t duration = 0;
    if (!check_cube(state->cube) || !check_playing(state->instance) ||
        !CNA_TEST_STAGE(cna_sound_effect_get_duration_ticks(state->sound, &duration) ==
                        CNA_RESULT_SUCCESS) || !CNA_TEST_STAGE(duration > 0) ||
        !CNA_TEST_STAGE(cna_sound_effect_create_instance(state->sound,
            &state->fresh_instance) == CNA_RESULT_SUCCESS) ||
        !start_loop(state->fresh_instance) ||
        !CNA_TEST_STAGE(cna_sound_effect_instance_destroy(state->fresh_instance) ==
                        CNA_RESULT_SUCCESS)) return 0;
    state->fresh_instance = CNA_INVALID_HANDLE;
    printf("PASS %s: 96 exact cube pixels, existing playback, new playable instance\n", boundary);
    return 1;
}

static int validate(CNA_Handle game, State* state)
{
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_GraphicsRendererType active = CNA_GRAPHICS_RENDERER_UNKNOWN;
    CNA_AudioCapabilities capabilities = {0};
    CNA_ContentManagerCreateInfo create_info = {0};
    capabilities.struct_size = (uint32_t)sizeof(capabilities);
    capabilities.struct_version = UINT32_C(1);
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.root_directory = view(CNA_DEMOS_ASSET_ROOT);
    if (!CNA_TEST_STAGE(cna_game_get_graphics_device(game, &device) == CNA_RESULT_SUCCESS) ||
        !CNA_TEST_STAGE(cna_graphics_renderer_get_active_ext(&active) == CNA_RESULT_SUCCESS) ||
        !CNA_TEST_STAGE(active == state->expected_renderer) ||
        !CNA_TEST_STAGE(cna_audio_get_capabilities(game, &capabilities) == CNA_RESULT_SUCCESS) ||
        !CNA_TEST_STAGE(capabilities.is_playback_available == CNA_TRUE) ||
        !CNA_TEST_STAGE(cna_content_manager_create(device, &create_info, &state->manager) ==
                        CNA_RESULT_SUCCESS) ||
        !CNA_TEST_STAGE(cna_content_manager_load_texture_cube(state->manager,
            view("textures/cube_faces"), &state->cube) == CNA_RESULT_SUCCESS) ||
        !CNA_TEST_STAGE(cna_content_manager_load_sound_effect(state->manager,
            view("sounds/tone_440"), &state->sound) == CNA_RESULT_SUCCESS) ||
        !CNA_TEST_STAGE(cna_sound_effect_create_instance(state->sound, &state->instance) ==
                        CNA_RESULT_SUCCESS) || !start_loop(state->instance) ||
        !check_handles(state, "before unload") ||
        !CNA_TEST_STAGE(cna_content_manager_unload(state->manager) == CNA_RESULT_SUCCESS) ||
        !check_handles(state, "after unload") ||
        !CNA_TEST_STAGE(cna_content_manager_destroy(state->manager) == CNA_RESULT_SUCCESS)) return 0;
    state->manager = CNA_INVALID_HANDLE;
    return check_handles(state, "after destroy");
}

static CNA_Result on_load(CNA_Handle game, const CNA_GameTime* game_time,
                          void* context, CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    (void)game_time;
    (void)out_error;
    state->validated = validate(game, state);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    State state = {0};
    CNA_GameCallbacks callbacks = {0};
    CNA_GameCreateInfo create_info = {0};
    CNA_Handle game = CNA_INVALID_HANDLE;
    if (!CNA_TEST_STAGE(argc == 2) ||
        !CNA_TEST_STAGE(cna_graphics_renderer_set_preferred_by_name_ext(view(argv[1])) ==
                        CNA_RESULT_SUCCESS) ||
        !CNA_TEST_STAGE(cna_graphics_renderer_get_selected_ext(&state.expected_renderer) ==
                        CNA_RESULT_SUCCESS)) return CNA_TEST_FAIL(1);
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = on_load;
    callbacks.context = &state;
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.window_title = view("Content independent C handle lifetime");
    create_info.callbacks = &callbacks;
    int ok = CNA_TEST_STAGE(cna_game_create(&create_info, &game) == CNA_RESULT_SUCCESS) &&
        CNA_TEST_STAGE(cna_game_run_one_frame(game) == CNA_RESULT_SUCCESS) &&
        CNA_TEST_STAGE(state.validated == 1);
    /* Try every cleanup even after a failed assertion; report teardown failures too. */
    if (state.fresh_instance != CNA_INVALID_HANDLE)
        ok &= CNA_TEST_STAGE(cna_sound_effect_instance_destroy(state.fresh_instance) == CNA_RESULT_SUCCESS);
    if (state.instance != CNA_INVALID_HANDLE)
        ok &= CNA_TEST_STAGE(cna_sound_effect_instance_destroy(state.instance) == CNA_RESULT_SUCCESS);
    if (state.sound != CNA_INVALID_HANDLE)
        ok &= CNA_TEST_STAGE(cna_sound_effect_destroy(state.sound) == CNA_RESULT_SUCCESS);
    if (state.cube != CNA_INVALID_HANDLE)
        ok &= CNA_TEST_STAGE(cna_texturecube_destroy(state.cube) == CNA_RESULT_SUCCESS);
    if (state.manager != CNA_INVALID_HANDLE)
        ok &= CNA_TEST_STAGE(cna_content_manager_destroy(state.manager) == CNA_RESULT_SUCCESS);
    if (game != CNA_INVALID_HANDLE)
        ok &= CNA_TEST_STAGE(cna_game_destroy(game) == CNA_RESULT_SUCCESS);
    return ok ? 0 : CNA_TEST_FAIL(1);
}
