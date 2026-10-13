// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.effect_concrete_types",
    .title = "EffectMaterial and SpriteEffect ownership in C",
    .description = "A C game turns an Effect into an EffectMaterial that owns parameter textures after caller "
                   "release, contrasts its inherited base-Effect clone with SpriteEffect's type-preserving "
                   "clone, and applies every surviving object independently.",
    .apis = {"cna_effect_material_create", "cna_effect_material_retain_parameter_texture_ext",
             "cna_effect_material_get_retained_parameter_texture_count_ext",
             "cna_sprite_effect_create", "cna_effect_clone", "cna_effect_apply",
             "cna_effect_get_graphics_device", "cna_effect_get_type_name_byte_count",
             "cna_effect_copy_type_name", "cna_effect_has_renderer",
             "cna_effect_is_exact_stock_sprite_effect", "cna_effect_dispose",
             "cna_texture2d_create_cpu_only_rgba8", "cna_texture2d_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "material", "sprite", "texture", "ownership", "clone"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C game. EffectMaterial's inherited Clone returns a base Effect by canonical XNA "
                    "design; CNA-DEMOS-ISSUE-069 records the overly broad C route comment. Texture ownership is "
                    "asserted by the dedicated retained count. Apply is state-only; there is no draw, pixel or "
                    "visual claim. Built only with CNA_BUILD_C_API=ON.",
})
