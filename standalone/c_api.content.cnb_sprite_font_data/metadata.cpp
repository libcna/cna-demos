// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_sprite_font_data",
    .title = "CNB SpriteFont data and codec in C",
    .description = "A renderer-free C program builds a compiled SpriteFont glyph table and atlas, "
                   "then round-trips the complete asset through a CNB document.",
    .apis = {"cna_cnb_sprite_font_data_create", "cna_cnb_sprite_font_data_destroy",
             "cna_cnb_sprite_font_data_get_info", "cna_cnb_sprite_font_data_set_info",
             "cna_cnb_sprite_font_data_add_glyph", "cna_cnb_sprite_font_data_get_glyph",
             "cna_cnb_sprite_font_data_set_glyph", "cna_cnb_sprite_font_data_set_atlas",
             "cna_cnb_sprite_font_data_copy_atlas", "cna_cnb_encode_sprite_font",
             "cna_cnb_decode_sprite_font"},
    .tags = {"c", "binding", "content", "cnb", "sprite-font", "glyph", "texture"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses synthetic glyphs and an in-memory atlas. No ContentManager, graphics "
                    "device, renderer, text layout or draw call is used.",
})
