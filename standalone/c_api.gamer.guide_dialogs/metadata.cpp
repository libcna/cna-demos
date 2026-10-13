// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.guide_dialogs",
    .title = "Headless Guide dialogs in C",
    .description = "A renderer-free C program inspects, cancels and resets pending password input, then validates "
                   "message-box focus, simulated selection, repeatable answers and callback timing.",
    .apis = {"cna_guide_begin_show_keyboard_input", "cna_guide_end_show_keyboard_input_size",
             "cna_guide_end_show_keyboard_input", "cna_guide_get_has_pending_keyboard_input_ext",
             "cna_guide_was_keyboard_input_canceled_ext",
             "cna_guide_get_pending_keyboard_input_title_size_ext",
             "cna_guide_copy_pending_keyboard_input_title_ext",
             "cna_guide_get_pending_keyboard_input_description_size_ext",
             "cna_guide_copy_pending_keyboard_input_description_ext",
             "cna_guide_get_pending_keyboard_input_display_text_size_ext",
             "cna_guide_copy_pending_keyboard_input_display_text_ext",
             "cna_guide_simulate_keyboard_input_cancel_ext",
             "cna_guide_reset_pending_keyboard_input_ext",
             "cna_guide_begin_show_message_box", "cna_guide_end_show_message_box",
             "cna_guide_get_has_pending_message_box_ext",
             "cna_guide_get_pending_message_box_focus_button_ext",
             "cna_guide_simulate_message_box_click_ext",
             "cna_guide_reset_pending_message_box_ext"},
    .tags = {"c", "binding", "gamer", "guide", "keyboard", "message-box", "async"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses the explicit headless simulation/reset extensions and opens no game, renderer or visible "
                    "Guide. Rendering, real keyboard/controller/mouse input and appearance are separate.",
})
