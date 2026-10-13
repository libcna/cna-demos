// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.bounding_sphere_values",
    .title = "BoundingSphere values in C",
    .description = "A renderer-independent C program constructs, transforms and combines bounding "
                   "spheres, then classifies containment and analytic intersections.",
    .apis = {"cna_bounding_sphere_init", "cna_bounding_sphere_init_center_radius",
             "cna_bounding_sphere_transform", "cna_bounding_sphere_contains_box",
             "cna_bounding_sphere_contains_frustum", "cna_bounding_sphere_contains_sphere",
             "cna_bounding_sphere_contains_point", "cna_bounding_sphere_equals",
             "cna_bounding_sphere_not_equals", "cna_bounding_sphere_create_from_box",
             "cna_bounding_sphere_create_from_frustum",
             "cna_bounding_sphere_create_from_points", "cna_bounding_sphere_create_merged",
             "cna_bounding_sphere_intersects_box", "cna_bounding_sphere_intersects_frustum",
             "cna_bounding_sphere_intersects_sphere", "cna_bounding_sphere_intersects_ray",
             "cna_bounding_sphere_intersects_plane", "cna_bounding_sphere_get_hash_code",
             "cna_bounding_sphere_get_string_size", "cna_bounding_sphere_copy_string",
             "CNA_BoundingSphere", "CNA_ContainmentType"},
    .tags = {"c", "binding", "math", "bounding-sphere", "containment", "intersection"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent analytic-geometry workflow. It creates no game or "
                    "graphics device and does not perform collision response or rendering.",
})
