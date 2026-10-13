// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.packed_vectors",
    .title = "Packed vectors in C",
    .description = "A renderer-independent C program packs and expands all 17 storage formats, "
                   "checks width-aware equality, atomic failures and consumed-component rules, "
                   "and exercises exact and special IEEE binary16 conversions.",
    .apis = {"cna_packed_vector_pack", "cna_packed_vector_unpack",
             "cna_packed_vector_equals", "cna_packed_vector_not_equals",
             "cna_half_from_single", "cna_half_from_single_bits", "cna_half_to_single",
             "CNA_PackedVectorFormat"},
    .tags = {"c", "binding", "graphics", "math", "packed-vector", "half", "format"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C value workflow; it creates no game or graphics device and makes no "
                    "renderer, vertex-fetch, endianness portability or image-quality claim.",
})
