/* SPDX-License-Identifier: MS-PL */
/* ShaderEffect from C: texture-family validation, replacement and retained ownership. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct TextureState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle effect;
    CNA_EffectTechniqueCollectionHandle techniques;
    CNA_EffectTechniqueHandle technique;
    CNA_EffectPassCollectionHandle passes;
    CNA_EffectPassHandle pass;
    CNA_Handle texture2d;
    CNA_Handle cube;
    CNA_Handle volume;
    CNA_Bool has_volume;
} TextureState;

static CNA_StringView view(const char* value)
{
    CNA_StringView result = {value, (uint64_t)strlen(value)};
    return result;
}

static CNA_Result create_shader(CNA_Handle device, CNA_EffectHandle* out_effect)
{
    static const char EsVertex[] =
        "#version 300 es\nprecision highp float;\nlayout(location=0) in vec3 aPosition;\n"
        "void main(){gl_Position=vec4(aPosition,1.0);}\n";
    static const char EsFragment[] =
        "#version 300 es\nprecision highp float;\nout vec4 FragColor;\n"
        "void main(){FragColor=vec4(1.0);}\n";
    static const char DesktopVertex[] =
        "#version 330 core\nlayout(location=0) in vec3 aPosition;\n"
        "void main(){gl_Position=vec4(aPosition,1.0);}\n";
    static const char DesktopFragment[] =
        "#version 330 core\nout vec4 FragColor;\nvoid main(){FragColor=vec4(1.0);}\n";
    static const char Placeholder[] = "void main() { }";
    CNA_ShaderDialect dialect = CNA_SHADER_DIALECT_UNKNOWN;
    const char* vertex = Placeholder;
    const char* fragment = Placeholder;
    CNA_Result result = cna_graphics_device_get_shader_dialect_ext(device, &dialect);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    if (dialect == CNA_SHADER_DIALECT_GLSL_ES) {
        vertex = EsVertex;
        fragment = EsFragment;
    } else if (dialect == CNA_SHADER_DIALECT_GLSL_DESKTOP) {
        vertex = DesktopVertex;
        fragment = DesktopFragment;
    }
    return cna_shader_effect_create(device, view(vertex), view(fragment), out_effect);
}

static CNA_Result create_textures(TextureState* state, CNA_Handle device)
{
    const CNA_Texture2DCreateInfo info2d = {
        sizeof(CNA_Texture2DCreateInfo), UINT32_C(1), 1U, 1U,
        CNA_FALSE, {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR};
    const CNA_TextureCubeCreateInfo cube_info = {
        sizeof(CNA_TextureCubeCreateInfo), UINT32_C(1), 1U,
        CNA_FALSE, {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR, 0U};
    const CNA_Texture3DCreateInfo volume_info = {
        sizeof(CNA_Texture3DCreateInfo), UINT32_C(1), 1U, 1U, 1U,
        CNA_FALSE, {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR, 0U};
    CNA_Result result = cna_texture2d_create(device, &info2d, &state->texture2d);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_texturecube_create(device, &cube_info, &state->cube);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_texture3d_create(device, &volume_info, &state->volume);
    if (result == CNA_RESULT_SUCCESS) {
        state->has_volume = CNA_TRUE;
        return CNA_RESULT_SUCCESS;
    }
    return result == CNA_RESULT_NOT_SUPPORTED && state->volume == CNA_INVALID_HANDLE
        ? CNA_RESULT_SUCCESS
        : result;
}

static void inspect_bindings(TextureState* state)
{
    CnaDemo* demo = state->demo;
    cna_demo_check(
        demo,
        cna_shader_effect_set_texture2d(state->effect, -1, state->texture2d) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_shader_effect_set_texture_cube(state->effect, -1, state->cube) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_shader_effect_set_texture3d(state->effect, -1, state->texture2d) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "every texture family refuses a negative sampler unit before binding");
    cna_demo_check(
        demo,
        cna_shader_effect_set_texture2d(state->effect, 1, state->cube) ==
                CNA_RESULT_INVALID_HANDLE &&
            cna_shader_effect_set_texture_cube(state->effect, 1, state->texture2d) ==
                CNA_RESULT_INVALID_HANDLE &&
            cna_shader_effect_set_texture3d(state->effect, 1, state->texture2d) ==
                CNA_RESULT_INVALID_HANDLE,
        "2D, cube and volume setters reject a live texture from the wrong family");
    cna_demo_check(
        demo,
        cna_shader_effect_set_texture2d(state->effect, 1, state->texture2d) == CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->texture2d) == CNA_RESULT_INVALID_STATE,
        "the ShaderEffect retains a bound Texture2D");
    cna_demo_check(
        demo,
        cna_shader_effect_set_texture_cube(state->effect, 1, state->cube) == CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(state->texture2d) == CNA_RESULT_SUCCESS &&
            cna_texturecube_destroy(state->cube) == CNA_RESULT_INVALID_STATE,
        "a cube replaces and releases the prior 2D binding on the same unit");
    state->texture2d = CNA_INVALID_HANDLE;
    if (state->has_volume == CNA_TRUE) {
        cna_demo_check(
            demo,
            cna_shader_effect_set_texture3d(state->effect, 1, state->volume) == CNA_RESULT_SUCCESS &&
                cna_texturecube_destroy(state->cube) == CNA_RESULT_SUCCESS &&
                cna_texture3d_destroy(state->volume) == CNA_RESULT_INVALID_STATE,
            "a supported Texture3D replaces the cube and is retained on the same unit");
        state->cube = CNA_INVALID_HANDLE;
    } else {
        cna_demo_check(
            demo, state->volume == CNA_INVALID_HANDLE,
            "an unsupported Texture3D constructor clears its output without disturbing the cube");
    }
}

static void retain_through_pass(TextureState* state)
{
    const CNA_Handle retained = state->has_volume == CNA_TRUE ? state->volume : state->cube;
    cna_demo_check(
        state->demo,
        cna_effect_get_techniques(state->effect, &state->techniques) == CNA_RESULT_SUCCESS &&
            cna_effect_technique_collection_get_at(
                state->techniques, 0U, &state->technique) == CNA_RESULT_SUCCESS &&
            cna_effect_technique_get_passes(state->technique, &state->passes) == CNA_RESULT_SUCCESS &&
            cna_effect_pass_collection_get_at(state->passes, 0U, &state->pass) == CNA_RESULT_SUCCESS,
        "the configured ShaderEffect publishes its canonical pass");
    cna_demo_check(
        state->demo,
        cna_effect_destroy(state->effect) == CNA_RESULT_SUCCESS &&
            cna_effect_pass_apply(state->pass) == CNA_RESULT_SUCCESS &&
            (state->has_volume == CNA_TRUE
                 ? cna_texture3d_destroy(retained)
                 : cna_texturecube_destroy(retained)) == CNA_RESULT_INVALID_STATE,
        "a retained pass keeps the configured effect and final texture binding alive");
    state->effect = CNA_INVALID_HANDLE;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    TextureState* state = (TextureState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;
    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = create_shader(device, &state->effect);
    if (!cna_demo_check_result(state->demo, result, "create the texture-binding ShaderEffect")) {
        return result;
    }
    result = create_textures(state, device);
    if (!cna_demo_check_result(state->demo, result, "create the three texture-family resources")) {
        return result;
    }
    inspect_bindings(state);
    retain_through_pass(state);
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

int main(int argc, char** argv)
{
    CnaDemo demo;
    TextureState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_Result final_texture_result = CNA_RESULT_SUCCESS;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.shader_effect_textures", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&create_info, 0, sizeof(create_info));
    state.demo = &demo;
    state.effect = state.techniques = state.technique = state.passes = state.pass = CNA_INVALID_HANDLE;
    state.texture2d = state.cube = state.volume = CNA_INVALID_HANDLE;
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
        &demo, cna_demo_use_hidef(game, &manager), "create the graphics device for texture bindings");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the ShaderEffect texture audit");
    }
    cna_demo_check(&demo, state.loads == 1, "LoadContent runs exactly once");
    if (state.techniques != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_effect_technique_collection_destroy(state.techniques), "release technique collection");
    }
    if (state.technique != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_effect_technique_destroy(state.technique), "release technique view");
    }
    if (state.passes != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_effect_pass_collection_destroy(state.passes), "release pass collection");
    }
    if (state.pass != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_effect_pass_destroy(state.pass), "release the final pass view");
    }
    if (state.has_volume == CNA_TRUE && state.volume != CNA_INVALID_HANDLE) {
        final_texture_result = cna_texture3d_destroy(state.volume);
    } else if (state.cube != CNA_INVALID_HANDLE) {
        final_texture_result = cna_texturecube_destroy(state.cube);
    }
    cna_demo_check_result(
        &demo, final_texture_result, "release the final texture after every effect view");
    if (state.effect != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_effect_destroy(state.effect), "destroy ShaderEffect after early failure");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager), "destroy the graphics manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
