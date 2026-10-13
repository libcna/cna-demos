// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.shader_effect_uniforms",
    .title = "ShaderEffect typed uniforms and blocks in C",
    .description = "A C game configures every scalar, vector, matrix and array ShaderEffect uniform family, "
                   "including tightly packed vec3/mat4 arrays and the portable uniform-block declaration.",
    .apis = {"cna_shader_effect_set_uniform_matrix", "cna_shader_effect_set_uniform_vector4",
             "cna_shader_effect_set_uniform_vector3", "cna_shader_effect_set_uniform_vector2",
             "cna_shader_effect_set_uniform_float", "cna_shader_effect_set_uniform_int32",
             "cna_shader_effect_declare_uniform_block_ext",
             "cna_shader_effect_set_uniform_float_array",
             "cna_shader_effect_set_uniform_vector2_array",
             "cna_shader_effect_set_uniform_vec3_array",
             "cna_shader_effect_set_uniform_mat4_array",
             "cna_shader_effect_create", "cna_effect_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "shader", "uniform", "array", "std140", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C game. Executing renderers compile source that keeps every supplied uniform "
                    "active; other renderers exercise the documented object contract. Existing pixel demos "
                    "cover rendered uniform results. Nothing is drawn and no visual claim is made. Built only "
                    "with CNA_BUILD_C_API=ON.",
})
