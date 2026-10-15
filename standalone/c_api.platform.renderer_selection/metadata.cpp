// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.platform.renderer_selection",
    .title = "Renderer selection and fallback in C",
    .description = "A standalone C game inspects compiled renderers, selects by name and identity, "
                   "then requests an absent public renderer with an explicit fallback and inspects "
                   "the active renderer, latch and diagnostic history. Its refusal paths retain exact "
                   "ArgumentException(name) and InvalidOperationException details.",
    .apis = {"cna_graphics_renderer_set_preferred_ext",
             "cna_graphics_renderer_set_preferred_by_name_ext",
             "cna_graphics_renderer_get_selected_ext", "cna_graphics_renderer_get_active_ext",
             "cna_graphics_renderer_get_is_latched_ext",
             "cna_graphics_renderer_get_available_count_ext",
             "cna_graphics_renderer_copy_available_ext",
             "cna_graphics_renderer_get_is_available_ext",
             "cna_graphics_renderer_set_fallback_chain_ext",
             "cna_graphics_renderer_set_automatic_fallback_ext",
             "cna_graphics_renderer_get_automatic_fallback_ext",
             "cna_graphics_renderer_get_fallback_count_ext",
             "cna_graphics_renderer_get_fallback_at_ext",
             "cna_graphics_renderer_fallback_get_message_size_ext",
             "cna_graphics_renderer_fallback_copy_message_ext",
             "cna_graphics_renderer_fallback_reason_get_name_size_ext",
             "cna_graphics_renderer_fallback_reason_copy_name_ext",
             "cna_graphics_renderer_try_parse_name_ext", "cna_error_get_last_info",
             "cna_error_get_last_exception_type_size_ext", "cna_error_copy_last_exception_type_ext",
             "cna_error_get_last_exception_param_name_size_ext",
             "cna_error_copy_last_exception_param_name_ext"},
    .tags = {"c", "binding", "platform", "renderer", "selection", "fallback", "lifecycle"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone process-global renderer workflow because selection latches at the first "
                    "GraphicsDevice. It needs at least one compiled and one uncompiled public renderer; "
                    "the linux-full configuration provides both. No pixels or visual quality are checked.",
})
