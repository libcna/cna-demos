// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.text_input",
    .title = "Input in C: text and IME events",
    .description = "A C program raises committed UTF-16, UTF-8 composition and candidate events, verifies their "
                   "synchronous copied payloads and registration lifecycle, then exercises bound and unbound text "
                   "activation, input hints, screen-keyboard queries and reset.",
    .apis = {"cna_text_input_subscribe_text_input_ext", "cna_text_input_subscribe_text_editing_ext",
             "cna_text_input_subscribe_text_editing_candidates_ext", "cna_text_input_unsubscribe_ext",
             "cna_text_input_raise_text_input_ext", "cna_text_input_raise_text_editing_ext",
             "cna_text_input_raise_text_editing_candidates_ext", "cna_text_input_get_window_handle_ext",
             "cna_text_input_set_window_handle_ext", "cna_text_input_is_active_ext",
             "cna_text_input_is_screen_keyboard_shown_ext",
             "cna_text_input_is_screen_keyboard_shown_for_window_ext", "cna_text_input_start_ext",
             "cna_text_input_stop_ext", "cna_text_input_start_with_type_ext",
             "cna_text_input_set_input_rectangle_ext", "cna_text_input_reset_for_tests_ext"},
    .tags = {"c", "binding", "input", "text", "ime", "events", "lifecycle", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). Public raise routes provide deterministic event data; no physical "
                    "typing, platform IME interaction or visible on-screen keyboard is claimed.",
})
