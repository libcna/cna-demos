// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.content_manager",
    .title = "Content in C: a ContentManager",
    .description = "A C program creates a ContentManager rooted at the demos' assets, loads a PNG texture, a "
                   "compiled SpriteFont and an authored stock Effect, measures and draws text with "
                   "cna_sprite_batch_draw_string after unloading and destroying the manager, and releases "
                   "the transferred textures and fonts in the documented order. The loaded Effect is disposed "
                   "by the manager and refuses Apply with the exact disposal diagnostic. Checks that a live font "
                   "prevents an early atlas release and still draws visible text afterwards; loaded managers refuse root "
                   "changes without losing cached names, and changes after Unload copy the caller's string and redirect "
                   "new loads. Also checks stable game-owned manager "
                   "borrowing, copied configuration and invalidation at game destruction. A real missing-asset failure "
                   "also checks the structured canonical exception identity needed by higher-level bindings.",
    .apis = {"cna_game_get_content_manager_ext", "cna_game_set_content_manager_ext", "cna_content_manager_create", "cna_content_manager_load_texture2d",
             "cna_content_manager_load_effect", "cna_effect_apply", "cna_effect_destroy",
             "cna_basic_effect_get_diffuse_color", "cna_basic_effect_get_emissive_color",
             "cna_basic_effect_get_specular_power", "cna_basic_effect_get_alpha",
             "cna_basic_effect_get_vertex_color_enabled",
             "cna_content_manager_load_sprite_font", "cna_content_manager_unload", "cna_content_manager_destroy",
             "cna_content_manager_set_root_directory", "cna_content_manager_get_root_directory_size",
             "cna_content_manager_copy_root_directory",
             "cna_error_get_last_info", "cna_error_get_last_exception_type_size_ext",
             "cna_error_copy_last_exception_type_ext",
             "cna_texture2d_get_info", "cna_texture2d_get_data", "cna_texture2d_destroy",
             "cna_sprite_font_measure_utf8", "cna_sprite_batch_draw_string", "cna_sprite_font_destroy",
             "cna_graphics_device_get_backbuffer_data_rgba8", "CNA_ContentManagerCreateInfo", "CNA_SpriteTextCommand"},
    .tags = {"c", "binding", "content", "font", "texture", "effect", "cnj"},
    .needs = CnaDemos::Need::CApi,
    .frames = 10,
    .deterministic = true,
    .assets = {"textures/checker.png", "textures/sprite_strip.png", "fonts/pixel_font.cnb",
               "effects/c_api_basic_effect.cnj"},
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
