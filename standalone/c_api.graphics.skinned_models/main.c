/* SPDX-License-Identifier: MS-PL */
/* SkinnedModelEXT skeleton, clips, moves and retained part resources through C. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct SkinnedDemo {
    CnaDemo* demo;
    int loads;
} SkinnedDemo;

static CNA_Matrix identity_matrix(void)
{
    CNA_Matrix value;
    (void)cna_matrix_get_identity(&value);
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

static int audit_animation_state(CnaDemo* demo)
{
    int32_t parents[2] = {-1, 0};
    int32_t copied_parents[2] = {99, 99};
    CNA_Matrix bind_pose[2] = {identity_matrix(), identity_matrix()};
    CNA_Matrix inverse_bind[2] = {identity_matrix(), identity_matrix()};
    CNA_Matrix copied_matrices[2];
    CNA_Matrix computed[2];
    CNA_Matrix sentinel = identity_matrix();
    CNA_KeyframeEXT keys[2] = {
        {0.0, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F, 1.0F},
         {1.0F, 1.0F, 1.0F}},
        {1.0, {2.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F, 1.0F},
         {1.0F, 1.0F, 1.0F}}};
    CNA_BoneTrackEXTDescriptor track = {0, 0U, keys, UINT64_C(2)};
    CNA_AnimationClipEXTDescriptor clip = {1.0, &track, UINT64_C(1)};
    CNA_NamedAnimationClipEXTDescriptor named;
    CNA_SkinnedModelEXTDescriptor descriptor;
    CNA_SkinnedModelEXTHandle model = CNA_INVALID_HANDLE;
    CNA_SkinnedModelEXTHandle moved = CNA_INVALID_HANDLE;
    CNA_SkinnedModelEXTHandle destination = CNA_INVALID_HANDLE;
    CNA_SkinnedModelEXTHandle donor = CNA_INVALID_HANDLE;
    CNA_SkinnedModelEXTHandle rejected = UINT64_MAX;
    CNA_KeyframeEXT copied_keys[2];
    CNA_Bool found = CNA_FALSE;
    uint64_t count = UINT64_MAX;
    uint64_t track_count = UINT64_MAX;
    uint64_t name_size = UINT64_MAX;
    double duration = -1.0;
    int32_t bone_index = INT32_C(-99);
    char name[4] = {0, 0, 0, 0};
    int ok = 1;

    bind_pose[1].m42 = 1.0F;
    memset(&named, 0, sizeof(named));
    named.name = text_view("Walk");
    named.clip = clip;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.bone_count = 2;
    descriptor.parent_bone_indices = parents;
    descriptor.bind_pose_local = bind_pose;
    descriptor.inverse_bind_pose_global = inverse_bind;
    descriptor.clips = &named;
    descriptor.clip_count = UINT64_C(1);

    descriptor.reserved = UINT32_C(1);
    cna_demo_check(
        demo,
        cna_skinned_model_ext_create(&descriptor, &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "a nonzero reserved descriptor field is refused atomically");
    descriptor.reserved = UINT32_C(0);
    if (!cna_demo_check_result(
            demo, cna_skinned_model_ext_create(&descriptor, &model),
            "deep-copy a two-bone skeleton and one named clip")) {
        return 0;
    }
    parents[1] = -1;
    bind_pose[1].m42 = 9.0F;
    keys[1].translation.x = 9.0F;
    cna_demo_check(
        demo,
        cna_skinned_model_ext_get_bone_count(model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_skinned_model_ext_copy_parent_bone_indices(
                model, copied_parents, UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) && copied_parents[0] == 99 && copied_parents[1] == 99 &&
            cna_skinned_model_ext_copy_parent_bone_indices(
                model, copied_parents, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            copied_parents[0] == -1 && copied_parents[1] == 0,
        "the copied hierarchy survives caller mutation and short output is atomic");
    cna_demo_check(
        demo,
        cna_skinned_model_ext_copy_bind_pose_local(
            model, copied_matrices, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            copied_matrices[1].m42 == 1.0F &&
            cna_skinned_model_ext_copy_inverse_bind_pose_global(
                model, copied_matrices, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            copied_matrices[0].m44 == 1.0F && copied_matrices[1].m44 == 1.0F,
        "local bind and inverse-global matrices remain independent copied arrays");
    cna_demo_check(
        demo,
        cna_skinned_model_ext_get_clip_count(model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_skinned_model_ext_get_clip_name_byte_count_at(
                model, UINT64_C(0), &name_size) == CNA_RESULT_SUCCESS &&
            name_size == UINT64_C(4) &&
            cna_skinned_model_ext_copy_clip_name_at(
                model, UINT64_C(0), name, sizeof(name), &name_size) == CNA_RESULT_SUCCESS &&
            memcmp(name, "Walk", sizeof(name)) == 0,
        "the sorted clip list owns and copies its exact name");
    cna_demo_check(
        demo,
        cna_skinned_model_ext_get_clip_info(
            model, text_view("Walk"), &found, &duration, &track_count) ==
                CNA_RESULT_SUCCESS &&
            found == CNA_TRUE && duration == 1.0 && track_count == UINT64_C(1) &&
            cna_skinned_model_ext_get_clip_info(
                model, text_view("Missing"), &found, &duration, &track_count) ==
                CNA_RESULT_SUCCESS &&
            found == CNA_FALSE && duration == 0.0 && track_count == UINT64_C(0),
        "clip lookup distinguishes exact presence from an ordinary absent result");
    cna_demo_check(
        demo,
        cna_skinned_model_ext_copy_clip_track(
            model, text_view("Walk"), UINT64_C(0), &bone_index, copied_keys,
            UINT64_C(1), &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) && bone_index == INT32_C(-99) &&
            cna_skinned_model_ext_copy_clip_track(
                model, text_view("Walk"), UINT64_C(0), &bone_index, copied_keys,
                UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            bone_index == INT32_C(0) && copied_keys[1].translation.x == 2.0F,
        "track copies are atomic and retain the authored bone index and key values");

    sentinel.m41 = 77.0F;
    cna_demo_check(
        demo,
        cna_skinned_model_ext_compute_bone_transforms(
            model, text_view("Walk"), 0.5, CNA_FALSE, &sentinel, UINT64_C(1),
            &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) && sentinel.m41 == 77.0F &&
            cna_skinned_model_ext_compute_bone_transforms(
                model, text_view("Walk"), 0.5, CNA_FALSE, computed, UINT64_C(2),
                &count) == CNA_RESULT_SUCCESS &&
            nearly_equal(computed[0].m41, 1.0F) &&
            nearly_equal(computed[1].m41, 1.0F) &&
            nearly_equal(computed[1].m42, 1.0F),
        "half-time evaluation composes the animated root with the child bind pose");
    cna_demo_check(
        demo,
        cna_skinned_model_ext_compute_bone_transforms(
            model, text_view("Walk"), 1.5, CNA_TRUE, computed, UINT64_C(2), &count) ==
                CNA_RESULT_SUCCESS &&
            nearly_equal(computed[0].m41, 1.0F) &&
            cna_skinned_model_ext_compute_bone_transforms(
                model, text_view("Walk"), -1.0, CNA_FALSE, computed, UINT64_C(2),
                &count) == CNA_RESULT_SUCCESS &&
            nearly_equal(computed[0].m41, 0.0F) &&
            cna_skinned_model_ext_compute_bone_transforms(
                model, text_view("Missing"), 0.0, CNA_FALSE, computed, UINT64_C(2),
                &count) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_model_ext_compute_bone_transforms(
                model, text_view("Walk"), NAN, CNA_FALSE, computed, UINT64_C(2),
                &count) == CNA_RESULT_INVALID_ARGUMENT,
        "loop, clamp, missing-clip and non-finite evaluation paths stay distinct");

    clip.duration_seconds = 2.0;
    clip.tracks = NULL;
    clip.track_count = UINT64_C(0);
    cna_demo_check(
        demo,
        cna_skinned_model_ext_set_clip(model, text_view("Idle"), &clip) ==
                CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_get_clip_count(model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_skinned_model_ext_copy_clip_name_at(
                model, UINT64_C(0), name, sizeof(name), &name_size) == CNA_RESULT_SUCCESS &&
            memcmp(name, "Idle", sizeof(name)) == 0 &&
            cna_skinned_model_ext_remove_clip(model, text_view("Idle")) ==
                CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_get_clip_count(model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "clip replacement uses sorted names and removal restores the original set");
    parents[0] = -1;
    parents[1] = 1;
    bind_pose[1] = identity_matrix();
    cna_demo_check(
        demo,
        cna_skinned_model_ext_set_skeleton(
            model, INT32_C(2), parents, bind_pose, inverse_bind) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_model_ext_get_bone_count(model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "a self-parented replacement skeleton is refused without changing the model");
    parents[1] = 0;

    cna_demo_check(
        demo,
        cna_skinned_model_ext_create_move(model, &moved) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_get_bone_count(model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_skinned_model_ext_get_bone_count(moved, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_skinned_model_ext_create_default(&destination) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_move_assign(destination, moved) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_get_bone_count(moved, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_skinned_model_ext_get_bone_count(destination, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "move construction and assignment transfer the complete graph and empty both sources");
    cna_demo_check(
        demo,
        cna_skinned_model_ext_create_default(&donor) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_attach_parts(destination, donor) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_model_ext_set_skeleton(
                donor, INT32_C(2), parents, bind_pose, inverse_bind) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_attach_parts(destination, donor) == CNA_RESULT_SUCCESS,
        "part attachment requires matching skeleton shape even when both part lists are empty");

    ok &= cna_skinned_model_ext_destroy(donor) == CNA_RESULT_SUCCESS;
    ok &= cna_skinned_model_ext_destroy(moved) == CNA_RESULT_SUCCESS;
    ok &= cna_skinned_model_ext_destroy(model) == CNA_RESULT_SUCCESS;
    ok &= cna_skinned_model_ext_destroy(destination) == CNA_RESULT_SUCCESS;
    cna_demo_check(demo, ok, "destroy every original, moved-from and destination model handle");
    return ok;
}

static CNA_Result create_vertex_buffer(CNA_Handle device, CNA_VertexBufferHandle* out_buffer)
{
    CNA_VertexBufferCreateInfo info;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.vertex_count = 3;
    return cna_vertex_buffer_create(device, &info, out_buffer);
}

static CNA_Result create_index_buffer(CNA_Handle device, CNA_IndexBufferHandle* out_buffer)
{
    CNA_IndexBufferCreateInfo info;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.index_count = 3;
    info.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    return cna_index_buffer_create(device, &info, out_buffer);
}

static int audit_part_resources(CnaDemo* demo, CNA_Handle device)
{
    CNA_Texture2DCreateInfo texture_info;
    CNA_SkinnedModelEXTHandle model = CNA_INVALID_HANDLE;
    CNA_SkinnedModelEXTHandle donor = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle part = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle replacement_part = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle alias = CNA_INVALID_HANDLE;
    CNA_VertexBufferHandle vertex = CNA_INVALID_HANDLE;
    CNA_VertexBufferHandle replacement_vertex = CNA_INVALID_HANDLE;
    CNA_IndexBufferHandle index = CNA_INVALID_HANDLE;
    CNA_IndexBufferHandle replacement_index = CNA_INVALID_HANDLE;
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Handle returned_texture = UINT64_MAX;
    CNA_Vector4 part_color = {0.0F, 0.0F, 0.0F, 0.0F};
    const CNA_Vector4 tint = {0.25F, 0.5F, 0.75F, 1.0F};
    const CNA_Vector4 replacement_tint = {0.8F, 0.6F, 0.4F, 0.2F};
    CNA_Bool has_texture = CNA_FALSE;
    uint64_t vertex_count = UINT64_MAX;
    uint64_t index_count = UINT64_MAX;
    uint64_t part_count = UINT64_MAX;
    uint64_t texture_count = UINT64_MAX;
    uint64_t byte_count = UINT64_MAX;
    char part_name[4];
    int ok = 1;

    texture_info = (CNA_Texture2DCreateInfo){
        sizeof(CNA_Texture2DCreateInfo), UINT32_C(1), 1U, 1U,
        CNA_FALSE, {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR};
    if (!cna_demo_check(
            demo,
            create_vertex_buffer(device, &vertex) == CNA_RESULT_SUCCESS &&
                create_index_buffer(device, &index) == CNA_RESULT_SUCCESS &&
                cna_texture2d_create(device, &texture_info, &texture) == CNA_RESULT_SUCCESS &&
                cna_model_mesh_part_create_default(&part) == CNA_RESULT_SUCCESS &&
                cna_skinned_model_ext_create_default(&model) == CNA_RESULT_SUCCESS,
            "create an empty skinned model and one complete same-device part resource set")) {
        return 0;
    }
    cna_demo_check(
        demo,
        cna_skinned_model_ext_add_part(
            model, text_view("Body"), vertex, index, part, texture) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_destroy(part) == CNA_RESULT_SUCCESS &&
            cna_vertex_buffer_destroy(vertex) == CNA_RESULT_INVALID_STATE &&
            cna_index_buffer_destroy(index) == CNA_RESULT_INVALID_STATE &&
            cna_texture2d_destroy(texture) == CNA_RESULT_INVALID_STATE,
        "adding a named part transfers part ownership and retains both buffers plus texture");
    part = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo,
        cna_skinned_model_ext_get_part_count(model, &part_count) == CNA_RESULT_SUCCESS &&
            part_count == UINT64_C(1) &&
            cna_skinned_model_ext_get_part_name_byte_count_at(
                model, UINT64_C(0), &byte_count) == CNA_RESULT_SUCCESS &&
            byte_count == UINT64_C(4) &&
            cna_skinned_model_ext_copy_part_name_at(
                model, UINT64_C(0), part_name, sizeof(part_name), &byte_count) ==
                CNA_RESULT_SUCCESS &&
            memcmp(part_name, "Body", sizeof(part_name)) == 0,
        "the retained part list preserves exact order and copied names");
    cna_demo_check(
        demo,
        cna_skinned_model_ext_get_part_diffuse_color_at(
            model, UINT64_C(0), &part_color) == CNA_RESULT_SUCCESS &&
            nearly_equal(part_color.x, 1.0F) && nearly_equal(part_color.y, 1.0F) &&
            nearly_equal(part_color.z, 1.0F) && nearly_equal(part_color.w, 1.0F) &&
            cna_skinned_model_ext_get_part_diffuse_color_at(
                model, UINT64_C(1), &part_color) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_model_ext_set_part_diffuse_color_at(
                model, UINT64_C(1), tint) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinned_model_ext_set_part_diffuse_color_at(
                model, UINT64_C(0), tint) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_get_part_diffuse_color_at(
                model, UINT64_C(0), &part_color) == CNA_RESULT_SUCCESS &&
            nearly_equal(part_color.x, tint.x) && nearly_equal(part_color.y, tint.y) &&
            nearly_equal(part_color.z, tint.z) && nearly_equal(part_color.w, tint.w),
        "part diffuse colour starts white, refuses an out-of-range index and round-trips a tint");
    cna_demo_check(
        demo,
        cna_skinned_model_ext_get_owned_resource_counts(
            model, &vertex_count, &index_count, &part_count, &texture_count) ==
                CNA_RESULT_SUCCESS &&
            vertex_count == UINT64_C(1) && index_count == UINT64_C(1) &&
            part_count == UINT64_C(1) && texture_count == UINT64_C(1) &&
            cna_skinned_model_ext_get_part_at(
                model, UINT64_C(0), &alias, &has_texture, &returned_texture) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_TRUE && returned_texture == texture &&
            cna_model_mesh_part_set_vertex_buffer(alias, CNA_INVALID_HANDLE) ==
                CNA_RESULT_INVALID_STATE &&
            cna_model_mesh_part_destroy(alias) == CNA_RESULT_SUCCESS,
        "resource counts and part lookup expose the retained texture and lock owned buffers");
    alias = CNA_INVALID_HANDLE;

    cna_demo_check(
        demo,
        create_vertex_buffer(device, &replacement_vertex) == CNA_RESULT_SUCCESS &&
            create_index_buffer(device, &replacement_index) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_create_default(&replacement_part) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_create_default(&donor) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_add_part(
                donor, text_view("Body"), replacement_vertex, replacement_index,
                replacement_part, CNA_INVALID_HANDLE) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_destroy(replacement_part) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_set_part_diffuse_color_at(
                donor, UINT64_C(0), replacement_tint) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_attach_parts(model, donor) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_get_part_count(donor, &part_count) == CNA_RESULT_SUCCESS &&
            part_count == UINT64_C(0),
        "attach moves donor parts and replaces an existing exact-name entry");
    replacement_part = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo,
        cna_skinned_model_ext_get_part_diffuse_color_at(
            model, UINT64_C(0), &part_color) == CNA_RESULT_SUCCESS &&
            nearly_equal(part_color.x, replacement_tint.x) &&
            nearly_equal(part_color.y, replacement_tint.y) &&
            nearly_equal(part_color.z, replacement_tint.z) &&
            nearly_equal(part_color.w, replacement_tint.w),
        "same-name replacement carries the donor part's independent diffuse colour");
    cna_demo_check(
        demo,
        cna_vertex_buffer_destroy(vertex) == CNA_RESULT_SUCCESS &&
            cna_index_buffer_destroy(index) == CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(texture) == CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_get_owned_resource_counts(
                model, &vertex_count, &index_count, &part_count, &texture_count) ==
                CNA_RESULT_SUCCESS &&
            vertex_count == UINT64_C(1) && index_count == UINT64_C(1) &&
            part_count == UINT64_C(1) && texture_count == UINT64_C(0),
        "replacement releases the old resource set and retains only the donor buffers");
    vertex = CNA_INVALID_HANDLE;
    index = CNA_INVALID_HANDLE;
    texture = CNA_INVALID_HANDLE;
    cna_demo_check(
        demo,
        cna_skinned_model_ext_get_part_at(
            model, UINT64_C(0), &alias, &has_texture, &returned_texture) ==
                CNA_RESULT_SUCCESS &&
            has_texture == CNA_FALSE && returned_texture == CNA_INVALID_HANDLE &&
            cna_skinned_model_ext_remove_part(model, text_view("Body")) ==
                CNA_RESULT_SUCCESS &&
            cna_skinned_model_ext_get_part_count(model, &part_count) == CNA_RESULT_SUCCESS &&
            part_count == UINT64_C(0) &&
            cna_model_mesh_part_destroy(alias) == CNA_RESULT_SUCCESS &&
            cna_vertex_buffer_destroy(replacement_vertex) == CNA_RESULT_SUCCESS &&
            cna_index_buffer_destroy(replacement_index) == CNA_RESULT_SUCCESS,
        "removing the replacement empties the model and releases its final resources");
    alias = CNA_INVALID_HANDLE;
    replacement_vertex = CNA_INVALID_HANDLE;
    replacement_index = CNA_INVALID_HANDLE;
    ok &= cna_skinned_model_ext_destroy(donor) == CNA_RESULT_SUCCESS;
    ok &= cna_skinned_model_ext_destroy(model) == CNA_RESULT_SUCCESS;
    cna_demo_check(demo, ok, "destroy both empty resource-owner models");
    return ok;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    SkinnedDemo* state = (SkinnedDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    (void)game_time;
    (void)out_error;
    ++state->loads;
    if (!cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &device),
            "borrow the device for SkinnedModel part ownership")) {
        return CNA_RESULT_SUCCESS;
    }
    (void)audit_part_resources(state->demo, device);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    SkinnedDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int ready;

    cna_demo_init(&demo, "c_api.graphics.skinned_models", argc, argv, 1);
    (void)audit_animation_state(&demo);
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
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    ready = cna_game_create(&create_info, &game) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_create(game, &manager) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_set_graphics_profile(
                manager, CNA_GRAPHICS_PROFILE_HI_DEF) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_apply_changes(manager) == CNA_RESULT_SUCCESS;
    if (cna_demo_check(&demo, ready, "create a HiDef owner for retained part resources")) {
        cna_demo_check_result(
            &demo, cna_game_run_one_frame(game),
            "run one frame containing the resource-ownership audit");
        cna_demo_check(&demo, state.loads == 1, "the resource audit ran in one load callback");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
