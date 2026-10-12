// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.storage.save_game",
    .title = "Save games in C",
    .description = "A C program, without a game object, selects a storage device, opens a container, writes a save "
                   "file as bytes, reads it back byte for byte and deletes it; XNA's Begin/End pairs are one "
                   "synchronous call each, their completion callback invoked before the call returns. Also checks parent "
                   "release guards, short EOF reads with untouched output padding, rewind, closed-handle expiry and "
                   "explicit read-only access that preserves the file after refused truncation and writing.",
    .apis = {"cna_storage_set_app_name_ext", "cna_storage_copy_root_ext", "cna_storage_device_show_selector",
             "cna_storage_device_get_is_connected", "cna_storage_container_open", "cna_storage_container_create_file",
             "cna_storage_stream_write", "cna_storage_stream_close", "cna_storage_container_file_exists",
             "cna_storage_container_get_file_name_count", "cna_storage_container_open_file",
             "cna_storage_container_open_file_access", "cna_storage_stream_get_can_read",
             "cna_storage_stream_get_can_write", "cna_storage_stream_get_can_seek",
             "cna_storage_stream_get_length", "cna_storage_stream_read", "cna_storage_stream_seek",
             "cna_storage_stream_get_position", "cna_storage_container_delete_file",
             "cna_storage_container_destroy", "cna_storage_device_destroy"},
    .tags = {"c", "binding", "storage", "save-game"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program; built only when CNA has its C API (CNA_BUILD_C_API=ON). "
                    "ISSUE-056: the underlying FileStream reports position -1 after a short EOF read.",
    .knownIssue = "CNA-DEMOS-ISSUE-056",
})
