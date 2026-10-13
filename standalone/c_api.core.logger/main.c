/* SPDX-License-Identifier: MS-PL */
/* Logger severity, category, filtering and sink-callback behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct CapturedLine {
    CNA_LogLevel level;
    CNA_LogCategory category;
    uint64_t byte_length;
    char text[96];
} CapturedLine;

typedef struct Capture {
    uint32_t count;
    int overflow;
    CapturedLine lines[32];
} Capture;

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static void capture_line(
    CNA_LogLevel level,
    CNA_LogCategory category,
    CNA_StringView message,
    void* context)
{
    Capture* capture = (Capture*)context;
    CapturedLine* line;
    if (capture->count >= 32U || message.data == 0 || message.byte_length >= 96U) {
        capture->overflow = 1;
        return;
    }
    line = &capture->lines[capture->count++];
    line->level = level;
    line->category = category;
    line->byte_length = message.byte_length;
    memcpy(line->text, message.data, (size_t)message.byte_length);
    line->text[message.byte_length] = '\0';
}

static int line_is(
    const Capture* capture,
    uint32_t index,
    CNA_LogLevel level,
    CNA_LogCategory category,
    const char* text)
{
    const CapturedLine* line = &capture->lines[index];
    return index < capture->count && line->level == level && line->category == category &&
           line->byte_length == (uint64_t)strlen(text) && strcmp(line->text, text) == 0;
}

int main(int argc, char** argv)
{
    static const CNA_LogCategory Categories[9] = {
        CNA_LOG_CATEGORY_APPLICATION, CNA_LOG_CATEGORY_ERROR, CNA_LOG_CATEGORY_SYSTEM,
        CNA_LOG_CATEGORY_AUDIO, CNA_LOG_CATEGORY_VIDEO, CNA_LOG_CATEGORY_RENDER,
        CNA_LOG_CATEGORY_INPUT, CNA_LOG_CATEGORY_TEST, CNA_LOG_CATEGORY_GPU
    };
    CnaDemo demo;
    Capture capture;
    CNA_LogLevel previous = UINT32_MAX;
    CNA_LogLevel minimum = UINT32_MAX;
    CNA_StringView malformed = {0, UINT64_C(4)};
    uint32_t count_before;
    uint32_t i;

    memset(&capture, 0, sizeof(capture));
    cna_demo_init(&demo, "c_api.core.logger", argc, argv, 1);

    cna_demo_check(
        &demo,
        CNA_LOG_LEVEL_FATAL == 0U && CNA_LOG_LEVEL_TRACE == 5U &&
            CNA_LOG_LEVEL_EXPERIMENT == 100U && CNA_LOG_CATEGORY_APPLICATION == 0U &&
            CNA_LOG_CATEGORY_GPU == 8U,
        "public severity and category identities retain their fixed ordinals");

    cna_demo_check(
        &demo,
        cna_logger_get_minimum_level(&previous) == CNA_RESULT_SUCCESS &&
            cna_logger_set_sink_ext(0, 0) == CNA_RESULT_SUCCESS &&
            cna_logger_set_minimum_level(CNA_LOG_LEVEL_EXPERIMENT) == CNA_RESULT_SUCCESS &&
            cna_logger_get_minimum_level(&minimum) == CNA_RESULT_SUCCESS &&
            minimum == CNA_LOG_LEVEL_EXPERIMENT &&
            cna_logger_set_sink_ext(capture_line, &capture) == CNA_RESULT_SUCCESS,
        "the process minimum level and caller-owned sink can be installed explicitly");

    cna_demo_check(
        &demo,
        cna_logger_fatal(view("fatal"), CNA_LOG_CATEGORY_APPLICATION) == CNA_RESULT_SUCCESS &&
            cna_logger_error(view("error"), CNA_LOG_CATEGORY_ERROR) == CNA_RESULT_SUCCESS &&
            cna_logger_warn(view("warn"), CNA_LOG_CATEGORY_SYSTEM) == CNA_RESULT_SUCCESS &&
            cna_logger_info(view("info"), CNA_LOG_CATEGORY_AUDIO) == CNA_RESULT_SUCCESS &&
            cna_logger_debug(view("debug"), CNA_LOG_CATEGORY_VIDEO) == CNA_RESULT_SUCCESS &&
            cna_logger_trace(view("trace"), CNA_LOG_CATEGORY_RENDER) == CNA_RESULT_SUCCESS &&
            cna_logger_experiment(view("experiment"), CNA_LOG_CATEGORY_GPU) ==
                CNA_RESULT_SUCCESS && capture.count == 7U && capture.overflow == 0,
        "every severity-specific route reaches the sink when enabled");
    cna_demo_check(
        &demo,
        line_is(&capture, 0U, CNA_LOG_LEVEL_FATAL, CNA_LOG_CATEGORY_APPLICATION,
                "[FATAL][APPLICATION] fatal") &&
            line_is(&capture, 3U, CNA_LOG_LEVEL_INFO, CNA_LOG_CATEGORY_AUDIO,
                    "[INFO][AUDIO] info") &&
            line_is(&capture, 6U, CNA_LOG_LEVEL_EXPERIMENT, CNA_LOG_CATEGORY_GPU,
                    "[EXPERIMENT][GPU] experiment"),
        "the sink receives exact level, category and formatted borrowed text");

    count_before = capture.count;
    cna_demo_check(
        &demo,
        cna_logger_fatal_if(view("fatal false"), CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_logger_error_if(view("error false"), CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_logger_warn_if(view("warn false"), CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_logger_info_if(view("info false"), CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_logger_debug_if(view("debug false"), CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_logger_trace_if(view("trace false"), CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_logger_experiment_if(view("experiment false"), CNA_FALSE) ==
                CNA_RESULT_SUCCESS && capture.count == count_before,
        "all conditional routes suppress false messages before sink delivery");

    cna_demo_check(
        &demo,
        cna_logger_fatal_if(view("fatal true"), CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_logger_error_if(view("error true"), CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_logger_warn_if(view("warn true"), CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_logger_info_if(view("info true"), CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_logger_debug_if(view("debug true"), CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_logger_trace_if(view("trace true"), CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_logger_experiment_if(view("experiment true"), CNA_TRUE) ==
                CNA_RESULT_SUCCESS && capture.count == count_before + 7U &&
            line_is(&capture, count_before + 6U, CNA_LOG_LEVEL_EXPERIMENT,
                    CNA_LOG_CATEGORY_APPLICATION, "[EXPERIMENT][APPLICATION] experiment true"),
        "true conditional routes use their severity and the Application category");

    count_before = capture.count;
    for (i = 0U; i < 9U; ++i) {
        if (cna_logger_log(
                CNA_LOG_LEVEL_TRACE, view("validated only"), Categories[i], CNA_FALSE) !=
            CNA_RESULT_SUCCESS) {
            break;
        }
    }
    cna_demo_check(
        &demo,
        i == 9U && capture.count == count_before &&
            cna_logger_log(
                CNA_LOG_LEVEL_INFO, view("explicit"), CNA_LOG_CATEGORY_TEST, CNA_TRUE) ==
                CNA_RESULT_SUCCESS && capture.count == count_before + 1U &&
            line_is(&capture, count_before, CNA_LOG_LEVEL_INFO, CNA_LOG_CATEGORY_TEST,
                    "[INFO][TEST] explicit"),
        "the general route validates every category and delivers only a true condition");

    count_before = capture.count;
    cna_demo_check(
        &demo,
        cna_logger_set_minimum_level(CNA_LOG_LEVEL_WARN) == CNA_RESULT_SUCCESS &&
            cna_logger_info(view("filtered"), CNA_LOG_CATEGORY_APPLICATION) ==
                CNA_RESULT_SUCCESS && capture.count == count_before &&
            cna_logger_warn(view("retained"), CNA_LOG_CATEGORY_INPUT) == CNA_RESULT_SUCCESS &&
            capture.count == count_before + 1U &&
            line_is(&capture, count_before, CNA_LOG_LEVEL_WARN, CNA_LOG_CATEGORY_INPUT,
                    "[WARN][INPUT] retained") &&
            cna_logger_get_minimum_level(&minimum) == CNA_RESULT_SUCCESS &&
            minimum == CNA_LOG_LEVEL_WARN,
        "the minimum level discards less-severe messages across categories");

    count_before = capture.count;
    cna_demo_check(
        &demo,
        cna_logger_log(UINT32_C(6), view("bad"), CNA_LOG_CATEGORY_TEST, CNA_TRUE) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_logger_log(CNA_LOG_LEVEL_INFO, view("bad"), UINT32_C(9), CNA_TRUE) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_logger_log(
                CNA_LOG_LEVEL_INFO, view("bad"), CNA_LOG_CATEGORY_TEST, UINT8_C(9)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_logger_info(malformed, CNA_LOG_CATEGORY_TEST) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_logger_info_if(malformed, CNA_TRUE) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_logger_set_minimum_level(UINT32_C(6)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_logger_get_minimum_level(&minimum) == CNA_RESULT_SUCCESS &&
            minimum == CNA_LOG_LEVEL_WARN && capture.count == count_before,
        "invalid identities, booleans and string views are refused without logging or state changes");

    cna_demo_check(
        &demo,
        cna_logger_reset_sink_ext() == CNA_RESULT_SUCCESS &&
            cna_logger_set_minimum_level(previous) == CNA_RESULT_SUCCESS &&
            cna_logger_get_minimum_level(&minimum) == CNA_RESULT_SUCCESS && minimum == previous,
        "the default sink and prior process minimum level are restored");

    return cna_demo_finish(&demo);
}
