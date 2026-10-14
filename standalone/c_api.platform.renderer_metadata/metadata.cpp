// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.platform.renderer_metadata",
    .title = "Renderer metadata in C",
    .description = "A renderer-independent C program classifies every public renderer by backend "
                   "technology and maturity, copies the stable enum names and correlates the "
                   "compiled-in renderer identity, name and classifications.",
    .apis = {"cna_graphics_backend_get_category", "cna_graphics_backend_get_current_category",
             "cna_graphics_backend_category_get_name_size",
             "cna_graphics_backend_category_copy_name", "cna_graphics_backend_get_maturity",
             "cna_graphics_backend_get_current_maturity",
             "cna_graphics_backend_maturity_get_name_size",
             "cna_graphics_backend_maturity_copy_name", "cna_graphics_renderer_get_current_type",
             "cna_graphics_renderer_get_current_name_size",
             "cna_graphics_renderer_copy_current_name"},
    .tags = {"c", "binding", "platform", "renderer", "backend", "metadata"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone compile-time and renderer-independent metadata workflow. It creates no game, "
                    "window or graphics device and makes no availability or capability claim.",
})
