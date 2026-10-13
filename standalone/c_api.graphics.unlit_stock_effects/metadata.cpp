// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.unlit_stock_effects",
    .title = "Alpha-test and dual-texture effects in C",
    .description = "A C game compares AlphaTestEffect and DualTextureEffect defaults and state, "
                   "uses their shared matrix and fog interfaces, and follows retained textures through "
                   "independent slots, clones, source destruction and final release.",
    .apis = {"cna_alpha_test_effect_create",
             "cna_alpha_test_effect_get_diffuse_color", "cna_alpha_test_effect_set_diffuse_color",
             "cna_alpha_test_effect_get_alpha", "cna_alpha_test_effect_set_alpha",
             "cna_alpha_test_effect_get_texture", "cna_alpha_test_effect_set_texture",
             "cna_alpha_test_effect_get_vertex_color_enabled",
             "cna_alpha_test_effect_set_vertex_color_enabled",
             "cna_alpha_test_effect_get_alpha_function", "cna_alpha_test_effect_set_alpha_function",
             "cna_alpha_test_effect_get_reference_alpha", "cna_alpha_test_effect_set_reference_alpha",
             "cna_dual_texture_effect_create",
             "cna_dual_texture_effect_get_diffuse_color", "cna_dual_texture_effect_set_diffuse_color",
             "cna_dual_texture_effect_get_alpha", "cna_dual_texture_effect_set_alpha",
             "cna_dual_texture_effect_get_texture", "cna_dual_texture_effect_set_texture",
             "cna_dual_texture_effect_get_vertex_color_enabled",
             "cna_dual_texture_effect_set_vertex_color_enabled",
             "cna_effect_matrices_get_world", "cna_effect_matrices_set_world",
             "cna_effect_fog_get_color", "cna_effect_fog_set_color",
             "cna_effect_fog_get_enabled", "cna_effect_fog_set_enabled",
             "cna_effect_fog_get_start", "cna_effect_fog_set_start",
             "cna_effect_fog_get_end", "cna_effect_fog_set_end",
             "cna_texture2d_create", "cna_texture2d_destroy",
             "cna_effect_clone", "cna_effect_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "alpha-test", "dual-texture", "fog", "texture", "ownership", "clone"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C game. Apply is state-only: the demo makes no draw, alpha-test pixel, "
                    "texture-sampling or visual claim. Built only with CNA_BUILD_C_API=ON.",
})
