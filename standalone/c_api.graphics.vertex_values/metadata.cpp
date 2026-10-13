// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.vertex_values",
    .title = "Built-in vertex values in C",
    .description = "A renderer-independent C program relates all seven built-in vertex structures to their "
                   "packed strides and canonical declarations, then checks default, comparison, hash and exact "
                   "formatting behavior for vertex values and declaration elements.",
    .apis = {"cna_vertex_value_init_default", "cna_vertex_value_equals",
             "cna_vertex_value_not_equals", "cna_vertex_value_get_hash_code",
             "cna_vertex_value_get_string_byte_count", "cna_vertex_value_copy_string",
             "cna_vertex_type_get_stride", "cna_vertex_type_copy_elements",
             "cna_vertex_element_equals", "cna_vertex_element_not_equals",
             "cna_vertex_element_get_hash_code", "cna_vertex_element_get_string_byte_count",
             "cna_vertex_element_copy_string", "CNA_VertexValue", "CNA_VertexElement"},
    .tags = {"c", "binding", "graphics", "vertex", "declaration", "value", "layout"},
    .needs = CnaDemos::Need::CApi,
    .restrictions = "Standalone renderer-independent C program. It inspects value/layout contracts only; no "
                    "graphics device, buffer, draw, screenshot or visual validation is involved. Built only "
                    "when CNA_BUILD_C_API=ON.",
})
