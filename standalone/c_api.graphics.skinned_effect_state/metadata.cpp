// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.skinned_effect_state",
    .title = "SkinnedEffect palette and material state in C",
    .description = "A C game round-trips SkinnedEffect material and shader-selection state, exercises the "
                   "one/two/four weight contract and atomic 72-matrix bone palette, and follows an owned "
                   "texture and copied palette through cloning and source destruction.",
    .apis = {"cna_skinned_effect_create",
             "cna_skinned_effect_get_diffuse_color", "cna_skinned_effect_set_diffuse_color",
             "cna_skinned_effect_get_emissive_color", "cna_skinned_effect_set_emissive_color",
             "cna_skinned_effect_get_specular_color", "cna_skinned_effect_set_specular_color",
             "cna_skinned_effect_get_specular_power", "cna_skinned_effect_set_specular_power",
             "cna_skinned_effect_get_alpha", "cna_skinned_effect_set_alpha",
             "cna_skinned_effect_get_prefer_per_pixel_lighting",
             "cna_skinned_effect_set_prefer_per_pixel_lighting",
             "cna_skinned_effect_get_texture", "cna_skinned_effect_set_texture",
             "cna_skinned_effect_get_weights_per_vertex", "cna_skinned_effect_set_weights_per_vertex",
             "cna_skinned_effect_set_bone_transforms", "cna_skinned_effect_copy_bone_transforms",
             "cna_skinned_effect_get_vertex_color_enabled",
             "cna_skinned_effect_set_vertex_color_enabled",
             "cna_effect_matrices_get_world", "cna_effect_matrices_set_world",
             "cna_effect_fog_get_color", "cna_effect_fog_set_color",
             "cna_effect_fog_get_enabled", "cna_effect_fog_set_enabled",
             "cna_effect_fog_get_start", "cna_effect_fog_set_start",
             "cna_effect_fog_get_end", "cna_effect_fog_set_end",
             "cna_effect_lights_get_ambient_color", "cna_effect_lights_set_ambient_color",
             "cna_effect_lights_get_enabled", "cna_effect_lights_set_enabled",
             "cna_effect_lights_enable_default",
             "cna_texture2d_create", "cna_texture2d_create_cpu_only_rgba8", "cna_texture2d_destroy",
             "cna_effect_clone", "cna_effect_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "skinning", "bones", "lighting", "fog", "texture", "ownership", "clone"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C game. Apply is state-only: the demo makes no skinned draw, vertex-weight, "
                    "pixel or visual claim. Built only with CNA_BUILD_C_API=ON.",
})
