// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.devices.dialogs",
    .title = "Device services in C: dialog contracts",
    .description = "A C game reports real message-box and file-dialog availability without opening UI, then uses "
                   "CNA's public immediate test service to verify severity, choices, versioned filters, borrowed "
                   "callback results, cancellation, invalid requests and backend restoration.",
    .apis = {"cna_message_box_get_is_supported_ext", "cna_message_box_show_simple_ext",
             "cna_message_box_show_ext", "cna_message_box_set_test_backend_ext",
             "cna_message_box_get_test_log_ext", "cna_file_dialog_get_is_supported_ext",
             "cna_file_dialog_show_open_file_ext", "cna_file_dialog_show_save_file_ext",
             "cna_file_dialog_show_open_folder_ext", "cna_file_dialog_set_test_backend_ext"},
    .tags = {"c", "binding", "devices", "dialogs", "message-box", "file-dialog", "callbacks", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = false,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API and device "
                    "extension (CNA_BUILD_C_API=ON, CNA_BUILD_DEVICES=ON). Real support is queried, but every "
                    "dialog request uses CNA's in-memory test service; no window is opened or manually answered.",
})
