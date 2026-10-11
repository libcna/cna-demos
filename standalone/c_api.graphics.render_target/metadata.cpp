// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.render_target",
    .title = "Render to texture in C",
    .description = "A C program draws four squares into a 128x128 render target (its own clear colour), reads the "
                   "target back with cna_texture2d_get_data_rgba8, restores the back buffer and draws the target as "
                   "a sprite texture three times larger.",
    .apis = {"cna_render_target2d_create", "CNA_RenderTarget2DCreateInfo", "cna_graphics_device_set_render_target2d",
             "cna_texture2d_get_data_rgba8", "cna_render_target_destroy", "cna_game_clear", "cna_sprite_batch_begin",
             "cna_sprite_batch_submit_many", "cna_sprite_batch_end"},
    .tags = {"c", "binding", "render-target", "texture"},
    .needs = CnaDemos::Need::CApi,
    .frames = 5,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
