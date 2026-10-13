/* SPDX-License-Identifier: MS-PL */
/*
 * Graphics-state values and live device round trips from C. Every public preset is checked through
 * all fields, distinctive complete states are applied and read back, pixel/vertex sampler slots are
 * kept independent, and the callback restores the exact object, scalar and geometric state it
 * borrowed. Device identity and the game-owned disposal boundary are checked in the same loan.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct DeviceStateDemo {
    CnaDemo* demo;
    int loads;
} DeviceStateDemo;

static int color_equal(CNA_Color left, CNA_Color right)
{
    return left.r == right.r && left.g == right.g && left.b == right.b &&
        left.a == right.a;
}

static int rectangle_equal(CNA_Rectangle left, CNA_Rectangle right)
{
    return left.x == right.x && left.y == right.y && left.width == right.width &&
        left.height == right.height;
}

static int blend_equal(const CNA_BlendState* left, const CNA_BlendState* right)
{
    return left->struct_size == right->struct_size &&
        left->struct_version == right->struct_version &&
        left->alpha_blend_function == right->alpha_blend_function &&
        left->alpha_destination_blend == right->alpha_destination_blend &&
        left->alpha_source_blend == right->alpha_source_blend &&
        left->color_blend_function == right->color_blend_function &&
        left->color_destination_blend == right->color_destination_blend &&
        left->color_source_blend == right->color_source_blend &&
        left->color_write_channels == right->color_write_channels &&
        left->color_write_channels1 == right->color_write_channels1 &&
        left->color_write_channels2 == right->color_write_channels2 &&
        left->color_write_channels3 == right->color_write_channels3 &&
        color_equal(left->blend_factor, right->blend_factor) &&
        left->multi_sample_mask == right->multi_sample_mask;
}

static int depth_equal(
    const CNA_DepthStencilState* left, const CNA_DepthStencilState* right)
{
    return left->struct_size == right->struct_size &&
        left->struct_version == right->struct_version &&
        left->depth_buffer_enable == right->depth_buffer_enable &&
        left->depth_buffer_write_enable == right->depth_buffer_write_enable &&
        left->stencil_enable == right->stencil_enable &&
        left->two_sided_stencil_mode == right->two_sided_stencil_mode &&
        left->depth_buffer_function == right->depth_buffer_function &&
        left->stencil_function == right->stencil_function &&
        left->stencil_mask == right->stencil_mask &&
        left->stencil_write_mask == right->stencil_write_mask &&
        left->reference_stencil == right->reference_stencil &&
        left->stencil_fail == right->stencil_fail &&
        left->stencil_depth_buffer_fail == right->stencil_depth_buffer_fail &&
        left->stencil_pass == right->stencil_pass &&
        left->counter_clockwise_stencil_function ==
            right->counter_clockwise_stencil_function &&
        left->counter_clockwise_stencil_fail == right->counter_clockwise_stencil_fail &&
        left->counter_clockwise_stencil_depth_buffer_fail ==
            right->counter_clockwise_stencil_depth_buffer_fail &&
        left->counter_clockwise_stencil_pass == right->counter_clockwise_stencil_pass &&
        left->reserved == right->reserved;
}

static int rasterizer_equal(
    const CNA_RasterizerState* left, const CNA_RasterizerState* right)
{
    return left->struct_size == right->struct_size &&
        left->struct_version == right->struct_version && left->cull_mode == right->cull_mode &&
        left->fill_mode == right->fill_mode && left->depth_bias == right->depth_bias &&
        left->slope_scale_depth_bias == right->slope_scale_depth_bias &&
        left->multi_sample_anti_alias == right->multi_sample_anti_alias &&
        left->scissor_test_enable == right->scissor_test_enable &&
        left->reserved[0] == right->reserved[0] &&
        left->reserved[1] == right->reserved[1];
}

static int sampler_equal(const CNA_SamplerState* left, const CNA_SamplerState* right)
{
    return left->struct_size == right->struct_size &&
        left->struct_version == right->struct_version && left->address_u == right->address_u &&
        left->address_v == right->address_v && left->address_w == right->address_w &&
        left->filter == right->filter && left->max_anisotropy == right->max_anisotropy &&
        left->max_mip_level == right->max_mip_level &&
        left->mip_map_level_of_detail_bias == right->mip_map_level_of_detail_bias &&
        left->reserved == right->reserved;
}

static CNA_BlendState expected_blend(
    CNA_Blend color_source, CNA_Blend alpha_source,
    CNA_Blend color_destination, CNA_Blend alpha_destination)
{
    CNA_BlendState state;
    memset(&state, 0, sizeof(state));
    state.struct_size = (uint32_t)sizeof(state);
    state.struct_version = UINT32_C(1);
    state.alpha_blend_function = CNA_BLEND_FUNCTION_ADD;
    state.alpha_destination_blend = alpha_destination;
    state.alpha_source_blend = alpha_source;
    state.color_blend_function = CNA_BLEND_FUNCTION_ADD;
    state.color_destination_blend = color_destination;
    state.color_source_blend = color_source;
    state.color_write_channels = CNA_COLOR_WRITE_ALL;
    state.color_write_channels1 = CNA_COLOR_WRITE_ALL;
    state.color_write_channels2 = CNA_COLOR_WRITE_ALL;
    state.color_write_channels3 = CNA_COLOR_WRITE_ALL;
    state.blend_factor = (CNA_Color){255U, 255U, 255U, 255U};
    state.multi_sample_mask = -1;
    return state;
}

static CNA_DepthStencilState expected_depth(CNA_Bool enabled, CNA_Bool write_enabled)
{
    CNA_DepthStencilState state;
    memset(&state, 0, sizeof(state));
    state.struct_size = (uint32_t)sizeof(state);
    state.struct_version = UINT32_C(1);
    state.depth_buffer_enable = enabled;
    state.depth_buffer_write_enable = write_enabled;
    state.stencil_enable = CNA_FALSE;
    state.two_sided_stencil_mode = CNA_FALSE;
    state.depth_buffer_function = CNA_COMPARE_LESS_EQUAL;
    state.stencil_function = CNA_COMPARE_ALWAYS;
    state.stencil_mask = -1;
    state.stencil_write_mask = -1;
    state.reference_stencil = 0;
    state.stencil_fail = CNA_STENCIL_KEEP;
    state.stencil_depth_buffer_fail = CNA_STENCIL_KEEP;
    state.stencil_pass = CNA_STENCIL_KEEP;
    state.counter_clockwise_stencil_function = CNA_COMPARE_ALWAYS;
    state.counter_clockwise_stencil_fail = CNA_STENCIL_KEEP;
    state.counter_clockwise_stencil_depth_buffer_fail = CNA_STENCIL_KEEP;
    state.counter_clockwise_stencil_pass = CNA_STENCIL_KEEP;
    return state;
}

static CNA_RasterizerState expected_rasterizer(CNA_CullMode cull_mode)
{
    CNA_RasterizerState state;
    memset(&state, 0, sizeof(state));
    state.struct_size = (uint32_t)sizeof(state);
    state.struct_version = UINT32_C(1);
    state.cull_mode = cull_mode;
    state.fill_mode = CNA_FILL_SOLID;
    state.multi_sample_anti_alias = CNA_TRUE;
    state.scissor_test_enable = CNA_FALSE;
    return state;
}

static CNA_SamplerState expected_sampler(
    CNA_TextureFilter filter, CNA_TextureAddressMode address)
{
    CNA_SamplerState state;
    memset(&state, 0, sizeof(state));
    state.struct_size = (uint32_t)sizeof(state);
    state.struct_version = UINT32_C(1);
    state.address_u = address;
    state.address_v = address;
    state.address_w = address;
    state.filter = filter;
    state.max_anisotropy = 4;
    return state;
}

static int check_blend_presets(CnaDemo* demo)
{
    static const CNA_BlendStatePreset Presets[] = {
        CNA_BLEND_STATE_PRESET_DEFAULT, CNA_BLEND_STATE_PRESET_ADDITIVE,
        CNA_BLEND_STATE_PRESET_ALPHA_BLEND, CNA_BLEND_STATE_PRESET_NON_PREMULTIPLIED,
        CNA_BLEND_STATE_PRESET_OPAQUE};
    CNA_BlendState expected[] = {
        expected_blend(CNA_BLEND_ONE, CNA_BLEND_ONE, CNA_BLEND_ZERO, CNA_BLEND_ZERO),
        expected_blend(
            CNA_BLEND_SOURCE_ALPHA, CNA_BLEND_SOURCE_ALPHA, CNA_BLEND_ONE, CNA_BLEND_ONE),
        expected_blend(
            CNA_BLEND_ONE, CNA_BLEND_ONE,
            CNA_BLEND_INVERSE_SOURCE_ALPHA, CNA_BLEND_INVERSE_SOURCE_ALPHA),
        expected_blend(
            CNA_BLEND_SOURCE_ALPHA, CNA_BLEND_SOURCE_ALPHA,
            CNA_BLEND_INVERSE_SOURCE_ALPHA, CNA_BLEND_INVERSE_SOURCE_ALPHA),
        expected_blend(CNA_BLEND_ONE, CNA_BLEND_ONE, CNA_BLEND_ZERO, CNA_BLEND_ZERO)};
    CNA_BlendState actual;
    size_t index;
    int ok = 1;
    for (index = 0U; index < sizeof(Presets) / sizeof(Presets[0]); ++index) {
        memset(&actual, 0xA5, sizeof(actual));
        ok = ok && cna_blend_state_init(Presets[index], &actual) == CNA_RESULT_SUCCESS &&
            blend_equal(&actual, &expected[index]);
    }
    return cna_demo_check(
        demo, ok,
        "all five blend presets initialize every factor, function, channel, color and mask field");
}

static int check_depth_presets(CnaDemo* demo)
{
    static const CNA_DepthStencilStatePreset Presets[] = {
        CNA_DEPTH_STENCIL_STATE_PRESET_DEFAULT,
        CNA_DEPTH_STENCIL_STATE_PRESET_DEPTH_READ,
        CNA_DEPTH_STENCIL_STATE_PRESET_NONE};
    CNA_DepthStencilState expected[] = {
        expected_depth(CNA_TRUE, CNA_TRUE), expected_depth(CNA_TRUE, CNA_FALSE),
        expected_depth(CNA_FALSE, CNA_FALSE)};
    CNA_DepthStencilState actual;
    size_t index;
    int ok = 1;
    for (index = 0U; index < sizeof(Presets) / sizeof(Presets[0]); ++index) {
        memset(&actual, 0xA5, sizeof(actual));
        ok = ok &&
            cna_depth_stencil_state_init(Presets[index], &actual) == CNA_RESULT_SUCCESS &&
            depth_equal(&actual, &expected[index]);
    }
    return cna_demo_check(
        demo, ok,
        "all three depth/stencil presets initialize every depth, stencil and two-sided field");
}

static int check_rasterizer_presets(CnaDemo* demo)
{
    static const CNA_RasterizerStatePreset Presets[] = {
        CNA_RASTERIZER_STATE_PRESET_DEFAULT, CNA_RASTERIZER_STATE_PRESET_CULL_CLOCKWISE,
        CNA_RASTERIZER_STATE_PRESET_CULL_COUNTER_CLOCKWISE,
        CNA_RASTERIZER_STATE_PRESET_CULL_NONE};
    CNA_RasterizerState expected[] = {
        expected_rasterizer(CNA_CULL_COUNTER_CLOCKWISE_FACE),
        expected_rasterizer(CNA_CULL_CLOCKWISE_FACE),
        expected_rasterizer(CNA_CULL_COUNTER_CLOCKWISE_FACE),
        expected_rasterizer(CNA_CULL_NONE)};
    CNA_RasterizerState actual;
    size_t index;
    int ok = 1;
    for (index = 0U; index < sizeof(Presets) / sizeof(Presets[0]); ++index) {
        memset(&actual, 0xA5, sizeof(actual));
        ok = ok && cna_rasterizer_state_init(Presets[index], &actual) == CNA_RESULT_SUCCESS &&
            rasterizer_equal(&actual, &expected[index]);
    }
    return cna_demo_check(
        demo, ok,
        "all four rasterizer presets initialize culling, fill, bias, multisampling and scissor fields");
}

static int check_sampler_presets(CnaDemo* demo)
{
    static const CNA_SamplerStatePreset Presets[] = {
        CNA_SAMPLER_STATE_PRESET_DEFAULT, CNA_SAMPLER_STATE_PRESET_ANISOTROPIC_CLAMP,
        CNA_SAMPLER_STATE_PRESET_ANISOTROPIC_WRAP, CNA_SAMPLER_STATE_PRESET_LINEAR_CLAMP,
        CNA_SAMPLER_STATE_PRESET_LINEAR_WRAP, CNA_SAMPLER_STATE_PRESET_POINT_CLAMP,
        CNA_SAMPLER_STATE_PRESET_POINT_WRAP};
    CNA_SamplerState expected[] = {
        expected_sampler(CNA_TEXTURE_FILTER_LINEAR, CNA_TEXTURE_ADDRESS_WRAP),
        expected_sampler(CNA_TEXTURE_FILTER_ANISOTROPIC, CNA_TEXTURE_ADDRESS_CLAMP),
        expected_sampler(CNA_TEXTURE_FILTER_ANISOTROPIC, CNA_TEXTURE_ADDRESS_WRAP),
        expected_sampler(CNA_TEXTURE_FILTER_LINEAR, CNA_TEXTURE_ADDRESS_CLAMP),
        expected_sampler(CNA_TEXTURE_FILTER_LINEAR, CNA_TEXTURE_ADDRESS_WRAP),
        expected_sampler(CNA_TEXTURE_FILTER_POINT, CNA_TEXTURE_ADDRESS_CLAMP),
        expected_sampler(CNA_TEXTURE_FILTER_POINT, CNA_TEXTURE_ADDRESS_WRAP)};
    CNA_SamplerState actual;
    size_t index;
    int ok = 1;
    for (index = 0U; index < sizeof(Presets) / sizeof(Presets[0]); ++index) {
        memset(&actual, 0xA5, sizeof(actual));
        ok = ok && cna_sampler_state_init(Presets[index], &actual) == CNA_RESULT_SUCCESS &&
            sampler_equal(&actual, &expected[index]);
    }
    return cna_demo_check(
        demo, ok,
        "all seven sampler presets initialize filtering, three address axes, limits and LOD bias");
}

static void initialize_output_prefixes(
    CNA_BlendState* blend, CNA_DepthStencilState* depth,
    CNA_RasterizerState* rasterizer, CNA_SamplerState* pixel, CNA_SamplerState* vertex)
{
    memset(blend, 0, sizeof(*blend));
    blend->struct_size = (uint32_t)sizeof(*blend);
    blend->struct_version = UINT32_C(1);
    memset(depth, 0, sizeof(*depth));
    depth->struct_size = (uint32_t)sizeof(*depth);
    depth->struct_version = UINT32_C(1);
    memset(rasterizer, 0, sizeof(*rasterizer));
    rasterizer->struct_size = (uint32_t)sizeof(*rasterizer);
    rasterizer->struct_version = UINT32_C(1);
    memset(pixel, 0, sizeof(*pixel));
    pixel->struct_size = (uint32_t)sizeof(*pixel);
    pixel->struct_version = UINT32_C(1);
    memset(vertex, 0, sizeof(*vertex));
    vertex->struct_size = (uint32_t)sizeof(*vertex);
    vertex->struct_version = UINT32_C(1);
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    DeviceStateDemo* state = (DeviceStateDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_BlendState saved_blend;
    CNA_DepthStencilState saved_depth;
    CNA_RasterizerState saved_rasterizer;
    CNA_SamplerState saved_pixel;
    CNA_SamplerState saved_vertex;
    CNA_BlendState custom_blend;
    CNA_DepthStencilState custom_depth;
    CNA_RasterizerState custom_rasterizer;
    CNA_SamplerState custom_pixel;
    CNA_SamplerState custom_vertex;
    CNA_BlendState blend_copy;
    CNA_DepthStencilState depth_copy;
    CNA_RasterizerState rasterizer_copy;
    CNA_SamplerState pixel_copy;
    CNA_SamplerState vertex_copy;
    CNA_Rectangle saved_scissor;
    CNA_Rectangle scissor_copy;
    CNA_Color saved_factor;
    CNA_Color factor_copy;
    CNA_Bool disposed = CNA_TRUE;
    CNA_GraphicsDeviceStatus device_status = UINT32_MAX;
    CNA_GraphicsProfile profile = UINT32_MAX;
    uint32_t adapter_index = UINT32_MAX;
    uint64_t adapter_count = 0U;
    int32_t saved_mask = 0;
    int32_t saved_reference = 0;
    int32_t mask_copy = 0;
    int32_t reference_copy = 0;
    CNA_Result get_blend;
    CNA_Result get_depth;
    CNA_Result get_rasterizer;
    CNA_Result get_pixel;
    CNA_Result get_vertex;
    CNA_Result restore_blend;
    CNA_Result restore_depth;
    CNA_Result restore_rasterizer;
    CNA_Result restore_pixel;
    CNA_Result restore_vertex;
    int saved;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    if (!cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &device),
            "borrow the live graphics device for this callback")) {
        return CNA_RESULT_SUCCESS;
    }
    check_blend_presets(state->demo);
    check_depth_presets(state->demo);
    check_rasterizer_presets(state->demo);
    check_sampler_presets(state->demo);

    cna_demo_check(
        state->demo,
        cna_graphics_device_get_is_disposed(device, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_graphics_device_get_status(device, &device_status) == CNA_RESULT_SUCCESS &&
            device_status == CNA_GRAPHICS_DEVICE_STATUS_NORMAL &&
            cna_graphics_device_get_graphics_profile(device, &profile) == CNA_RESULT_SUCCESS &&
            profile == CNA_GRAPHICS_PROFILE_HI_DEF &&
            cna_graphics_device_get_adapter_index(device, &adapter_index) == CNA_RESULT_SUCCESS &&
            cna_graphics_adapter_get_count(device, &adapter_count) == CNA_RESULT_SUCCESS &&
            (uint64_t)adapter_index < adapter_count,
        "the live device is normal, undisposed, HiDef and attached to an enumerated adapter");
    cna_demo_check(
        state->demo,
        cna_graphics_device_dispose(device) == CNA_RESULT_NOT_SUPPORTED &&
            cna_graphics_device_get_is_disposed(device, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_graphics_device_get_status(device, &device_status) == CNA_RESULT_SUCCESS &&
            device_status == CNA_GRAPHICS_DEVICE_STATUS_NORMAL,
        "C cannot dispose the callback-scoped game-owned device and it remains usable");

    initialize_output_prefixes(
        &saved_blend, &saved_depth, &saved_rasterizer, &saved_pixel, &saved_vertex);
    get_blend = cna_graphics_device_get_blend_state(device, &saved_blend);
    get_depth = cna_graphics_device_get_depth_stencil_state(device, &saved_depth);
    get_rasterizer = cna_graphics_device_get_rasterizer_state(device, &saved_rasterizer);
    get_pixel = cna_graphics_device_get_sampler_state(
        device, CNA_SHADER_STAGE_PIXEL, UINT32_C(3), &saved_pixel);
    get_vertex = cna_graphics_device_get_sampler_state(
        device, CNA_SHADER_STAGE_VERTEX, UINT32_C(3), &saved_vertex);
    memset(&saved_scissor, 0, sizeof(saved_scissor));
    memset(&saved_factor, 0, sizeof(saved_factor));
    saved = cna_demo_check(
        state->demo,
        get_blend == CNA_RESULT_SUCCESS && get_depth == CNA_RESULT_SUCCESS &&
            get_rasterizer == CNA_RESULT_SUCCESS && get_pixel == CNA_RESULT_SUCCESS &&
            get_vertex == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_scissor_rectangle(device, &saved_scissor) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_blend_factor(device, &saved_factor) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_multi_sample_mask(device, &saved_mask) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_reference_stencil(device, &saved_reference) ==
                CNA_RESULT_SUCCESS,
        "save all object, sampler, scalar and geometric device state values");
    if (!saved) {
        return CNA_RESULT_SUCCESS;
    }

    custom_blend = expected_blend(
        CNA_BLEND_INVERSE_DESTINATION_COLOR, CNA_BLEND_INVERSE_DESTINATION_ALPHA,
        CNA_BLEND_DESTINATION_COLOR, CNA_BLEND_DESTINATION_ALPHA);
    custom_blend.alpha_blend_function = CNA_BLEND_FUNCTION_SUBTRACT;
    custom_blend.color_blend_function = CNA_BLEND_FUNCTION_REVERSE_SUBTRACT;
    custom_blend.color_write_channels = CNA_COLOR_WRITE_RED | CNA_COLOR_WRITE_GREEN;
    custom_blend.color_write_channels1 = CNA_COLOR_WRITE_BLUE | CNA_COLOR_WRITE_ALPHA;
    custom_blend.color_write_channels2 = CNA_COLOR_WRITE_RED;
    custom_blend.color_write_channels3 = CNA_COLOR_WRITE_NONE;
    custom_blend.blend_factor = (CNA_Color){17U, 34U, 51U, 68U};
    custom_blend.multi_sample_mask = INT32_C(0x13579bdf);
    initialize_output_prefixes(
        &blend_copy, &depth_copy, &rasterizer_copy, &pixel_copy, &vertex_copy);
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_blend_state(device, &custom_blend) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_blend_state(device, &blend_copy) == CNA_RESULT_SUCCESS &&
            blend_equal(&blend_copy, &custom_blend),
        "apply and read back every field of a distinctive blend state");

    custom_depth = expected_depth(CNA_TRUE, CNA_FALSE);
    custom_depth.stencil_enable = CNA_TRUE;
    custom_depth.two_sided_stencil_mode = CNA_TRUE;
    custom_depth.depth_buffer_function = CNA_COMPARE_GREATER_EQUAL;
    custom_depth.stencil_function = CNA_COMPARE_EQUAL;
    custom_depth.stencil_mask = INT32_C(0x0f0f);
    custom_depth.stencil_write_mask = INT32_C(0x00ff);
    custom_depth.reference_stencil = 37;
    custom_depth.stencil_fail = CNA_STENCIL_REPLACE;
    custom_depth.stencil_depth_buffer_fail = CNA_STENCIL_INCREMENT;
    custom_depth.stencil_pass = CNA_STENCIL_INVERT;
    custom_depth.counter_clockwise_stencil_function = CNA_COMPARE_NOT_EQUAL;
    custom_depth.counter_clockwise_stencil_fail = CNA_STENCIL_DECREMENT;
    custom_depth.counter_clockwise_stencil_depth_buffer_fail = CNA_STENCIL_INCREMENT_SATURATION;
    custom_depth.counter_clockwise_stencil_pass = CNA_STENCIL_ZERO;
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_depth_stencil_state(device, &custom_depth) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_depth_stencil_state(device, &depth_copy) ==
                CNA_RESULT_SUCCESS &&
            depth_equal(&depth_copy, &custom_depth),
        "apply and read back every field of a two-sided depth/stencil state");

    custom_rasterizer = expected_rasterizer(CNA_CULL_NONE);
    custom_rasterizer.fill_mode = CNA_FILL_WIREFRAME;
    custom_rasterizer.depth_bias = 0.125f;
    custom_rasterizer.slope_scale_depth_bias = -0.75f;
    custom_rasterizer.multi_sample_anti_alias = CNA_FALSE;
    custom_rasterizer.scissor_test_enable = CNA_TRUE;
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_rasterizer_state(device, &custom_rasterizer) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_rasterizer_state(device, &rasterizer_copy) ==
                CNA_RESULT_SUCCESS &&
            rasterizer_equal(&rasterizer_copy, &custom_rasterizer),
        "apply and read back every field of a distinctive rasterizer state");

    custom_pixel = expected_sampler(CNA_TEXTURE_FILTER_POINT_MIP_LINEAR, CNA_TEXTURE_ADDRESS_WRAP);
    custom_pixel.address_u = CNA_TEXTURE_ADDRESS_MIRROR;
    custom_pixel.address_v = CNA_TEXTURE_ADDRESS_CLAMP;
    custom_pixel.max_anisotropy = 7;
    custom_pixel.max_mip_level = 3;
    custom_pixel.mip_map_level_of_detail_bias = 0.5f;
    initialize_output_prefixes(
        &blend_copy, &depth_copy, &rasterizer_copy, &pixel_copy, &vertex_copy);
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_sampler_state(
            device, CNA_SHADER_STAGE_PIXEL, UINT32_C(3), &custom_pixel) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_sampler_state(
                device, CNA_SHADER_STAGE_PIXEL, UINT32_C(3), &pixel_copy) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_sampler_state(
                device, CNA_SHADER_STAGE_VERTEX, UINT32_C(3), &vertex_copy) ==
                CNA_RESULT_SUCCESS &&
            sampler_equal(&pixel_copy, &custom_pixel) &&
            sampler_equal(&vertex_copy, &saved_vertex),
        "a distinctive pixel sampler round-trips without changing the matching vertex slot");

    custom_vertex = expected_sampler(
        CNA_TEXTURE_FILTER_LINEAR_MIP_POINT, CNA_TEXTURE_ADDRESS_WRAP);
    custom_vertex.address_v = CNA_TEXTURE_ADDRESS_MIRROR;
    custom_vertex.address_w = CNA_TEXTURE_ADDRESS_CLAMP;
    custom_vertex.max_anisotropy = 5;
    custom_vertex.max_mip_level = 2;
    custom_vertex.mip_map_level_of_detail_bias = -0.25f;
    initialize_output_prefixes(
        &blend_copy, &depth_copy, &rasterizer_copy, &pixel_copy, &vertex_copy);
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_sampler_state(
            device, CNA_SHADER_STAGE_VERTEX, UINT32_C(3), &custom_vertex) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_sampler_state(
                device, CNA_SHADER_STAGE_PIXEL, UINT32_C(3), &pixel_copy) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_sampler_state(
                device, CNA_SHADER_STAGE_VERTEX, UINT32_C(3), &vertex_copy) ==
                CNA_RESULT_SUCCESS &&
            sampler_equal(&pixel_copy, &custom_pixel) &&
            sampler_equal(&vertex_copy, &custom_vertex),
        "a distinctive vertex sampler round-trips while the pixel-stage slot stays independent");

    cna_demo_check(
        state->demo,
        cna_graphics_device_set_scissor_rectangle(
            device, (CNA_Rectangle){11, 13, 101, 79}) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_blend_factor(
                device, (CNA_Color){91U, 82U, 73U, 64U}) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_multi_sample_mask(device, INT32_C(0x2468ace0)) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_reference_stencil(device, 113) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_scissor_rectangle(device, &scissor_copy) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_blend_factor(device, &factor_copy) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_multi_sample_mask(device, &mask_copy) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_reference_stencil(device, &reference_copy) ==
                CNA_RESULT_SUCCESS &&
            rectangle_equal(scissor_copy, (CNA_Rectangle){11, 13, 101, 79}) &&
            color_equal(factor_copy, (CNA_Color){91U, 82U, 73U, 64U}) &&
            mask_copy == INT32_C(0x2468ace0) && reference_copy == 113,
        "distinctive scissor, blend-factor, sample-mask and stencil-reference values round-trip");

    restore_blend = cna_graphics_device_set_blend_state(device, &saved_blend);
    restore_depth = cna_graphics_device_set_depth_stencil_state(device, &saved_depth);
    restore_rasterizer = cna_graphics_device_set_rasterizer_state(device, &saved_rasterizer);
    restore_pixel = cna_graphics_device_set_sampler_state(
        device, CNA_SHADER_STAGE_PIXEL, UINT32_C(3), &saved_pixel);
    restore_vertex = cna_graphics_device_set_sampler_state(
        device, CNA_SHADER_STAGE_VERTEX, UINT32_C(3), &saved_vertex);
    cna_demo_check(
        state->demo,
        restore_blend == CNA_RESULT_SUCCESS && restore_depth == CNA_RESULT_SUCCESS &&
            restore_rasterizer == CNA_RESULT_SUCCESS && restore_pixel == CNA_RESULT_SUCCESS &&
            restore_vertex == CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_scissor_rectangle(device, saved_scissor) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_blend_factor(device, saved_factor) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_multi_sample_mask(device, saved_mask) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_reference_stencil(device, saved_reference) ==
                CNA_RESULT_SUCCESS,
        "restore all saved object, sampler, scalar and geometric states before leaving the callback");

    initialize_output_prefixes(
        &blend_copy, &depth_copy, &rasterizer_copy, &pixel_copy, &vertex_copy);
    cna_demo_check(
        state->demo,
        cna_graphics_device_get_blend_state(device, &blend_copy) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_depth_stencil_state(device, &depth_copy) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_rasterizer_state(device, &rasterizer_copy) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_sampler_state(
                device, CNA_SHADER_STAGE_PIXEL, UINT32_C(3), &pixel_copy) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_sampler_state(
                device, CNA_SHADER_STAGE_VERTEX, UINT32_C(3), &vertex_copy) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_scissor_rectangle(device, &scissor_copy) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_blend_factor(device, &factor_copy) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_multi_sample_mask(device, &mask_copy) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_reference_stencil(device, &reference_copy) ==
                CNA_RESULT_SUCCESS &&
            blend_equal(&blend_copy, &saved_blend) && depth_equal(&depth_copy, &saved_depth) &&
            rasterizer_equal(&rasterizer_copy, &saved_rasterizer) &&
            sampler_equal(&pixel_copy, &saved_pixel) &&
            sampler_equal(&vertex_copy, &saved_vertex) &&
            rectangle_equal(scissor_copy, saved_scissor) && color_equal(factor_copy, saved_factor) &&
            mask_copy == saved_mask && reference_copy == saved_reference,
        "read back every exact original device state after restoration");
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
    DeviceStateDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.device_states", argc, argv, 1);
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
        "create the graphics device whose active states are audited");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through load and the immediate exit request");
        cna_demo_check(&demo, state.loads == 1,
                       "the graphics-state audit ran in exactly one load callback");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
