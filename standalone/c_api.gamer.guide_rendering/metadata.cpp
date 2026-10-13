// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.guide_rendering",
    .title = "Visible Guide dialogs in C",
    .description = "A C game renders a pending password keyboard and a three-button message box through the two "
                   "Guide drawing extensions, then proves both overlays changed the real back buffer.",
    .apis = {"cna_guide_render_pending_keyboard_input_ext",
             "cna_guide_render_pending_message_box_ext", "cna_guide_begin_show_keyboard_input",
             "cna_guide_simulate_keyboard_input_cancel_ext", "cna_guide_reset_pending_keyboard_input_ext",
             "cna_guide_begin_show_message_box", "cna_guide_reset_pending_message_box_ext",
             "cna_game_get_graphics_device", "cna_content_manager_create",
             "cna_content_manager_load_sprite_font", "cna_texture2d_create",
             "cna_texture2d_set_data_rgba8", "cna_sprite_batch_create", "cna_sprite_batch_begin",
             "cna_sprite_batch_end", "cna_graphics_device_get_backbuffer_data_rgba8"},
    .tags = {"c", "binding", "gamer", "guide", "keyboard", "message-box", "rendering"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Window,
    .frames = 2,
    .deterministic = true,
    .restrictions = "Runs a real game and checks rendered pixels. Automated execution cancels the keyboard and "
                    "resets the message box; it does not claim real keyboard, mouse or controller input. Use "
                    "--frames 1 to capture the keyboard or --frames 2 to capture the message box.",
})
