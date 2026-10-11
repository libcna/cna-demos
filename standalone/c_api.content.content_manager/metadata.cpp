// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.content_manager",
    .title = "Content in C: a ContentManager",
    .description = "A C program creates a ContentManager rooted at the demos' assets, loads a PNG texture and a "
                   "compiled SpriteFont (a font handle plus its atlas texture), measures and draws text with "
                   "cna_sprite_batch_draw_string, and releases everything in the documented order.",
    .apis = {"cna_content_manager_create", "cna_content_manager_load_texture2d",
             "cna_content_manager_load_sprite_font", "cna_content_manager_destroy", "cna_texture2d_get_info",
             "cna_sprite_font_measure_utf8", "cna_sprite_batch_draw_string", "cna_sprite_font_destroy",
             "CNA_ContentManagerCreateInfo", "CNA_SpriteTextCommand"},
    .tags = {"c", "binding", "content", "font", "texture"},
    .needs = CnaDemos::Need::CApi,
    .frames = 10,
    .deterministic = true,
    .assets = {"textures/checker.png", "fonts/pixel_font.cnb"},
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
