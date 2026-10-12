// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.core.error_handling",
    .title = "Errors in C: results and diagnostics",
    .description = "A C program makes deliberate mistakes -- a null output, an invalid volume, an unissued "
                   "handle, a short buffer and a failing callback -- and reads back what the C ABI says about "
                   "each: the CNA_Result, the error category and the thread's last message. Checks failed "
                   "diagnostic queries (including incompatible layouts) preserve that message, a larger compatible output prefix preserves its unknown tail, structured exception type/parameter details "
                   "distinguish canonical exceptions, exact byte-buffer capacities preserve guards, and a failed frame stays "
                   "failed while destruction still invokes content cleanup once.",
    .apis = {"cna_error_get_last_info", "cna_error_get_last_message_size", "cna_error_copy_last_message",
             "cna_game_get_is_active", "cna_game_get_type_name_size", "cna_game_copy_type_name",
             "cna_game_run_one_frame", "cna_sound_effect_set_master_volume",
             "cna_error_get_last_exception_type_size_ext", "cna_error_copy_last_exception_type_ext",
             "cna_error_get_last_exception_param_name_size_ext", "cna_error_copy_last_exception_param_name_ext",
             "CNA_ErrorInfo", "CNA_CallbackError", "CNA_Result"},
    .tags = {"c", "binding", "errors", "diagnostics"},
    .needs = CnaDemos::Need::CApi,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
