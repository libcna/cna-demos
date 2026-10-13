// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.basic_effect_state",
    .title = "BasicEffect state and light ownership in C",
    .description = "A C game round-trips BasicEffect material, matrix, fog, lighting and texture state; "
                   "inspects the canonical three-light preset; and proves that standalone and nested "
                   "DirectionalLight handles have distinct, explicit ownership.",
    .apis = {"cna_basic_effect_create",
             "cna_basic_effect_get_vertex_color_enabled", "cna_basic_effect_set_vertex_color_enabled",
             "cna_basic_effect_get_prefer_per_pixel_lighting",
             "cna_basic_effect_set_prefer_per_pixel_lighting",
             "cna_basic_effect_get_diffuse_color", "cna_basic_effect_set_diffuse_color",
             "cna_basic_effect_get_emissive_color", "cna_basic_effect_set_emissive_color",
             "cna_basic_effect_get_specular_color", "cna_basic_effect_set_specular_color",
             "cna_basic_effect_get_specular_power", "cna_basic_effect_set_specular_power",
             "cna_basic_effect_get_alpha", "cna_basic_effect_set_alpha",
             "cna_basic_effect_get_texture_enabled", "cna_basic_effect_set_texture_enabled",
             "cna_basic_effect_get_texture", "cna_basic_effect_set_texture",
             "cna_effect_matrices_get_world", "cna_effect_matrices_set_world",
             "cna_effect_matrices_get_view", "cna_effect_matrices_set_view",
             "cna_effect_matrices_get_projection", "cna_effect_matrices_set_projection",
             "cna_effect_fog_get_color", "cna_effect_fog_set_color",
             "cna_effect_fog_get_enabled", "cna_effect_fog_set_enabled",
             "cna_effect_fog_get_start", "cna_effect_fog_set_start",
             "cna_effect_fog_get_end", "cna_effect_fog_set_end",
             "cna_effect_lights_get_ambient_color", "cna_effect_lights_set_ambient_color",
             "cna_effect_lights_get_directional_light", "cna_effect_lights_get_enabled",
             "cna_effect_lights_set_enabled", "cna_effect_lights_enable_default",
             "cna_directional_light_create", "cna_directional_light_destroy",
             "cna_directional_light_get_diffuse_color", "cna_directional_light_set_diffuse_color",
             "cna_directional_light_get_direction", "cna_directional_light_set_direction",
             "cna_directional_light_get_specular_color", "cna_directional_light_set_specular_color",
             "cna_directional_light_get_enabled", "cna_directional_light_set_enabled",
             "cna_texture2d_create", "cna_texture2d_destroy", "cna_effect_create_empty",
             "cna_effect_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "basic-effect", "fog", "lighting", "texture", "ownership"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C game. Apply is state-only: the demo makes no draw, pixel or visual claim. "
                    "Built only with CNA_BUILD_C_API=ON.",
})
