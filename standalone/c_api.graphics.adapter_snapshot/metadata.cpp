// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.adapter_snapshot",
    .title = "Graphics adapter snapshot in C",
    .description = "A C program uses a game handle before its first frame to enumerate the system adapters, copy "
                   "the default adapter's exact UTF-8 metadata, inspect its current and supported display modes, "
                   "filter them by Color and verify atomic buffer and upper-index refusal.",
    .apis = {"cna_graphics_adapter_get_count", "cna_graphics_adapter_get_info",
             "cna_graphics_adapter_copy_description", "cna_graphics_adapter_copy_device_name",
             "cna_graphics_adapter_get_current_display_mode", "cna_graphics_adapter_get_display_mode_count",
             "cna_graphics_adapter_copy_display_modes", "CNA_GraphicsAdapterInfo", "CNA_DisplayMode"},
    .tags = {"c", "binding", "graphics", "adapter", "display", "metadata", "buffers"},
    .needs = CnaDemos::Need::CApi,
    .restrictions = "Standalone C program; host adapter names and modes vary. Built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
