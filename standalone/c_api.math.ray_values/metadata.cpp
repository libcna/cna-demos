// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.ray_values",
    .title = "Ray values in C",
    .description = "A renderer-independent C program inspects Ray value identity, exact formatting and "
                   "nearest-hit distances against boxes, spheres, planes and frusta.",
    .apis = {"cna_ray_init", "cna_ray_init_position_direction", "cna_ray_equals",
             "cna_ray_not_equals", "cna_ray_get_hash_code", "cna_ray_intersects_box",
             "cna_ray_intersects_sphere", "cna_ray_intersects_plane",
             "cna_ray_intersects_frustum", "cna_ray_get_string_size",
             "cna_ray_copy_string", "CNA_Ray"},
    .tags = {"c", "binding", "math", "ray", "intersection", "distance"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent analytic-intersection workflow. It creates no game "
                    "or graphics device and does not perform physics or rendering.",
})
