// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.net.packet_buffers",
    .title = "Network packet buffers in C",
    .description = "A renderer-free C program writes every public packet value, copies the exact bytes into a "
                   "reader, reads every value back, seeks, overwrites and verifies atomic buffer failures.",
    .apis = {"cna_packet_writer_create", "cna_packet_writer_get_length",
             "cna_packet_writer_get_position", "cna_packet_writer_set_position",
             "cna_packet_writer_write_color", "cna_packet_writer_write_matrix",
             "cna_packet_writer_write_quaternion", "cna_packet_writer_write_vector2",
             "cna_packet_writer_write_vector3", "cna_packet_writer_write_vector4",
             "cna_packet_writer_write_single", "cna_packet_writer_write_double",
             "cna_packet_writer_copy_data_ext", "cna_packet_writer_destroy",
             "cna_packet_reader_create", "cna_packet_reader_set_data_ext",
             "cna_packet_reader_copy_data_ext", "cna_packet_reader_get_length",
             "cna_packet_reader_get_position", "cna_packet_reader_set_position",
             "cna_packet_reader_read_color", "cna_packet_reader_read_matrix",
             "cna_packet_reader_read_quaternion", "cna_packet_reader_read_vector2",
             "cna_packet_reader_read_vector3", "cna_packet_reader_read_vector4",
             "cna_packet_reader_read_single", "cna_packet_reader_read_double",
             "cna_packet_reader_destroy"},
    .tags = {"c", "binding", "net", "packet", "serialization", "buffer", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Pure in-memory PacketWriter/PacketReader behavior; creates no game, session, socket or "
                    "renderer and does not claim wire transport, peer interoperability or network byte order.",
})
