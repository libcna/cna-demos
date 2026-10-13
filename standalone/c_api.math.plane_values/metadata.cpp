// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.plane_values",
    .title = "Plane values in C",
    .description = "A renderer-independent C program constructs and normalizes planes, measures signed "
                   "distances, classifies three volume kinds and applies matrix and quaternion transforms.",
    .apis = {"cna_plane_init", "cna_plane_init_vector4", "cna_plane_init_normal_d",
             "cna_plane_init_points", "cna_plane_init_abcd", "cna_plane_dot",
             "cna_plane_dot_coordinate", "cna_plane_dot_normal",
             "cna_plane_normalize_in_place", "cna_plane_intersects_box",
             "cna_plane_intersects_sphere", "cna_plane_intersects_frustum",
             "cna_plane_equals", "cna_plane_not_equals", "cna_plane_get_hash_code",
             "cna_plane_get_string_size", "cna_plane_copy_string", "cna_plane_normalize",
             "cna_plane_transform_matrix", "cna_plane_transform_quaternion", "CNA_Plane"},
    .tags = {"c", "binding", "math", "plane", "classification", "transform"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent plane workflow. It performs analytic geometry only "
                    "and creates no game or graphics device.",
})
