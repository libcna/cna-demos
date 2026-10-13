// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.object_dictionary",
    .title = "Compiled object-dictionary values in C",
    .description = "A deterministic XNB dictionary exposes ordered keys, all public scalar value kinds, "
                   "a packed array, a string, an unknown diagnostic type and a C-created object.",
    .apis = {"cna_content_manager_load_object_dictionary_ext",
             "cna_object_dictionary_ext_destroy",
             "cna_object_dictionary_ext_get_runtime_type_name_size",
             "cna_object_dictionary_ext_copy_runtime_type_name",
             "cna_object_dictionary_ext_get_count", "cna_object_dictionary_ext_contains_key",
             "cna_object_dictionary_ext_get_key_size_at",
             "cna_object_dictionary_ext_copy_key_at", "cna_object_dictionary_ext_get_entry",
             "cna_object_dictionary_ext_get_type_name_size",
             "cna_object_dictionary_ext_copy_type_name", "cna_object_dictionary_ext_copy_value",
             "cna_object_dictionary_ext_copy_array", "cna_object_dictionary_ext_get_string_size",
             "cna_object_dictionary_ext_copy_string",
             "cna_object_dictionary_ext_get_foreign_object", "CNA_ObjectDictionaryEntry",
             "CNA_ObjectDictionaryValueKind"},
    .tags = {"c", "binding", "content", "xnb", "dictionary", "serialization", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Loads the committed hand-authored XNB fixture. It validates decoded values and "
                    "ownership without making a rendered-output or content-compiler claim.",
})
