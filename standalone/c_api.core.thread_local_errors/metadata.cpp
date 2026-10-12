// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.core.thread_local_errors",
    .title = "C diagnostics stay with their thread",
    .description = "Interleaves distinct failures on two C worker threads, re-queries each after the other "
                   "writes, and checks that their results, categories and messages and the calling thread's "
                   "diagnostic stay independent. Then interleaves a native owner exception with worker C validation to verify canonical exception type and parameter-name isolation.",
    .apis = {"cna_error_get_last_info", "cna_error_get_last_message_size", "cna_error_copy_last_message",
             "cna_error_get_last_exception_type_size_ext", "cna_error_copy_last_exception_type_ext",
             "cna_error_get_last_exception_param_name_size_ext", "cna_error_copy_last_exception_param_name_ext",
             "cna_game_create", "cna_sound_effect_set_master_volume", "cna_game_destroy",
             "cna_point_init", "cna_vector2_add", "cna_math_clamp_float", "CNA_ErrorInfo"},
    .tags = {"c", "binding", "errors", "diagnostics", "threads"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C11 program; requires CNA::CApi and C11 threads/atomics. "
                    "The first interleaving uses game-independent routes; a later Game belongs only to the caller thread. "
                    "No game loop, foreign-thread Game dispatch, rendered image or audio playback is exercised.",
})
