// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.vertex_buffer_data",
    .title = "Vertex declarations and buffer data in C",
    .description = "A one-frame C game builds copied vertex declarations, inspects static and dynamic "
                   "buffer metadata, transfers typed and raw vertex windows, applies streaming "
                   "options, initializes a binding, and owns a ContentLost subscription.",
    .apis = {"cna_vertex_declaration_create_empty", "cna_vertex_declaration_create",
             "cna_vertex_declaration_create_with_stride", "cna_vertex_declaration_destroy",
             "cna_vertex_declaration_get_stride", "cna_vertex_declaration_copy_elements",
             "cna_vertex_buffer_binding_init", "cna_vertex_buffer_create",
             "cna_vertex_buffer_destroy", "cna_vertex_buffer_get_info",
             "cna_vertex_buffer_copy_declaration_elements", "cna_vertex_buffer_set_data",
             "cna_vertex_buffer_get_data", "cna_vertex_buffer_set_data_raw",
             "cna_vertex_buffer_set_data_raw_at", "cna_vertex_buffer_get_data_raw",
             "cna_vertex_buffer_set_data_raw_with_options",
             "cna_vertex_buffer_set_data_raw_at_with_options",
             "cna_vertex_buffer_subscribe_content_lost",
             "cna_vertex_buffer_unsubscribe_content_lost", "CNA_VertexBufferCreateInfo",
             "CNA_VertexBufferInfo", "CNA_VertexBufferTransfer",
             "CNA_VertexBufferBinding"},
    .tags = {"c", "binding", "graphics", "vertex", "declaration", "buffer", "streaming"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Standalone C program. It validates CPU-visible resource state and transfer "
                    "semantics; nothing is bound, drawn, captured or visually inspected. The "
                    "selected renderers cannot raise ContentLost, so only subscription ownership "
                    "is exercised. Built only when CNA_BUILD_C_API=ON.",
})
