// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.reader_values",
    .title = "Compiled content reader values and limits in C",
    .description = "A storage-backed C content reader exposes its identity, advances the XNB object "
                   "protocol, reads canonical value types and enforces exact-byte and size limits.",
    .apis = {"cna_content_reader_create", "cna_content_reader_get_content_manager",
             "cna_content_reader_get_asset_name_size", "cna_content_reader_copy_asset_name",
             "cna_content_reader_get_version", "cna_content_reader_get_platform",
             "cna_content_reader_initialize_type_readers",
             "cna_content_reader_read_shared_resources", "cna_content_reader_read_object_tag",
             "cna_content_reader_read_vector2", "cna_content_reader_read_vector3",
             "cna_content_reader_read_vector4", "cna_content_reader_read_quaternion",
             "cna_content_reader_read_matrix", "cna_content_reader_read_color",
             "cna_content_reader_read_bounding_sphere",
             "cna_content_reader_check_collection_element_count",
             "cna_content_reader_check_decoded_byte_size", "cna_content_reader_read_bytes_exact",
             "cna_content_reader_destroy", "CNA_ContentReaderCreateInfo"},
    .tags = {"c", "binding", "content", "reader", "xnb", "serialization", "storage", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 0,
    .deterministic = true,
    .restrictions = "Storage-only synthetic reader stream. It demonstrates the public reader protocol "
                    "and canonical value layout, not content-pipeline production or asset rendering.",
})
