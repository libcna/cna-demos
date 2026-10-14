// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_texture_data",
    .title = "CNB texture values in C",
    .description = "A renderer-free C program audits frozen texture-format identities, exact storage sizes, "
                   "runtime mappings and caller-owned multi-representation texture data.",
    .apis = {"cna_cnb_is_known_texture_format", "cna_cnb_get_texture_format_name_size",
             "cna_cnb_copy_texture_format_name", "cna_cnb_is_block_compressed_texture_format",
             "cna_cnb_get_texture_format_unit_bytes", "cna_cnb_get_texture_level_byte_size",
             "cna_cnb_texture_format_to_surface_format",
             "cna_cnb_texture_format_from_surface_format", "cna_cnb_texture_data_create",
             "cna_cnb_texture_data_create_rgba8", "cna_cnb_texture_data_destroy",
             "cna_cnb_texture_data_get_info", "cna_cnb_texture_data_get_level_dimensions",
             "cna_cnb_texture_data_add_representation",
             "cna_cnb_texture_data_get_representation_count",
             "cna_cnb_texture_data_get_representation_format",
             "cna_cnb_texture_data_get_level_count", "cna_cnb_texture_data_set_level",
             "cna_cnb_texture_data_copy_level",
             "cna_cnb_texture_data_select_representation"},
    .tags = {"c", "binding", "content", "cnb", "texture", "format", "mipmaps"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Pure value and caller-owned byte work: no CNB file, graphics device, GPU texture, "
                    "renderer or pixel display is used.",
})
