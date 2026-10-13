// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.shader_effect_textures",
    .title = "ShaderEffect texture families and ownership in C",
    .description = "A C game binds real 2D, cube and capability-gated 3D textures to one ShaderEffect "
                   "sampler unit, proving family checks, replacement and retained pass lifetime.",
    .apis = {"cna_shader_effect_set_texture2d", "cna_shader_effect_set_texture_cube",
             "cna_shader_effect_set_texture3d", "cna_texture2d_create", "cna_texture2d_destroy",
             "cna_texturecube_create", "cna_texturecube_destroy", "cna_texture3d_create",
             "cna_texture3d_destroy", "cna_effect_get_techniques",
             "cna_effect_technique_collection_get_at", "cna_effect_technique_get_passes",
             "cna_effect_pass_collection_get_at", "cna_effect_pass_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "shader", "texture", "cube", "volume", "lifetime"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C game. Texture3D construction is capability-gated; refusal is a documented "
                    "outcome. Bindings and ownership are validated without sampling or drawing. No pixel or "
                    "visual claim is made. Built only with CNA_BUILD_C_API=ON.",
})
