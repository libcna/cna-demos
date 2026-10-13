// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.bounding_frustum_values",
    .title = "BoundingFrustum values in C",
    .description = "A renderer-independent C program derives clipping planes and ordered corners from a "
                   "frustum matrix, then classifies volumes, points, planes and rays.",
    .apis = {"cna_bounding_frustum_init_matrix", "cna_bounding_frustum_get_near",
             "cna_bounding_frustum_get_far", "cna_bounding_frustum_get_left",
             "cna_bounding_frustum_get_right", "cna_bounding_frustum_get_top",
             "cna_bounding_frustum_get_bottom", "cna_bounding_frustum_contains_frustum",
             "cna_bounding_frustum_contains_box", "cna_bounding_frustum_contains_sphere",
             "cna_bounding_frustum_contains_point", "cna_bounding_frustum_copy_corners",
             "cna_bounding_frustum_intersects_frustum",
             "cna_bounding_frustum_intersects_box",
             "cna_bounding_frustum_intersects_sphere",
             "cna_bounding_frustum_intersects_plane", "cna_bounding_frustum_intersects_ray",
             "cna_bounding_frustum_equals", "cna_bounding_frustum_not_equals",
             "cna_bounding_frustum_get_hash_code", "cna_bounding_frustum_get_string_size",
             "cna_bounding_frustum_copy_string", "CNA_BoundingFrustum",
             "CNA_BOUNDING_FRUSTUM_CORNER_COUNT", "CNA_ContainmentType"},
    .tags = {"c", "binding", "math", "frustum", "culling", "intersection"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent analytic-frustum workflow. It creates no game or "
                    "graphics device and does not claim a rendered culling result.",
})
