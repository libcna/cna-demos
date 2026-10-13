// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.devices.system_tray",
    .title = "Device services in C: system tray model",
    .description = "A C game reports real tray availability without creating a host icon, then uses CNA's public "
                   "in-memory tray to verify owned lifetime, menu indices, checked/enabled state, ignored upper "
                   "bounds, click callbacks and stale-handle refusal.",
    .apis = {"cna_system_tray_get_is_supported_ext",
             "cna_system_tray_create_with_test_backend_ext", "cna_system_tray_set_tooltip",
             "cna_system_tray_add_entry", "cna_system_tray_set_entry_label",
             "cna_system_tray_set_entry_checked", "cna_system_tray_get_entry_checked",
             "cna_system_tray_set_entry_enabled", "cna_system_tray_get_entry_enabled",
             "cna_system_tray_click_entry_for_tests_ext", "cna_system_tray_destroy"},
    .tags = {"c", "binding", "devices", "system-tray", "menu", "callbacks", "lifecycle", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = false,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API and device "
                    "extension (CNA_BUILD_C_API=ON, CNA_BUILD_DEVICES=ON). Real support is queried, but no host "
                    "tray icon is created; all menu behavior uses CNA's per-handle in-memory test backend.",
})
