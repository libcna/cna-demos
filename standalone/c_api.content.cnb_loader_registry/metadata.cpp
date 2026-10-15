// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_loader_registry",
    .title = "CNB loader registry and callbacks in C",
    .description = "A standalone C program creates the resource-backed ContentManager required by "
                   "a custom CNB loader, resolves and invokes copied loaders, and verifies "
                   "callback-scoped handle ownership.",
    .apis = {"cna_content_manager_create_resource",
             "cna_cnb_loader_registry_register", "cna_cnb_loader_registry_remove",
             "cna_cnb_loader_registry_clear", "cna_cnb_loader_registry_is_registered",
             "cna_cnb_loader_registry_find",
             "cna_cnb_loader_registry_get_registered_type_name_size",
             "cna_cnb_loader_registry_copy_registered_type_name",
             "cna_cnb_loader_registry_resolve_for_document",
             "cna_cnb_loader_registry_register_builtins", "cna_cnb_loader_destroy",
             "cna_cnb_loader_invoke"},
    .tags = {"c", "binding", "content", "cnb", "loader", "registry", "callback"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "The resource-backed manager supplies the loader callback but no embedded "
                    "resource is loaded. No asset runtime, pixels or drawing is used.",
})
