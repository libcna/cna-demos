// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.bounding_box_values",
    .title = "BoundingBox values in C",
    .description = "A renderer-independent C program constructs and combines axis-aligned boxes, "
                   "classifies contained values, copies ordered corners and tests analytic intersections.",
    .apis = {"cna_bounding_box_init", "cna_bounding_box_init_min_max",
             "cna_bounding_box_contains_box", "cna_bounding_box_contains_sphere",
             "cna_bounding_box_contains_point", "cna_bounding_box_contains_frustum",
             "cna_bounding_box_copy_corners", "cna_bounding_box_intersects_ray",
             "cna_bounding_box_intersects_frustum", "cna_bounding_box_intersects_sphere",
             "cna_bounding_box_intersects_box", "cna_bounding_box_intersects_plane",
             "cna_bounding_box_equals", "cna_bounding_box_not_equals",
             "cna_bounding_box_get_hash_code", "cna_bounding_box_get_string_size",
             "cna_bounding_box_copy_string", "cna_bounding_box_create_from_points",
             "cna_bounding_box_create_from_sphere", "cna_bounding_box_create_merged",
             "CNA_BoundingBox", "CNA_BOUNDING_BOX_CORNER_COUNT", "CNA_ContainmentType"},
    .tags = {"c", "binding", "math", "bounding-box", "containment", "intersection"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent analytic-geometry workflow. It creates no game or "
                    "graphics device and does not perform collision response or rendering.",
})
