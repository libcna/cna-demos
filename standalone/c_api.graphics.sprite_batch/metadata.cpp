// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.sprite_batch",
    .title = "Sprites in C: a texture and a SpriteBatch",
    .description = "A C program builds a small Texture2D from RGBA pixels it computes, then draws a ring of tinted, "
                   "rotating sprites of it each frame with one cna_sprite_batch_submit_many call between "
                   "cna_sprite_batch_begin and cna_sprite_batch_end. A final scaled array proves position, source, origin and "
                   "nonuniform-scale geometry. Explicit-state and transformed default-effect Begin routes add pixel-proven "
                   "opaque blending and translated geometry. Checks the back buffer, active-batch texture retention, "
                   "cancellation without flushing, successful End release and whole-array refusal for both command shapes.",
    .apis = {"cna_game_get_graphics_device", "cna_texture2d_create", "cna_texture2d_set_data_rgba8",
             "cna_sprite_batch_create", "cna_sprite_batch_begin", "cna_sprite_batch_submit_many",
             "cna_sprite_batch_submit_scaled_many",
             "cna_sprite_batch_begin_with_states", "cna_sprite_batch_begin_with_effect",
             "cna_sprite_batch_end", "cna_sprite_batch_destroy", "cna_texture2d_destroy",
             "cna_blend_state_init", "cna_sampler_state_init", "cna_depth_stencil_state_init",
             "cna_rasterizer_state_init", "cna_matrix_create_translation_xyz",
             "cna_graphics_device_get_backbuffer_data_rgba8", "CNA_Texture2DCreateInfo", "CNA_SpriteCommand",
             "CNA_SpriteScaledCommand", "CNA_SpriteBatchBeginInfo"},
    .tags = {"c", "binding", "2d", "sprites", "texture"},
    .needs = CnaDemos::Need::CApi,
    .frames = 20,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
