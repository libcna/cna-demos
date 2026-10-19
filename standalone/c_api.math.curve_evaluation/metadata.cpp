// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.curve_evaluation",
    .title = "Curve evaluation in C",
    .description = "A renderer-independent C program edits a Curve through its retained key view, "
                   "computes whole-curve and per-key tangents, evaluates loop modes and clones it.",
    .apis = {"cna_curve_create", "cna_curve_destroy", "cna_curve_get_is_constant",
             "cna_curve_get_keys", "cna_curve_get_pre_loop", "cna_curve_set_pre_loop",
             "cna_curve_get_post_loop", "cna_curve_set_post_loop", "cna_curve_clone",
             "cna_curve_evaluate", "cna_curve_compute_tangents",
             "cna_curve_compute_tangents_in_out", "cna_curve_compute_tangent",
             "cna_curve_compute_tangent_in_out", "cna_curve_key_set_continuity",
             "cna_curve_key_set_tangent_out", "CNA_CurveHandle", "CNA_CurveLoopType",
             "CNA_CurveTangent"},
    .tags = {"c", "binding", "math", "curve", "tangent", "loop", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent curve workflow, including the shared Step and "
                    "post-loop regressions for CNA-DEMOS-ISSUE-002 and CNA-DEMOS-ISSUE-036.",
})
