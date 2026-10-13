// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.reflective_reader",
    .title = "Reflective content-reader builders in C",
    .description = "C-declared reflective readers map every inline field kind by byte offset, register "
                   "enum and custom members and demonstrate value-shaped and shared registrations.",
    .apis = {"cna_reflective_type_reader_builder_create",
             "cna_reflective_type_reader_builder_destroy",
             "cna_reflective_type_reader_builder_add_field",
             "cna_reflective_type_reader_builder_add_enum_field",
             "cna_reflective_type_reader_builder_add_custom",
             "cna_reflective_type_reader_builder_register",
             "cna_reflective_type_reader_get_canonical_name_size",
             "cna_reflective_type_reader_copy_canonical_name",
             "cna_enum_type_reader_get_canonical_name_size",
             "cna_enum_type_reader_copy_canonical_name",
             "cna_reflective_type_reader_builder_register_shared", "CNA_ContentFieldKind",
             "CNA_ReflectiveObjectCreateCallback", "CNA_ReflectiveFieldCallback"},
    .tags = {"c", "binding", "content", "reader", "reflection", "serialization", "callbacks"},
    .needs = CnaDemos::Need::CApi,
    .frames = 0,
    .deterministic = true,
    .restrictions = "Storage-only synthetic payloads invoke registered readers directly. They prove the "
                    "reader shapes and field mapping without claiming a content-pipeline-produced XNB.",
})
