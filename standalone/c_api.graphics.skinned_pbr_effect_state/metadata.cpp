// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.skinned_pbr_effect_state",
    .title = "SkinnedPbrEffect palette and shared PBR state in C",
    .description = "A C game proves that shared PBR material, texture and effect-interface routes accept "
                   "SkinnedPbrEffect, then exercises its one/two/four-weight contract, copied 72-bone palette "
                   "boundaries and material/texture/palette clone lifetime.",
    .apis = {"cna_skinned_pbr_effect_create",
             "cna_skinned_pbr_effect_get_weights_per_vertex",
             "cna_skinned_pbr_effect_set_weights_per_vertex",
             "cna_skinned_pbr_effect_set_bone_transforms",
             "cna_skinned_pbr_effect_copy_bone_transforms",
             "cna_pbr_effect_get_diffuse_color", "cna_pbr_effect_set_diffuse_color",
             "cna_pbr_effect_get_alpha", "cna_pbr_effect_set_alpha",
             "cna_pbr_effect_get_metallic_factor", "cna_pbr_effect_set_metallic_factor",
             "cna_pbr_effect_get_roughness_factor", "cna_pbr_effect_set_roughness_factor",
             "cna_pbr_effect_get_emissive_factor", "cna_pbr_effect_set_emissive_factor",
             "cna_pbr_effect_get_vertex_color_enabled_ext", "cna_pbr_effect_set_vertex_color_enabled_ext",
             "cna_pbr_effect_get_texture", "cna_pbr_effect_set_texture",
             "cna_effect_matrices_get_world", "cna_effect_matrices_set_world",
             "cna_effect_fog_get_enabled", "cna_effect_fog_set_enabled",
             "cna_effect_lights_get_enabled", "cna_effect_lights_set_enabled",
             "cna_texture2d_create", "cna_texture2d_destroy",
             "cna_effect_clone", "cna_effect_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "pbr", "skinning", "bones", "weights", "texture", "ownership", "clone", "cnaext"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C game. Apply is state-only: no skinned PBR draw, vertex deformation, BRDF pixel "
                    "or visual claim is made. Built only with CNA_BUILD_C_API=ON.",
})
