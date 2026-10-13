// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.sprite_font_state",
    .title = "Caller-built SpriteFont state in C",
    .description = "A C game builds a SpriteFont from an atlas and asymmetric glyph records, proves that the "
                   "caller tables are copied, reads both parallel collections atomically, mutates layout and "
                   "fallback state, measures the resulting text and verifies atlas retention.",
    .apis = {"cna_sprite_font_create", "cna_sprite_font_get_info",
             "cna_sprite_font_copy_characters", "cna_sprite_font_copy_glyphs",
             "cna_sprite_font_set_default_character", "cna_sprite_font_set_line_spacing",
             "cna_sprite_font_set_spacing", "cna_sprite_font_measure_utf8",
             "cna_sprite_font_destroy", "CNA_SpriteFontGlyph", "CNA_SpriteFontCreateInfo",
             "CNA_SpriteFontInfo"},
    .tags = {"c", "binding", "graphics", "sprite-font", "glyph", "kerning", "lifetime"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone one-frame C game. Glyph tables, measurements and resource lifetime are checked "
                    "programmatically under both renderer selections. Nothing is drawn and no screenshot or "
                    "visual inspection is claimed. Built only when CNA_BUILD_C_API=ON.",
})
