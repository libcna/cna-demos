// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.retro_effects",
    .title = "Retro graphics effects in C",
    .description = "A C game exercises CRT and color-depth effect state where shader effects are supported, then "
                   "runs the renderer-independent ASCII post-process over an authored texture and verifies its "
                   "resulting glyph-grid dimensions and lifetime.",
    .apis = {"cna_crt_effect_create",
             "cna_crt_effect_get_scanline_intensity", "cna_crt_effect_set_scanline_intensity",
             "cna_crt_effect_get_curvature", "cna_crt_effect_set_curvature",
             "cna_crt_effect_get_vignette_intensity", "cna_crt_effect_set_vignette_intensity",
             "cna_crt_effect_get_mask_intensity", "cna_crt_effect_set_mask_intensity",
             "cna_crt_effect_get_mask_type", "cna_crt_effect_set_mask_type",
             "cna_depth_effect_create", "cna_depth_effect_get_mode", "cna_depth_effect_set_mode",
             "cna_depth_effect_get_dither_mode", "cna_depth_effect_set_dither_mode",
             "cna_ascii_post_process_effect_create",
             "cna_ascii_post_process_effect_get_cell_size",
             "cna_ascii_post_process_effect_set_cell_size",
             "cna_ascii_post_process_effect_get_quantize_mode",
             "cna_ascii_post_process_effect_set_quantize_mode",
             "cna_ascii_post_process_effect_draw",
             "cna_ascii_post_process_effect_get_last_grid_dimensions",
             "cna_ascii_post_process_effect_destroy"},
    .tags = {"c", "binding", "graphics", "extension", "crt", "depth", "dither", "ascii", "post-process"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C game. CRT/Depth state accepts an explicit NOT_SUPPORTED result on renderers without "
                    "custom shaders. ASCII executes a real texture readback/quantize/draw and checks grid state, but "
                    "no screenshot or manual visual-quality inspection is claimed. Built only with CNA_BUILD_C_API=ON "
                    "and CNA_CNAEXT=ON.",
})
