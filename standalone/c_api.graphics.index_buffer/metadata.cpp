// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.index_buffer",
    .title = "Dynamic index-buffer transfers in C",
    .description = "A C game uploads 16-bit indices from a caller-array window, reads them back "
                   "atomically into another window, patches a byte-offset slice without disturbing "
                   "the other indices and owns a silent ContentLost subscription.",
    .apis = {"cna_index_buffer_create", "cna_index_buffer_get_info",
             "cna_index_buffer_set_data", "cna_index_buffer_get_data",
             "cna_index_buffer_set_data_at", "cna_index_buffer_subscribe_content_lost",
             "cna_index_buffer_unsubscribe_content_lost", "cna_index_buffer_destroy",
             "CNA_IndexBufferCreateInfo", "CNA_IndexBufferInfo", "CNA_IndexBufferTransfer"},
    .tags = {"c", "binding", "graphics", "index-buffer", "dynamic", "transfer", "events"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C program; current Linux renderers do not lose devices, so ContentLost "
                    "registration is exercised but notification delivery remains renderer-specific. No "
                    "drawing or visual result is claimed. Built only with CNA_BUILD_C_API=ON.",
})
