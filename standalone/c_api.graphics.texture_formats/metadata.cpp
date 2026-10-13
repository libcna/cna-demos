// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.texture_formats",
    .title = "Texture format rules in C",
    .description = "A deterministic C program audits every SurfaceFormat's compression footprint, storage-unit "
                   "size, pixel-store alignment, compatible transfer widths and base Texture support contract.",
    .apis = {"cna_texture_get_block_size_squared", "cna_texture_get_format_size",
             "cna_texture_get_pixel_store_alignment", "cna_texture_validate_get_data_format",
             "cna_texture_validate_format", "CNA_SurfaceFormat"},
    .tags = {"c", "binding", "graphics", "texture", "format", "transfer"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program; validates renderer-independent format policy without creating a "
                    "graphics device, allocating a texture or claiming backend format support. Built only when "
                    "CNA_BUILD_C_API=ON.",
})
