// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.buffer_bindings",
    .title = "Graphics-device buffer bindings in C",
    .description = "A C game applies single and ordered multi-stream vertex-buffer bindings with "
                   "offsets and instance frequencies, copies them atomically, binds an index "
                   "buffer, and observes explicit and destruction-driven device cleanup.",
    .apis = {"cna_graphics_device_set_vertex_buffer",
             "cna_graphics_device_set_vertex_buffer_offset",
             "cna_graphics_device_set_vertex_buffers",
             "cna_graphics_device_get_vertex_buffer_count",
             "cna_graphics_device_copy_vertex_buffers",
             "cna_graphics_device_get_vertex_buffer",
             "cna_graphics_device_set_index_buffer",
             "cna_graphics_device_get_index_buffer", "CNA_VertexBufferBinding"},
    .tags = {"c", "binding", "graphics", "device", "vertex-buffer", "index-buffer", "lifecycle"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C program; buffer storage is allocated but no vertex/index data is "
                    "uploaded or drawn. Every device binding is cleared before callback return. "
                    "Built only with CNA_BUILD_C_API=ON.",
})
