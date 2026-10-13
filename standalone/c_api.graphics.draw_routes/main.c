/* SPDX-License-Identifier: MS-PL */
/*
 * Graphics-device draw routes from C. Bound, user-array, indexed and instanced geometry produce
 * pixel-proven results. Explicit current-effect, blending, depth-test and depth-write changes are
 * observed rather than treated as successful setters alone.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct DrawRouteState {
    CnaDemo* demo;
    CNA_VertexDeclarationHandle declaration;
    CNA_VertexBufferHandle vertices;
    CNA_IndexBufferHandle indices;
    CNA_EffectHandle effect;
    int loads;
    int draws;
} DrawRouteState;

static const CNA_Color Black = {0U, 0U, 0U, 255U};
static const CNA_Color Red = {255U, 0U, 0U, 255U};
static const CNA_Color Green = {0U, 255U, 0U, 255U};
static const CNA_Color Blue = {0U, 0U, 255U, 255U};
static const CNA_Color Yellow = {255U, 255U, 0U, 255U};
static const CNA_Color HalfRed = {255U, 0U, 0U, 128U};

static void make_quad(CNA_VertexPositionColor* out_vertices, float depth, CNA_Color color)
{
    out_vertices[0] = (CNA_VertexPositionColor){{-0.8F, 0.8F, depth}, color};
    out_vertices[1] = (CNA_VertexPositionColor){{-0.8F, -0.8F, depth}, color};
    out_vertices[2] = (CNA_VertexPositionColor){{0.8F, -0.8F, depth}, color};
    out_vertices[3] = (CNA_VertexPositionColor){{-0.8F, 0.8F, depth}, color};
    out_vertices[4] = (CNA_VertexPositionColor){{0.8F, -0.8F, depth}, color};
    out_vertices[5] = (CNA_VertexPositionColor){{0.8F, 0.8F, depth}, color};
}

static CNA_Result read_probe(CNA_Handle device, CNA_Color* out_color)
{
    CNA_BackBufferReadback readback = {
        sizeof(CNA_BackBufferReadback), UINT32_C(1), CNA_TRUE, {0U, 0U, 0U},
        {400, 200, 1, 1}, UINT64_C(0), UINT64_C(1)};
    memset(out_color, 0xA5, sizeof(*out_color));
    return cna_graphics_device_get_backbuffer_data_window(
        device, &readback, out_color, UINT64_C(1));
}

static int color_equal(CNA_Color left, CNA_Color right)
{
    return left.r == right.r && left.g == right.g && left.b == right.b &&
        left.a == right.a;
}

static CNA_UserPrimitives user_quad(
    const CNA_VertexPositionColor* vertices, int32_t vertex_offset)
{
    CNA_UserPrimitives value;
    memset(&value, 0, sizeof(value));
    value.struct_size = (uint32_t)sizeof(value);
    value.struct_version = UINT32_C(1);
    value.primitive_type = CNA_PRIMITIVE_TRIANGLE_LIST;
    value.vertex_source = CNA_USER_VERTEX_SOURCE_POSITION_COLOR;
    value.vertex_data = vertices;
    value.vertex_declaration = CNA_INVALID_HANDLE;
    value.vertex_offset = vertex_offset;
    value.num_vertices = 6;
    value.primitive_count = 2;
    return value;
}

static CNA_Result draw_user_quad(
    CNA_Handle device, float depth, CNA_Color color)
{
    CNA_VertexPositionColor vertices[6];
    CNA_UserPrimitives request;
    make_quad(vertices, depth, color);
    request = user_quad(vertices, 0);
    return cna_graphics_device_draw_user_primitives(device, &request);
}

static CNA_Result clear_frame(CNA_Handle device)
{
    return cna_graphics_device_clear_color_depth(device, Black, 1.0F);
}

static int create_buffers(DrawRouteState* state, CNA_Handle device)
{
    CNA_VertexElement elements[8];
    CNA_VertexBufferCreateInfo vertex_info;
    CNA_IndexBufferCreateInfo index_info;
    CNA_IndexBufferTransfer transfer;
    CNA_VertexPositionColor vertices[6];
    const uint16_t indices[6] = {0U, 1U, 2U, 3U, 4U, 5U};
    uint64_t element_count = 0U;
    uint32_t stride = 0U;
    make_quad(vertices, 0.5F, HalfRed);
    if (cna_vertex_type_get_stride(CNA_VERTEX_TYPE_POSITION_COLOR, &stride) !=
            CNA_RESULT_SUCCESS ||
        stride != sizeof(CNA_VertexPositionColor) ||
        cna_vertex_type_copy_elements(
            CNA_VERTEX_TYPE_POSITION_COLOR, elements,
            (uint64_t)(sizeof(elements) / sizeof(elements[0])), &element_count) !=
            CNA_RESULT_SUCCESS ||
        cna_vertex_declaration_create_with_stride(
            (int32_t)stride, elements, element_count, &state->declaration) !=
            CNA_RESULT_SUCCESS) {
        return 0;
    }

    memset(&vertex_info, 0, sizeof(vertex_info));
    vertex_info.struct_size = (uint32_t)sizeof(vertex_info);
    vertex_info.struct_version = UINT32_C(1);
    vertex_info.vertex_declaration = state->declaration;
    vertex_info.vertex_count = 6;
    vertex_info.buffer_usage = CNA_BUFFER_USAGE_NONE;
    if (cna_vertex_buffer_create(device, &vertex_info, &state->vertices) !=
            CNA_RESULT_SUCCESS ||
        cna_vertex_buffer_set_data_raw(
            state->vertices, vertices, sizeof(vertices), UINT64_C(6), stride) !=
            CNA_RESULT_SUCCESS) {
        return 0;
    }

    memset(&index_info, 0, sizeof(index_info));
    index_info.struct_size = (uint32_t)sizeof(index_info);
    index_info.struct_version = UINT32_C(1);
    index_info.index_count = 6;
    index_info.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    index_info.buffer_usage = CNA_BUFFER_USAGE_NONE;
    index_info.dynamic = CNA_FALSE;
    if (cna_index_buffer_create(device, &index_info, &state->indices) !=
        CNA_RESULT_SUCCESS) {
        return 0;
    }
    memset(&transfer, 0, sizeof(transfer));
    transfer.struct_size = (uint32_t)sizeof(transfer);
    transfer.struct_version = UINT32_C(1);
    transfer.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    transfer.options = CNA_SET_DATA_NONE;
    transfer.element_count = UINT64_C(6);
    return cna_index_buffer_set_data(
               state->indices, &transfer, indices, UINT64_C(6)) == CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    DrawRouteState* state = (DrawRouteState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Matrix identity;
    CNA_RasterizerState rasterizer;
    CNA_BlendState blend;
    CNA_DepthStencilState depth;
    int32_t count = -1;
    int helper_ok;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    if (!cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &device),
            "borrow the live graphics device")) {
        return CNA_RESULT_SUCCESS;
    }
    helper_ok =
        cna_primitive_type_get_vertex_count(
            CNA_PRIMITIVE_TRIANGLE_LIST, 2, &count) == CNA_RESULT_SUCCESS && count == 6 &&
        cna_primitive_type_get_vertex_count(
            CNA_PRIMITIVE_TRIANGLE_STRIP, 2, &count) == CNA_RESULT_SUCCESS && count == 4 &&
        cna_primitive_type_get_vertex_count(
            CNA_PRIMITIVE_LINE_LIST, 3, &count) == CNA_RESULT_SUCCESS && count == 6 &&
        cna_primitive_type_get_vertex_count(
            CNA_PRIMITIVE_LINE_STRIP, 3, &count) == CNA_RESULT_SUCCESS && count == 4 &&
        cna_primitive_type_get_vertex_count(
            CNA_PRIMITIVE_POINT_LIST_EXT, 5, &count) == CNA_RESULT_SUCCESS && count == 5;
    cna_demo_check(
        state->demo, helper_ok,
        "the primitive helper returns exact element counts for all five topologies");

    cna_demo_check(
        state->demo, create_buffers(state, device),
        "create and fill the bound vertex and index buffers");
    cna_demo_check(
        state->demo,
        cna_basic_effect_create(device, &state->effect) == CNA_RESULT_SUCCESS &&
            cna_matrix_get_identity(&identity) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_set_world(state->effect, identity) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_set_view(state->effect, identity) == CNA_RESULT_SUCCESS &&
            cna_effect_matrices_set_projection(state->effect, identity) == CNA_RESULT_SUCCESS &&
            cna_basic_effect_set_vertex_color_enabled(state->effect, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_apply(state->effect) == CNA_RESULT_SUCCESS,
        "create and apply an identity BasicEffect that consumes vertex color");
    cna_demo_check(
        state->demo,
        cna_rasterizer_state_init(
            CNA_RASTERIZER_STATE_PRESET_CULL_NONE, &rasterizer) == CNA_RESULT_SUCCESS &&
            cna_blend_state_init(CNA_BLEND_STATE_PRESET_OPAQUE, &blend) ==
                CNA_RESULT_SUCCESS &&
            cna_depth_stencil_state_init(CNA_DEPTH_STENCIL_STATE_PRESET_DEFAULT, &depth) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_rasterizer_state(device, &rasterizer) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_blend_state(device, &blend) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_depth_stencil_state(device, &depth) == CNA_RESULT_SUCCESS,
        "install explicit cull-none, opaque and default-depth state");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    DrawRouteState* state = (DrawRouteState*)context;
    (void)game_time;
    (void)out_error;
    if (state->draws != 0) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    DrawRouteState* state = (DrawRouteState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Color pixel;
    CNA_VertexPositionColor user_vertices[7];
    CNA_VertexPositionColor indexed_vertices[4];
    const uint16_t user_indices[7] = {UINT16_C(99), 0U, 1U, 2U, 0U, 2U, 3U};
    CNA_UserPrimitives user;
    CNA_UserPrimitives indexed;
    CNA_UserIndices index_request;
    CNA_Result refused;
    int setup_ok;
    int draw_ok;
    (void)game_time;
    (void)out_error;
    ++state->draws;
    if (cna_game_get_graphics_device(game, &device) != CNA_RESULT_SUCCESS ||
        state->effect == CNA_INVALID_HANDLE || state->vertices == CNA_INVALID_HANDLE ||
        state->indices == CNA_INVALID_HANDLE) {
        return CNA_RESULT_INVALID_STATE;
    }

    make_quad(&user_vertices[1], 0.5F, Blue);
    user_vertices[0] = (CNA_VertexPositionColor){{-1.0F, -1.0F, 0.5F}, Green};
    user = user_quad(user_vertices, 1);
    setup_ok = clear_frame(device) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_current_effect(device, CNA_INVALID_HANDLE) ==
            CNA_RESULT_SUCCESS;
    refused = cna_graphics_device_draw_user_primitives(device, &user);
    draw_ok = setup_ok && refused == CNA_RESULT_INVALID_STATE &&
        read_probe(device, &pixel) == CNA_RESULT_SUCCESS && color_equal(pixel, Black) &&
        cna_graphics_device_set_current_effect(device, state->effect) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_draw_user_primitives(device, &user) == CNA_RESULT_SUCCESS &&
        read_probe(device, &pixel) == CNA_RESULT_SUCCESS && color_equal(pixel, Blue);
    cna_demo_check(
        state->demo, draw_ok,
        "an explicit current effect changes user-array drawing from refused/no pixels to exact blue pixels");

    draw_ok = clear_frame(device) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_vertex_buffer(device, state->vertices) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_draw_primitives(
            device, CNA_PRIMITIVE_TRIANGLE_LIST, 0, 2) == CNA_RESULT_SUCCESS &&
        read_probe(device, &pixel) == CNA_RESULT_SUCCESS &&
        pixel.r == 255U && pixel.g == 0U && pixel.b == 0U;
    cna_demo_check(
        state->demo, draw_ok,
        "the bound non-indexed route draws its complete two-triangle red quad");

    indexed_vertices[0] = (CNA_VertexPositionColor){{-0.8F, 0.8F, 0.5F}, Yellow};
    indexed_vertices[1] = (CNA_VertexPositionColor){{-0.8F, -0.8F, 0.5F}, Yellow};
    indexed_vertices[2] = (CNA_VertexPositionColor){{0.8F, -0.8F, 0.5F}, Yellow};
    indexed_vertices[3] = (CNA_VertexPositionColor){{0.8F, 0.8F, 0.5F}, Yellow};
    indexed = user_quad(indexed_vertices, 0);
    indexed.num_vertices = 4;
    memset(&index_request, 0, sizeof(index_request));
    index_request.struct_size = (uint32_t)sizeof(index_request);
    index_request.struct_version = UINT32_C(1);
    index_request.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    index_request.index_offset = 1;
    index_request.index_data = user_indices;
    draw_ok = clear_frame(device) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_draw_user_indexed_primitives(
            device, &indexed, &index_request) == CNA_RESULT_SUCCESS &&
        read_probe(device, &pixel) == CNA_RESULT_SUCCESS && color_equal(pixel, Yellow);
    cna_demo_check(
        state->demo, draw_ok,
        "the caller-indexed route honors its nonzero index offset and draws exact yellow");

    draw_ok = clear_frame(device) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_vertex_buffer(device, state->vertices) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_index_buffer(device, state->indices) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_blend_enabled(device, CNA_TRUE) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_draw_instanced_primitives(
            device, CNA_PRIMITIVE_TRIANGLE_LIST, 0, 0, 6, 0, 2, 2) ==
            CNA_RESULT_SUCCESS &&
        read_probe(device, &pixel) == CNA_RESULT_SUCCESS &&
        pixel.r >= 185U && pixel.r <= 195U && pixel.g == 0U && pixel.b == 0U;
    printf("  instanced blend probe: %u,%u,%u,%u\n", (unsigned)pixel.r,
           (unsigned)pixel.g, (unsigned)pixel.b, (unsigned)pixel.a);
    cna_demo_check(
        state->demo, draw_ok,
        "two identical half-alpha instances accumulate to the expected three-quarter red pixel");
    draw_ok = clear_frame(device) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_blend_enabled(device, CNA_FALSE) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_draw_instanced_primitives(
            device, CNA_PRIMITIVE_TRIANGLE_LIST, 0, 0, 6, 0, 2, 2) ==
            CNA_RESULT_SUCCESS &&
        read_probe(device, &pixel) == CNA_RESULT_SUCCESS &&
        pixel.r == 255U && pixel.g == 0U && pixel.b == 0U;
    cna_demo_check(
        state->demo, draw_ok,
        "disabling blending makes the same instanced source overwrite the pixel with full red");

    draw_ok = clear_frame(device) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_depth_test_enabled(device, CNA_TRUE) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_depth_write_enabled(device, CNA_TRUE) == CNA_RESULT_SUCCESS &&
        draw_user_quad(device, 0.2F, Red) == CNA_RESULT_SUCCESS &&
        draw_user_quad(device, 0.8F, Blue) == CNA_RESULT_SUCCESS &&
        read_probe(device, &pixel) == CNA_RESULT_SUCCESS && color_equal(pixel, Red);
    cna_demo_check(
        state->demo, draw_ok,
        "enabled depth testing and writes keep the first nearer red surface");
    draw_ok = clear_frame(device) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_depth_test_enabled(device, CNA_FALSE) == CNA_RESULT_SUCCESS &&
        draw_user_quad(device, 0.2F, Red) == CNA_RESULT_SUCCESS &&
        draw_user_quad(device, 0.8F, Blue) == CNA_RESULT_SUCCESS &&
        read_probe(device, &pixel) == CNA_RESULT_SUCCESS && color_equal(pixel, Blue);
    cna_demo_check(
        state->demo, draw_ok,
        "disabling depth testing lets the later farther blue surface overwrite red");
    draw_ok = clear_frame(device) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_depth_test_enabled(device, CNA_TRUE) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_depth_write_enabled(device, CNA_FALSE) == CNA_RESULT_SUCCESS &&
        draw_user_quad(device, 0.2F, Red) == CNA_RESULT_SUCCESS &&
        draw_user_quad(device, 0.8F, Blue) == CNA_RESULT_SUCCESS &&
        read_probe(device, &pixel) == CNA_RESULT_SUCCESS && color_equal(pixel, Blue);
    cna_demo_check(
        state->demo, draw_ok,
        "disabling depth writes leaves the clear depth so the later farther surface still passes");

    setup_ok = cna_graphics_device_set_depth_write_enabled(device, CNA_TRUE) ==
            CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_depth_test_enabled(device, CNA_TRUE) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_blend_enabled(device, CNA_FALSE) == CNA_RESULT_SUCCESS;
    refused = setup_ok
        ? cna_graphics_device_set_depth_test_enabled(device, UINT32_C(2))
        : CNA_RESULT_INVALID_STATE;
    draw_ok = setup_ok && refused == CNA_RESULT_INVALID_ARGUMENT &&
        cna_graphics_device_set_depth_write_enabled(device, UINT32_C(2)) ==
            CNA_RESULT_INVALID_ARGUMENT &&
        cna_graphics_device_set_blend_enabled(device, UINT32_C(2)) ==
            CNA_RESULT_INVALID_ARGUMENT &&
        clear_frame(device) == CNA_RESULT_SUCCESS &&
        draw_user_quad(device, 0.2F, Green) == CNA_RESULT_SUCCESS &&
        draw_user_quad(device, 0.8F, Blue) == CNA_RESULT_SUCCESS &&
        read_probe(device, &pixel) == CNA_RESULT_SUCCESS && color_equal(pixel, Green);
    cna_demo_check(
        state->demo, draw_ok,
        "undefined Boolean identities are refused while the restored opaque depth policy stays active");

    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    DrawRouteState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.draw_routes", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.declaration = CNA_INVALID_HANDLE;
    state.vertices = CNA_INVALID_HANDLE;
    state.indices = CNA_INVALID_HANDLE;
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
    manager_ready = cna_graphics_device_manager_create(game, &manager) == CNA_RESULT_SUCCESS &&
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
            "run the complete draw-route workflow and immediate exit request");
        cna_demo_check(
            &demo, state.loads == 1 && state.draws == 1,
            "the draw-route audit used exactly one load and one draw callback");
    }

    if (state.effect != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_effect_destroy(state.effect), "destroy the BasicEffect owner");
    }
    if (state.indices != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_index_buffer_destroy(state.indices), "destroy the index-buffer owner");
    }
    if (state.vertices != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_vertex_buffer_destroy(state.vertices), "destroy the vertex-buffer owner");
    }
    if (state.declaration != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_vertex_declaration_destroy(state.declaration),
            "destroy the vertex-declaration owner");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
