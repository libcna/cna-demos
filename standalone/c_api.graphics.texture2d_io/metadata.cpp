// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.texture2d_io",
    .title = "Texture2D data and image I/O in C",
    .description = "A C program contrasts empty and file-backed standalone textures, performs atomic typed "
                   "rectangle transfers and PNG/JPEG output, reloads a saved PNG, then creates, uploads and "
                   "decodes renderer-backed textures on a live graphics device.",
    .apis = {"cna_texture_get_info", "cna_texture2d_create_standalone",
             "cna_texture2d_create_from_file", "cna_texture2d_create_from_file_with_device",
             "cna_texture2d_create_from_rgba8", "cna_texture2d_set_data_rgba8_bytes",
             "cna_texture2d_get_storage_info", "cna_texture2d_set_data", "cna_texture2d_get_data",
             "cna_texture2d_create_from_encoded_memory", "cna_texture2d_get_encoded_byte_count",
             "cna_texture2d_copy_encoded", "cna_texture2d_save_file", "cna_texture2d_get_info",
             "cna_texture2d_destroy", "cna_game_get_graphics_device", "CNA_Texture2DTransfer",
             "CNA_Texture2DDecodeInfo", "CNA_Texture2DStorageInfo"},
    .tags = {"c", "binding", "graphics", "texture", "data", "png", "jpeg", "transfer"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .assets = {"textures/checker.png"},
    .restrictions = "Standalone C program; creates and removes one private PNG under /tmp. Exact data and image "
                    "signatures are checked programmatically under both renderer selections. Nothing is drawn and "
                    "no screenshot or visual inspection is claimed. Built only when CNA_BUILD_C_API=ON.",
})
