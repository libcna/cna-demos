// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_texture_codecs",
    .title = "CNB texture codecs in C",
    .description = "A renderer-free C program round-trips 2D, cube, volume and embedded texture data "
                   "through complete CNB documents while enforcing shape and schema boundaries.",
    .apis = {"cna_cnb_encode_texture2d", "cna_cnb_encode_texture_cube",
             "cna_cnb_encode_texture3d", "cna_cnb_decode_texture2d",
             "cna_cnb_decode_texture_cube", "cna_cnb_decode_texture3d",
             "cna_cnb_writer_append_embedded_texture2d",
             "cna_cnb_document_read_embedded_texture2d"},
    .tags = {"c", "binding", "content", "cnb", "texture", "codec", "mipmaps"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses synthetic Rgba8 bytes and parsed in-memory CNB documents. No graphics device, "
                    "GPU texture, renderer, filesystem or displayed pixels are used.",
})
