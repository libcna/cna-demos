// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_foundations",
    .title = "CNB container foundations in C",
    .description = "A renderer-free C program validates CNB wire identities, safe arithmetic, read limits, "
                   "CRC-32C and available chunk-compression round trips.",
    .apis = {"cna_cnb_read_limits_init", "cna_cnb_copy_format_magic",
             "cna_cnb_make_chunk_id", "cna_cnb_get_chunk_id_string_size",
             "cna_cnb_copy_chunk_id_string", "cna_cnb_is_well_formed_chunk_id",
             "cna_cnb_asset_type_id_from_name", "cna_cnb_is_custom_asset_type_id",
             "cna_cnb_get_asset_type_name_size", "cna_cnb_copy_asset_type_name",
             "cna_cnb_get_logical_name_problem_size",
             "cna_cnb_copy_logical_name_problem", "cna_cnb_checked_add",
             "cna_cnb_checked_multiply", "cna_cnb_crc32c", "cna_cnb_crc32c_continue",
             "cna_cnb_crc32c_uses_hardware", "cna_cnb_crc32c_portable",
             "cna_cnb_is_compression_supported", "cna_cnb_get_compression_name_size",
             "cna_cnb_copy_compression_name", "cna_cnb_get_compressed_byte_count",
             "cna_cnb_copy_compressed", "cna_cnb_copy_decompressed",
             "cna_cnb_chunk_entry_is_mandatory", "cna_cnb_has_magic"},
    .tags = {"c", "binding", "content", "cnb", "format", "crc32c", "compression"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Pure caller-owned byte/value work: no game, filesystem, content asset or renderer is used. "
                    "Zstandard is round-tripped when the linked CNA build provides it; otherwise the documented "
                    "NOT_SUPPORTED result is validated.",
})
