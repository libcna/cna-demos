// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.rectangle_values",
    .title = "Rectangle values in C",
    .description = "A renderer-independent C program inspects and mutates integer rectangles, demonstrates "
                   "half-open containment and strict-area overlap, then computes exact intersections and unions.",
    .apis = {"cna_rectangle_init", "cna_rectangle_init_xywh", "cna_rectangle_get_empty",
             "cna_rectangle_get_left", "cna_rectangle_get_right", "cna_rectangle_get_top",
             "cna_rectangle_get_bottom", "cna_rectangle_get_location",
             "cna_rectangle_set_location", "cna_rectangle_get_center",
             "cna_rectangle_get_is_empty", "cna_rectangle_contains_xy",
             "cna_rectangle_contains_point", "cna_rectangle_contains_rectangle",
             "cna_rectangle_offset_point", "cna_rectangle_offset_xy",
             "cna_rectangle_inflate", "cna_rectangle_equals", "cna_rectangle_not_equals",
             "cna_rectangle_get_hash_code", "cna_rectangle_get_string_size",
             "cna_rectangle_copy_string", "cna_rectangle_intersects",
             "cna_rectangle_intersect", "cna_rectangle_union", "CNA_Rectangle"},
    .tags = {"c", "binding", "math", "rectangle", "containment", "intersection", "value"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent C value workflow. It creates no game or graphics "
                    "device and makes no clipping, rasterization or pixel-coverage claim.",
})
