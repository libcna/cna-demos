// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.quaternion_values",
    .title = "Quaternion values in C",
    .description = "A renderer-independent C program constructs and inspects quaternion values, relates "
                   "axis-angle, matrix and yaw-pitch-roll rotations, distinguishes multiplication from "
                   "concatenation order, and exercises inverse, interpolation and component arithmetic.",
    .apis = {"cna_quaternion_init_xyzw", "cna_quaternion_init_vector3_w",
             "cna_quaternion_get_identity", "cna_quaternion_conjugate_in_place",
             "cna_quaternion_equals", "cna_quaternion_not_equals",
             "cna_quaternion_get_hash_code", "cna_quaternion_length",
             "cna_quaternion_length_squared", "cna_quaternion_normalize_in_place",
             "cna_quaternion_get_string_size", "cna_quaternion_copy_string",
             "cna_quaternion_add", "cna_quaternion_concatenate",
             "cna_quaternion_conjugate", "cna_quaternion_create_from_axis_angle",
             "cna_quaternion_create_from_rotation_matrix",
             "cna_quaternion_create_from_yaw_pitch_roll", "cna_quaternion_divide",
             "cna_quaternion_dot", "cna_quaternion_inverse", "cna_quaternion_lerp",
             "cna_quaternion_slerp", "cna_quaternion_subtract",
             "cna_quaternion_multiply", "cna_quaternion_multiply_scalar",
             "cna_quaternion_negate", "cna_quaternion_normalize", "CNA_Quaternion"},
    .tags = {"c", "binding", "math", "quaternion", "rotation", "interpolation", "value"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent C value workflow. It creates no game or graphics "
                    "device and does not establish matrix/vector transform routes or renderer behavior.",
})
