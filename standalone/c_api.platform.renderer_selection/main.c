/* SPDX-License-Identifier: MS-PL */
/* Process-global renderer selection, real fallback and diagnostic history through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct RendererName {
    CNA_GraphicsRendererType type;
    const char* name;
} RendererName;

static const RendererName Renderers[] = {
    {CNA_GRAPHICS_RENDERER_OPENGLES3, "OPENGLES3"},
    {CNA_GRAPHICS_RENDERER_OPENGL33, "OPENGL33"},
    {CNA_GRAPHICS_RENDERER_WEBGL2, "WEBGL2"},
    {CNA_GRAPHICS_RENDERER_WEBGPU, "WEBGPU"},
    {CNA_GRAPHICS_RENDERER_HEADLESS, "HEADLESS"},
    {CNA_GRAPHICS_RENDERER_SOFTWARE, "SOFTWARE"},
    {CNA_GRAPHICS_RENDERER_STUB, "STUB"},
    {CNA_GRAPHICS_RENDERER_DIRECTX9, "DIRECTX9"},
    {CNA_GRAPHICS_RENDERER_SDL_GPU, "SDL_GPU"},
    {CNA_GRAPHICS_RENDERER_FNA3D, "FNA3D"},
};

static const char* const FallbackReasonNames[] = {
    "NotCompiledIn", "ProbeUnavailable", "InitializationFailed", "WindowKindConflict"
};

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static int read_last_exception_details(
    char* type, size_t type_capacity, char* parameter, size_t parameter_capacity)
{
    uint64_t type_bytes = UINT64_MAX;
    uint64_t parameter_bytes = UINT64_MAX;
    uint64_t copied = UINT64_MAX;

    if (cna_error_get_last_exception_type_size_ext(&type_bytes) != CNA_RESULT_SUCCESS ||
        type_bytes >= type_capacity ||
        cna_error_copy_last_exception_type_ext(type, type_bytes, &copied) != CNA_RESULT_SUCCESS ||
        copied != type_bytes) {
        return 0;
    }
    type[type_bytes] = '\0';
    if (cna_error_get_last_exception_param_name_size_ext(&parameter_bytes) != CNA_RESULT_SUCCESS ||
        parameter_bytes >= parameter_capacity) {
        return 0;
    }
    if (parameter_bytes != 0U &&
        (cna_error_copy_last_exception_param_name_ext(parameter, parameter_bytes, &copied) !=
             CNA_RESULT_SUCCESS ||
         copied != parameter_bytes)) {
        return 0;
    }
    parameter[parameter_bytes] = '\0';
    return 1;
}

static const char* renderer_name(CNA_GraphicsRendererType type)
{
    size_t index;
    for (index = 0U; index < sizeof(Renderers) / sizeof(Renderers[0]); ++index) {
        if (Renderers[index].type == type) {
            return Renderers[index].name;
        }
    }
    return 0;
}

static int contains_renderer(
    const CNA_GraphicsRendererType* values, uint64_t count, CNA_GraphicsRendererType type)
{
    uint64_t index;
    for (index = 0U; index < count; ++index) {
        if (values[index] == type) {
            return 1;
        }
    }
    return 0;
}

static int check_availability(
    CNA_GraphicsRendererType* available, uint64_t capacity, uint64_t* out_count,
    CNA_GraphicsRendererType* out_absent)
{
    CNA_GraphicsRendererType short_copy[16];
    CNA_GraphicsRendererType before[16];
    CNA_Bool is_available = UINT8_C(9);
    uint64_t required = UINT64_MAX;
    size_t index;

    if (cna_graphics_renderer_get_available_count_ext(out_count) != CNA_RESULT_SUCCESS ||
        *out_count == 0U || *out_count > capacity) {
        return 0;
    }
    memset(short_copy, 0xA5, sizeof(short_copy));
    memcpy(before, short_copy, sizeof(short_copy));
    if (cna_graphics_renderer_copy_available_ext(
            short_copy, *out_count - 1U, &required) != CNA_RESULT_BUFFER_TOO_SMALL ||
        required != *out_count || memcmp(short_copy, before, sizeof(short_copy)) != 0 ||
        cna_graphics_renderer_copy_available_ext(available, capacity, &required) !=
            CNA_RESULT_SUCCESS ||
        required != *out_count) {
        return 0;
    }

    *out_absent = CNA_GRAPHICS_RENDERER_UNKNOWN;
    for (index = 0U; index < sizeof(Renderers) / sizeof(Renderers[0]); ++index) {
        const int found = contains_renderer(available, *out_count, Renderers[index].type);
        if (cna_graphics_renderer_get_is_available_ext(
                Renderers[index].type, &is_available) != CNA_RESULT_SUCCESS ||
            is_available != (found ? CNA_TRUE : CNA_FALSE)) {
            return 0;
        }
        if (!found && *out_absent == CNA_GRAPHICS_RENDERER_UNKNOWN) {
            *out_absent = Renderers[index].type;
        }
    }
    return *out_absent != CNA_GRAPHICS_RENDERER_UNKNOWN;
}

static int check_parsing(void)
{
    CNA_GraphicsRendererType parsed = UINT32_MAX;
    CNA_Bool recognized = UINT8_C(9);

    return cna_graphics_renderer_try_parse_name_ext(
               view("headless"), &parsed, &recognized) == CNA_RESULT_SUCCESS &&
        recognized == CNA_TRUE && parsed == CNA_GRAPHICS_RENDERER_HEADLESS &&
        cna_graphics_renderer_try_parse_name_ext(
            view("not_a_renderer"), &parsed, &recognized) == CNA_RESULT_SUCCESS &&
        recognized == CNA_FALSE && parsed == CNA_GRAPHICS_RENDERER_UNKNOWN &&
        cna_graphics_renderer_try_parse_name_ext(view("HEADLESS"), 0, &recognized) ==
            CNA_RESULT_INVALID_ARGUMENT &&
        cna_graphics_renderer_try_parse_name_ext(view("HEADLESS"), &parsed, 0) ==
            CNA_RESULT_INVALID_ARGUMENT;
}

static int check_fallback_reason_names(void)
{
    char text[32];
    char before[32];
    uint64_t bytes = UINT64_MAX;
    uint32_t reason;

    for (reason = 0U; reason <= CNA_GRAPHICS_RENDERER_FALLBACK_MAXIMUM; ++reason) {
        memset(text, 0, sizeof(text));
        if (cna_graphics_renderer_fallback_reason_get_name_size_ext(reason, &bytes) !=
                CNA_RESULT_SUCCESS ||
            bytes != (uint64_t)strlen(FallbackReasonNames[reason]) ||
            cna_graphics_renderer_fallback_reason_copy_name_ext(
                reason, text, sizeof(text), &bytes) != CNA_RESULT_SUCCESS ||
            strcmp(text, FallbackReasonNames[reason]) != 0) {
            return 0;
        }
    }
    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(text));
    return cna_graphics_renderer_fallback_reason_copy_name_ext(
               CNA_GRAPHICS_RENDERER_FALLBACK_INITIALIZATION_FAILED, text, UINT64_C(3),
               &bytes) == CNA_RESULT_BUFFER_TOO_SMALL &&
        bytes == (uint64_t)strlen("InitializationFailed") &&
        memcmp(text, before, sizeof(text)) == 0 &&
        cna_graphics_renderer_fallback_reason_get_name_size_ext(
            CNA_GRAPHICS_RENDERER_FALLBACK_MAXIMUM + 1U, &bytes) ==
            CNA_RESULT_INVALID_ARGUMENT;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GraphicsRendererType available[16];
    CNA_GraphicsRendererType initial = CNA_GRAPHICS_RENDERER_UNKNOWN;
    CNA_GraphicsRendererType selected = CNA_GRAPHICS_RENDERER_UNKNOWN;
    CNA_GraphicsRendererType active = CNA_GRAPHICS_RENDERER_UNKNOWN;
    CNA_GraphicsRendererType absent = CNA_GRAPHICS_RENDERER_UNKNOWN;
    CNA_GraphicsRendererType invalid_chain[2];
    CNA_GraphicsRendererFallbackRecord record;
    CNA_ErrorInfo error_info;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_Bool flag = UINT8_C(9);
    CNA_LogLevel prior_level = CNA_LOG_LEVEL_WARN;
    uint64_t count = UINT64_MAX;
    uint64_t message_bytes = UINT64_MAX;
    uint64_t copied = UINT64_MAX;
    char lower_name[32];
    char message[512];
    char exception_type[128];
    char exception_parameter[32];
    char short_message[32];
    char before[32];
    const char* initial_name;
    size_t index;

    cna_demo_init(&demo, "c_api.platform.renderer_selection", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_graphics_renderer_get_selected_ext(&initial) == CNA_RESULT_SUCCESS &&
            (initial_name = renderer_name(initial)) != 0 &&
            cna_graphics_renderer_get_is_latched_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_graphics_renderer_get_active_ext(&active) == CNA_RESULT_INVALID_STATE,
        "the requested renderer is selected but no active renderer exists before device creation");

    memset(available, 0, sizeof(available));
    cna_demo_check(
        &demo,
        check_availability(
            available, sizeof(available) / sizeof(available[0]), &count, &absent) &&
            contains_renderer(available, count, initial),
        "count/copy availability is atomic and agrees with every live public renderer identity");
    cna_demo_check(
        &demo,
        cna_graphics_renderer_get_available_count_ext(0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_copy_available_ext(0, 1U, &count) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_copy_available_ext(available, 16U, 0) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_get_is_available_ext(
                CNA_GRAPHICS_RENDERER_UNKNOWN, &flag) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_get_is_available_ext(UINT32_C(1), &flag) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_get_is_available_ext(initial, 0) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "availability queries refuse null outputs, unknown identities and retired numeric gaps");
    cna_demo_check(&demo, check_parsing(),
                   "renderer names parse case-insensitively while an unknown name is a clean miss");

    memset(lower_name, 0, sizeof(lower_name));
    for (index = 0U; initial_name != 0 && initial_name[index] != '\0'; ++index) {
        const char character = initial_name[index];
        lower_name[index] = character >= 'A' && character <= 'Z'
            ? (char)(character + ('a' - 'A'))
            : character;
    }
    cna_demo_check(
        &demo,
        initial_name != 0 &&
            cna_graphics_renderer_set_preferred_by_name_ext(view(lower_name)) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_renderer_get_selected_ext(&selected) == CNA_RESULT_SUCCESS &&
            selected == initial &&
            cna_graphics_renderer_set_preferred_ext(initial) == CNA_RESULT_SUCCESS,
        "name and numeric selection forms choose the same available renderer before the latch");
    cna_demo_check(
        &demo,
        cna_graphics_renderer_set_preferred_ext(absent) == CNA_RESULT_INVALID_STATE &&
            cna_graphics_renderer_set_preferred_ext(UINT32_MAX) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_set_preferred_by_name_ext(view("NOT_A_RENDERER")) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_get_selected_ext(&selected) == CNA_RESULT_SUCCESS &&
            selected == initial,
        "an unavailable renderer needs fallback while malformed requests preserve the selection");
    memset(&error_info, 0, sizeof(error_info));
    error_info.struct_size = (uint32_t)sizeof(error_info);
    error_info.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_error_get_last_info(&error_info) == CNA_RESULT_SUCCESS &&
            error_info.result == CNA_RESULT_INVALID_ARGUMENT &&
            error_info.category == CNA_ERROR_CATEGORY_ARGUMENT &&
            read_last_exception_details(
                exception_type, sizeof(exception_type), exception_parameter,
                sizeof(exception_parameter)) &&
            strcmp(exception_type, "System.ArgumentException") == 0 &&
            strcmp(exception_parameter, "name") == 0,
        "an unknown renderer preserves canonical ArgumentException and parameter-name details");

    cna_demo_check(
        &demo,
        cna_graphics_renderer_get_automatic_fallback_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_graphics_renderer_set_automatic_fallback_ext(CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_graphics_renderer_get_automatic_fallback_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_graphics_renderer_set_automatic_fallback_ext(CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_graphics_renderer_set_automatic_fallback_ext(UINT8_C(2)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_get_automatic_fallback_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "automatic fallback toggles before latching and refuses a noncanonical boolean atomically");
    cna_demo_check(&demo, check_fallback_reason_names(),
                   "all fallback reasons have exact stable names and atomic count/copy behavior");

    invalid_chain[0] = initial;
    invalid_chain[1] = UINT32_MAX;
    cna_demo_check(
        &demo,
        cna_graphics_renderer_set_fallback_chain_ext(invalid_chain, 2U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_get_automatic_fallback_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_graphics_renderer_set_fallback_chain_ext(0, 1U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_set_fallback_chain_ext(&initial, 1U) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_renderer_get_automatic_fallback_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_graphics_renderer_set_preferred_ext(absent) == CNA_RESULT_SUCCESS &&
            cna_graphics_renderer_get_selected_ext(&selected) == CNA_RESULT_SUCCESS &&
            selected == absent,
        "an invalid chain is atomic before an explicit available fallback permits an absent preference");

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    (void)cna_logger_get_minimum_level(&prior_level);
    (void)cna_logger_set_minimum_level(CNA_LOG_LEVEL_FATAL);
    if (cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game), "create the fallback game")) {
        cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager),
                              "create a real graphics device through the fallback chain");
    }
    (void)cna_logger_set_minimum_level(prior_level);

    cna_demo_check(
        &demo,
        cna_graphics_renderer_get_is_latched_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_graphics_renderer_get_active_ext(&active) == CNA_RESULT_SUCCESS &&
            active == initial && active != selected,
        "device creation latches the available fallback as active while preserving the preference");

    memset(&record, 0, sizeof(record));
    record.struct_size = (uint32_t)sizeof(record);
    record.struct_version = UINT32_C(1);
    memset(message, 0, sizeof(message));
    cna_demo_check(
        &demo,
        cna_graphics_renderer_get_fallback_count_ext(&count) == CNA_RESULT_SUCCESS && count == 1U &&
            cna_graphics_renderer_get_fallback_at_ext(0U, &record) == CNA_RESULT_SUCCESS &&
            record.type == absent &&
            record.reason == CNA_GRAPHICS_RENDERER_FALLBACK_NOT_COMPILED_IN &&
            cna_graphics_renderer_fallback_get_message_size_ext(0U, &message_bytes) ==
                CNA_RESULT_SUCCESS &&
            message_bytes > 0U && message_bytes < sizeof(message) &&
            cna_graphics_renderer_fallback_copy_message_ext(
                0U, message, sizeof(message), &copied) == CNA_RESULT_SUCCESS &&
            copied == message_bytes && message[message_bytes] == '\0' &&
            strstr(message, "not compiled") != 0,
        "fallback history records the absent preference, reason and nonempty diagnostic message");

    memset(short_message, 0xA5, sizeof(short_message));
    memcpy(before, short_message, sizeof(short_message));
    cna_demo_check(
        &demo,
        cna_graphics_renderer_fallback_copy_message_ext(
            0U, short_message, 1U, &copied) == CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == message_bytes && memcmp(short_message, before, sizeof(short_message)) == 0 &&
            cna_graphics_renderer_get_fallback_at_ext(count, &record) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_fallback_get_message_size_ext(count, &copied) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "fallback message copying is atomic and positions outside the history are refused");

    record.struct_version = UINT32_C(2);
    cna_demo_check(
        &demo,
        cna_graphics_renderer_get_fallback_at_ext(0U, &record) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_get_fallback_count_ext(0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_fallback_copy_message_ext(0U, message, sizeof(message), 0) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "fallback history rejects unknown structure versions and null outputs");
    cna_demo_check(
        &demo,
        cna_graphics_renderer_set_preferred_ext(initial) == CNA_RESULT_INVALID_STATE &&
            cna_graphics_renderer_set_preferred_by_name_ext(view(initial_name)) ==
                CNA_RESULT_INVALID_STATE &&
            cna_graphics_renderer_set_fallback_chain_ext(&initial, 1U) ==
                CNA_RESULT_INVALID_STATE &&
            cna_graphics_renderer_set_automatic_fallback_ext(CNA_FALSE) ==
                CNA_RESULT_INVALID_STATE &&
            cna_graphics_renderer_get_selected_ext(&selected) == CNA_RESULT_SUCCESS &&
            selected == absent &&
            cna_graphics_renderer_get_active_ext(&active) == CNA_RESULT_SUCCESS &&
            active == initial,
        "all selection mutation is refused after latching without changing selected or active identity");
    memset(&error_info, 0, sizeof(error_info));
    error_info.struct_size = (uint32_t)sizeof(error_info);
    error_info.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_error_get_last_info(&error_info) == CNA_RESULT_SUCCESS &&
            error_info.result == CNA_RESULT_INVALID_STATE &&
            error_info.category == CNA_ERROR_CATEGORY_STATE &&
            read_last_exception_details(
                exception_type, sizeof(exception_type), exception_parameter,
                sizeof(exception_parameter)) &&
            strcmp(exception_type, "System.InvalidOperationException") == 0 &&
            exception_parameter[0] == '\0',
        "a latched renderer mutation preserves canonical InvalidOperationException without a parameter");

    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the fallback graphics manager");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the fallback game");
    }
    return cna_demo_finish(&demo);
}
