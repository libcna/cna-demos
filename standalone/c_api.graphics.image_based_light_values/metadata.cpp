// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.image_based_light_values",
    .title = "Image-based light values in C",
    .description = "A deterministic C game queries graphics-extension availability, initializes an image-based "
                   "light, validates its versioned shape and completeness with real cube/2D textures, then "
                   "distinguishes stale and wrong-kind resource handles.",
    .apis = {"cna_graphics_ext_is_available", "cna_image_based_light_ext_init",
             "cna_image_based_light_ext_is_valid", "CNA_ImageBasedLightEXT"},
    .tags = {"c", "binding", "graphics", "extension", "pbr", "ibl", "texture", "values"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C game. Validity and handle kinds are checked with real resources; no PBR effect, "
                    "draw, environment-lighting result, screenshot or visual inspection is claimed. The wrong-kind "
                    "handle check covers CNA-DEMOS-ISSUE-091. Built only with CNA_BUILD_C_API=ON.",
})
