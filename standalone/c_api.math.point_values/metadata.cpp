// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.point_values",
    .title = "Point values in C",
    .description = "A renderer-independent C program constructs and formats integer points, checks value "
                   "identity, then demonstrates component arithmetic, defined wraparound and atomic division "
                   "failures.",
    .apis = {"cna_point_init", "cna_point_init_xy", "cna_point_get_zero",
             "cna_point_equals", "cna_point_not_equals", "cna_point_get_hash_code",
             "cna_point_get_string_size", "cna_point_copy_string", "cna_point_add",
             "cna_point_subtract", "cna_point_multiply", "cna_point_divide", "CNA_Point"},
    .tags = {"c", "binding", "math", "point", "integer", "arithmetic", "value"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent C value workflow. It creates no game or graphics "
                    "device and does not imply floating-point vector or rectangle behavior.",
})
