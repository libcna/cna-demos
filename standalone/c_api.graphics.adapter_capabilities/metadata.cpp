// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.adapter_capabilities",
    .title = "Graphics adapter capabilities in C",
    .description = "A C program queries the default adapter's Reach support and exact render-target and "
                   "backbuffer formats, changes and exactly restores both device-preference flags, then "
                   "demonstrates the stable ABI's explicit native-monitor and adapter-refresh limitations.",
    .apis = {"cna_graphics_adapter_set_device_preferences",
             "cna_graphics_adapter_is_profile_supported",
             "cna_graphics_adapter_query_render_target_format",
             "cna_graphics_adapter_query_backbuffer_format",
             "cna_graphics_adapter_get_native_monitor_handle", "cna_graphics_adapters_refresh",
             "CNA_GraphicsFormatSelection"},
    .tags = {"c", "binding", "graphics", "adapter", "capabilities", "formats", "limitations"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C program; adapter capabilities vary by host. The preference change is "
                    "restored before device creation, native monitor handles are intentionally unavailable, "
                    "and no rendering or visual result is claimed. Built only with CNA_BUILD_C_API=ON.",
})
