/* SPDX-License-Identifier: MS-PL */
/*
 * Occlusion queries from C: enforce the Begin/End/result state machine, draw one full-NDC quad,
 * poll without stalling, interpret exact and boolean-only counts, and release the owned query.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct OcclusionState {
    CnaDemo* demo;
    CNA_OcclusionQueryHandle query;
    CNA_EffectHandle effect;
    CNA_Bool precise;
    int loads;
    int draws;
    int started;
    int finished;
    int query_destroyed;
} OcclusionState;

static CNA_UserPrimitives full_screen_quad(CNA_VertexPositionColor* vertices)
{
    static const CNA_Color red = {255U, 0U, 0U, 255U};
    CNA_UserPrimitives request;
    vertices[0] = (CNA_VertexPositionColor){{-1.0F, -1.0F, 0.5F}, red};
    vertices[1] = (CNA_VertexPositionColor){{-1.0F, 1.0F, 0.5F}, red};
    vertices[2] = (CNA_VertexPositionColor){{1.0F, -1.0F, 0.5F}, red};
    vertices[3] = (CNA_VertexPositionColor){{1.0F, 1.0F, 0.5F}, red};
    memset(&request, 0, sizeof(request));
    request.struct_size = (uint32_t)sizeof(request);
    request.struct_version = UINT32_C(1);
    request.primitive_type = CNA_PRIMITIVE_TRIANGLE_STRIP;
    request.vertex_source = CNA_USER_VERTEX_SOURCE_POSITION_COLOR;
    request.vertex_data = vertices;
    request.vertex_declaration = CNA_INVALID_HANDLE;
    request.vertex_offset = 0;
    request.num_vertices = 4;
    request.primitive_count = 2;
    return request;
}

static CNA_Result read_centre(CNA_Handle device, CNA_Color* out_color)
{
    CNA_BackBufferReadback readback = {
        sizeof(CNA_BackBufferReadback), UINT32_C(1), CNA_TRUE, {0U, 0U, 0U},
        {400, 240, 1, 1}, UINT64_C(0), UINT64_C(1)};
    memset(out_color, 0xA5, sizeof(*out_color));
    return cna_graphics_device_get_backbuffer_data_window(
        device, &readback, out_color, UINT64_C(1));
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    OcclusionState* state = (OcclusionState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Bool has_renderer = UINT32_C(7);
    CNA_Bool complete = UINT32_C(7);
    CNA_Matrix identity;
    CNA_RasterizerState rasterizer;
    CNA_BlendState blend;
    CNA_DepthStencilState depth;
    int32_t pixel_count = -73;
    CNA_Result pixel_result;
    CNA_Result end_result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    if (!cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &device),
            "borrow the live graphics device")) {
        return CNA_RESULT_SUCCESS;
    }
    if (!cna_demo_check_result(
            state->demo, cna_occlusion_query_create(device, &state->query),
            "create an owned HiDef occlusion query")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_occlusion_query_has_renderer(state->query, &has_renderer) == CNA_RESULT_SUCCESS &&
            has_renderer == CNA_TRUE &&
            cna_occlusion_query_get_is_pixel_count_precise_ext(
                state->query, &state->precise) == CNA_RESULT_SUCCESS &&
            (state->precise == CNA_FALSE || state->precise == CNA_TRUE),
        "the query owns a renderer and reports canonical count precision");
    cna_demo_check(
        state->demo,
        cna_occlusion_query_get_is_complete(state->query, &complete) == CNA_RESULT_SUCCESS &&
            complete == CNA_FALSE,
        "a fresh query has no completed result");
    pixel_result = cna_occlusion_query_get_pixel_count(state->query, &pixel_count);
    cna_demo_check(
        state->demo,
        pixel_result == CNA_RESULT_INVALID_STATE && pixel_count == -73,
        "pixel count before Begin is refused without changing the output");
    end_result = cna_occlusion_query_end(state->query);
    cna_demo_check(
        state->demo, end_result == CNA_RESULT_INVALID_STATE,
        "End without a matching Begin is refused");

    cna_demo_check(
        state->demo,
        cna_basic_effect_create(device, &state->effect) == CNA_RESULT_SUCCESS &&
            cna_matrix_get_identity(&identity) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_set_world(state->effect, identity) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_set_view(state->effect, identity) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_set_projection(state->effect, identity) == CNA_RESULT_SUCCESS &&
            cna_basic_effect_set_vertex_color_enabled(state->effect, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_rasterizer_state_init(
                CNA_RASTERIZER_STATE_PRESET_CULL_NONE, &rasterizer) == CNA_RESULT_SUCCESS &&
            cna_blend_state_init(CNA_BLEND_STATE_PRESET_OPAQUE, &blend) == CNA_RESULT_SUCCESS &&
            cna_depth_stencil_state_init(
                CNA_DEPTH_STENCIL_STATE_PRESET_NONE, &depth) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_rasterizer_state(device, &rasterizer) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_blend_state(device, &blend) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_depth_stencil_state(device, &depth) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_apply(state->effect) == CNA_RESULT_SUCCESS,
        "prepare an opaque cull-none full-screen vertex-color draw");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    OcclusionState* state = (OcclusionState*)context;
    (void)game_time;
    (void)out_error;
    if (state->finished) {
        return cna_game_request_exit(game);
    }
    if (state->demo->frames > 0 && state->draws >= state->demo->frames) {
        cna_demo_check(
            state->demo, 0,
            "the visible query completes within the bounded frame budget");
        state->finished = 1;
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    static const CNA_Color black = {0U, 0U, 0U, 255U};
    OcclusionState* state = (OcclusionState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_VertexPositionColor vertices[4];
    CNA_UserPrimitives request;
    CNA_Color pixel;
    CNA_Bool complete = CNA_FALSE;
    int32_t count = -1;
    CNA_Result nested_begin;
    CNA_Result repeated_end;
    CNA_Result premature_reuse;
    CNA_Result count_result;
    (void)game_time;
    (void)out_error;
    ++state->draws;
    if (cna_game_get_graphics_device(game, &device) != CNA_RESULT_SUCCESS) {
        return CNA_RESULT_INVALID_STATE;
    }

    if (!state->started) {
        request = full_screen_quad(vertices);
        cna_demo_check_result(
            state->demo, cna_graphics_device_clear_color_depth(device, black, 1.0F),
            "clear color and depth before the measured draw");
        nested_begin = cna_occlusion_query_begin(state->query);
        if (nested_begin == CNA_RESULT_SUCCESS) {
            nested_begin = cna_occlusion_query_begin(state->query);
        }
        cna_demo_check(
            state->demo, nested_begin == CNA_RESULT_INVALID_STATE,
            "Begin opens the query and a nested Begin is refused");
        cna_demo_check(
            state->demo,
            cna_graphics_device_draw_user_primitives(device, &request) ==
                    CNA_RESULT_SUCCESS &&
                cna_occlusion_query_end(state->query) == CNA_RESULT_SUCCESS,
            "the query encloses one full-screen user-primitive draw");
        repeated_end = cna_occlusion_query_end(state->query);
        premature_reuse = cna_occlusion_query_begin(state->query);
        cna_demo_check(
            state->demo,
            repeated_end == CNA_RESULT_INVALID_STATE &&
                premature_reuse == CNA_RESULT_INVALID_STATE,
            "repeated End and reuse before a completion observation are refused");
        cna_demo_check(
            state->demo,
            read_centre(device, &pixel) == CNA_RESULT_SUCCESS && pixel.r == 255U &&
                pixel.g == 0U && pixel.b == 0U && pixel.a == 255U,
            "the measured quad visibly covers the center pixel in exact red");
        state->started = 1;
    } else {
        /* Readback synchronizes pending GL commands without blocking inside the query getter. */
        (void)read_centre(device, &pixel);
    }

    if (cna_occlusion_query_get_is_complete(state->query, &complete) !=
        CNA_RESULT_SUCCESS) {
        return CNA_RESULT_INVALID_STATE;
    }
    if (complete == CNA_TRUE) {
        CNA_OcclusionQueryHandle stale = state->query;
        CNA_Bool stale_renderer = UINT32_C(9);
        cna_demo_check(
            state->demo, 1,
            "the visible query completes within the bounded frame budget");
        count_result = cna_occlusion_query_get_pixel_count(state->query, &count);
        if (count_result == CNA_RESULT_SUCCESS) {
            printf(
                "  query result: precise=%s, visible samples=%d\n",
                state->precise == CNA_TRUE ? "true" : "false", count);
        }
        cna_demo_check(
            state->demo,
            count_result == CNA_RESULT_SUCCESS &&
                ((state->precise == CNA_TRUE && count > 4000) ||
                 (state->precise == CNA_FALSE && count == 1)),
            "the result is a real visible-sample tally or the documented one-bit flag");
        cna_demo_check_result(
            state->demo, cna_occlusion_query_destroy(state->query),
            "destroy the owned query after reading its result");
        state->query_destroyed = 1;
        cna_demo_check(
            state->demo,
            cna_occlusion_query_destroy(stale) == CNA_RESULT_INVALID_HANDLE &&
                cna_occlusion_query_has_renderer(stale, &stale_renderer) ==
                    CNA_RESULT_INVALID_HANDLE &&
                stale_renderer == UINT32_C(9),
            "destroy is one-shot and stale queries preserve caller output");
        state->finished = 1;
    }
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    OcclusionState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.occlusion_query", argc, argv, 30);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.query = CNA_INVALID_HANDLE;
    state.effect = CNA_INVALID_HANDLE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.draw = draw;
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
    manager_ready = cna_graphics_device_manager_create(game, &manager) ==
            CNA_RESULT_SUCCESS &&
        cna_graphics_device_manager_set_graphics_profile(
            manager, CNA_GRAPHICS_PROFILE_HI_DEF) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_manager_set_preferred_back_buffer_width(manager, 800) ==
            CNA_RESULT_SUCCESS &&
        cna_graphics_device_manager_set_preferred_back_buffer_height(manager, 480) ==
            CNA_RESULT_SUCCESS &&
        cna_graphics_device_manager_set_preferred_depth_stencil_format(
            manager, CNA_DEPTH_FORMAT_DEPTH24_STENCIL8) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_manager_apply_changes(manager) == CNA_RESULT_SUCCESS;
    manager_ready = cna_demo_check(
        &demo, manager_ready,
        "create an explicit HiDef 800x480 Color/Depth24Stencil8 device");
    if (manager_ready) {
        cna_demo_check_result(
            &demo, cna_game_run(game),
            "run the bounded asynchronous query workflow");
        cna_demo_check(
            &demo, state.loads == 1 && state.draws >= 1 && state.finished,
            "the query audit used one load and at least one draw callback");
    }

    if (!state.query_destroyed && state.query != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_occlusion_query_destroy(state.query),
            "destroy the query during fallback cleanup");
    }
    if (state.effect != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_effect_destroy(state.effect), "destroy the BasicEffect owner");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
