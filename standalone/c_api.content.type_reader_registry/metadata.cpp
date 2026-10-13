// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.type_reader_registry",
    .title = "Content type-reader registry and callbacks in C",
    .description = "A process-local C type-reader registry exercises placeholder behavior, caller-owned "
                   "factories, callback-scoped reader borrowing and registration lifetime.",
    .apis = {"cna_content_type_reader_manager_clear_type_creators",
             "cna_content_type_reader_manager_get_is_registered",
             "cna_content_type_reader_manager_create_reader",
             "cna_content_type_reader_manager_register",
             "cna_content_type_reader_manager_unregister",
             "cna_content_register_known_unsupported_xnb_readers",
             "cna_known_unsupported_content_type_reader_create",
             "cna_content_type_reader_get_can_deserialize_into_existing_object",
             "cna_content_type_reader_get_target_type_name_size",
             "cna_content_type_reader_copy_target_type_name",
             "cna_content_type_reader_get_type_version", "cna_content_type_reader_supports_version",
             "cna_content_type_reader_initialize", "cna_content_type_reader_read_untyped",
             "cna_content_type_reader_destroy", "CNA_ContentTypeReaderCallbacks",
             "CNA_ContentTypeReaderCreateCallback", "CNA_ContentTypeReaderReadCallback",
             "CNA_ContentTypeReaderDestroyCallback",
             "CNA_UNSUPPORTED_CONTENT_READER_REASON_COMPILED_PLATFORM_SHADER_BYTECODE"},
    .tags = {"c", "binding", "content", "reader", "registry", "callbacks", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 0,
    .deterministic = true,
    .restrictions = "Own-process registry workflow over a synthetic empty storage stream. Built-in "
                    "reader decoding and compiled content production are outside this demo.",
})
