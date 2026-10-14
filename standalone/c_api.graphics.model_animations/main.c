/* SPDX-License-Identifier: MS-PL */
/* ModelAnimationsEXT values and its two direct Model pose helpers through C. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct AnimationDemo {
    CnaDemo* demo;
    CNA_ModelAnimationsEXTHandle animations;
    CNA_ModelBoneHandle root;
    CNA_ModelBoneHandle child;
    CNA_ModelMeshPartHandle part;
    CNA_ModelMeshHandle mesh;
    CNA_EffectHandle effect;
    CNA_ModelHandle model;
    CNA_SkinningDataHandle skinning;
    int loads;
} AnimationDemo;

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

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static int nearly_equal(float left, float right)
{
    return fabsf(left - right) < 0.0001F;
}

static int audit_collection(
    CnaDemo* demo, CNA_ModelAnimationsEXTHandle* out_animations)
{
    static const char expected_type[] =
        "Microsoft.Xna.Framework.Graphics.ModelAnimationsEXT";
    CNA_KeyframeEXT keys[2] = {
        {0.0, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F, 1.0F},
         {1.0F, 1.0F, 1.0F}},
        {1.0, {2.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F, 1.0F},
         {1.0F, 1.0F, 1.0F}}};
    CNA_BoneTrackEXTDescriptor track = {0, 0U, keys, UINT64_C(2)};
    CNA_NamedAnimationClipEXTDescriptor named[2];
    CNA_NamedAnimationClipEXTDescriptor duplicate[2];
    CNA_ModelAnimationsEXTHandle empty = CNA_INVALID_HANDLE;
    CNA_ModelAnimationsEXTHandle rejected = UINT64_MAX;
    CNA_ClipTargetSpaceEXT space = CNA_CLIP_TARGET_SPACE_MAXIMUM_EXT;
    double duration = -1.0;
    uint64_t count = UINT64_MAX;
    uint64_t length = UINT64_MAX;
    char buffer[64];

    *out_animations = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo,
        cna_model_animations_ext_create(NULL, UINT64_C(0), &empty) ==
                CNA_RESULT_SUCCESS &&
            cna_model_animations_ext_get_clip_count(empty, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_model_animations_ext_destroy(empty) == CNA_RESULT_SUCCESS,
        "an explicitly empty animation collection owns zero clips and releases cleanly");
    empty = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo,
        cna_model_animations_ext_create(NULL, UINT64_C(1), &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "a nonzero count requires a clip array and returns no partial handle");

    memset(named, 0, sizeof(named));
    named[0].name = text_view("Walk");
    named[0].clip.duration_seconds = 1.0;
    named[0].clip.tracks = &track;
    named[0].clip.track_count = UINT64_C(1);
    named[1].name = text_view("Idle");
    named[1].clip.duration_seconds = 2.0;
    named[1].clip.tracks = &track;
    named[1].clip.track_count = UINT64_C(1);
    duplicate[0] = named[0];
    duplicate[1] = named[0];
    rejected = UINT64_MAX;
    cna_demo_check(
        demo,
        cna_model_animations_ext_create(duplicate, UINT64_C(2), &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "duplicate clip names are refused rather than silently collapsed");
    if (!cna_demo_check_result(
            demo,
            cna_model_animations_ext_create(named, UINT64_C(2), out_animations),
            "deep-copy two named clips into an owned collection")) {
        return 0;
    }
    keys[1].translation.x = 9.0F;
    named[0].clip.duration_seconds = 9.0;

    cna_demo_check(
        demo,
        cna_model_animations_ext_get_clip_count(*out_animations, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "the collection retains both clips after caller descriptors change");
    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(
        demo,
        cna_model_animations_ext_get_clip_name_byte_count_at(
            *out_animations, UINT64_C(0), &length) == CNA_RESULT_SUCCESS &&
            length == UINT64_C(4) &&
            cna_model_animations_ext_copy_clip_name_at(
                *out_animations, UINT64_C(0), buffer, UINT64_C(2), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(4) && buffer[0] == '#' &&
            cna_model_animations_ext_copy_clip_name_at(
                *out_animations, UINT64_C(0), buffer, UINT64_C(4), &length) ==
                CNA_RESULT_SUCCESS &&
            memcmp(buffer, "Idle", 4U) == 0 &&
            cna_model_animations_ext_copy_clip_name_at(
                *out_animations, UINT64_C(1), buffer, UINT64_C(4), &length) ==
                CNA_RESULT_SUCCESS &&
            memcmp(buffer, "Walk", 4U) == 0,
        "clip names are sorted, exact and atomic on a short destination");
    cna_demo_check(
        demo,
        cna_model_animations_ext_get_clip_info_at(
            *out_animations, UINT64_C(0), &duration, &count, &space) ==
                CNA_RESULT_SUCCESS &&
            duration == 2.0 && count == UINT64_C(1) &&
            space == CNA_CLIP_TARGET_SPACE_JOINT_PALETTE_EXT &&
            cna_model_animations_ext_get_clip_info_at(
                *out_animations, UINT64_C(1), &duration, &count, &space) ==
                CNA_RESULT_SUCCESS &&
            duration == 1.0 && count == UINT64_C(1),
        "each sorted clip reports retained duration, track count and target space");

    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(
        demo,
        cna_model_animations_ext_get_type_name_byte_count(
            *out_animations, &length) == CNA_RESULT_SUCCESS &&
            length == sizeof(expected_type) - 1U &&
            cna_model_animations_ext_copy_type_name(
                *out_animations, buffer, UINT64_C(4), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == sizeof(expected_type) - 1U && buffer[0] == '#' &&
            cna_model_animations_ext_copy_type_name(
                *out_animations, buffer, sizeof(expected_type) - 1U, &length) ==
                CNA_RESULT_SUCCESS &&
            memcmp(buffer, expected_type, sizeof(expected_type) - 1U) == 0,
        "the fixed collection type name reports exact bytes and copies atomically");
    cna_demo_check(
        demo,
        cna_model_animations_ext_get_clip_info_at(
            *out_animations, UINT64_C(2), &duration, &count, &space) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_animations_ext_copy_clip_name_at(
                *out_animations, UINT64_C(2), buffer, sizeof(buffer), &length) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_animations_ext_set_clip_target_space_at(
                *out_animations, UINT64_C(0),
                CNA_CLIP_TARGET_SPACE_MAXIMUM_EXT + 1U) == CNA_RESULT_INVALID_ARGUMENT,
        "Count bounds and undefined target-space identities are rejected");
    return 1;
}

static int cleanup_state(AnimationDemo* state)
{
    int ok = 1;
    if (state->part != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_part_set_effect(state->part, CNA_INVALID_HANDLE) ==
              CNA_RESULT_SUCCESS;
    }
    if (state->model != CNA_INVALID_HANDLE) {
        ok &= cna_model_destroy(state->model) == CNA_RESULT_SUCCESS;
    }
    if (state->mesh != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_destroy(state->mesh) == CNA_RESULT_SUCCESS;
    }
    if (state->child != CNA_INVALID_HANDLE) {
        ok &= cna_model_bone_destroy(state->child) == CNA_RESULT_SUCCESS;
    }
    if (state->root != CNA_INVALID_HANDLE) {
        ok &= cna_model_bone_destroy(state->root) == CNA_RESULT_SUCCESS;
    }
    if (state->part != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_part_destroy(state->part) == CNA_RESULT_SUCCESS;
    }
    if (state->effect != CNA_INVALID_HANDLE) {
        ok &= cna_effect_destroy(state->effect) == CNA_RESULT_SUCCESS;
    }
    if (state->skinning != CNA_INVALID_HANDLE) {
        ok &= cna_skinning_data_destroy(state->skinning) == CNA_RESULT_SUCCESS;
    }
    return ok;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    AnimationDemo* state = (AnimationDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle bones[2];
    CNA_ModelMeshHandle meshes[1];
    CNA_ModelMeshPartHandle parts[1];
    CNA_Matrix local[2];
    CNA_Matrix absolute[2];
    CNA_Matrix bind_pose[2] = {identity_matrix(), identity_matrix()};
    CNA_Matrix inverse_bind[2] = {identity_matrix(), identity_matrix()};
    int32_t hierarchy[2] = {-1, 0};
    CNA_SkinningDataDescriptor descriptor;
    uint64_t count = UINT64_MAX;
    uint64_t posed = UINT64_MAX;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    if (!cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &device),
            "borrow the device for a two-bone Model and SkinnedEffect")) {
        return CNA_RESULT_SUCCESS;
    }
    local[0] = identity_matrix();
    local[1] = identity_matrix();
    local[1].m42 = 4.0F;
    cna_demo_check(
        state->demo,
        cna_model_bone_create(INT32_C(0), text_view("Root"), &state->root) ==
                CNA_RESULT_SUCCESS &&
            cna_model_bone_create(INT32_C(1), text_view("UntrackedChild"), &state->child) ==
                CNA_RESULT_SUCCESS &&
            cna_model_bone_set_transform(state->child, local[1]) == CNA_RESULT_SUCCESS &&
            cna_model_bone_add_child(state->root, state->child) == CNA_RESULT_SUCCESS,
        "build a two-bone hierarchy whose child is absent from the clip");
    cna_demo_check(
        state->demo,
        cna_skinned_effect_create(device, &state->effect) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_create_default(&state->part) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_set_effect(state->part, state->effect) ==
                CNA_RESULT_SUCCESS,
        "attach a real SkinnedEffect to an empty retained mesh part");
    parts[0] = state->part;
    bones[0] = state->root;
    bones[1] = state->child;
    if (!cna_demo_check_result(
            state->demo,
            cna_model_mesh_create_named(
                device, text_view("Skin"), parts, UINT64_C(1), &state->mesh),
            "construct a named mesh retaining the effect part")) {
        return CNA_RESULT_SUCCESS;
    }
    meshes[0] = state->mesh;
    if (!cna_demo_check_result(
            state->demo,
            cna_model_create(
                device, bones, UINT64_C(2), meshes, UINT64_C(1), &state->model),
            "construct a Model retaining the hierarchy and mesh")) {
        return CNA_RESULT_SUCCESS;
    }

    cna_demo_check(
        state->demo,
        cna_model_apply_clip_to_bones_ext(
            state->model, state->animations, UINT64_C(1), 0.5) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_copy_bone_transforms(
                state->model, local, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            local[0].m41 == 0.0F && local[1].m42 == 4.0F,
        "a joint-palette clip cannot silently pose scene-node Model bones");
    cna_demo_check(
        state->demo,
        cna_model_animations_ext_set_clip_target_space_at(
            state->animations, UINT64_C(1), CNA_CLIP_TARGET_SPACE_SCENE_NODE_EXT) ==
                CNA_RESULT_SUCCESS &&
            cna_model_apply_clip_to_bones_ext(
                state->model, state->animations, UINT64_C(1), 0.5) ==
                CNA_RESULT_SUCCESS &&
            cna_model_copy_bone_transforms(
                state->model, local, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            nearly_equal(local[0].m41, 1.0F) && local[1].m42 == 4.0F,
        "scene-node half-time playback poses only the tracked root bone");
    cna_demo_check(
        state->demo,
        cna_model_copy_absolute_bone_transforms(
            state->model, absolute, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            nearly_equal(absolute[0].m41, 1.0F) &&
            nearly_equal(absolute[1].m41, 1.0F) && absolute[1].m42 == 4.0F,
        "the Model hierarchy composes the animated root with the unchanged child");
    cna_demo_check(
        state->demo,
        cna_model_apply_clip_to_bones_ext(
            state->model, state->animations, UINT64_C(1), 2.0) ==
                CNA_RESULT_SUCCESS &&
            cna_model_copy_bone_transforms(
                state->model, local, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            nearly_equal(local[0].m41, 2.0F) &&
            cna_model_apply_clip_to_bones_ext(
                state->model, state->animations, UINT64_C(1), NAN) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_copy_bone_transforms(
                state->model, local, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            nearly_equal(local[0].m41, 2.0F),
        "direct posing clamps at duration and rejects non-finite time without mutation");

    bind_pose[0].m41 = 0.25F;
    bind_pose[1].m42 = 0.5F;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.bone_count = 2;
    descriptor.skeleton_hierarchy = hierarchy;
    descriptor.bind_pose = bind_pose;
    descriptor.inverse_bind_pose = inverse_bind;
    if (!cna_demo_check_result(
            state->demo, cna_skinning_data_create(&descriptor, &state->skinning),
            "create a two-bone bind palette for the SkinnedEffect")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_model_apply_bind_pose_bone_transforms_ext(
            state->model, state->skinning, &posed) == CNA_RESULT_SUCCESS &&
            posed == UINT64_C(1) &&
            cna_skinned_effect_copy_bone_transforms(
                state->effect, UINT64_C(2), absolute, UINT64_C(2), &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && nearly_equal(absolute[0].m41, 0.25F) &&
            nearly_equal(absolute[1].m41, 0.25F) &&
            nearly_equal(absolute[1].m42, 0.5F),
        "bind-pose helper uploads the composed two-bone palette to one SkinnedEffect");
    cna_demo_check(
        state->demo,
        cna_model_apply_bind_pose_bone_transforms_ext(
            state->model, state->skinning, NULL) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_apply_clip_to_bones_ext(
                state->model, state->animations, UINT64_C(2), 0.0) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "pose helpers reject a missing output and an out-of-range clip index");
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    AnimationDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    uint64_t count = UINT64_MAX;
    int device_ready;

    cna_demo_init(&demo, "c_api.graphics.model_animations", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.animations = CNA_INVALID_HANDLE;
    state.root = CNA_INVALID_HANDLE;
    state.child = CNA_INVALID_HANDLE;
    state.part = CNA_INVALID_HANDLE;
    state.mesh = CNA_INVALID_HANDLE;
    state.effect = CNA_INVALID_HANDLE;
    state.model = CNA_INVALID_HANDLE;
    state.skinning = CNA_INVALID_HANDLE;
    if (!audit_collection(&demo, &state.animations)) {
        return cna_demo_finish(&demo);
    }

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game),
            "create the graphics owner for direct Model posing")) {
        (void)cna_model_animations_ext_destroy(state.animations);
        return cna_demo_finish(&demo);
    }
    device_ready = cna_graphics_device_manager_create(game, &manager) == CNA_RESULT_SUCCESS &&
                   cna_graphics_device_manager_set_graphics_profile(
                       manager, CNA_GRAPHICS_PROFILE_HI_DEF) == CNA_RESULT_SUCCESS &&
                   cna_graphics_device_manager_apply_changes(manager) == CNA_RESULT_SUCCESS;
    if (cna_demo_check(
            &demo, device_ready,
            "create a HiDef graphics device for Model and SkinnedEffect ownership")) {
        cna_demo_check_result(
            &demo, cna_game_run_one_frame(game),
            "run one frame containing both direct pose workflows");
        cna_demo_check(
            &demo, state.loads == 1,
            "the pose audit ran exactly once in the load callback");
    }
    cna_demo_check(
        &demo, cleanup_state(&state),
        "release the Model graph, SkinningData and retained effect resources");
    cna_demo_check(
        &demo,
        cna_model_animations_ext_destroy(state.animations) == CNA_RESULT_SUCCESS &&
            cna_model_animations_ext_get_clip_count(state.animations, &count) ==
                CNA_RESULT_INVALID_HANDLE,
        "destroy the animation collection and reject stale access");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
