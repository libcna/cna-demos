/* SPDX-License-Identifier: MS-PL */
/* ShaderEffect from C: typed loose uniforms, packed arrays and std140 metadata. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct UniformState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle effect;
} UniformState;

static CNA_StringView view(const char* value)
{
    CNA_StringView result = {value, (uint64_t)strlen(value)};
    return result;
}

static CNA_Matrix identity_matrix(void)
{
    CNA_Matrix value;
    memset(&value, 0, sizeof(value));
    value.m11 = value.m22 = value.m33 = value.m44 = 1.0F;
    return value;
}

static CNA_Result create_shader(CNA_Handle device, CNA_EffectHandle* out_effect)
{
    static const char EsVertex[] =
        "#version 300 es\nprecision highp float;\nlayout(location=0) in vec3 aPosition;\n"
        "uniform mat4 UMatrix; uniform vec4 UVector4; uniform vec3 UVector3; uniform vec2 UVector2;\n"
        "uniform float UFloat; uniform int UInt; uniform float UFloatArray[3];\n"
        "uniform vec2 UVector2Array[2]; uniform vec3 UVector3Array[2]; uniform mat4 UMatrixArray[2];\n"
        "void main(){float s=UVector4.x+UVector3.y+UVector2.x+UFloat+float(UInt)+UFloatArray[2]+"
        "UVector2Array[1].y+UVector3Array[1].z+UMatrixArray[1][3][0];"
        "gl_Position=UMatrix*vec4(aPosition+s*0.000001,1.0);}\n";
    static const char EsFragment[] =
        "#version 300 es\nprecision highp float;\nout vec4 FragColor;\n"
        "void main(){FragColor=vec4(1.0);}\n";
    static const char DesktopVertex[] =
        "#version 330 core\nlayout(location=0) in vec3 aPosition;\n"
        "uniform mat4 UMatrix; uniform vec4 UVector4; uniform vec3 UVector3; uniform vec2 UVector2;\n"
        "uniform float UFloat; uniform int UInt; uniform float UFloatArray[3];\n"
        "uniform vec2 UVector2Array[2]; uniform vec3 UVector3Array[2]; uniform mat4 UMatrixArray[2];\n"
        "void main(){float s=UVector4.x+UVector3.y+UVector2.x+UFloat+float(UInt)+UFloatArray[2]+"
        "UVector2Array[1].y+UVector3Array[1].z+UMatrixArray[1][3][0];"
        "gl_Position=UMatrix*vec4(aPosition+s*0.000001,1.0);}\n";
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

static void inspect_scalar_uniforms(UniformState* state)
{
    CNA_Matrix matrix = identity_matrix();
    static const char BadNameBytes[] = {'B', 'a', 'd', '\0', 'X'};
    const CNA_StringView bad_name = {BadNameBytes, (uint64_t)sizeof(BadNameBytes)};
    matrix.m12 = 2.0F;
    matrix.m41 = 3.0F;

    cna_demo_check(
        state->demo,
        cna_shader_effect_set_uniform_matrix(state->effect, view("UMatrix"), matrix) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_uniform_vector4(
                state->effect, view("UVector4"), (CNA_Vector4){1.0F, 2.0F, 3.0F, 4.0F}) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_uniform_vector3(
                state->effect, view("UVector3"), (CNA_Vector3){5.0F, 6.0F, 7.0F}) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_uniform_vector2(
                state->effect, view("UVector2"), (CNA_Vector2){8.0F, 9.0F}) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_uniform_float(state->effect, view("UFloat"), 10.0F) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_uniform_int32(state->effect, view("UInt"), -11) == CNA_RESULT_SUCCESS,
        "matrix, vec4, vec3, vec2, float and signed-int uniforms accept distinct typed values");
    cna_demo_check(
        state->demo,
        cna_shader_effect_set_uniform_float(state->effect, bad_name, 1.0F) == CNA_RESULT_ENCODING,
        "a length-aware uniform name refuses an embedded NUL");
}

static void inspect_array_uniforms(UniformState* state)
{
    const float scalars[3] = {1.25F, -2.5F, 3.75F};
    const CNA_Vector2 vectors2[2] = {{4.0F, 5.0F}, {-6.0F, 7.0F}};
    const float vectors3[6] = {8.0F, 9.0F, 10.0F, -11.0F, 12.0F, 13.0F};
    float matrices[32] = {0};
    matrices[0] = matrices[5] = matrices[10] = matrices[15] = 1.0F;
    matrices[16] = matrices[21] = matrices[26] = matrices[31] = 2.0F;

    cna_demo_check(
        state->demo,
        cna_shader_effect_set_uniform_float_array(
            state->effect, view("UFloatArray"), scalars, 3U) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_uniform_vector2_array(
                state->effect, view("UVector2Array"), vectors2, 2U) == CNA_RESULT_SUCCESS,
        "float and vec2 arrays accept complete typed sequences");
    cna_demo_check(
        state->demo,
        cna_shader_effect_set_uniform_vec3_array(
            state->effect, view("UVector3Array"), vectors3, 2) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_uniform_mat4_array(
                state->effect, view("UMatrixArray"), matrices, 2) == CNA_RESULT_SUCCESS,
        "vec3 and mat4 arrays accept tightly packed three- and sixteen-float elements");
    cna_demo_check(
        state->demo,
        cna_shader_effect_set_uniform_float_array(
            state->effect, view("UFloatArray"), NULL, 0U) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_uniform_vector2_array(
                state->effect, view("UVector2Array"), NULL, 0U) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_uniform_vec3_array(
                state->effect, view("UVector3Array"), NULL, 0) == CNA_RESULT_SUCCESS &&
            cna_shader_effect_set_uniform_mat4_array(
                state->effect, view("UMatrixArray"), NULL, 0) == CNA_RESULT_SUCCESS,
        "all four array families share the documented empty null sequence");
    cna_demo_check(
        state->demo,
        cna_shader_effect_set_uniform_float_array(
            state->effect, view("UFloatArray"), NULL, 1U) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_shader_effect_set_uniform_vector2_array(
                state->effect, view("UVector2Array"), NULL, 1U) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_shader_effect_set_uniform_vec3_array(
                state->effect, view("UVector3Array"), NULL, 1) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_shader_effect_set_uniform_mat4_array(
                state->effect, view("UMatrixArray"), NULL, 1) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_shader_effect_set_uniform_vec3_array(
                state->effect, view("UVector3Array"), vectors3, -1) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_shader_effect_set_uniform_mat4_array(
                state->effect, view("UMatrixArray"), matrices, -1) == CNA_RESULT_INVALID_ARGUMENT,
        "nonempty null and signed negative array requests are refused consistently");
}

static void inspect_uniform_block(UniformState* state)
{
    char tint[] = "Tint";
    char scale[] = "Scale";
    CNA_StringView names[2] = {{tint, 4U}, {scale, 5U}};
    int32_t offsets[2] = {0, 16};

    cna_demo_check(
        state->demo,
        cna_shader_effect_declare_uniform_block_ext(
            state->effect, 32, names, offsets, 2U) == CNA_RESULT_SUCCESS,
        "a std140 block accepts two named members at explicit byte offsets");
    memset(tint, 'X', sizeof(tint));
    memset(scale, 'Y', sizeof(scale));
    offsets[0] = offsets[1] = -1;
    cna_demo_check(
        state->demo,
        cna_shader_effect_declare_uniform_block_ext(
            state->effect, 0, NULL, NULL, 0U) == CNA_RESULT_SUCCESS,
        "zero members clear the block declaration after caller storage is overwritten");
    cna_demo_check(
        state->demo,
        cna_shader_effect_declare_uniform_block_ext(
            state->effect, -1, names, offsets, 2U) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_shader_effect_declare_uniform_block_ext(
                state->effect, 32, NULL, offsets, 2U) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_shader_effect_declare_uniform_block_ext(
                state->effect, 32, names, NULL, 2U) == CNA_RESULT_INVALID_ARGUMENT,
        "negative size and incomplete uniform-block array pairs are refused");
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    UniformState* state = (UniformState*)context;
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
    if (!cna_demo_check_result(state->demo, result, "create the uniform-bearing ShaderEffect")) {
        return result;
    }
    inspect_scalar_uniforms(state);
    inspect_array_uniforms(state);
    inspect_uniform_block(state);
    cna_demo_check(
        state->demo, cna_effect_apply(state->effect) == CNA_RESULT_SUCCESS,
        "the fully configured ShaderEffect applies without drawing");
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
    UniformState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.shader_effect_uniforms", argc, argv, 1);
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
        &demo, cna_demo_use_hidef(game, &manager), "create the graphics device for uniforms");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game), "run the ShaderEffect uniform audit");
    }
    cna_demo_check(&demo, state.loads == 1, "LoadContent runs exactly once");
    if (state.effect != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_effect_destroy(state.effect), "destroy ShaderEffect");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager), "destroy the graphics manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
