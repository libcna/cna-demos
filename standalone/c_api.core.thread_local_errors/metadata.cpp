// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.core.thread_local_errors",
    .title = "C diagnostics stay with their thread",
    .description = "Interleaves distinct failures on two C worker threads, re-queries each after the other "
                   "writes, and checks that their results, categories and messages and the calling thread's "
                   "diagnostic stay independent.",
    .apis = {"cna_error_get_last_info", "cna_error_get_last_message_size", "cna_error_copy_last_message",
             "cna_point_init", "cna_vector2_add", "cna_math_clamp_float", "CNA_ErrorInfo"},
    .tags = {"c", "binding", "errors", "diagnostics", "threads"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C11 program; requires CNA::CApi and C11 threads/atomics. "
                    "Only argument failures of game-independent routes are exercised; no Game or rendering.",
})
