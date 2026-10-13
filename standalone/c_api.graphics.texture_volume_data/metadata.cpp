// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.texture_volume_data",
    .title = "Texture3D and TextureCube data in C",
    .description = "A C game inspects and transfers exact Color payloads through a 2x2x2 Texture3D and all six "
                   "faces of a TextureCube, checks atomic short reads, uploads raw volume bytes and decodes a "
                   "minimal in-memory DXT1 cube DDS.",
    .apis = {"cna_texture3d_create", "cna_texture3d_get_info", "cna_texture3d_set_data",
             "cna_texture3d_get_data", "cna_texture3d_set_data_bytes", "cna_texture3d_destroy",
             "cna_texturecube_create", "cna_texturecube_create_from_dds_memory",
             "cna_texturecube_get_info", "cna_texturecube_set_data", "cna_texturecube_get_data",
             "cna_texturecube_destroy", "cna_texture_get_info", "CNA_Texture3DTransfer",
             "CNA_TextureCubeTransfer"},
    .tags = {"c", "binding", "graphics", "texture", "volume", "cube", "dds", "transfer"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C game. Texture3D and cube DDS paths preserve documented NOT_SUPPORTED outcomes "
                    "on renderers without the required storage. Exact CPU readback is checked where storage exists; "
                    "nothing is sampled or drawn and no screenshot or visual inspection is claimed. Built only "
                    "when CNA_BUILD_C_API=ON.",
})
