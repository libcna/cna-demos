// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.curve_key_values",
    .title = "CurveKey values in C",
    .description = "A renderer-independent C program inspects the fixed CurveKey record, constructor "
                   "defaults, mutable fields, position ordering, cloning, equality and hashing.",
    .apis = {"cna_curve_key_init_position_value", "cna_curve_key_init_tangents",
             "cna_curve_key_init_full", "cna_curve_key_get_continuity",
             "cna_curve_key_set_continuity", "cna_curve_key_get_position",
             "cna_curve_key_get_tangent_in", "cna_curve_key_set_tangent_in",
             "cna_curve_key_get_tangent_out", "cna_curve_key_set_tangent_out",
             "cna_curve_key_get_value", "cna_curve_key_set_value", "cna_curve_key_clone",
             "cna_curve_key_compare_to", "cna_curve_key_equals", "cna_curve_key_not_equals",
             "cna_curve_key_get_hash_code", "CNA_CurveKey", "CNA_CurveContinuity"},
    .tags = {"c", "binding", "math", "curve", "keyframe", "value"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent CurveKey value workflow. Collection ownership and "
                    "curve evaluation are covered separately.",
})
