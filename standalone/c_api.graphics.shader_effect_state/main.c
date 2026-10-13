/* SPDX-License-Identifier: MS-PL */
/* ShaderEffect from C: creation/validity/diagnostics and independent matrix state. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ShaderState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle effect;
} ShaderState;

static CNA_StringView view(const char* value)
{
    CNA_StringView result = {value, (uint64_t)strlen(value)};
    return result;
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

static CNA_Result create_shader(ShaderState* state, CNA_Handle device, CNA_Bool* out_executes)
{
    static const char EsVertex[] =
        "#version 300 es\nprecision highp float;\nlayout(location=0) in vec3 aPosition;\n"
        "uniform mat4 World; uniform mat4 View; uniform mat4 Projection;\n"
        "void main(){gl_Position=Projection*View*World*vec4(aPosition,1.0);}\n";
    static const char EsFragment[] =
        "#version 300 es\nprecision highp float;\nout vec4 FragColor;\n"
        "void main(){FragColor=vec4(0.25,0.5,0.75,1.0);}\n";
    static const char DesktopVertex[] =
        "#version 330 core\nlayout(location=0) in vec3 aPosition;\n"
        "uniform mat4 World; uniform mat4 View; uniform mat4 Projection;\n"
        "void main(){gl_Position=Projection*View*World*vec4(aPosition,1.0);}\n";
    static const char DesktopFragment[] =
        "#version 330 core\nout vec4 FragColor;\n"
        "void main(){FragColor=vec4(0.25,0.5,0.75,1.0);}\n";
    static const char Placeholder[] = "void main() { }";
    CNA_ShaderDialect dialect = CNA_SHADER_DIALECT_UNKNOWN;
    const char* vertex = Placeholder;
    const char* fragment = Placeholder;
    CNA_Result result;

    result = cna_graphics_device_get_shader_dialect_ext(device, &dialect);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_graphics_device_executes_shader_effect_source_ext(device, out_executes);
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
    cna_demo_check(
        state->demo,
        dialect <= CNA_SHADER_DIALECT_MAXIMUM &&
            (*out_executes == CNA_FALSE || dialect != CNA_SHADER_DIALECT_UNKNOWN),
        "the active renderer reports a named shader dialect whenever it executes source");
    return cna_shader_effect_create(device, view(vertex), view(fragment), &state->effect);
}

static void inspect_validity_and_diagnostics(ShaderState* state, CNA_Bool executes)
{
    CNA_Bool valid = CNA_FALSE;
    CNA_Bool has_renderer = CNA_FALSE;
    uint64_t bytes = UINT64_MAX;
    char guard = 'X';

    cna_demo_check(
        state->demo,
        cna_shader_effect_is_valid(state->effect, &valid) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_has_renderer(state->effect, &has_renderer) == CNA_RESULT_SUCCESS &&
            (valid == CNA_FALSE || valid == CNA_TRUE) &&
            (has_renderer == CNA_FALSE || has_renderer == CNA_TRUE) &&
            (valid == CNA_FALSE || has_renderer == CNA_TRUE),
        "validity and renderer ownership return canonical, internally consistent verdicts");
    cna_demo_check(
        state->demo,
        executes == CNA_FALSE || (valid == CNA_TRUE && has_renderer == CNA_TRUE),
        "a renderer advertising source execution accepts this dialect-correct shader pair");
    cna_demo_check(
        state->demo,
        cna_shader_effect_copy_compile_error_ext(
            state->effect, NULL, 0U, &bytes) == CNA_RESULT_SUCCESS && bytes == 0U &&
            cna_shader_effect_copy_compile_error_ext(
                state->effect, &guard, 0U, &bytes) == CNA_RESULT_SUCCESS && bytes == 0U &&
            guard == 'X',
        "the accepted shader has an exact empty compile-error byte view");
}

static void inspect_creation_boundary(ShaderState* state, CNA_Handle device)
{
    CNA_EffectHandle empty = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_shader_effect_create(device, view(""), view(""), &empty) ==
                CNA_RESULT_INVALID_ARGUMENT && empty == CNA_INVALID_HANDLE,
        "creation uniformly refuses two empty sources and clears the output handle");
}

static void inspect_matrices(ShaderState* state)
{
    const CNA_Matrix identity = identity_matrix();
    CNA_Matrix world = identity;
    CNA_Matrix view_matrix = identity;
    CNA_Matrix projection = identity;
    CNA_Matrix actual = {0};

    cna_demo_check(
        state->demo,
        cna_shader_effect_get_world(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, identity) &&
            cna_shader_effect_get_view(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, identity) &&
            cna_shader_effect_get_projection(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, identity),
        "World, View and Projection each start as identity");
    world.m41 = 7.0F;
    world.m12 = 0.25F;
    view_matrix.m42 = 8.0F;
    view_matrix.m23 = -0.5F;
    projection.m43 = 9.0F;
    projection.m31 = 1.5F;
    cna_demo_check(
        state->demo,
        cna_shader_effect_set_world(state->effect, world) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_view(state->effect, view_matrix) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_projection(state->effect, projection) == CNA_RESULT_SUCCESS,
        "the three ShaderEffect matrix setters accept distinct complete values");
    cna_demo_check(
        state->demo,
        cna_shader_effect_get_world(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, world) &&
            cna_shader_effect_get_view(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, view_matrix) &&
            cna_shader_effect_get_projection(state->effect, &actual) == CNA_RESULT_SUCCESS &&
            matrix_is(actual, projection),
        "World, View and Projection round-trip independently without transposition");
    cna_demo_check(
        state->demo, cna_effect_apply(state->effect) == CNA_RESULT_SUCCESS,
        "the configured ShaderEffect applies without drawing");
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    ShaderState* state = (ShaderState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Bool executes = CNA_FALSE;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = create_shader(state, device, &executes);
    if (!cna_demo_check_result(state->demo, result, "create the dialect-appropriate ShaderEffect")) {
        return result;
    }
    inspect_validity_and_diagnostics(state, executes);
    inspect_creation_boundary(state, device);
    inspect_matrices(state);
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
    ShaderState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.shader_effect_state", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&create_info, 0, sizeof(create_info));
    state.demo = &demo;
    state.effect = CNA_INVALID_HANDLE;
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
        &demo, cna_demo_use_hidef(game, &manager), "create the graphics device for ShaderEffect");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the ShaderEffect state audit");
    }
    cna_demo_check(&demo, state.loads == 1, "LoadContent runs exactly once");
    if (state.effect != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_effect_destroy(state.effect), "destroy ShaderEffect");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
