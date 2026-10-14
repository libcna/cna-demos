/* SPDX-License-Identifier: MS-PL */
/* SkinningData ownership, rig metadata and AnimationPlayer transforms through C. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

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

int main(int argc, char** argv)
{
    static const char expected_type[] =
        "Microsoft.Xna.Framework.Graphics.SkinningData";
    int32_t hierarchy[2] = {-1, 0};
    int32_t copied_hierarchy[2] = {99, 99};
    CNA_Matrix bind_pose[2] = {identity_matrix(), identity_matrix()};
    CNA_Matrix inverse_bind_pose[2] = {identity_matrix(), identity_matrix()};
    CNA_Matrix root_prefix[2] = {identity_matrix(), identity_matrix()};
    CNA_Matrix transforms[2];
    CNA_Matrix sentinel = identity_matrix();
    CNA_KeyframeEXT walk_keys[2] = {
        {0.0, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F, 1.0F},
         {1.0F, 1.0F, 1.0F}},
        {1.0, {2.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F, 1.0F},
         {1.0F, 1.0F, 1.0F}}};
    CNA_BoneTrackEXTDescriptor walk_track = {0, 0U, walk_keys, UINT64_C(2)};
    CNA_NamedAnimationClipEXTDescriptor clips[2];
    CNA_SkinningDataDescriptor descriptor;
    CNA_SkinningDataHandle data = CNA_INVALID_HANDLE;
    CNA_SkinningDataHandle rejected = UINT64_MAX;
    CNA_AnimationPlayerHandle player = CNA_INVALID_HANDLE;
    CNA_KeyframeEXT copied_keys[2];
    CNA_Bool found = CNA_FALSE;
    CNA_Bool has_clip = CNA_TRUE;
    CNA_ClipTargetSpaceEXT target_space = CNA_CLIP_TARGET_SPACE_MAXIMUM_EXT;
    uint64_t count = UINT64_MAX;
    uint64_t byte_count = UINT64_MAX;
    uint64_t track_count = UINT64_MAX;
    double duration = -1.0;
    double position = -1.0;
    int32_t bone_index = INT32_C(-99);
    int32_t root_node = INT32_MIN;
    char buffer[64];
    CnaDemo demo;
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.graphics.skinning_data_player", argc, argv, 1);
    bind_pose[1].m42 = 1.0F;
    root_prefix[0].m43 = 3.0F;
    memset(clips, 0, sizeof(clips));
    clips[0].name = text_view("Walk");
    clips[0].clip.duration_seconds = 1.0;
    clips[0].clip.tracks = &walk_track;
    clips[0].clip.track_count = UINT64_C(1);
    clips[1].name = text_view("Idle");
    clips[1].clip.duration_seconds = 2.0;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.bone_count = 2;
    descriptor.skeleton_hierarchy = hierarchy;
    descriptor.bind_pose = bind_pose;
    descriptor.inverse_bind_pose = inverse_bind_pose;
    descriptor.skeleton_root_prefix = root_prefix;
    descriptor.skeleton_root_prefix_count = UINT64_C(2);
    descriptor.clips = clips;
    descriptor.clip_count = UINT64_C(2);

    descriptor.skeleton_root_prefix_count = UINT64_C(1);
    cna_demo_check(
        &demo,
        cna_skinning_data_create(&descriptor, &rejected) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "reject a mismatched root-prefix count without returning a partial handle");
    descriptor.skeleton_root_prefix_count = UINT64_C(2);
    descriptor.reserved = UINT32_C(1);
    rejected = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_skinning_data_create(&descriptor, &rejected) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "reject nonzero reserved descriptor state atomically");
    descriptor.reserved = UINT32_C(0);
    if (!cna_demo_check_result(
            &demo, cna_skinning_data_create(&descriptor, &data),
            "deep-copy a two-bone skeleton, root prefix and two named clips")) {
        return cna_demo_finish(&demo);
    }

    hierarchy[1] = -1;
    bind_pose[1].m42 = 9.0F;
    inverse_bind_pose[0].m41 = 9.0F;
    root_prefix[0].m43 = 9.0F;
    walk_keys[1].translation.x = 9.0F;
    cna_demo_check(
        &demo,
        cna_skinning_data_get_bone_count(data, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_skinning_data_copy_skeleton_hierarchy(
                data, copied_hierarchy, UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) && copied_hierarchy[0] == 99 &&
            copied_hierarchy[1] == 99 &&
            cna_skinning_data_copy_skeleton_hierarchy(
                data, copied_hierarchy, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            copied_hierarchy[0] == -1 && copied_hierarchy[1] == 0,
        "the hierarchy is deeply copied and a short destination stays untouched");
    cna_demo_check(
        &demo,
        cna_skinning_data_copy_bind_pose(data, transforms, UINT64_C(2), &count) ==
                CNA_RESULT_SUCCESS &&
            transforms[1].m42 == 1.0F &&
            cna_skinning_data_copy_inverse_bind_pose(
                data, transforms, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            transforms[0].m41 == 0.0F &&
            cna_skinning_data_copy_skeleton_root_prefix(
                data, transforms, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && transforms[0].m43 == 3.0F,
        "bind, inverse-bind and root-prefix arrays own independent caller copies");

    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(
        &demo,
        cna_skinning_data_get_type_name_byte_count(data, &byte_count) ==
                CNA_RESULT_SUCCESS &&
            byte_count == sizeof(expected_type) - 1U &&
            cna_skinning_data_copy_type_name(data, buffer, UINT64_C(4), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == sizeof(expected_type) - 1U && buffer[0] == '#' &&
            cna_skinning_data_copy_type_name(
                data, buffer, sizeof(expected_type) - 1U, &byte_count) ==
                CNA_RESULT_SUCCESS &&
            memcmp(buffer, expected_type, sizeof(expected_type) - 1U) == 0,
        "the fixed SkinningData type name reports exact bytes and copies atomically");
    cna_demo_check(
        &demo,
        cna_skinning_data_get_clip_count(data, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_skinning_data_get_clip_name_byte_count_at(
                data, UINT64_C(0), &byte_count) == CNA_RESULT_SUCCESS &&
            byte_count == UINT64_C(4) &&
            cna_skinning_data_copy_clip_name_at(
                data, UINT64_C(0), buffer, UINT64_C(4), &byte_count) ==
                CNA_RESULT_SUCCESS &&
            memcmp(buffer, "Idle", 4U) == 0 &&
            cna_skinning_data_copy_clip_name_at(
                data, UINT64_C(1), buffer, UINT64_C(4), &byte_count) ==
                CNA_RESULT_SUCCESS &&
            memcmp(buffer, "Walk", 4U) == 0,
        "clip enumeration is sorted and copies exact un-terminated names");
    cna_demo_check(
        &demo,
        cna_skinning_data_get_clip_info(
            data, text_view("Walk"), &found, &duration, &track_count) ==
                CNA_RESULT_SUCCESS &&
            found == CNA_TRUE && duration == 1.0 && track_count == UINT64_C(1) &&
            cna_skinning_data_get_clip_info(
                data, text_view("Missing"), &found, &duration, &track_count) ==
                CNA_RESULT_SUCCESS &&
            found == CNA_FALSE && duration == 0.0 && track_count == UINT64_C(0),
        "exact clip lookup distinguishes a retained clip from ordinary absence");
    cna_demo_check(
        &demo,
        cna_skinning_data_copy_clip_track(
            data, text_view("Walk"), UINT64_C(0), &bone_index, copied_keys,
            UINT64_C(1), &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) && bone_index == INT32_C(-99) &&
            cna_skinning_data_copy_clip_track(
                data, text_view("Walk"), UINT64_C(0), &bone_index, copied_keys,
                UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            bone_index == INT32_C(0) && copied_keys[1].translation.x == 2.0F,
        "a clip track retains its bone and keys and refuses partial output");

    cna_demo_check(
        &demo,
        cna_skinning_data_get_clip_target_space_ext(
            data, UINT64_C(1), &target_space) == CNA_RESULT_SUCCESS &&
            target_space == CNA_CLIP_TARGET_SPACE_JOINT_PALETTE_EXT &&
            cna_skinning_data_set_clip_target_space_ext(
                data, UINT64_C(1), CNA_CLIP_TARGET_SPACE_SCENE_NODE_EXT) ==
                CNA_RESULT_SUCCESS &&
            cna_skinning_data_get_clip_target_space_ext(
                data, UINT64_C(1), &target_space) == CNA_RESULT_SUCCESS &&
            target_space == CNA_CLIP_TARGET_SPACE_SCENE_NODE_EXT &&
            cna_skinning_data_set_clip_target_space_ext(
                data, UINT64_C(1), CNA_CLIP_TARGET_SPACE_JOINT_PALETTE_EXT) ==
                CNA_RESULT_SUCCESS,
        "clip target space defaults to the joint palette and changes explicitly");
    cna_demo_check(
        &demo,
        cna_skinning_data_get_clip_target_space_ext(
            data, UINT64_C(2), &target_space) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_skinning_data_set_clip_target_space_ext(
                data, UINT64_C(1), CNA_CLIP_TARGET_SPACE_MAXIMUM_EXT + 1U) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "target-space routes reject an out-of-range clip and undefined identity");

    cna_demo_check(
        &demo,
        cna_skinning_data_get_skeleton_root_node_index_ext(data, &root_node) ==
                CNA_RESULT_SUCCESS &&
            root_node == -1 &&
            cna_skinning_data_get_skeleton_root_name_byte_count_ext(
                data, &byte_count) == CNA_RESULT_SUCCESS &&
            byte_count == UINT64_C(0),
        "rig-root metadata starts with no scene-node index or name");
    cna_demo_check(
        &demo,
        cna_skinning_data_set_skeleton_root_node_index_ext(data, INT32_C(4)) ==
                CNA_RESULT_SUCCESS &&
            cna_skinning_data_get_skeleton_root_node_index_ext(data, &root_node) ==
                CNA_RESULT_SUCCESS &&
            root_node == INT32_C(4),
        "the declared rig-root scene-node index round-trips exactly");
    cna_demo_check(
        &demo,
        cna_skinning_data_set_skeleton_root_name_ext(data, text_view("Armature")) ==
                CNA_RESULT_SUCCESS &&
            cna_skinning_data_get_skeleton_root_name_byte_count_ext(
                data, &byte_count) == CNA_RESULT_SUCCESS &&
            byte_count == UINT64_C(8) &&
            cna_skinning_data_copy_skeleton_root_name_ext(
                data, buffer, sizeof(buffer), &byte_count) == CNA_RESULT_SUCCESS &&
            byte_count == UINT64_C(8) && memcmp(buffer, "Armature", 8U) == 0,
        "the declared rig-root name reports and copies its exact UTF-8 bytes");
    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(
        &demo,
        cna_skinning_data_copy_skeleton_root_name_ext(
            data, buffer, UINT64_C(2), &byte_count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            byte_count == UINT64_C(8) && buffer[0] == '#',
        "a short rig-root-name destination is reported without a partial write");

    if (!cna_demo_check_result(
            &demo, cna_animation_player_create(data, &player),
            "create a player that retains the copied skinning data")) {
        releases_ok &= cna_skinning_data_destroy(data) == CNA_RESULT_SUCCESS;
        cna_demo_check(&demo, releases_ok, "release skinning data after player creation failure");
        return cna_demo_finish(&demo);
    }
    cna_demo_check(
        &demo,
        cna_animation_player_get_current_position(player, &position) ==
                CNA_RESULT_SUCCESS &&
            position == 0.0 &&
            cna_animation_player_get_current_clip_info(
                player, &has_clip, &duration, &track_count) == CNA_RESULT_SUCCESS &&
            has_clip == CNA_FALSE && duration == 0.0 && track_count == UINT64_C(0) &&
            cna_animation_player_get_current_clip_name_byte_count(
                player, &byte_count) == CNA_RESULT_SUCCESS &&
            byte_count == UINT64_C(0),
        "a new player has bind-pose position and no selected clip");
    cna_demo_check(
        &demo,
        cna_animation_player_copy_bone_transforms(
            player, transforms, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            transforms[1].m42 == 1.0F &&
            cna_animation_player_copy_world_transforms(
                player, transforms, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            transforms[0].m43 == 3.0F && transforms[1].m42 == 1.0F &&
            transforms[1].m43 == 3.0F &&
            cna_animation_player_copy_skin_transforms(
                player, transforms, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            transforms[0].m43 == 3.0F,
        "bind-pose local, model-space and final skin transforms include the root prefix");
    cna_demo_check(
        &demo,
        cna_animation_player_start_clip(player, text_view("Missing")) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_animation_player_start_clip(player, text_view("Walk")) ==
                CNA_RESULT_SUCCESS,
        "starting playback rejects an absent name and accepts the retained Walk clip");
    cna_demo_check(
        &demo,
        cna_animation_player_get_current_clip_info(
            player, &has_clip, &duration, &track_count) == CNA_RESULT_SUCCESS &&
            has_clip == CNA_TRUE && duration == 1.0 && track_count == UINT64_C(1) &&
            cna_animation_player_get_current_clip_name_byte_count(
                player, &byte_count) == CNA_RESULT_SUCCESS &&
            byte_count == UINT64_C(4),
        "current-clip metadata exposes exact duration, track and name sizes");
    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(
        &demo,
        cna_animation_player_copy_current_clip_name(
            player, buffer, UINT64_C(2), &byte_count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            byte_count == UINT64_C(4) && buffer[0] == '#' &&
            cna_animation_player_copy_current_clip_name(
                player, buffer, UINT64_C(4), &byte_count) == CNA_RESULT_SUCCESS &&
            memcmp(buffer, "Walk", 4U) == 0,
        "the current clip name copies exactly and short output remains atomic");

    sentinel.m41 = 77.0F;
    cna_demo_check(
        &demo,
        cna_animation_player_update(player, 0.5, CNA_TRUE, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_animation_player_get_current_position(player, &position) ==
                CNA_RESULT_SUCCESS &&
            position == 0.5 &&
            cna_animation_player_copy_bone_transforms(
                player, &sentinel, UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) && sentinel.m41 == 77.0F,
        "relative half-time playback advances and short transform output is atomic");
    cna_demo_check(
        &demo,
        cna_animation_player_copy_bone_transforms(
            player, transforms, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            nearly_equal(transforms[0].m41, 1.0F) &&
            cna_animation_player_copy_world_transforms(
                player, transforms, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            nearly_equal(transforms[0].m41, 1.0F) && transforms[0].m43 == 3.0F &&
            nearly_equal(transforms[1].m41, 1.0F) && transforms[1].m42 == 1.0F &&
            transforms[1].m43 == 3.0F &&
            cna_animation_player_copy_skin_transforms(
                player, transforms, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            nearly_equal(transforms[1].m41, 1.0F),
        "half-time local, world and skin transforms compose animation and hierarchy");
    cna_demo_check(
        &demo,
        cna_animation_player_update(player, 2.0, CNA_FALSE, CNA_FALSE) ==
                CNA_RESULT_SUCCESS &&
            cna_animation_player_get_current_position(player, &position) ==
                CNA_RESULT_SUCCESS &&
            position == 1.0 &&
            cna_animation_player_update(player, 1.5, CNA_FALSE, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_animation_player_get_current_position(player, &position) ==
                CNA_RESULT_SUCCESS &&
            position == 0.5,
        "absolute playback clamps at duration or wraps when looping is requested");
    cna_demo_check(
        &demo,
        cna_animation_player_update(player, NAN, CNA_FALSE, CNA_TRUE) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_animation_player_update(player, 0.0, (CNA_Bool)2U, CNA_TRUE) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_animation_player_update(player, 0.0, CNA_FALSE, (CNA_Bool)2U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_animation_player_get_current_position(player, &position) ==
                CNA_RESULT_SUCCESS &&
            position == 0.5,
        "non-finite time and invalid boolean identities leave the current position unchanged");

    cna_demo_check_result(
        &demo, cna_skinning_data_destroy(data),
        "release the original SkinningData while the player retains its state");
    data = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_animation_player_update(player, 0.25, CNA_FALSE, CNA_FALSE) ==
                CNA_RESULT_SUCCESS &&
            cna_animation_player_get_current_position(player, &position) ==
                CNA_RESULT_SUCCESS &&
            position == 0.25,
        "the player keeps the skeleton and clip alive after the source handle is released");
    releases_ok &= cna_animation_player_destroy(player) == CNA_RESULT_SUCCESS;
    cna_demo_check(
        &demo,
        releases_ok &&
            cna_animation_player_get_current_position(player, &position) ==
                CNA_RESULT_INVALID_HANDLE,
        "destroy the player and reject later access through its stale handle");
    return cna_demo_finish(&demo);
}
