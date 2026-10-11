// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.core.error_handling",
    .title = "Errors in C: results and diagnostics",
    .description = "A C program makes four deliberate mistakes -- a null output, a handle that was never issued, a "
                   "buffer one byte too small, a callback that fails -- and reads back what the C ABI says about "
                   "each: the CNA_Result, the error category and the thread's last message.",
    .apis = {"cna_error_get_last_info", "cna_error_get_last_message_size", "cna_error_copy_last_message",
             "cna_game_get_is_active", "cna_game_get_type_name_size", "cna_game_copy_type_name",
             "cna_game_run_one_frame", "CNA_ErrorInfo", "CNA_CallbackError", "CNA_Result"},
    .tags = {"c", "binding", "errors", "diagnostics"},
    .needs = CnaDemos::Need::CApi,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
