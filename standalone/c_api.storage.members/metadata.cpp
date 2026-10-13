// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.storage.members",
    .title = "Storage device, container and stream members in C",
    .description = "A storage-only C workflow covers selector variants, capacity, directory and file listings, "
                   "explicit sharing, stream resizing and disposal-event ownership.",
    .apis = {"cna_storage_set_app_name_ext", "cna_storage_get_root_size_ext",
             "cna_storage_copy_root_ext", "cna_storage_device_show_selector",
             "cna_storage_device_show_selector_for_player",
             "cna_storage_device_show_selector_with_space",
             "cna_storage_device_show_selector_for_player_with_space",
             "cna_storage_device_get_free_space", "cna_storage_device_get_total_space",
             "cna_storage_device_delete_container",
             "cna_storage_device_subscribe_device_changed",
             "cna_storage_device_unsubscribe_device_changed", "cna_storage_device_destroy",
             "cna_storage_container_open", "cna_storage_container_get_display_name_size",
             "cna_storage_container_copy_display_name", "cna_storage_container_get_is_disposed",
             "cna_storage_container_get_storage_device", "cna_storage_container_dispose",
             "cna_storage_container_subscribe_disposing",
             "cna_storage_container_unsubscribe_disposing",
             "cna_storage_container_create_directory", "cna_storage_container_directory_exists",
             "cna_storage_container_delete_directory",
             "cna_storage_container_get_directory_name_count",
             "cna_storage_container_copy_directory_name", "cna_storage_container_create_file",
             "cna_storage_container_get_file_name_count", "cna_storage_container_copy_file_name",
             "cna_storage_container_delete_file", "cna_storage_container_open_file_share",
             "cna_storage_container_destroy", "cna_storage_stream_read", "cna_storage_stream_write",
             "cna_storage_stream_get_length", "cna_storage_stream_set_length",
             "cna_storage_stream_flush", "cna_storage_stream_close", "CNA_StorageCompletionCallback"},
    .tags = {"c", "binding", "storage", "filesystem", "events", "stream", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 0,
    .deterministic = true,
    .restrictions = "Storage-only workflow using the process storage root. File-share enforcement is not "
                    "claimed because CNA currently records but ignores the requested share bits.",
})
