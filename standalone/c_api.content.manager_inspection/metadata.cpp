// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.manager_inspection",
    .title = "Content paths, manifest and device association in C",
    .description = "A C-owned content manager resolves paths and cache keys, inspects its device and "
                   "walks the deployed asset manifest including native extensions and XNB reader usage.",
    .apis = {"cna_content_manager_create", "cna_content_manager_get_asset_path_size",
             "cna_content_manager_copy_asset_path", "cna_content_manager_get_normalized_key_size",
             "cna_content_manager_copy_normalized_key", "cna_content_manager_register_builtin_loaders",
             "cna_content_manager_get_has_service_provider",
             "cna_content_manager_get_graphics_device", "cna_content_manager_set_graphics_device",
             "cna_content_manager_refresh_content_manifest",
             "cna_content_manager_get_manifest_entry_count", "cna_content_manager_get_manifest_entry",
             "cna_content_manager_copy_manifest_relative_path",
             "cna_content_manager_copy_manifest_native_extension",
             "cna_content_manager_copy_manifest_xnb_reader_name",
             "cna_content_manager_get_xnb_reader_usage_count",
             "cna_content_manager_get_xnb_reader_usage",
             "cna_content_manager_copy_xnb_reader_usage_name", "cna_content_manager_destroy",
             "CNA_ContentManifestEntryInfo", "CNA_ContentReaderUsageInfo"},
    .tags = {"c", "binding", "content", "manifest", "path", "device", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone game workflow over the deployed assets tree. It inspects a point-in-time "
                    "manifest and makes no asset-loading or rendered-output claim.",
})
