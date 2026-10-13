// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.core.logger",
    .title = "Logger levels and sink in C",
    .description = "A renderer-independent C program captures every severity and conditional logger "
                   "route, verifies category and minimum-level filtering, then restores global state.",
    .apis = {"cna_logger_log", "cna_logger_fatal", "cna_logger_error", "cna_logger_warn",
             "cna_logger_info", "cna_logger_debug", "cna_logger_trace",
             "cna_logger_experiment", "cna_logger_fatal_if", "cna_logger_error_if",
             "cna_logger_warn_if", "cna_logger_info_if", "cna_logger_debug_if",
             "cna_logger_trace_if", "cna_logger_experiment_if",
             "cna_logger_set_minimum_level", "cna_logger_get_minimum_level",
             "cna_logger_set_sink_ext", "cna_logger_reset_sink_ext", "CNA_LogLevel",
             "CNA_LogCategory", "CNA_LogSinkCallback"},
    .tags = {"c", "binding", "core", "logging", "diagnostics", "callback"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent process logger workflow. The callback copies "
                    "borrowed text during delivery and every process-global setting is restored.",
})
