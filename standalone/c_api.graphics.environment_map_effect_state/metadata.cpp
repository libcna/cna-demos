// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.environment_map_effect_state",
    .title = "EnvironmentMapEffect state and textures in C",
    .description = "A C game round-trips EnvironmentMapEffect material and shared lighting/fog/matrix state, "
                   "shows that lighting cannot be disabled, and follows a diffuse Texture2D and environment "
                   "TextureCube through cloning, source destruction and release.",
    .apis = {"cna_environment_map_effect_create",
             "cna_environment_map_effect_get_diffuse_color", "cna_environment_map_effect_set_diffuse_color",
             "cna_environment_map_effect_get_emissive_color", "cna_environment_map_effect_set_emissive_color",
             "cna_environment_map_effect_get_alpha", "cna_environment_map_effect_set_alpha",
             "cna_environment_map_effect_get_texture", "cna_environment_map_effect_set_texture",
             "cna_environment_map_effect_get_environment_map",
             "cna_environment_map_effect_set_environment_map",
             "cna_environment_map_effect_get_amount", "cna_environment_map_effect_set_amount",
             "cna_environment_map_effect_get_specular", "cna_environment_map_effect_set_specular",
             "cna_environment_map_effect_get_fresnel_factor",
             "cna_environment_map_effect_set_fresnel_factor",
             "cna_effect_matrices_get_world", "cna_effect_matrices_set_world",
             "cna_effect_fog_get_color", "cna_effect_fog_set_color",
             "cna_effect_fog_get_enabled", "cna_effect_fog_set_enabled",
             "cna_effect_fog_get_start", "cna_effect_fog_set_start",
             "cna_effect_fog_get_end", "cna_effect_fog_set_end",
             "cna_effect_lights_get_ambient_color", "cna_effect_lights_set_ambient_color",
             "cna_effect_lights_get_directional_light", "cna_effect_lights_get_enabled",
             "cna_effect_lights_set_enabled", "cna_effect_lights_enable_default",
             "cna_directional_light_get_enabled", "cna_directional_light_destroy",
             "cna_texture2d_create", "cna_texture2d_destroy",
             "cna_texturecube_create", "cna_texturecube_destroy",
             "cna_effect_clone", "cna_effect_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "environment-map", "lighting", "fog", "texture-cube", "ownership", "clone"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C game. Apply is state-only: the demo makes no draw, cube-sampling, pixel "
                    "or visual claim. Built only with CNA_BUILD_C_API=ON.",
})
