/* SPDX-License-Identifier: MS-PL */
/* AvatarRenderer state, skeleton and both draw inputs through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct AvatarRendererState {
    CnaDemo* demo;
    CNA_AvatarRendererHandle renderer;
    CNA_AvatarAnimationHandle animation;
    int updates;
    int ready;
    int skeleton_audited;
    int draw_phase;
} AvatarRendererState;

static const CNA_Color Background = {18U, 28U, 44U, 255U};

static CNA_StringView text_view(const char* data, uint64_t byte_length)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = byte_length;
    return value;
}

static void init_renderer_info(CNA_AvatarRendererInfo* info)
{
    memset(info, 0xA5, sizeof(*info));
    info->struct_size = (uint32_t)sizeof(*info);
    info->struct_version = UINT32_C(1);
}

static int matrix_is_finite(CNA_Matrix value)
{
    const float* elements = &value.m11;
    int index;
    for (index = 0; index < 16; ++index) {
        if (!isfinite(elements[index])) {
            return 0;
        }
    }
    return 1;
}

static int vector_is(CNA_Vector3 value, CNA_Vector3 expected)
{
    return value.x == expected.x && value.y == expected.y && value.z == expected.z;
}

static CNA_Matrix filled_matrix(float value)
{
    CNA_Matrix result;
    float* elements = &result.m11;
    int index;
    for (index = 0; index < 16; ++index) {
        elements[index] = value;
    }
    return result;
}

static void audit_properties(AvatarRendererState* state)
{
    CNA_Matrix identity;
    CNA_Matrix world;
    CNA_Matrix view;
    CNA_Matrix projection;
    CNA_Matrix observed_world;
    CNA_Matrix observed_view;
    CNA_Matrix observed_projection;
    const CNA_Vector3 eye = {0.0F, 1.0F, 3.2F};
    const CNA_Vector3 target = {0.0F, 0.9F, 0.0F};
    const CNA_Vector3 up = {0.0F, 1.0F, 0.0F};
    const CNA_Vector3 light = {0.85F, 0.75F, 0.65F};
    const CNA_Vector3 direction = {-0.3F, -1.0F, -0.2F};
    const CNA_Vector3 ambient = {0.20F, 0.25F, 0.30F};
    CNA_Vector3 observed_light;
    CNA_Vector3 observed_direction;
    CNA_Vector3 observed_ambient;
    CNA_Vector3 invalid_light = light;

    cna_demo_check(
        state->demo,
        cna_matrix_get_identity(&identity) == CNA_RESULT_SUCCESS &&
            cna_avatar_renderer_get_transforms(
                state->renderer, &observed_world, &observed_view, &observed_projection) ==
                CNA_RESULT_SUCCESS &&
            memcmp(&observed_world, &identity, sizeof(identity)) == 0 &&
            memcmp(&observed_view, &identity, sizeof(identity)) == 0 &&
            memcmp(&observed_projection, &identity, sizeof(identity)) == 0,
        "a new renderer exposes identity world, view and projection transforms");

    cna_demo_check(
        state->demo,
        cna_matrix_create_rotation_y(3.14159265F, &world) == CNA_RESULT_SUCCESS &&
            cna_matrix_create_look_at(eye, target, up, &view) == CNA_RESULT_SUCCESS &&
            cna_matrix_create_perspective_field_of_view(
                0.7853982F, 800.0F / 480.0F, 0.1F, 50.0F, &projection) ==
                CNA_RESULT_SUCCESS &&
            cna_avatar_renderer_set_transforms(
                state->renderer, &world, &view, &projection) == CNA_RESULT_SUCCESS &&
            cna_avatar_renderer_get_transforms(
                state->renderer, &observed_world, &observed_view, &observed_projection) ==
                CNA_RESULT_SUCCESS &&
            memcmp(&observed_world, &world, sizeof(world)) == 0 &&
            memcmp(&observed_view, &view, sizeof(view)) == 0 &&
            memcmp(&observed_projection, &projection, sizeof(projection)) == 0,
        "world, view and projection round-trip exactly through their combined route");
    observed_world = filled_matrix(9.0F);
    cna_demo_check(
        state->demo,
        cna_avatar_renderer_get_transforms(
            state->renderer, &observed_world, NULL, NULL) == CNA_RESULT_SUCCESS &&
            memcmp(&observed_world, &world, sizeof(world)) == 0,
        "transform outputs are independently optional");

    cna_demo_check(
        state->demo,
        cna_avatar_renderer_set_lighting(
            state->renderer, &light, &direction, &ambient) == CNA_RESULT_SUCCESS &&
            cna_avatar_renderer_get_lighting(
                state->renderer, &observed_light, &observed_direction, &observed_ambient) ==
                CNA_RESULT_SUCCESS &&
            vector_is(observed_light, light) && vector_is(observed_direction, direction) &&
            vector_is(observed_ambient, ambient),
        "all three lighting vectors round-trip exactly");
    invalid_light.x = NAN;
    cna_demo_check(
        state->demo,
        cna_avatar_renderer_set_lighting(
            state->renderer, &invalid_light, NULL, NULL) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_avatar_renderer_get_lighting(
                state->renderer, &observed_light, &observed_direction, &observed_ambient) ==
                CNA_RESULT_SUCCESS &&
            vector_is(observed_light, light) && vector_is(observed_direction, direction) &&
            vector_is(observed_ambient, ambient),
        "non-finite lighting is rejected before changing any vector");
    observed_light = (CNA_Vector3){9.0F, 9.0F, 9.0F};
    cna_demo_check(
        state->demo,
        cna_avatar_renderer_get_lighting(
            state->renderer, NULL, NULL, &observed_light) == CNA_RESULT_SUCCESS &&
            vector_is(observed_light, ambient),
        "lighting outputs are independently optional");
}

static void audit_skeleton(AvatarRendererState* state)
{
    CNA_Matrix bind_pose[CNA_AVATAR_RENDERER_BONE_COUNT];
    int32_t parents[CNA_AVATAR_RENDERER_BONE_COUNT];
    CNA_Matrix sentinel = filled_matrix(9.0F);
    CNA_Matrix sentinel_before = sentinel;
    int32_t parent = 99;
    int hierarchy_ok = 1;
    int bind_ok = 1;
    int index;
    int current;
    float head_y = 0.0F;

    for (index = 0; index < CNA_AVATAR_RENDERER_BONE_COUNT; ++index) {
        if (cna_avatar_renderer_get_parent_bone_at(
                state->renderer, index, &parents[index]) != CNA_RESULT_SUCCESS ||
            (index == 0 ? parents[index] != -1
                        : (parents[index] < 0 || parents[index] >= index))) {
            hierarchy_ok = 0;
        }
        if (cna_avatar_renderer_get_bind_pose_at(
                state->renderer, index, &bind_pose[index]) != CNA_RESULT_SUCCESS ||
            !matrix_is_finite(bind_pose[index])) {
            bind_ok = 0;
        }
    }
    cna_demo_check(
        state->demo,
        hierarchy_ok && parents[1] == 0 && parents[5] == 1 && parents[37] == 33 &&
            parents[70] == 60,
        "all 71 parent indices form the canonical ordered avatar hierarchy");
    cna_demo_check(
        state->demo, bind_ok,
        "the Ready renderer exposes 71 finite local bind-pose transforms");
    for (current = (int)CNA_AVATAR_BONE_HEAD; current >= 0; current = parents[current]) {
        head_y += bind_pose[current].m42;
    }
    cna_demo_check(
        state->demo, head_y > 1.0F && head_y < 2.1F,
        "composing bind-pose translations places the head at a human-scale height");
    cna_demo_check(
        state->demo,
        cna_avatar_renderer_get_parent_bone_at(state->renderer, -1, &parent) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            parent == 99 &&
            cna_avatar_renderer_get_parent_bone_at(
                state->renderer, CNA_AVATAR_RENDERER_BONE_COUNT, &parent) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            parent == 99,
        "parent lookup rejects both skeleton bounds without changing its output");
    cna_demo_check(
        state->demo,
        cna_avatar_renderer_get_bind_pose_at(state->renderer, -1, &sentinel) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&sentinel, &sentinel_before, sizeof(sentinel)) == 0 &&
            cna_avatar_renderer_get_bind_pose_at(
                state->renderer, CNA_AVATAR_RENDERER_BONE_COUNT, &sentinel) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&sentinel, &sentinel_before, sizeof(sentinel)) == 0,
        "bind-pose lookup rejects both skeleton bounds without changing its output");
    state->skeleton_audited = 1;
}

static void check_pixels(AvatarRendererState* state, CNA_Handle device, const char* label)
{
    CNA_BackBufferInfo info;
    CNA_Color* pixels = NULL;
    uint64_t pixel_count;
    uint64_t copied = 0U;
    uint64_t changed = 0U;
    uint32_t min_x = UINT32_MAX;
    uint32_t min_y = UINT32_MAX;
    uint32_t max_x = 0U;
    uint32_t max_y = 0U;
    uint64_t index;

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    if (!cna_demo_check_result(
            state->demo, cna_graphics_device_get_backbuffer_info(device, &info),
            "query the avatar back-buffer dimensions")) {
        return;
    }
    pixel_count = (uint64_t)info.width * (uint64_t)info.height;
    pixels = (CNA_Color*)malloc((size_t)pixel_count * sizeof(*pixels));
    if (pixels == NULL) {
        cna_demo_check(state->demo, 0, "allocate the avatar back-buffer readback");
        return;
    }
    if (!cna_demo_check_result(
            state->demo,
            cna_graphics_device_get_backbuffer_data_rgba8(
                device, pixels, pixel_count, &copied),
            "read the rendered avatar back buffer")) {
        free(pixels);
        return;
    }
    for (index = 0U; index < copied; ++index) {
        const CNA_Color pixel = pixels[index];
        if (pixel.r != Background.r || pixel.g != Background.g ||
            pixel.b != Background.b || pixel.a != Background.a) {
            const uint32_t x = (uint32_t)(index % info.width);
            const uint32_t y = (uint32_t)(index / info.width);
            ++changed;
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
    }
    cna_demo_check(
        state->demo,
        copied == pixel_count && changed > UINT64_C(1000) && min_x < max_x && min_y < max_y &&
            max_x - min_x > 60U && max_y - min_y > 160U,
        label);
    free(pixels);
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    AvatarRendererState* state = (AvatarRendererState*)context;
    CNA_AvatarDescriptionHandle description = CNA_INVALID_HANDLE;
    CNA_AvatarRendererHandle invalid_renderer = UINT64_C(99);
    CNA_AvatarRendererInfo info;
    CNA_AvatarRendererInfo malformed;
    unsigned char malformed_before[sizeof(malformed)];
    int32_t root_parent = 99;
    CNA_Result result;
    (void)game;
    (void)game_time;
    (void)out_error;

    result = cna_avatar_description_create_random_for_body_type(
        CNA_AVATAR_BODY_TYPE_FEMALE, &description);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    cna_demo_check(
        state->demo,
        cna_avatar_renderer_create(description, (CNA_Bool)2, &invalid_renderer) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            invalid_renderer == CNA_INVALID_HANDLE,
        "renderer creation rejects a noncanonical loading-effect flag atomically");
    result = cna_avatar_renderer_create(description, CNA_TRUE, &state->renderer);
    cna_demo_check_result(state->demo, result, "create a female avatar renderer with its loading effect");
    if (result != CNA_RESULT_SUCCESS) {
        (void)cna_avatar_description_destroy(description);
        return result;
    }
    cna_demo_check_result(
        state->demo, cna_avatar_description_destroy(description),
        "release the public description handle while its renderer retains the avatar");

    init_renderer_info(&info);
    cna_demo_check(
        state->demo,
        cna_avatar_renderer_get_info(state->renderer, &info) == CNA_RESULT_SUCCESS &&
            info.state <= CNA_AVATAR_RENDERER_STATE_MAXIMUM &&
            info.is_disposed == CNA_FALSE && info.reserved[0] == 0U &&
            info.reserved[1] == 0U && info.reserved[2] == 0U,
        "a new renderer reports versioned live state with zero reserve");
    memset(&malformed, 0xA5, sizeof(malformed));
    malformed.struct_size = (uint32_t)sizeof(malformed);
    malformed.struct_version = UINT32_C(2);
    memcpy(malformed_before, &malformed, sizeof(malformed));
    cna_demo_check(
        state->demo,
        cna_avatar_renderer_get_info(state->renderer, &malformed) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&malformed, malformed_before, sizeof(malformed)) == 0,
        "an unknown renderer-info version is rejected without changing the output");
    cna_demo_check(
        state->demo,
        cna_avatar_renderer_get_parent_bone_at(state->renderer, 0, &root_parent) ==
                CNA_RESULT_SUCCESS &&
            root_parent == -1,
        "the root-parent hierarchy is readable while avatar assets load");

    audit_properties(state);
    return cna_avatar_animation_create(CNA_AVATAR_ANIMATION_PRESET_WAVE, &state->animation);
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    AvatarRendererState* state = (AvatarRendererState*)context;
    CNA_AvatarRendererInfo info;
    CNA_Result result;
    (void)game_time;
    (void)out_error;

    ++state->updates;
    if (state->draw_phase >= 2) {
        return cna_game_request_exit(game);
    }
    init_renderer_info(&info);
    result = cna_avatar_renderer_get_info(state->renderer, &info);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    if (info.state == CNA_AVATAR_RENDERER_STATE_UNAVAILABLE) {
        cna_demo_check(state->demo, 0, "the canonical random avatar remained available");
        return cna_game_request_exit(game);
    }
    if (info.state == CNA_AVATAR_RENDERER_STATE_READY && !state->ready) {
        state->ready = 1;
        cna_demo_check(
            state->demo, info.is_disposed == CNA_FALSE,
            "the retained-description renderer reaches Ready without becoming disposed");
        audit_skeleton(state);
        cna_demo_check_result(
            state->demo,
            cna_avatar_animation_set_current_position(state->animation, INT64_C(12000000)),
            "sample Wave at 1.2 seconds for both draw inputs");
    }
    if (state->demo->frames > 0 && state->updates >= state->demo->frames) {
        cna_demo_check(state->demo, 0, "the avatar renderer reached Ready before the frame limit");
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    AvatarRendererState* state = (AvatarRendererState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Matrix bones[CNA_AVATAR_RENDERER_BONE_COUNT];
    CNA_AvatarExpression expression;
    CNA_AvatarExpression invalid_expression;
    CNA_Result result;
    int index;
    (void)game_time;
    (void)out_error;

    result = cna_game_get_graphics_device(game, &device);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_game_clear(game, Background);
    }
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    if (!state->ready) {
        return cna_avatar_renderer_draw_animation(state->renderer, state->animation);
    }
    if (state->draw_phase == 0) {
        result = cna_avatar_renderer_draw_animation(state->renderer, state->animation);
        if (cna_demo_check_result(
                state->demo, result, "draw the Ready avatar from its Wave animation")) {
            check_pixels(
                state, device,
                "the animation draw covers a substantial two-dimensional avatar region");
            state->draw_phase = 1;
        }
        return CNA_RESULT_SUCCESS;
    }
    for (index = 0; index < CNA_AVATAR_RENDERER_BONE_COUNT; ++index) {
        result = cna_avatar_animation_get_bone_transform_at(
            state->animation, index, &bones[index]);
        if (result != CNA_RESULT_SUCCESS) {
            return result;
        }
    }
    memset(&expression, 0, sizeof(expression));
    expression.struct_size = (uint32_t)sizeof(expression);
    expression.struct_version = UINT32_C(1);
    result = cna_avatar_animation_get_expression(state->animation, &expression);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    expression.mouth = CNA_AVATAR_MOUTH_HAPPY;
    expression.left_eye = CNA_AVATAR_EYE_HAPPY;
    expression.right_eye = CNA_AVATAR_EYE_BLINK;
    expression.left_eyebrow = CNA_AVATAR_EYEBROW_RAISED;
    expression.right_eyebrow = CNA_AVATAR_EYEBROW_NEUTRAL;
    invalid_expression = expression;
    invalid_expression.mouth = CNA_AVATAR_MOUTH_MAXIMUM + UINT32_C(1);
    cna_demo_check(
        state->demo,
        cna_avatar_renderer_draw_bones(
            state->renderer, bones, CNA_AVATAR_RENDERER_BONE_COUNT - UINT64_C(1),
            &expression) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_avatar_renderer_draw_bones(
                state->renderer, bones, CNA_AVATAR_RENDERER_BONE_COUNT,
                &invalid_expression) == CNA_RESULT_INVALID_ARGUMENT,
        "caller-supplied drawing rejects a wrong bone count and undefined expression");
    result = cna_avatar_renderer_draw_bones(
        state->renderer, bones, CNA_AVATAR_RENDERER_BONE_COUNT, &expression);
    if (cna_demo_check_result(
            state->demo, result,
            "draw the Ready avatar from caller-supplied bones and expression")) {
        check_pixels(
            state, device,
            "the bones-and-expression draw covers a substantial two-dimensional avatar region");
        cna_demo_capture(state->demo, device);
        state->draw_phase = 2;
    }
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    AvatarRendererState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_Handle component = CNA_INVALID_HANDLE;
    CNA_AvatarRendererHandle released = CNA_INVALID_HANDLE;
    CNA_AvatarRendererInfo info;

    cna_demo_init(&demo, "c_api.gamer.avatar_renderer", argc, argv, 600);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.renderer = CNA_INVALID_HANDLE;
    state.animation = CNA_INVALID_HANDLE;
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
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.window_title = text_view(
        "C avatar renderer audit", sizeof("C avatar renderer audit") - 1U);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game),
            "create the avatar renderer game") ||
        !cna_demo_check_result(
            &demo, cna_graphics_device_manager_create(game, &manager),
            "create the avatar graphics-device manager") ||
        !cna_demo_check(
            &demo,
            cna_graphics_device_manager_set_graphics_profile(
                manager, CNA_GRAPHICS_PROFILE_HI_DEF) == CNA_RESULT_SUCCESS &&
                cna_graphics_device_manager_set_preferred_back_buffer_width(manager, 800) ==
                    CNA_RESULT_SUCCESS &&
                cna_graphics_device_manager_set_preferred_back_buffer_height(manager, 480) ==
                    CNA_RESULT_SUCCESS &&
                cna_graphics_device_manager_set_preferred_depth_stencil_format(
                    manager, CNA_DEPTH_FORMAT_DEPTH24_STENCIL8) == CNA_RESULT_SUCCESS &&
                cna_graphics_device_manager_apply_changes(manager) == CNA_RESULT_SUCCESS,
            "select an explicit HiDef 800x480 depth-buffered device") ||
        !cna_demo_check_result(
            &demo, cna_gamer_services_component_create(game, &component),
            "create the gamer-services component") ||
        !cna_demo_check_result(
            &demo, cna_game_components_add(game, component),
            "add the component that owns the avatar drawing device")) {
        goto cleanup;
    }
    cna_demo_check_result(
        &demo, cna_game_run(game),
        "run until both Ready avatar draw inputs are pixel-checked");
    cna_demo_check(
        &demo, state.ready && state.skeleton_audited && state.draw_phase == 2,
        "the renderer, skeleton and both draw paths completed before exit");

cleanup:
    if (state.animation != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_avatar_animation_destroy(state.animation),
            "release the Wave animation");
    }
    if (state.renderer != CNA_INVALID_HANDLE) {
        released = state.renderer;
        cna_demo_check_result(
            &demo, cna_avatar_renderer_destroy(state.renderer),
            "dispose and release the avatar renderer once");
        state.renderer = CNA_INVALID_HANDLE;
        init_renderer_info(&info);
        info.state = UINT32_C(99);
        cna_demo_check(
            &demo,
            cna_avatar_renderer_get_info(released, &info) == CNA_RESULT_INVALID_HANDLE &&
                info.state == UINT32_C(99) &&
                cna_avatar_renderer_destroy(released) == CNA_RESULT_INVALID_HANDLE,
            "a released renderer rejects stale inspection and repeated destruction atomically");
    }
    if (component != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_game_component_destroy(component),
            "remove and release the gamer-services component");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the avatar graphics-device manager");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the avatar renderer game");
    }
    return cna_demo_finish(&demo);
}
