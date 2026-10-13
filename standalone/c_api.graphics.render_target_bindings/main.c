/* SPDX-License-Identifier: MS-PL */
/*
 * Render-target state and binding arrays from C. Matching 2D/cube targets form a real two-entry
 * MRT array, whose exact normalized binding state is copied back atomically. A single cube face
 * then replaces it before an empty array restores the backbuffer. ContentLost subscriptions are
 * valid but silent on the current non-device-losing Linux renderers.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define TARGET_SIZE 16U

typedef struct RenderTargetBindingState {
    CnaDemo* demo;
    CNA_Handle target_2d;
    CNA_Handle target_cube;
    CNA_RenderTargetEventRegistrationHandle registration;
    int content_lost_events;
    int loads;
} RenderTargetBindingState;

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

static int binding_equals(const CNA_RenderTargetBinding* binding, CNA_Handle target,
                          CNA_CubeMapFace face)
{
    return binding->struct_size == sizeof(*binding) &&
        binding->struct_version == UINT32_C(1) && binding->render_target == target &&
        binding->array_slice == 0 && binding->cube_map_face == face;
}

static void content_lost(CNA_Handle render_target, void* context)
{
    RenderTargetBindingState* state = (RenderTargetBindingState*)context;
    if (render_target == state->target_cube) {
        ++state->content_lost_events;
    }
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    RenderTargetBindingState* state = (RenderTargetBindingState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_RenderTarget2DCreateInfo create_2d;
    CNA_RenderTargetCubeCreateInfo create_cube;
    CNA_RenderTargetInfo info_2d;
    CNA_RenderTargetInfo info_cube;
    CNA_RenderTargetBinding requested[2];
    CNA_RenderTargetBinding copied[2];
    CNA_RenderTargetBinding short_copy;
    CNA_RenderTargetBinding cube_copy;
    CNA_Bool discard_preserves = CNA_TRUE;
    CNA_Bool preserve_preserves = CNA_FALSE;
    CNA_Bool platform_preserves = CNA_FALSE;
    CNA_Result result;
    CNA_Result unsubscribe_result;
    CNA_Result second_unsubscribe_result;
    uint64_t count;
    int targets_ready;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &graphics_device),
                               "borrow the live graphics device")) {
        return CNA_RESULT_SUCCESS;
    }

    cna_demo_check(
        state->demo,
        cna_render_target_usage_preserves_contents(
            CNA_RENDER_TARGET_USAGE_DISCARD_CONTENTS, &discard_preserves) == CNA_RESULT_SUCCESS &&
            discard_preserves == CNA_FALSE &&
            cna_render_target_usage_preserves_contents(
                CNA_RENDER_TARGET_USAGE_PRESERVE_CONTENTS, &preserve_preserves) ==
                CNA_RESULT_SUCCESS &&
            preserve_preserves == CNA_TRUE &&
            cna_render_target_usage_preserves_contents(
                CNA_RENDER_TARGET_USAGE_PLATFORM_CONTENTS, &platform_preserves) ==
                CNA_RESULT_SUCCESS &&
            platform_preserves == CNA_TRUE,
        "the three usage identities distinguish discard from both preservation policies");

    memset(&create_2d, 0, sizeof(create_2d));
    create_2d.struct_size = (uint32_t)sizeof(create_2d);
    create_2d.struct_version = UINT32_C(1);
    create_2d.width = TARGET_SIZE;
    create_2d.height = TARGET_SIZE;
    create_2d.mip_map = CNA_FALSE;
    create_2d.format = CNA_SURFACE_FORMAT_COLOR;
    create_2d.depth_format = CNA_DEPTH_FORMAT_NONE;
    create_2d.multi_sample_count = 0;
    create_2d.usage = CNA_RENDER_TARGET_USAGE_PRESERVE_CONTENTS;
    memset(&create_cube, 0, sizeof(create_cube));
    create_cube.struct_size = (uint32_t)sizeof(create_cube);
    create_cube.struct_version = UINT32_C(1);
    create_cube.size = TARGET_SIZE;
    create_cube.mip_map = CNA_FALSE;
    create_cube.format = CNA_SURFACE_FORMAT_COLOR;
    create_cube.depth_format = CNA_DEPTH_FORMAT_NONE;
    create_cube.multi_sample_count = 0;
    create_cube.usage = CNA_RENDER_TARGET_USAGE_DISCARD_CONTENTS;

    result = cna_render_target2d_create(graphics_device, &create_2d, &state->target_2d);
    cna_demo_check(state->demo, result == CNA_RESULT_SUCCESS,
                   "create the 2D member of the binding array");
    result = cna_render_target_cube_create(graphics_device, &create_cube, &state->target_cube);
    cna_demo_check(state->demo, result == CNA_RESULT_SUCCESS,
                   "create the matching cube render target");
    targets_ready = state->target_2d != CNA_INVALID_HANDLE &&
        state->target_cube != CNA_INVALID_HANDLE;

    memset(&info_2d, 0, sizeof(info_2d));
    info_2d.struct_size = (uint32_t)sizeof(info_2d);
    info_2d.struct_version = UINT32_C(1);
    memset(&info_cube, 0, sizeof(info_cube));
    info_cube.struct_size = (uint32_t)sizeof(info_cube);
    info_cube.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        targets_ready &&
            cna_render_target_get_info(state->target_2d, &info_2d) == CNA_RESULT_SUCCESS &&
            info_2d.kind == CNA_RENDER_TARGET_KIND_2D && info_2d.width == TARGET_SIZE &&
            info_2d.height == TARGET_SIZE && info_2d.level_count == 1U &&
            info_2d.format == CNA_SURFACE_FORMAT_COLOR &&
            info_2d.depth_format == CNA_DEPTH_FORMAT_NONE && info_2d.multi_sample_count == 0 &&
            info_2d.usage == CNA_RENDER_TARGET_USAGE_PRESERVE_CONTENTS &&
            info_2d.is_content_lost == CNA_FALSE && info_2d.renderer_available == CNA_TRUE &&
            info_2d.reserved[0] == 0U && info_2d.reserved[1] == 0U,
        "the 2D target reports its complete applied state and bindable storage");
    cna_demo_check(
        state->demo,
        targets_ready &&
            cna_render_target_get_info(state->target_cube, &info_cube) == CNA_RESULT_SUCCESS &&
            info_cube.kind == CNA_RENDER_TARGET_KIND_CUBE && info_cube.width == TARGET_SIZE &&
            info_cube.height == TARGET_SIZE && info_cube.level_count == 1U &&
            info_cube.format == CNA_SURFACE_FORMAT_COLOR &&
            info_cube.depth_format == CNA_DEPTH_FORMAT_NONE &&
            info_cube.multi_sample_count == 0 &&
            info_cube.usage == CNA_RENDER_TARGET_USAGE_DISCARD_CONTENTS &&
            info_cube.is_content_lost == CNA_FALSE && info_cube.renderer_available == CNA_TRUE &&
            info_cube.reserved[0] == 0U && info_cube.reserved[1] == 0U,
        "the cube target reports its complete applied state and bindable storage");

    state->registration = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        targets_ready &&
            cna_render_target_subscribe_content_lost(
                state->target_cube, content_lost, state, &state->registration) ==
                CNA_RESULT_SUCCESS &&
            state->registration != CNA_INVALID_HANDLE && state->content_lost_events == 0,
        "subscribe to the cube target's renderer-specific ContentLost event");

    requested[0] = (CNA_RenderTargetBinding){
        sizeof(CNA_RenderTargetBinding), UINT32_C(1), state->target_2d, 0,
        CNA_CUBE_MAP_FACE_POSITIVE_X};
    requested[1] = (CNA_RenderTargetBinding){
        sizeof(CNA_RenderTargetBinding), UINT32_C(1), state->target_cube, 0,
        CNA_CUBE_MAP_FACE_NEGATIVE_Z};
    result = targets_ready
        ? cna_graphics_device_set_render_targets(graphics_device, requested, UINT64_C(2))
        : CNA_RESULT_INVALID_STATE;
    cna_demo_check(state->demo, result == CNA_RESULT_SUCCESS,
                   "bind the 2D target and one cube face as a two-entry MRT array");

    count = UINT64_C(0);
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_render_target_count(graphics_device, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "the active binding count reports both MRT entries");

    memset(&short_copy, 0x6B, sizeof(short_copy));
    count = UINT64_C(99);
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_SUCCESS &&
            cna_graphics_device_copy_render_targets(
                graphics_device, &short_copy, UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) && all_bytes_are(&short_copy, sizeof(short_copy), 0x6B),
        "a short binding array reports the complete count without a partial write");

    memset(copied, 0x5A, sizeof(copied));
    count = UINT64_C(0);
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_SUCCESS &&
            cna_graphics_device_copy_render_targets(
                graphics_device, copied, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            binding_equals(&copied[0], state->target_2d, CNA_CUBE_MAP_FACE_POSITIVE_X) &&
            binding_equals(&copied[1], state->target_cube, CNA_CUBE_MAP_FACE_NEGATIVE_Z),
        "an exact copy preserves both normalized MRT bindings in order");

    result = targets_ready
        ? cna_graphics_device_set_render_target_cube(
              graphics_device, state->target_cube, CNA_CUBE_MAP_FACE_NEGATIVE_Y)
        : CNA_RESULT_INVALID_STATE;
    memset(&cube_copy, 0, sizeof(cube_copy));
    count = UINT64_C(0);
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_render_target_count(graphics_device, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_graphics_device_copy_render_targets(
                graphics_device, &cube_copy, UINT64_C(1), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            binding_equals(&cube_copy, state->target_cube, CNA_CUBE_MAP_FACE_NEGATIVE_Y),
        "a single cube-face bind replaces the MRT array and round-trips its selected face");

    count = UINT64_C(77);
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_render_targets(graphics_device, NULL, UINT64_C(0)) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_render_target_count(graphics_device, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_graphics_device_copy_render_targets(
                graphics_device, NULL, UINT64_C(0), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0),
        "an empty binding array restores the backbuffer and copies as an exact empty collection");

    unsubscribe_result = state->registration != CNA_INVALID_HANDLE
        ? cna_render_target_unsubscribe_content_lost(state->registration)
        : CNA_RESULT_INVALID_HANDLE;
    second_unsubscribe_result = unsubscribe_result == CNA_RESULT_SUCCESS
        ? cna_render_target_unsubscribe_content_lost(state->registration)
        : CNA_RESULT_INVALID_STATE;
    cna_demo_check(
        state->demo,
        state->content_lost_events == 0 && unsubscribe_result == CNA_RESULT_SUCCESS &&
            second_unsubscribe_result == CNA_RESULT_INVALID_HANDLE,
        "ordinary binding raises no ContentLost event and the owned subscription releases once");
    if (unsubscribe_result == CNA_RESULT_SUCCESS) {
        state->registration = CNA_INVALID_HANDLE;
    }
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
    RenderTargetBindingState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.render_target_bindings", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.target_2d = CNA_INVALID_HANDLE;
    state.target_cube = CNA_INVALID_HANDLE;
    state.registration = CNA_INVALID_HANDLE;
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
    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }
    manager_ready = cna_demo_check_result(
        &demo, cna_demo_use_hidef(game, &manager),
        "create the graphics device used for render-target binding");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through load and the immediate exit request");
        cna_demo_check(&demo, state.loads == 1,
                       "the render-target binding audit ran in exactly one load callback");
    }
    if (state.registration != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_render_target_unsubscribe_content_lost(state.registration),
                              "release a remaining ContentLost subscription");
    }
    if (state.target_cube != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_render_target_destroy(state.target_cube),
                              "destroy the cube render target");
    }
    if (state.target_2d != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_render_target_destroy(state.target_2d),
                              "destroy the 2D render target");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
