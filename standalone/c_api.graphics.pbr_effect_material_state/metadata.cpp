// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.pbr_effect_material_state",
    .title = "PBR material and coverage state in C",
    .description = "A C game exercises the complete non-texture PbrEffect material surface: core metallic-roughness "
                   "factors, glTF specular and coverage extensions, vertex/output switches, required lighting, "
                   "fog/matrices and type-preserving clone lifetime.",
    .apis = {"cna_pbr_effect_create",
             "cna_pbr_effect_get_diffuse_color", "cna_pbr_effect_set_diffuse_color",
             "cna_pbr_effect_get_alpha", "cna_pbr_effect_set_alpha",
             "cna_pbr_effect_get_metallic_factor", "cna_pbr_effect_set_metallic_factor",
             "cna_pbr_effect_get_roughness_factor", "cna_pbr_effect_set_roughness_factor",
             "cna_pbr_effect_get_emissive_factor", "cna_pbr_effect_set_emissive_factor",
             "cna_pbr_effect_get_vertex_color_enabled_ext", "cna_pbr_effect_set_vertex_color_enabled_ext",
             "cna_pbr_effect_get_ior_ext", "cna_pbr_effect_set_ior_ext",
             "cna_pbr_effect_get_specular_factor_ext", "cna_pbr_effect_set_specular_factor_ext",
             "cna_pbr_effect_get_specular_color_factor_ext", "cna_pbr_effect_set_specular_color_factor_ext",
             "cna_pbr_effect_get_normal_scale_ext", "cna_pbr_effect_set_normal_scale_ext",
             "cna_pbr_effect_get_occlusion_strength_ext", "cna_pbr_effect_set_occlusion_strength_ext",
             "cna_pbr_effect_get_encode_output_to_srgb_ext", "cna_pbr_effect_set_encode_output_to_srgb_ext",
             "cna_pbr_effect_get_alpha_mode_ext", "cna_pbr_effect_set_alpha_mode_ext",
             "cna_pbr_effect_get_alpha_cutoff_ext", "cna_pbr_effect_set_alpha_cutoff_ext",
             "cna_pbr_effect_get_double_sided_ext", "cna_pbr_effect_set_double_sided_ext",
             "cna_effect_matrices_get_world", "cna_effect_matrices_set_world",
             "cna_effect_fog_get_enabled", "cna_effect_fog_set_enabled",
             "cna_effect_fog_get_end", "cna_effect_fog_set_end",
             "cna_effect_lights_get_ambient_color", "cna_effect_lights_set_ambient_color",
             "cna_effect_lights_get_enabled", "cna_effect_lights_set_enabled",
             "cna_effect_lights_enable_default",
             "cna_effect_clone", "cna_effect_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "pbr", "gltf", "material", "lighting", "fog", "clone", "cnaext"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C game. Apply is state-only: no PBR draw, BRDF result, pixel or visual claim is "
                    "made. Built only with CNA_BUILD_C_API=ON.",
})
