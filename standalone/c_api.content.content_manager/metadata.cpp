// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.content_manager",
    .title = "Content in C: a ContentManager",
    .description = "A C program creates a ContentManager rooted at the demos' assets, loads a PNG texture and a "
                   "compiled SpriteFont (a font handle plus its atlas texture), measures and draws text with "
                   "cna_sprite_batch_draw_string after unloading and destroying the manager, and releases "
                   "the independently owned resources in the documented order. Checks that a live font "
                   "prevents an early atlas release and still draws visible text afterwards; root changes preserve cached "
                   "names, redirect new loads and copy the caller's root string.",
    .apis = {"cna_content_manager_create", "cna_content_manager_load_texture2d",
             "cna_content_manager_load_sprite_font", "cna_content_manager_unload", "cna_content_manager_destroy",
             "cna_content_manager_set_root_directory", "cna_content_manager_get_root_directory_size",
             "cna_content_manager_copy_root_directory",
             "cna_texture2d_get_info", "cna_texture2d_get_data", "cna_texture2d_destroy",
             "cna_sprite_font_measure_utf8", "cna_sprite_batch_draw_string", "cna_sprite_font_destroy",
             "cna_graphics_device_get_backbuffer_data_rgba8", "CNA_ContentManagerCreateInfo", "CNA_SpriteTextCommand"},
    .tags = {"c", "binding", "content", "font", "texture"},
    .needs = CnaDemos::Need::CApi,
    .frames = 10,
    .deterministic = true,
    .assets = {"textures/checker.png", "textures/sprite_strip.png", "fonts/pixel_font.cnb"},
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
