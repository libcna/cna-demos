// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.display_values",
    .title = "Display values in C",
    .description = "A deterministic C program constructs DisplayMode values, checks their aspect ratio and "
                   "defining-field equality, then initializes, edits, native-clones and derives bounds from a "
                   "versioned PresentationParameters value without creating a game or graphics device.",
    .apis = {"cna_display_mode_init", "cna_display_mode_equals", "cna_presentation_parameters_init",
             "cna_presentation_parameters_clone", "cna_presentation_parameters_get_bounds", "CNA_DisplayMode",
             "CNA_PresentationParameters"},
    .tags = {"c", "binding", "graphics", "display", "presentation", "values"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program; built only when CNA has its C API (CNA_BUILD_C_API=ON).",
})
