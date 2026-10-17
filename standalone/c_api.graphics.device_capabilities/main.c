/* SPDX-License-Identifier: MS-PL */
/*
 * Graphics-device identity and capability discovery from C. The demo cross-checks the compact
 * legacy capability bit set against individual queries, enumerates the defined detailed profile,
 * copies its English diagnostic report atomically, and relates the logical backbuffer description
 * to the device's applied presentation parameters. Values are printed rather than fixed because
 * they legitimately differ between renderers and hosts.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct DeviceCapabilityState {
    CnaDemo* demo;
    int loads;
} DeviceCapabilityState;

/* ISSUE-067: the public C routes accept these native ordinals but define no C identity names. */
#define DISPLAY_COLOR_SPACE_SRGB UINT32_C(0)
#define DISPLAY_COLOR_SPACE_SCRGB UINT32_C(1)
#define DISPLAY_COLOR_SPACE_HDR10 UINT32_C(2)
#define DISPLAY_COLOR_SPACE_MAXIMUM DISPLAY_COLOR_SPACE_HDR10

static int all_bytes_are(const void* data, size_t count, unsigned char value)
{
    const unsigned char* bytes = (const unsigned char*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != value) {
            return 0;
        }
    }
    return 1;
}

static int is_reserved_feature(CNA_RendererFeature feature)
{
    return feature == UINT32_C(18) || feature == UINT32_C(19) ||
        feature == UINT32_C(20) || feature == UINT32_C(23);
}

static int is_reserved_limit(CNA_RendererLimit limit)
{
    return (limit >= UINT32_C(2) && limit <= UINT32_C(10)) ||
        limit == UINT32_C(12) || limit == UINT32_C(13) || limit == UINT32_C(15) ||
        limit == UINT32_C(19) || limit == UINT32_C(21);
}

static int audit_renderer_identity(DeviceCapabilityState* state, CNA_Handle device,
                                   CNA_RendererInfo* out_info, char** out_name)
{
    uint64_t name_bytes = UINT64_C(0);
    uint64_t required = UINT64_C(0);
    char short_name[8];
    CNA_Result info_result;
    CNA_Result size_result;
    CNA_Result short_result;
    CNA_Result copy_result;
    size_t short_capacity;

    memset(out_info, 0xA5, sizeof(*out_info));
    out_info->struct_size = (uint32_t)sizeof(*out_info);
    out_info->struct_version = UINT32_C(1);
    info_result = cna_graphics_device_get_renderer_info(device, out_info);
    cna_demo_check(
        state->demo,
        info_result == CNA_RESULT_SUCCESS && out_info->struct_size == sizeof(*out_info) &&
            out_info->struct_version == UINT32_C(1) &&
            out_info->renderer_name_byte_length > UINT64_C(0) &&
            out_info->renderer_type != CNA_GRAPHICS_RENDERER_UNKNOWN &&
            out_info->renderer_type <= CNA_GRAPHICS_RENDERER_MAXIMUM &&
            out_info->max_texture_dimension > 0U &&
            (out_info->capability_flags >> (CNA_GRAPHICS_CAPABILITY_MAXIMUM + UINT32_C(1))) ==
                UINT64_C(0),
        "renderer info returns a complete bounded identity and compact capability summary");

    size_result = cna_graphics_device_get_renderer_name_size(device, &name_bytes);
    short_capacity = name_bytes < sizeof(short_name) ? (size_t)name_bytes : sizeof(short_name);
    if (short_capacity > 0U) {
        --short_capacity;
    }
    memset(short_name, 0x6B, sizeof(short_name));
    short_result = size_result == CNA_RESULT_SUCCESS && name_bytes > UINT64_C(0)
        ? cna_graphics_device_copy_renderer_name(
              device, short_name, (uint64_t)short_capacity, &required)
        : CNA_RESULT_INVALID_STATE;
    cna_demo_check(
        state->demo,
        size_result == CNA_RESULT_SUCCESS && name_bytes == out_info->renderer_name_byte_length &&
            short_result == CNA_RESULT_BUFFER_TOO_SMALL && required == name_bytes &&
            all_bytes_are(short_name, sizeof(short_name), 0x6B),
        "a short renderer-name destination reports the exact size without a partial write");

    if (name_bytes < (uint64_t)SIZE_MAX) {
        *out_name = (char*)malloc((size_t)name_bytes + 1U);
    }
    required = UINT64_C(0);
    copy_result = *out_name != NULL
        ? cna_graphics_device_copy_renderer_name(device, *out_name, name_bytes, &required)
        : CNA_RESULT_OUT_OF_MEMORY;
    if (*out_name != NULL && required == name_bytes) {
        (*out_name)[name_bytes] = '\0';
    }
    cna_demo_check(
        state->demo,
        copy_result == CNA_RESULT_SUCCESS && required == name_bytes && *out_name != NULL &&
            strlen(*out_name) == (size_t)name_bytes,
        "an exact renderer-name destination receives the complete UTF-8 name");
    return info_result == CNA_RESULT_SUCCESS && copy_result == CNA_RESULT_SUCCESS;
}

static void audit_legacy_capabilities(DeviceCapabilityState* state, CNA_Handle device,
                                      const CNA_RendererInfo* info)
{
    CNA_GraphicsCapability capability;
    int coherent = 1;
    unsigned supported_count = 0U;
    for (capability = CNA_GRAPHICS_CAPABILITY_THREE_D;
         capability <= CNA_GRAPHICS_CAPABILITY_MAXIMUM; ++capability) {
        CNA_Bool supported = CNA_FALSE;
        CNA_Result result = cna_graphics_device_supports_capability(
            device, capability, &supported);
        CNA_Bool summarized =
            (info->capability_flags & (UINT64_C(1) << capability)) != UINT64_C(0)
            ? CNA_TRUE
            : CNA_FALSE;
        if (result != CNA_RESULT_SUCCESS || supported > CNA_TRUE || supported != summarized) {
            coherent = 0;
        }
        if (supported == CNA_TRUE) {
            ++supported_count;
        }
    }
    cna_demo_check(
        state->demo, coherent,
        "every legacy capability query agrees with the corresponding renderer-info bit");
    printf("  legacy capabilities supported: %u of %u\n", supported_count,
           (unsigned)CNA_GRAPHICS_CAPABILITY_MAXIMUM + 1U);
}

static void audit_detailed_features(DeviceCapabilityState* state, CNA_Handle device)
{
    CNA_RendererFeature feature;
    unsigned supported = 0U;
    unsigned unsupported = 0U;
    unsigned restricted = 0U;
    unsigned reserved = 0U;
    int coherent = 1;
    for (feature = UINT32_C(0); feature <= CNA_RENDERER_FEATURE_MAXIMUM; ++feature) {
        CNA_RendererFeatureSupport support = UINT32_MAX;
        CNA_Result result = cna_graphics_device_get_renderer_feature_support_ext(
            device, feature, &support);
        if (is_reserved_feature(feature)) {
            ++reserved;
            if (result != CNA_RESULT_INVALID_ARGUMENT) {
                coherent = 0;
            }
        } else if (result != CNA_RESULT_SUCCESS ||
                   support == CNA_RENDERER_FEATURE_SUPPORT_UNKNOWN ||
                   support > CNA_RENDERER_FEATURE_SUPPORT_MAXIMUM) {
            coherent = 0;
        } else if (support == CNA_RENDERER_FEATURE_SUPPORT_SUPPORTED) {
            ++supported;
        } else if (support == CNA_RENDERER_FEATURE_SUPPORT_RESTRICTED) {
            ++restricted;
        } else {
            ++unsupported;
        }
    }
    cna_demo_check(
        state->demo, coherent && reserved == 4U,
        "the detailed feature range classifies every live identity and refuses all reserved gaps");
    printf("  detailed features: %u supported, %u restricted, %u unsupported\n",
           supported, restricted, unsupported);
}

static void audit_limits(DeviceCapabilityState* state, CNA_Handle device,
                         const CNA_RendererInfo* info)
{
    CNA_RendererLimit limit;
    unsigned known_count = 0U;
    unsigned reserved_count = 0U;
    int coherent = 1;
    int maximum_matches = 0;
    for (limit = UINT32_C(0); limit <= CNA_RENDERER_LIMIT_MAXIMUM; ++limit) {
        CNA_Bool known = CNA_FALSE;
        uint64_t value = UINT64_MAX;
        CNA_Result result = cna_graphics_device_get_renderer_limit_ext(
            device, limit, &known, &value);
        if (is_reserved_limit(limit)) {
            ++reserved_count;
            if (result != CNA_RESULT_INVALID_ARGUMENT) {
                coherent = 0;
            }
        } else {
            if (result != CNA_RESULT_SUCCESS || known != CNA_TRUE) {
                coherent = 0;
            } else {
                ++known_count;
            }
            if (limit == CNA_RENDERER_LIMIT_MAX_TEXTURE_DIMENSION) {
                maximum_matches = known == CNA_TRUE && value == info->max_texture_dimension;
            }
        }
    }
    cna_demo_check(
        state->demo, coherent && known_count == 8U && reserved_count == 13U,
        "all eight live numeric limits are known and every retired identity remains reserved");
    cna_demo_check(
        state->demo, maximum_matches,
        "the detailed maximum-texture limit agrees with the renderer-info summary");
}

static void audit_formats(DeviceCapabilityState* state, CNA_Handle device)
{
    CNA_SurfaceFormat format;
    unsigned classified_formats = 0U;
    int coherent = 1;
    int direct_agreement = 1;
    int color_baseline = 0;
    for (format = CNA_SURFACE_FORMAT_COLOR; format <= CNA_SURFACE_FORMAT_USHORT_EXT; ++format) {
        CNA_RendererFormatUsageFlags known = UINT32_MAX;
        CNA_RendererFormatUsageFlags supported = UINT32_MAX;
        CNA_Bool direct = UINT32_C(7);
        CNA_Result result = cna_graphics_device_get_surface_format_support_ext(
            device, format, &known, &supported);
        CNA_Result direct_result =
            cna_graphics_device_supports_surface_format_as_render_target_ext(
                device, format, &direct);
        if (result != CNA_RESULT_SUCCESS || (known & ~CNA_RENDERER_FORMAT_USAGE_ALL) != 0U ||
            (supported & ~known) != 0U) {
            coherent = 0;
        }
        if (direct_result != CNA_RESULT_SUCCESS || direct > CNA_TRUE ||
            direct != ((supported & CNA_RENDERER_FORMAT_USAGE_RENDER_TARGET) != 0U
                    ? CNA_TRUE
                    : CNA_FALSE)) {
            direct_agreement = 0;
        }
        if (known != 0U) {
            ++classified_formats;
        }
        if (format == CNA_SURFACE_FORMAT_COLOR) {
            const CNA_RendererFormatUsageFlags baseline =
                CNA_RENDERER_FORMAT_USAGE_TEXTURE_STORAGE |
                CNA_RENDERER_FORMAT_USAGE_RENDER_TARGET |
                CNA_RENDERER_FORMAT_USAGE_COLOR_TRANSFER;
            color_baseline = (known & baseline) == baseline;
        }
    }
    cna_demo_check(
        state->demo, coherent && classified_formats > 0U,
        "every public surface format returns bounded known and supported usage masks");
    cna_demo_check(
        state->demo, color_baseline,
        "Color classifies the baseline storage, render-target and color-transfer usages");
    cna_demo_check(
        state->demo, direct_agreement,
        "the direct render-target verdict agrees with every detailed format-usage mask");
    printf("  surface formats with classified usages: %u of %u\n", classified_formats,
           (unsigned)CNA_SURFACE_FORMAT_USHORT_EXT + 1U);
}

static void audit_execution_and_color_space(
    DeviceCapabilityState* state, CNA_Handle device, const CNA_RendererInfo* info)
{
    CNA_Bool executes_source = UINT32_C(7);
    CNA_Bool supports_ibl = UINT32_C(7);
    uint32_t original = UINT32_MAX;
    CNA_Bool supported[DISPLAY_COLOR_SPACE_MAXIMUM + UINT32_C(1)];
    uint32_t space;
    int execution_coherent;
    int support_coherent = 1;
    int set_coherent = 1;

    execution_coherent =
        cna_graphics_device_executes_shader_effect_source_ext(
            device, &executes_source) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_supports_image_based_lighting_ext(
            device, &supports_ibl) == CNA_RESULT_SUCCESS &&
        executes_source <= CNA_TRUE && supports_ibl <= CNA_TRUE;
    if (info->renderer_type == CNA_GRAPHICS_RENDERER_SOFTWARE) {
        execution_coherent = execution_coherent && executes_source == CNA_FALSE &&
            supports_ibl == CNA_TRUE;
    } else if (info->renderer_type == CNA_GRAPHICS_RENDERER_OPENGLES3 ||
               info->renderer_type == CNA_GRAPHICS_RENDERER_OPENGL33) {
        execution_coherent = execution_coherent && executes_source == CNA_TRUE &&
            supports_ibl == CNA_TRUE;
    }
    cna_demo_check(
        state->demo, execution_coherent,
        "effect-source execution and image-based lighting report renderer-specific truth");
    printf(
        "  executes effect source: %s, image-based lighting: %s\n",
        executes_source == CNA_TRUE ? "yes" : "no",
        supports_ibl == CNA_TRUE ? "yes" : "no");

    if (cna_graphics_device_get_display_color_space_ext(device, &original) !=
            CNA_RESULT_SUCCESS ||
        original > DISPLAY_COLOR_SPACE_MAXIMUM) {
        support_coherent = 0;
        set_coherent = 0;
    }
    for (space = DISPLAY_COLOR_SPACE_SRGB;
         space <= DISPLAY_COLOR_SPACE_MAXIMUM; ++space) {
        uint32_t after = UINT32_MAX;
        supported[space] = UINT32_C(7);
        if (cna_graphics_device_supports_display_color_space_ext(
                device, space, &supported[space]) != CNA_RESULT_SUCCESS ||
            supported[space] > CNA_TRUE ||
            cna_graphics_device_get_display_color_space_ext(device, &after) !=
                CNA_RESULT_SUCCESS ||
            after != original) {
            support_coherent = 0;
        }
    }
    if (original <= DISPLAY_COLOR_SPACE_MAXIMUM && supported[original] != CNA_TRUE) {
        support_coherent = 0;
    }
    cna_demo_check(
        state->demo, support_coherent,
        "all three display-color-space probes are bounded and preserve the active space");

    for (space = DISPLAY_COLOR_SPACE_SRGB;
         space <= DISPLAY_COLOR_SPACE_MAXIMUM; ++space) {
        CNA_Bool changed = UINT32_C(7);
        uint32_t after = UINT32_MAX;
        if (cna_graphics_device_set_display_color_space_ext(
                device, space, &changed) != CNA_RESULT_SUCCESS ||
            changed != supported[space] ||
            cna_graphics_device_get_display_color_space_ext(device, &after) !=
                CNA_RESULT_SUCCESS ||
            after != (supported[space] == CNA_TRUE ? space : original)) {
            set_coherent = 0;
        }
        if (supported[space] == CNA_TRUE && space != original) {
            changed = CNA_FALSE;
            if (cna_graphics_device_set_display_color_space_ext(
                    device, original, &changed) != CNA_RESULT_SUCCESS ||
                changed != CNA_TRUE) {
                set_coherent = 0;
            }
        }
    }
    {
        CNA_Bool changed = UINT32_C(9);
        CNA_Bool unsupported = UINT32_C(9);
        uint32_t restored = UINT32_MAX;
        if (cna_graphics_device_set_display_color_space_ext(
                device, DISPLAY_COLOR_SPACE_MAXIMUM + UINT32_C(1), &changed) !=
                CNA_RESULT_INVALID_ARGUMENT ||
            changed != UINT32_C(9) ||
            cna_graphics_device_supports_display_color_space_ext(
                device, DISPLAY_COLOR_SPACE_MAXIMUM + UINT32_C(1), &unsupported) !=
                CNA_RESULT_INVALID_ARGUMENT ||
            unsupported != UINT32_C(9) ||
            cna_graphics_device_get_display_color_space_ext(device, &restored) !=
                CNA_RESULT_SUCCESS ||
            restored != original) {
            set_coherent = 0;
        }
    }
    cna_demo_check(
        state->demo, set_coherent,
        "color-space changes follow support, restore the original and atomically refuse identity three");
    printf(
        "  display color space %u; support sRGB/scRGB/HDR10: %u/%u/%u\n",
        (unsigned)original, (unsigned)supported[DISPLAY_COLOR_SPACE_SRGB],
        (unsigned)supported[DISPLAY_COLOR_SPACE_SCRGB],
        (unsigned)supported[DISPLAY_COLOR_SPACE_HDR10]);
}

static void audit_report(DeviceCapabilityState* state, CNA_Handle device, const char* renderer_name)
{
    uint64_t report_bytes = UINT64_C(0);
    uint64_t required = UINT64_C(0);
    unsigned char guard[32];
    char* report = NULL;
    CNA_Result size_result;
    CNA_Result short_result;
    CNA_Result copy_result = CNA_RESULT_OUT_OF_MEMORY;

    size_result = cna_graphics_device_get_capability_report_size_ext(device, &report_bytes);
    memset(guard, 0x7C, sizeof(guard));
    short_result = size_result == CNA_RESULT_SUCCESS && report_bytes > sizeof(guard)
        ? cna_graphics_device_copy_capability_report_ext(
              device, (char*)guard, sizeof(guard), &required)
        : CNA_RESULT_INVALID_STATE;
    cna_demo_check(
        state->demo,
        size_result == CNA_RESULT_SUCCESS && report_bytes > sizeof(guard) &&
            short_result == CNA_RESULT_BUFFER_TOO_SMALL && required == report_bytes &&
            all_bytes_are(guard, sizeof(guard), 0x7C),
        "a short report destination returns its exact size without a partial write");

    if (report_bytes < (uint64_t)SIZE_MAX) {
        report = (char*)malloc((size_t)report_bytes + 1U);
    }
    required = UINT64_C(0);
    if (report != NULL) {
        copy_result = cna_graphics_device_copy_capability_report_ext(
            device, report, report_bytes, &required);
        if (copy_result == CNA_RESULT_SUCCESS && required == report_bytes) {
            report[report_bytes] = '\0';
        }
    }
    cna_demo_check(
        state->demo,
        copy_result == CNA_RESULT_SUCCESS && required == report_bytes &&
            strlen(report) == (size_t)report_bytes &&
            strstr(report, "Renderer capability report") != NULL &&
            strstr(report, "Detailed features") != NULL &&
            strstr(report, "Additional limitations") != NULL &&
            (renderer_name == NULL || strstr(report, renderer_name) != NULL),
        "the exact copy contains the complete English renderer-capability diagnostic");
    printf("  capability report bytes: %llu\n", (unsigned long long)report_bytes);
    free(report);
}

static void audit_dialect_and_backbuffer(DeviceCapabilityState* state, CNA_Handle device)
{
    CNA_ShaderDialect dialect = UINT32_MAX;
    CNA_BackBufferInfo backbuffer;
    CNA_PresentationParameters presentation;
    CNA_Result dialect_result;
    CNA_Result backbuffer_result;
    CNA_Result presentation_result;

    dialect_result = cna_graphics_device_get_shader_dialect_ext(device, &dialect);
    cna_demo_check(
        state->demo,
        dialect_result == CNA_RESULT_SUCCESS && dialect <= CNA_SHADER_DIALECT_MAXIMUM,
        "the device reports one bounded renderer-specific shader payload dialect");

    memset(&backbuffer, 0xA5, sizeof(backbuffer));
    backbuffer.struct_size = (uint32_t)sizeof(backbuffer);
    backbuffer.struct_version = UINT32_C(1);
    backbuffer_result = cna_graphics_device_get_backbuffer_info(device, &backbuffer);
    memset(&presentation, 0, sizeof(presentation));
    presentation.struct_size = (uint32_t)sizeof(presentation);
    presentation.struct_version = UINT32_C(1);
    presentation_result = cna_graphics_device_get_presentation_parameters(device, &presentation);
    cna_demo_check(
        state->demo,
        backbuffer_result == CNA_RESULT_SUCCESS && presentation_result == CNA_RESULT_SUCCESS &&
            backbuffer.struct_size == sizeof(backbuffer) &&
            backbuffer.struct_version == UINT32_C(1) && backbuffer.width > 0U &&
            backbuffer.height > 0U && backbuffer.format <= CNA_SURFACE_FORMAT_USHORT_EXT &&
            backbuffer.reserved == 0U &&
            backbuffer.width == (uint32_t)presentation.back_buffer_width &&
            backbuffer.height == (uint32_t)presentation.back_buffer_height &&
            backbuffer.format == presentation.back_buffer_format,
        "logical backbuffer information is complete and agrees with applied presentation state");
    printf("  shader dialect %u, backbuffer %ux%u format %u\n", (unsigned)dialect,
           (unsigned)backbuffer.width, (unsigned)backbuffer.height, (unsigned)backbuffer.format);
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    DeviceCapabilityState* state = (DeviceCapabilityState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_RendererInfo info;
    char* renderer_name = NULL;
    CNA_Result borrow_result;
    int identity_ready;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    borrow_result = cna_game_get_graphics_device(game, &device);
    cna_demo_check(state->demo, borrow_result == CNA_RESULT_SUCCESS,
                   "borrow the live graphics device for this callback");
    if (borrow_result != CNA_RESULT_SUCCESS) {
        return CNA_RESULT_SUCCESS;
    }

    identity_ready = audit_renderer_identity(state, device, &info, &renderer_name);
    if (identity_ready) {
        printf("  renderer: %s (type %u, max texture %u)\n", renderer_name,
               (unsigned)info.renderer_type, (unsigned)info.max_texture_dimension);
        audit_legacy_capabilities(state, device, &info);
        audit_detailed_features(state, device);
        audit_limits(state, device, &info);
        audit_formats(state, device);
        audit_execution_and_color_space(state, device, &info);
        audit_report(state, device, renderer_name);
        audit_dialect_and_backbuffer(state, device);
    }
    free(renderer_name);
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    DeviceCapabilityState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.device_capabilities", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_FALSE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }

    manager_ready = cna_demo_check_result(
        &demo, cna_demo_use_hidef(game, &manager),
        "create the HiDef graphics device whose capability profile is audited");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through load and the immediate exit request");
        cna_demo_check(&demo, state.loads == 1,
                       "the capability audit ran in exactly one load callback");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
