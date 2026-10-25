// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.clipboard",
    .title = "Input in C: clipboard text",
    .description = "A C program performs read-only has-text and race-tolerant exact size/copy inspection of the "
                   "desktop clipboard. Mutating C++ clipboard demonstrations require explicit user activation.",
    .apis = {"cna_clipboard_get_text_size", "cna_clipboard_copy_text", "cna_clipboard_get_has_text"},
    .tags = {"c", "binding", "input", "clipboard", "text", "utf-8", "ownership", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API. It never "
                    "writes the host clipboard; another process can still race size/copy, so reads retry safely.",
})
