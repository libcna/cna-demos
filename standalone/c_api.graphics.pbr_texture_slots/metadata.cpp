// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.pbr_texture_slots",
    .title = "PBR texture-slot state and ownership in C",
    .description = "A C game exercises all seven PBR texture slots, their independent packed-UV selectors and "
                   "KHR texture transforms, the three meaningful sRGB flags, invalid-value preservation and "
                   "two-texture ownership through cloning and source destruction.",
    .apis = {"cna_texture_transform_ext_init", "cna_texture_transform_ext_equals",
             "cna_pbr_effect_create",
             "cna_pbr_effect_get_texture", "cna_pbr_effect_set_texture",
             "cna_pbr_effect_get_texture_coordinate_set_ext",
             "cna_pbr_effect_set_texture_coordinate_set_ext",
             "cna_pbr_effect_get_texture_transform_ext", "cna_pbr_effect_set_texture_transform_ext",
             "cna_pbr_effect_get_texture_is_srgb_ext", "cna_pbr_effect_set_texture_is_srgb_ext",
             "cna_texture2d_create", "cna_texture2d_create_cpu_only_rgba8", "cna_texture2d_destroy",
             "cna_effect_clone", "cna_effect_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "pbr", "gltf", "texture", "uv", "srgb", "ownership", "clone", "cnaext"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C game. State and ownership only: no texture sampling, PBR draw, pixel or visual "
                    "claim is made. Built only with CNA_BUILD_C_API=ON.",
})
