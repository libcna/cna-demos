/* SPDX-License-Identifier: MS-PL */
/* Built-in AvatarAnimation timelines, bones, expressions and ownership through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static void init_info(CNA_AvatarAnimationInfo* info)
{
    memset(info, 0xA5, sizeof(*info));
    info->struct_size = (uint32_t)sizeof(*info);
    info->struct_version = UINT32_C(1);
}

static int info_is_live_clip(const CNA_AvatarAnimationInfo* info)
{
    return info->struct_size == (uint32_t)sizeof(*info) &&
        info->struct_version == UINT32_C(1) &&
        info->bone_transform_count == CNA_AVATAR_RENDERER_BONE_COUNT &&
        info->is_disposed == CNA_FALSE && info->reserved[0] == 0U &&
        info->reserved[1] == 0U && info->reserved[2] == 0U &&
        info->current_position_ticks >= INT64_C(0) &&
        info->length_ticks >= INT64_C(25000000) &&
        info->length_ticks <= INT64_C(100000000);
}

static float matrix_distance(CNA_Matrix left, CNA_Matrix right)
{
    return fabsf(left.m11 - right.m11) + fabsf(left.m12 - right.m12) +
        fabsf(left.m13 - right.m13) + fabsf(left.m14 - right.m14) +
        fabsf(left.m21 - right.m21) + fabsf(left.m22 - right.m22) +
        fabsf(left.m23 - right.m23) + fabsf(left.m24 - right.m24) +
        fabsf(left.m31 - right.m31) + fabsf(left.m32 - right.m32) +
        fabsf(left.m33 - right.m33) + fabsf(left.m34 - right.m34) +
        fabsf(left.m41 - right.m41) + fabsf(left.m42 - right.m42) +
        fabsf(left.m43 - right.m43) + fabsf(left.m44 - right.m44);
}

static CNA_Matrix filled(float value)
{
    CNA_Matrix result;
    result.m11 = result.m12 = result.m13 = result.m14 = value;
    result.m21 = result.m22 = result.m23 = result.m24 = value;
    result.m31 = result.m32 = result.m33 = result.m34 = value;
    result.m41 = result.m42 = result.m43 = result.m44 = value;
    return result;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_AvatarAnimationHandle animation = CNA_INVALID_HANDLE;
    CNA_AvatarAnimationHandle wave = CNA_INVALID_HANDLE;
    CNA_AvatarAnimationHandle celebrate = CNA_INVALID_HANDLE;
    CNA_AvatarAnimationHandle released = CNA_INVALID_HANDLE;
    CNA_AvatarAnimationInfo info;
    CNA_AvatarAnimationInfo malformed;
    unsigned char malformed_before[sizeof(CNA_AvatarAnimationInfo)];
    CNA_AvatarExpression expression;
    CNA_AvatarExpression malformed_expression;
    unsigned char expression_before[sizeof(CNA_AvatarExpression)];
    CNA_Matrix start;
    CNA_Matrix moved;
    CNA_Matrix sentinel;
    CNA_Matrix sentinel_before;
    int64_t length = INT64_C(0);
    int all_presets = 1;
    uint32_t preset;

    cna_demo_init(&demo, "c_api.gamer.avatar_animations", argc, argv, 1);

    animation = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_avatar_animation_create(
            CNA_AVATAR_ANIMATION_PRESET_MAXIMUM + UINT32_C(1), &animation) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            animation == CNA_INVALID_HANDLE,
        "an undefined animation preset is rejected without publishing a handle");
    for (preset = 0U; preset <= CNA_AVATAR_ANIMATION_PRESET_MAXIMUM; ++preset) {
        animation = CNA_INVALID_HANDLE;
        init_info(&info);
        if (cna_avatar_animation_create(preset, &animation) != CNA_RESULT_SUCCESS ||
            cna_avatar_animation_get_info(animation, &info) != CNA_RESULT_SUCCESS ||
            !info_is_live_clip(&info) || info.current_position_ticks != INT64_C(0) ||
            cna_avatar_animation_destroy(animation) != CNA_RESULT_SUCCESS) {
            all_presets = 0;
            if (animation != CNA_INVALID_HANDLE) {
                (void)cna_avatar_animation_destroy(animation);
            }
        }
        animation = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo, all_presets,
        "all 31 built-in presets expose a real 2.5-to-10-second 71-bone timeline");

    if (!cna_demo_check_result(
            &demo,
            cna_avatar_animation_create(CNA_AVATAR_ANIMATION_PRESET_WAVE, &wave),
            "create a wave clip for timeline sampling") ||
        !cna_demo_check_result(
            &demo,
            cna_avatar_animation_create(CNA_AVATAR_ANIMATION_PRESET_CELEBRATE, &celebrate),
            "create a celebration clip for facial keys")) {
        goto cleanup;
    }
    init_info(&info);
    cna_demo_check(
        &demo,
        cna_avatar_animation_get_info(wave, &info) == CNA_RESULT_SUCCESS &&
            info_is_live_clip(&info) && info.current_position_ticks == INT64_C(0),
        "a new wave reports live versioned state, zero position and zero reserve");
    length = info.length_ticks;
    memset(&malformed, 0xA5, sizeof(malformed));
    malformed.struct_size = (uint32_t)sizeof(malformed);
    malformed.struct_version = UINT32_C(2);
    memcpy(malformed_before, &malformed, sizeof(malformed));
    cna_demo_check(
        &demo,
        cna_avatar_animation_get_info(wave, &malformed) == CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&malformed, malformed_before, sizeof(malformed)) == 0,
        "an unknown animation-info version is rejected without changing the output");

    cna_demo_check_result(
        &demo,
        cna_avatar_animation_get_bone_transform_at(
            wave, (int32_t)CNA_AVATAR_BONE_ELBOW_RIGHT, &start),
        "sample the wave's initial right-elbow transform");
    cna_demo_check(
        &demo,
        cna_avatar_animation_set_current_position(wave, INT64_C(12000000)) ==
                CNA_RESULT_SUCCESS &&
            cna_avatar_animation_get_bone_transform_at(
                wave, (int32_t)CNA_AVATAR_BONE_ELBOW_RIGHT, &moved) ==
                CNA_RESULT_SUCCESS &&
            matrix_distance(start, moved) > 0.01F &&
            cna_avatar_animation_get_info(wave, &info) == CNA_RESULT_SUCCESS &&
            info.current_position_ticks == INT64_C(12000000),
        "sampling at 1.2 seconds visibly changes the local right-elbow transform");
    cna_demo_check(
        &demo,
        cna_avatar_animation_set_current_position(wave, length + INT64_C(10000000)) ==
                CNA_RESULT_SUCCESS &&
            cna_avatar_animation_get_info(wave, &info) == CNA_RESULT_SUCCESS &&
            info.current_position_ticks == length &&
            cna_avatar_animation_set_current_position(wave, -INT64_C(1)) ==
                CNA_RESULT_SUCCESS &&
            cna_avatar_animation_get_info(wave, &info) == CNA_RESULT_SUCCESS &&
            info.current_position_ticks == INT64_C(0),
        "direct position changes clamp above the clip length and below zero");
    cna_demo_check(
        &demo,
        cna_avatar_animation_update(wave, INT64_C(10000000), CNA_FALSE) ==
                CNA_RESULT_SUCCESS &&
            cna_avatar_animation_get_info(wave, &info) == CNA_RESULT_SUCCESS &&
            info.current_position_ticks == INT64_C(10000000) &&
            cna_avatar_animation_set_current_position(wave, INT64_C(0)) ==
                CNA_RESULT_SUCCESS &&
            cna_avatar_animation_update(
                wave, length * INT64_C(2) + INT64_C(12345), CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_avatar_animation_get_info(wave, &info) == CNA_RESULT_SUCCESS &&
            info.current_position_ticks == INT64_C(12345),
        "non-looping update advances exactly and looping update wraps by its remainder");
    cna_demo_check(
        &demo,
        cna_avatar_animation_update(wave, INT64_C(1), (CNA_Bool)2) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_avatar_animation_get_info(wave, &info) == CNA_RESULT_SUCCESS &&
            info.current_position_ticks == INT64_C(12345),
        "a noncanonical loop flag is rejected before changing animation position");

    memset(&expression, 0xA5, sizeof(expression));
    expression.struct_size = (uint32_t)sizeof(expression);
    expression.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_avatar_animation_get_expression(celebrate, &expression) ==
                CNA_RESULT_SUCCESS &&
            expression.mouth == CNA_AVATAR_MOUTH_HAPPY &&
            cna_avatar_animation_set_current_position(celebrate, INT64_C(10000000)) ==
                CNA_RESULT_SUCCESS &&
            cna_avatar_animation_get_expression(celebrate, &expression) ==
                CNA_RESULT_SUCCESS &&
            expression.mouth == CNA_AVATAR_MOUTH_LAUGHING &&
            expression.left_eye == CNA_AVATAR_EYE_LAUGHING &&
            expression.right_eye == CNA_AVATAR_EYE_LAUGHING &&
            expression.left_eyebrow == CNA_AVATAR_EYEBROW_RAISED &&
            expression.right_eyebrow == CNA_AVATAR_EYEBROW_RAISED,
        "the celebration clip changes from a happy face to its keyed laughing expression");
    memset(&malformed_expression, 0xA5, sizeof(malformed_expression));
    malformed_expression.struct_size = (uint32_t)sizeof(malformed_expression);
    malformed_expression.struct_version = UINT32_C(2);
    memcpy(expression_before, &malformed_expression, sizeof(malformed_expression));
    cna_demo_check(
        &demo,
        cna_avatar_animation_get_expression(celebrate, &malformed_expression) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&malformed_expression, expression_before, sizeof(malformed_expression)) == 0,
        "an unknown expression structure version is rejected atomically");

    sentinel = filled(9.0F);
    sentinel_before = sentinel;
    cna_demo_check(
        &demo,
        cna_avatar_animation_get_bone_transform_at(wave, -1, &sentinel) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&sentinel, &sentinel_before, sizeof(sentinel)) == 0 &&
            cna_avatar_animation_get_bone_transform_at(
                wave, CNA_AVATAR_RENDERER_BONE_COUNT, &sentinel) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&sentinel, &sentinel_before, sizeof(sentinel)) == 0,
        "bone sampling rejects both bounds without changing the transform output");

    released = wave;
    cna_demo_check_result(
        &demo, cna_avatar_animation_destroy(wave),
        "dispose and release the wave animation once");
    wave = CNA_INVALID_HANDLE;
    init_info(&info);
    info.current_position_ticks = INT64_C(999);
    cna_demo_check(
        &demo,
        cna_avatar_animation_get_info(released, &info) == CNA_RESULT_INVALID_HANDLE &&
            info.current_position_ticks == INT64_C(999) &&
            cna_avatar_animation_destroy(released) == CNA_RESULT_INVALID_HANDLE,
        "a released animation rejects stale inspection and repeated destruction atomically");

cleanup:
    if (animation != CNA_INVALID_HANDLE) {
        (void)cna_avatar_animation_destroy(animation);
    }
    if (celebrate != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_avatar_animation_destroy(celebrate),
            "release the celebration animation");
    }
    if (wave != CNA_INVALID_HANDLE) {
        (void)cna_avatar_animation_destroy(wave);
    }
    return cna_demo_finish(&demo);
}
