// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.clipboard",
    .title = "Input in C: clipboard text",
    .description = "A C program preserves the desktop clipboard, requests an explicit UTF-8 value with the "
                   "device extension's acceptance result, checks has-text and exact size/copy answers including "
                   "atomic short-buffer refusal, exercises the empty value, then restores the original bytes.",
    .apis = {"cna_clipboard_get_text_size", "cna_clipboard_copy_text", "cna_clipboard_set_text",
             "cna_clipboard_get_has_text", "cna_devices_clipboard_set_text_ext"},
    .tags = {"c", "binding", "input", "clipboard", "text", "utf-8", "ownership", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). The clipboard belongs to the desktop: the demo restores the captured "
                    "text, but another process can still race any count/copy or replace it concurrently.",
})
