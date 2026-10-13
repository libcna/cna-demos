// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.scalar_helpers",
    .title = "Scalar math helpers in C",
    .description = "A renderer-independent C program applies CNA's scalar interpolation, bounds, angle, "
                   "epsilon and multisample helpers, including their endpoint and invalid-input behavior.",
    .apis = {"cna_math_barycentric", "cna_math_catmull_rom", "cna_math_clamp_float",
             "cna_math_distance", "cna_math_hermite", "cna_math_lerp", "cna_math_max",
             "cna_math_min", "cna_math_smooth_step", "cna_math_to_degrees",
             "cna_math_to_radians", "cna_math_wrap_angle", "cna_math_clamp_int32",
             "cna_math_within_epsilon", "cna_math_closest_msaa_power"},
    .tags = {"c", "binding", "math", "scalar", "interpolation", "angle", "epsilon"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent C value workflow. It creates no game or graphics "
                    "device; MSAA results demonstrate the pure selection helper, not device support.",
})
