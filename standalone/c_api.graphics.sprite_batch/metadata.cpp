// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.sprite_batch",
    .title = "Sprites in C: a texture and a SpriteBatch",
    .description = "A C program builds a small Texture2D from RGBA pixels it computes, then draws a ring of tinted, "
                   "rotating sprites of it each frame with one cna_sprite_batch_submit_many call between "
                   "cna_sprite_batch_begin and cna_sprite_batch_end, and checks the result in the back buffer.",
    .apis = {"cna_game_get_graphics_device", "cna_texture2d_create", "cna_texture2d_set_data_rgba8",
             "cna_sprite_batch_create", "cna_sprite_batch_begin", "cna_sprite_batch_submit_many",
             "cna_sprite_batch_end", "cna_sprite_batch_destroy", "cna_texture2d_destroy",
             "cna_graphics_device_get_backbuffer_data_rgba8", "CNA_Texture2DCreateInfo", "CNA_SpriteCommand",
             "CNA_SpriteBatchBeginInfo"},
    .tags = {"c", "binding", "2d", "sprites", "texture"},
    .needs = CnaDemos::Need::CApi,
    .frames = 20,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
