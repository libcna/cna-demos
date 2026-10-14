/* SPDX-License-Identifier: MS-PL */
/* DebugDraw's public C geometry, depth-list routing and frame lifecycle. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

enum { MAX_DEBUG_VERTICES = 846 };

typedef struct DebugDrawState {
    CnaDemo* demo;
    int loads;
} DebugDrawState;

static CNA_Matrix identity_matrix(void)
{
    CNA_Matrix value;
    memset(&value, 0, sizeof(value));
    value.m11 = 1.0F;
    value.m22 = 1.0F;
    value.m33 = 1.0F;
    value.m44 = 1.0F;
    return value;
}

static int vector_is(CNA_Vector3 value, float x, float y, float z)
{
    return value.x == x && value.y == y && value.z == z;
}

static int color_is(CNA_Color value, CNA_Color expected)
{
    return value.r == expected.r && value.g == expected.g &&
           value.b == expected.b && value.a == expected.a;
}

static void check_list_routing(DebugDrawState* state, CNA_DebugDrawHandle debug,
                               const CNA_Matrix* identity)
{
    const CNA_Vector3 depth_from = {0.0F, 0.0F, 0.0F};
    const CNA_Vector3 depth_to = {1.0F, 2.0F, 3.0F};
    const CNA_Vector3 overlay_from = {-1.0F, 0.0F, 0.0F};
    const CNA_Vector3 overlay_to = {0.0F, -2.0F, 0.0F};
    const CNA_Color red = {255U, 10U, 20U, 255U};
    const CNA_Color green = {30U, 240U, 50U, 255U};
    CNA_VertexPositionColor depth_vertices[4];
    CNA_VertexPositionColor overlay_vertices[2];
    CNA_VertexPositionColor before[4];
    CNA_Bool depth_tested = CNA_TRUE;
    uint64_t count = UINT64_C(99);
    int32_t lines = -1;

    cna_demo_check(
        state->demo,
        cna_debug_draw_end(debug) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_end(debug) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 0,
        "ending without an open DebugDraw frame is an idempotent empty no-op");
    cna_demo_check(
        state->demo,
        cna_debug_draw_set_depth_tested(debug, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_is_depth_tested(debug, &depth_tested) == CNA_RESULT_SUCCESS &&
            depth_tested == CNA_FALSE &&
            cna_debug_draw_begin(debug, identity, identity) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_is_depth_tested(debug, &depth_tested) == CNA_RESULT_SUCCESS &&
            depth_tested == CNA_TRUE &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 0,
        "Begin clears both lists and restores depth-tested submission mode");
    cna_demo_check(
        state->demo,
        cna_debug_draw_begin(debug, 0, identity) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_begin(debug, identity, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 0,
        "null view and projection matrices are refused without changing the open empty frame");

    cna_demo_check(
        state->demo,
        cna_debug_draw_add_line(debug, &depth_from, &depth_to, red) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_set_depth_tested(debug, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_add_line(debug, &overlay_from, &overlay_to, green) ==
                CNA_RESULT_SUCCESS &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 2,
        "line submission routes one segment into each depth mode within the same frame");

    cna_demo_check(
        state->demo,
        cna_debug_draw_copy_vertices(debug, CNA_TRUE, 0, 0U, &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL && count == 2U &&
            cna_debug_draw_copy_vertices(debug, CNA_FALSE, 0, 0U, &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL && count == 2U,
        "count-only vertex queries report two vertices for each independently routed list");

    memset(depth_vertices, 0xA5, sizeof(depth_vertices));
    memcpy(before, depth_vertices, sizeof(before));
    cna_demo_check(
        state->demo,
        cna_debug_draw_copy_vertices(debug, CNA_TRUE, depth_vertices, 1U, &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL && count == 2U &&
            memcmp(depth_vertices, before, sizeof(depth_vertices)) == 0,
        "a short DebugDraw vertex destination reports the need without a partial write");

    cna_demo_check(
        state->demo,
        cna_debug_draw_copy_vertices(debug, CNA_TRUE, depth_vertices, 4U, &count) ==
                CNA_RESULT_SUCCESS && count == 2U &&
            vector_is(depth_vertices[0].position, 0.0F, 0.0F, 0.0F) &&
            vector_is(depth_vertices[1].position, 1.0F, 2.0F, 3.0F) &&
            color_is(depth_vertices[0].color, red) && color_is(depth_vertices[1].color, red) &&
            cna_debug_draw_copy_vertices(debug, CNA_FALSE, overlay_vertices, 2U, &count) ==
                CNA_RESULT_SUCCESS && count == 2U &&
            vector_is(overlay_vertices[0].position, -1.0F, 0.0F, 0.0F) &&
            vector_is(overlay_vertices[1].position, 0.0F, -2.0F, 0.0F) &&
            color_is(overlay_vertices[0].color, green) &&
            color_is(overlay_vertices[1].color, green),
        "copied depth and overlay vertices retain exact submission order, positions and colors");

    cna_demo_check(
        state->demo,
        cna_debug_draw_set_depth_tested(debug, UINT32_C(2)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_copy_vertices(debug, UINT32_C(2), 0, 0U, &count) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_copy_vertices(debug, CNA_TRUE, 0, 1U, &count) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_copy_vertices(debug, CNA_TRUE, depth_vertices, 4U, 0) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_is_depth_tested(debug, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_get_line_count(debug, 0) == CNA_RESULT_INVALID_ARGUMENT,
        "noncanonical list selection and invalid copy/state outputs are refused");

    cna_demo_check(
        state->demo,
        cna_debug_draw_add_line(debug, 0, &depth_to, red) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_add_line(debug, &depth_from, 0, red) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_add_box(debug, 0, red) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_add_sphere(debug, 0, 1.0F, red, 8) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_add_bounding_sphere(debug, 0, red, 8) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_add_cross(debug, 0, 1.0F, red) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 2,
        "null shape inputs are refused without changing either queued line list");

    cna_demo_check(
        state->demo,
        cna_debug_draw_clear(debug) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 0 &&
            cna_debug_draw_is_depth_tested(debug, &depth_tested) == CNA_RESULT_SUCCESS &&
            depth_tested == CNA_FALSE &&
            cna_debug_draw_add_cross(debug, &depth_to, 0.5F, green) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 3,
        "Clear empties both lists while leaving the frame open and preserving overlay mode");
}

static void check_shape_geometry(DebugDrawState* state, CNA_DebugDrawHandle debug,
                                 const CNA_Matrix* identity)
{
    const CNA_BoundingBox box = {{-1.0F, -2.0F, -3.0F}, {4.0F, 5.0F, 6.0F}};
    const CNA_Vector3 sphere_center = {1.0F, 2.0F, 3.0F};
    const CNA_BoundingSphere sphere = {{5.0F, 6.0F, 7.0F}, 2.0F};
    const CNA_BoundingFrustum frustum = {*identity};
    const CNA_Vector3 cross = {10.0F, 20.0F, 30.0F};
    const CNA_Color yellow = {255U, 255U, 0U, 255U};
    const CNA_Color blue = {0U, 40U, 255U, 255U};
    const CNA_Color white = {255U, 255U, 255U, 255U};
    CNA_VertexPositionColor vertices[MAX_DEBUG_VERTICES];
    uint64_t count = 0U;
    int32_t lines = -1;
    int geometry_ok;
    int index;

    cna_demo_check(
        state->demo,
        cna_debug_draw_begin(debug, identity, identity) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_add_box(debug, &box, yellow) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 12,
        "a box contributes exactly its twelve wireframe edges");
    cna_demo_check(
        state->demo,
        cna_debug_draw_add_sphere(debug, &sphere_center, 1.0F, blue, 0) ==
                CNA_RESULT_SUCCESS &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 24,
        "a sphere below the segment minimum is clamped to three four-segment rings");
    cna_demo_check(
        state->demo,
        cna_debug_draw_add_bounding_sphere(debug, &sphere, blue, 999) ==
                CNA_RESULT_SUCCESS &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 408,
        "the BoundingSphere overload clamps to three 128-segment rings");
    cna_demo_check(
        state->demo,
        cna_debug_draw_add_frustum(debug, frustum, yellow) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 420,
        "an identity-matrix frustum contributes twelve edges");
    cna_demo_check(
        state->demo,
        cna_debug_draw_add_cross(debug, &cross, 2.0F, white) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 423 &&
            cna_debug_draw_copy_vertices(debug, CNA_TRUE, vertices, MAX_DEBUG_VERTICES, &count) ==
                CNA_RESULT_SUCCESS && count == MAX_DEBUG_VERTICES,
        "a cross adds three arms and the complete shape batch copies as 846 vertices");

    geometry_ok =
        vector_is(vertices[MAX_DEBUG_VERTICES - 6].position, 8.0F, 20.0F, 30.0F) &&
        vector_is(vertices[MAX_DEBUG_VERTICES - 5].position, 12.0F, 20.0F, 30.0F) &&
        vector_is(vertices[MAX_DEBUG_VERTICES - 4].position, 10.0F, 18.0F, 30.0F) &&
        vector_is(vertices[MAX_DEBUG_VERTICES - 3].position, 10.0F, 22.0F, 30.0F) &&
        vector_is(vertices[MAX_DEBUG_VERTICES - 2].position, 10.0F, 20.0F, 28.0F) &&
        vector_is(vertices[MAX_DEBUG_VERTICES - 1].position, 10.0F, 20.0F, 32.0F);
    for (index = MAX_DEBUG_VERTICES - 6; geometry_ok && index < MAX_DEBUG_VERTICES; ++index) {
        geometry_ok = color_is(vertices[index].color, white);
    }
    cna_demo_check(state->demo, geometry_ok,
                   "the final copied cross has three exact centered axes and preserves its color");

    geometry_ok = 1;
    for (index = 48; geometry_ok && index < 48 + 768; ++index) {
        const float dx = vertices[index].position.x - sphere.center.x;
        const float dy = vertices[index].position.y - sphere.center.y;
        const float dz = vertices[index].position.z - sphere.center.z;
        geometry_ok = fabsf(sqrtf(dx * dx + dy * dy + dz * dz) - sphere.radius) < 0.001F;
    }
    cna_demo_check(state->demo, geometry_ok,
                   "every max-clamped BoundingSphere vertex remains on the authored radius");

    cna_demo_check(
        state->demo,
        cna_debug_draw_end(debug) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_get_line_count(debug, &lines) == CNA_RESULT_SUCCESS && lines == 0 &&
            cna_debug_draw_end(debug) == CNA_RESULT_SUCCESS,
        "End issues the real line draw, clears the batch and stays idempotent afterwards");
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    DebugDrawState* state = (DebugDrawState*)context;
    const CNA_Matrix identity = identity_matrix();
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_DebugDrawHandle debug = CNA_INVALID_HANDLE;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "get the device for C DebugDraw")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_debug_draw_create(device, 0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_debug_draw_create(CNA_INVALID_HANDLE, &debug) == CNA_RESULT_INVALID_HANDLE &&
            debug == CNA_INVALID_HANDLE,
        "DebugDraw creation rejects null output and invalid-device inputs atomically");
    if (!cna_demo_check_result(state->demo, cna_debug_draw_create(device, &debug),
                               "create the owned C DebugDraw renderer")) {
        return CNA_RESULT_SUCCESS;
    }

    check_list_routing(state, debug, &identity);
    check_shape_geometry(state, debug, &identity);
    cna_demo_check(
        state->demo,
        cna_debug_draw_destroy(debug) == CNA_RESULT_SUCCESS &&
            cna_debug_draw_destroy(debug) == CNA_RESULT_INVALID_HANDLE &&
            cna_debug_draw_get_line_count(debug, &(int32_t){-1}) == CNA_RESULT_INVALID_HANDLE,
        "DebugDraw destruction releases the owned handle once and invalidates later access");
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    DebugDrawState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.graphics.debug_draw", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    if (cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager),
                              "create a HiDef graphics device")) {
        cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                              "run the C DebugDraw geometry callback");
    }
    cna_demo_check(&demo, state.loads == 1, "the C DebugDraw callback ran exactly once");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "cna_graphics_device_manager_destroy");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
