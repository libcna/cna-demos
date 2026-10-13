// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_byte_io",
    .title = "CNB bounded byte I/O in C",
    .description = "A renderer-free C program writes every primitive into deterministic little-endian bytes, "
                   "then reads them through a bounded copied cursor and exercises its refusal contracts.",
    .apis = {"cna_cnb_reader_create", "cna_cnb_reader_destroy",
             "cna_cnb_reader_get_remaining", "cna_cnb_reader_get_position",
             "cna_cnb_reader_get_size", "cna_cnb_reader_get_context_size",
             "cna_cnb_reader_copy_context", "cna_cnb_reader_read_u8",
             "cna_cnb_reader_read_u16", "cna_cnb_reader_read_u32",
             "cna_cnb_reader_read_u64", "cna_cnb_reader_read_i32",
             "cna_cnb_reader_read_f32", "cna_cnb_reader_read_f64",
             "cna_cnb_reader_read_string", "cna_cnb_reader_copy_string",
             "cna_cnb_reader_read_count", "cna_cnb_reader_read_bytes",
             "cna_cnb_reader_skip", "cna_cnb_reader_require_exhausted",
             "cna_cnb_reader_fail", "cna_cnb_is_well_formed_utf8",
             "cna_cnb_byte_writer_create", "cna_cnb_byte_writer_create_from_bytes",
             "cna_cnb_byte_writer_destroy", "cna_cnb_byte_writer_write_u8",
             "cna_cnb_byte_writer_write_u16", "cna_cnb_byte_writer_write_u32",
             "cna_cnb_byte_writer_write_u64", "cna_cnb_byte_writer_write_i32",
             "cna_cnb_byte_writer_write_f32", "cna_cnb_byte_writer_write_f64",
             "cna_cnb_byte_writer_write_string", "cna_cnb_byte_writer_write_bytes",
             "cna_cnb_byte_writer_write_zeros", "cna_cnb_byte_writer_get_size",
             "cna_cnb_byte_writer_copy_bytes", "cna_cnb_byte_writer_take"},
    .tags = {"c", "binding", "content", "cnb", "reader", "writer", "serialization"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Pure caller-owned memory work: no game, filesystem, content asset or renderer is used. "
                    "The demo validates byte representation and bounded cursor behavior, not a complete CNB document.",
})
