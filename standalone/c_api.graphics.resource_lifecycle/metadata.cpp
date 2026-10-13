// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.resource_lifecycle",
    .title = "Graphics resource lifecycle in C",
    .description = "A C game treats a live Texture2D through CNA's generic GraphicsResource surface: it "
                   "round-trips a UTF-8 name and opaque tag, copies the default and named string forms, "
                   "observes synchronous one-shot disposal and finally releases the independent C handle.",
    .apis = {"cna_graphics_resource_get_graphics_device",
             "cna_graphics_resource_get_is_disposed",
             "cna_graphics_resource_get_name_byte_count", "cna_graphics_resource_copy_name",
             "cna_graphics_resource_set_name", "cna_graphics_resource_get_string_byte_count",
             "cna_graphics_resource_copy_string", "cna_graphics_resource_get_tag",
             "cna_graphics_resource_set_tag", "cna_graphics_resource_dispose",
             "cna_graphics_resource_subscribe_disposing",
             "cna_graphics_resource_unsubscribe_disposing", "CNA_GraphicsResourceTag",
             "CNA_GraphicsResourceEventRegistrationHandle"},
    .tags = {"c", "binding", "graphics", "resource", "lifecycle", "events", "strings"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C program; the Texture2D is explicitly disposed and its separate C handle is "
                    "then destroyed. No rendering or visual result is claimed. Built only with "
                    "CNA_BUILD_C_API=ON.",
})
