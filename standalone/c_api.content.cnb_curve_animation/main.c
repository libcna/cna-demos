/* SPDX-License-Identifier: MS-PL */
/* CNB Curve and AnimationClip codecs plus the shared keyframe byte format through C. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static int bytes_are(const uint8_t* bytes, size_t count, uint8_t expected)
{
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != expected) {
            return 0;
        }
    }
    return 1;
}

static CNA_KeyframeEXT make_key(double time, float x)
{
    CNA_KeyframeEXT key;
    memset(&key, 0, sizeof(key));
    key.time_seconds = time;
    key.translation.x = x;
    key.translation.y = x + 1.0F;
    key.translation.z = x + 2.0F;
    key.rotation.w = 1.0F;
    key.scale.x = 1.0F;
    key.scale.y = 2.0F;
    key.scale.z = 3.0F;
    return key;
}

static int key_is(const CNA_KeyframeEXT* key, double time, float x)
{
    return key->time_seconds == time && key->translation.x == x &&
           key->translation.y == x + 1.0F && key->translation.z == x + 2.0F &&
           key->rotation.x == 0.0F && key->rotation.y == 0.0F &&
           key->rotation.z == 0.0F && key->rotation.w == 1.0F &&
           key->scale.x == 1.0F && key->scale.y == 2.0F && key->scale.z == 3.0F;
}

static int curve_is(CNA_CurveHandle curve)
{
    CNA_CurveKeyCollectionHandle keys = CNA_INVALID_HANDLE;
    CNA_CurveLoopType pre = UINT32_MAX;
    CNA_CurveLoopType post = UINT32_MAX;
    CNA_CurveKey first;
    CNA_CurveKey second;
    uint64_t count = UINT64_C(0);
    int ok;

    memset(&first, 0, sizeof(first));
    memset(&second, 0, sizeof(second));
    ok = cna_curve_get_pre_loop(curve, &pre) == CNA_RESULT_SUCCESS &&
         pre == CNA_CURVE_LOOP_OSCILLATE &&
         cna_curve_get_post_loop(curve, &post) == CNA_RESULT_SUCCESS &&
         post == CNA_CURVE_LOOP_CYCLE_OFFSET &&
         cna_curve_get_keys(curve, &keys) == CNA_RESULT_SUCCESS &&
         cna_curve_key_collection_get_count(keys, &count) == CNA_RESULT_SUCCESS &&
         count == UINT64_C(2) &&
         cna_curve_key_collection_get(keys, INT32_C(0), &first) == CNA_RESULT_SUCCESS &&
         cna_curve_key_collection_get(keys, INT32_C(1), &second) == CNA_RESULT_SUCCESS &&
         first.position == 0.0F && first.value == 1.0F && first.tangent_in == 0.25F &&
         first.tangent_out == 0.5F && first.continuity == CNA_CURVE_CONTINUITY_SMOOTH &&
         second.position == 2.0F && second.value == -3.0F && second.tangent_in == 1.5F &&
         second.tangent_out == -1.5F && second.continuity == CNA_CURVE_CONTINUITY_STEP;
    if (keys != CNA_INVALID_HANDLE &&
        cna_curve_key_collection_destroy(keys) != CNA_RESULT_SUCCESS) {
        ok = 0;
    }
    return ok;
}

static int release_curve(CNA_CurveHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_curve_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_clip(CNA_CnbAnimationClipHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_animation_clip_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_reader(CNA_CnbReaderHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_reader_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_writer(CNA_CnbByteWriterHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_byte_writer_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_document(CNA_CnbDocumentHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_document_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_CurveHandle curve = CNA_INVALID_HANDLE;
    CNA_CurveHandle decoded_curve = CNA_INVALID_HANDLE;
    CNA_CurveKeyCollectionHandle curve_keys = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle curve_document = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle clip_document = CNA_INVALID_HANDLE;
    CNA_CnbAnimationClipHandle decoded_clip = CNA_INVALID_HANDLE;
    CNA_CnbAnimationClipHandle stale_clip = CNA_INVALID_HANDLE;
    CNA_CnbByteWriterHandle writer = CNA_INVALID_HANDLE;
    CNA_CnbByteWriterHandle seconds_writer = CNA_INVALID_HANDLE;
    CNA_CnbReaderHandle reader = CNA_INVALID_HANDLE;
    CNA_CnbReaderHandle seconds_reader = CNA_INVALID_HANDLE;
    CNA_CurveKey curve_key;
    CNA_KeyframeEXT keys[3];
    CNA_KeyframeEXT copied[3];
    CNA_KeyframeEXT before[3];
    CNA_KeyframeEXT shared;
    CNA_KeyframeEXT read_key;
    CNA_BoneTrackEXTDescriptor tracks[2];
    CNA_AnimationClipEXTDescriptor clip;
    uint8_t curve_image[16384];
    uint8_t clip_image[32768];
    uint8_t short_image[32768];
    uint8_t key_bytes[64];
    uint8_t seconds_bytes[8];
    CNA_ClipTargetSpaceEXT target_space = UINT32_MAX;
    double duration = 0.0;
    double seconds = 0.0;
    int32_t bone = INT32_C(-1);
    uint64_t track_count = UINT64_C(0);
    uint64_t key_count = UINT64_C(0);
    uint64_t image_size = UINT64_C(0);
    uint64_t second_size = UINT64_C(0);
    uint64_t byte_count = UINT64_C(0);
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_curve_animation", argc, argv, 1);
    cna_demo_check(
        &demo,
        CNA_CNB_CURVE_SCHEMA_VERSION == UINT32_C(1) &&
            CNA_CNB_CURVE_KEY_STRIDE == UINT32_C(20) &&
            CNA_CNB_ANIMATION_CLIP_SCHEMA_VERSION == UINT32_C(1) &&
            CNA_CNB_ANIMATION_TRACK_STRIDE == UINT32_C(12) &&
            CNA_CNB_ANIMATION_KEY_STRIDE == UINT32_C(48),
        "Curve and AnimationClip schemas publish their fixed versions and row strides");

    if (!cna_demo_check_result(&demo, cna_curve_create(&curve), "create a runtime Curve value")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_curve_set_pre_loop(curve, CNA_CURVE_LOOP_OSCILLATE) == CNA_RESULT_SUCCESS &&
            cna_curve_set_post_loop(curve, CNA_CURVE_LOOP_CYCLE_OFFSET) == CNA_RESULT_SUCCESS &&
            cna_curve_get_keys(curve, &curve_keys) == CNA_RESULT_SUCCESS &&
            cna_curve_key_init_full(
                0.0F, 1.0F, 0.25F, 0.5F, CNA_CURVE_CONTINUITY_SMOOTH, &curve_key) ==
                CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_add(curve_keys, curve_key) == CNA_RESULT_SUCCESS &&
            cna_curve_key_init_full(
                2.0F, -3.0F, 1.5F, -1.5F, CNA_CURVE_CONTINUITY_STEP, &curve_key) ==
                CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_add(curve_keys, curve_key) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_destroy(curve_keys) == CNA_RESULT_SUCCESS && curve_is(curve),
        "build two exact Curve keys with distinct loop and continuity behavior");
    curve_keys = CNA_INVALID_HANDLE;
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_curve(
                curve, text_view("Curves/Fade"), NULL, UINT64_C(0), &image_size) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                image_size > UINT64_C(0) && image_size <= sizeof(curve_image),
            "size a complete Curve document from the ordinary runtime handle")) {
        goto cleanup;
    }
    memset(short_image, 0xA5, sizeof(short_image));
    cna_demo_check(
        &demo,
        cna_cnb_encode_curve(
            curve, text_view("Curves/Fade"), short_image, image_size - UINT64_C(1),
            &second_size) == CNA_RESULT_BUFFER_TOO_SMALL &&
            second_size == image_size && bytes_are(short_image, sizeof(short_image), 0xA5U),
        "a short Curve destination is sized without a partial write");
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_curve(
                curve, text_view("Curves/Fade"), curve_image, sizeof(curve_image),
                &second_size) == CNA_RESULT_SUCCESS &&
                second_size == image_size &&
                cna_cnb_document_parse(
                    curve_image, image_size, text_view("curve-memory"), NULL,
                    &curve_document) == CNA_RESULT_SUCCESS &&
                cna_cnb_decode_curve(curve_document, &decoded_curve) == CNA_RESULT_SUCCESS,
            "encode, parse and decode the complete Curve document")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_curve_destroy(curve) == CNA_RESULT_SUCCESS && curve_is(decoded_curve),
        "the decoded ordinary Curve independently restores every key and loop");
    curve = CNA_INVALID_HANDLE;

    keys[0] = make_key(0.0, 0.0F);
    keys[1] = make_key(0.5, 2.0F);
    keys[2] = make_key(1.0, 4.0F);
    memset(tracks, 0, sizeof(tracks));
    tracks[0].bone_index = INT32_C(3);
    tracks[0].keyframes = keys;
    tracks[0].keyframe_count = UINT64_C(3);
    tracks[1].bone_index = INT32_C(7);
    tracks[1].keyframes = keys;
    tracks[1].keyframe_count = UINT64_C(2);
    memset(&clip, 0, sizeof(clip));
    clip.duration_seconds = 1.0;
    clip.tracks = tracks;
    clip.track_count = UINT64_C(2);
    image_size = UINT64_C(0);
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_animation_clip(
                &clip, CNA_CLIP_TARGET_SPACE_SCENE_NODE_EXT, text_view("Clips/Walk"),
                NULL, UINT64_C(0), &image_size) == CNA_RESULT_BUFFER_TOO_SMALL &&
                image_size > UINT64_C(0) && image_size <= sizeof(clip_image),
            "size a scene-node AnimationClip document from borrowed tracks")) {
        goto cleanup;
    }
    memset(short_image, 0xA5, sizeof(short_image));
    cna_demo_check(
        &demo,
        cna_cnb_encode_animation_clip(
            &clip, CNA_CLIP_TARGET_SPACE_SCENE_NODE_EXT, text_view("Clips/Walk"),
            short_image, image_size - UINT64_C(1), &second_size) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            second_size == image_size && bytes_are(short_image, sizeof(short_image), 0xA5U),
        "a short AnimationClip destination is sized without a partial write");
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_animation_clip(
                &clip, CNA_CLIP_TARGET_SPACE_SCENE_NODE_EXT, text_view("Clips/Walk"),
                clip_image, sizeof(clip_image), &second_size) == CNA_RESULT_SUCCESS &&
                second_size == image_size &&
                cna_cnb_document_parse(
                    clip_image, image_size, text_view("clip-memory"), NULL,
                    &clip_document) == CNA_RESULT_SUCCESS &&
                cna_cnb_decode_animation_clip(clip_document, &decoded_clip) == CNA_RESULT_SUCCESS,
            "encode, parse and decode the complete AnimationClip document")) {
        goto cleanup;
    }
    memset(keys, 0xEE, sizeof(keys));
    cna_demo_check(
        &demo,
        cna_cnb_animation_clip_get(
            decoded_clip, &duration, &track_count, &target_space) == CNA_RESULT_SUCCESS &&
            duration == 1.0 && track_count == UINT64_C(2) &&
            target_space == CNA_CLIP_TARGET_SPACE_SCENE_NODE_EXT &&
            cna_cnb_animation_clip_get_track(
                decoded_clip, UINT64_C(0), &bone, &key_count) == CNA_RESULT_SUCCESS &&
            bone == INT32_C(3) && key_count == UINT64_C(3) &&
            cna_cnb_animation_clip_get_track(
                decoded_clip, UINT64_C(1), &bone, &key_count) == CNA_RESULT_SUCCESS &&
            bone == INT32_C(7) && key_count == UINT64_C(2),
        "decoded clip shape, target space and tracks outlive the caller's borrowed arrays");
    bone = INT32_C(99);
    key_count = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_cnb_animation_clip_get_track(
            decoded_clip, UINT64_C(2), &bone, &key_count) == CNA_RESULT_INVALID_ARGUMENT &&
            bone == INT32_C(99) && key_count == UINT64_C(99),
        "a track index at Count is refused without changing either output");
    memset(copied, 0xA5, sizeof(copied));
    memcpy(before, copied, sizeof(copied));
    cna_demo_check(
        &demo,
        cna_cnb_animation_clip_copy_keyframes(
            decoded_clip, UINT64_C(0), copied, UINT64_C(2), &key_count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            key_count == UINT64_C(3) && memcmp(copied, before, sizeof(copied)) == 0 &&
            cna_cnb_animation_clip_copy_keyframes(
                decoded_clip, UINT64_C(0), copied, UINT64_C(3), &key_count) ==
                CNA_RESULT_SUCCESS &&
            key_count == UINT64_C(3) && key_is(&copied[0], 0.0, 0.0F) &&
            key_is(&copied[1], 0.5, 2.0F) && key_is(&copied[2], 1.0, 4.0F),
        "keyframe copying is atomic when short and exact when fully sized");
    cna_demo_check(
        &demo,
        cna_cnb_encode_animation_clip(
            &clip, CNA_CLIP_TARGET_SPACE_MAXIMUM_EXT + UINT32_C(1), text_view(""),
            clip_image, sizeof(clip_image), &second_size) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_encode_animation_clip(
                NULL, CNA_CLIP_TARGET_SPACE_JOINT_PALETTE_EXT, text_view(""),
                clip_image, sizeof(clip_image), &second_size) == CNA_RESULT_INVALID_ARGUMENT,
        "an unknown target space and null borrowed clip are caller argument errors");
    cna_demo_check_result(
        &demo, cna_cnb_document_destroy(clip_document),
        "release the AnimationClip document before its decoded value");
    clip_document = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_cnb_animation_clip_get(
            decoded_clip, &duration, &track_count, &target_space) == CNA_RESULT_SUCCESS &&
            track_count == UINT64_C(2),
        "the decoded clip independently survives its source document");
    stale_clip = decoded_clip;
    cna_demo_check_result(
        &demo, cna_cnb_animation_clip_destroy(decoded_clip),
        "release the decoded AnimationClip value");
    decoded_clip = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_cnb_animation_clip_get(stale_clip, NULL, NULL, NULL) == CNA_RESULT_INVALID_HANDLE,
        "the released AnimationClip rejects stale access");

    shared = make_key(2.75, 1.0F);
    if (!cna_demo_check(
            &demo,
            cna_cnb_byte_writer_create(&writer) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_keyframe(writer, &shared) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_get_size(writer, &byte_count) == CNA_RESULT_SUCCESS &&
                byte_count == (uint64_t)CNA_CNB_ANIMATION_KEY_STRIDE &&
                cna_cnb_byte_writer_copy_bytes(
                    writer, key_bytes, sizeof(key_bytes), &second_size) == CNA_RESULT_SUCCESS &&
                second_size == byte_count,
            "the shared writer emits exactly one published 48-byte keyframe row")) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo,
            cna_cnb_reader_create(
                key_bytes, byte_count, text_view("shared-keyframe"), NULL, &reader),
            "open a bounded reader over the shared keyframe row")) {
        goto cleanup;
    }
    memset(&read_key, 0, sizeof(read_key));
    cna_demo_check(
        &demo,
        cna_cnb_reader_read_keyframe(reader, &read_key) == CNA_RESULT_SUCCESS &&
            key_is(&read_key, 2.75, 1.0F) &&
            cna_cnb_reader_read_keyframe(reader, &read_key) == CNA_RESULT_IO,
        "the shared reader restores the row exactly and rejects a truncated second read");
    shared.time_seconds = INFINITY;
    cna_demo_check(
        &demo,
        cna_cnb_byte_writer_write_keyframe(writer, &shared) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_byte_writer_get_size(writer, &second_size) == CNA_RESULT_SUCCESS &&
            second_size == byte_count,
        "an unrepresentable keyframe time is refused before changing the writer");

    if (!cna_demo_check(
            &demo,
            cna_cnb_byte_writer_create(&seconds_writer) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_f64(seconds_writer, 12.5) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_copy_bytes(
                    seconds_writer, seconds_bytes, sizeof(seconds_bytes), &second_size) ==
                    CNA_RESULT_SUCCESS &&
                second_size == UINT64_C(8) &&
                cna_cnb_reader_create(
                    seconds_bytes, second_size, text_view("seconds"), NULL,
                    &seconds_reader) == CNA_RESULT_SUCCESS,
            "write and open one standalone seconds value")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_reader_read_seconds(seconds_reader, text_view("duration"), &seconds) ==
                CNA_RESULT_SUCCESS &&
            seconds == 12.5,
        "seconds reading returns an exactly representable TimeSpan value");
    cna_demo_check_result(&demo, cna_cnb_reader_destroy(seconds_reader), "release the seconds reader");
    seconds_reader = CNA_INVALID_HANDLE;
    cna_demo_check_result(&demo, cna_cnb_byte_writer_destroy(seconds_writer), "release the seconds writer");
    seconds_writer = CNA_INVALID_HANDLE;
    if (!cna_demo_check(
            &demo,
            cna_cnb_byte_writer_create(&seconds_writer) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_f64(seconds_writer, INFINITY) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_copy_bytes(
                    seconds_writer, seconds_bytes, sizeof(seconds_bytes), &second_size) ==
                    CNA_RESULT_SUCCESS &&
                cna_cnb_reader_create(
                    seconds_bytes, second_size, text_view("invalid-seconds"), NULL,
                    &seconds_reader) == CNA_RESULT_SUCCESS,
            "open an encoded non-finite seconds value for guarded reading")) {
        goto cleanup;
    }
    seconds = 77.0;
    cna_demo_check(
        &demo,
        cna_cnb_reader_read_seconds(
            seconds_reader, text_view("the clip duration"), &seconds) == CNA_RESULT_IO &&
            seconds == 77.0,
        "an unrepresentable seconds value is a content failure that preserves output");

cleanup:
    if (curve_keys != CNA_INVALID_HANDLE) {
        releases_ok &= cna_curve_key_collection_destroy(curve_keys) == CNA_RESULT_SUCCESS;
        curve_keys = CNA_INVALID_HANDLE;
    }
    releases_ok &= release_reader(&seconds_reader);
    releases_ok &= release_reader(&reader);
    releases_ok &= release_writer(&seconds_writer);
    releases_ok &= release_writer(&writer);
    releases_ok &= release_clip(&decoded_clip);
    releases_ok &= release_document(&clip_document);
    releases_ok &= release_curve(&decoded_curve);
    releases_ok &= release_curve(&curve);
    releases_ok &= release_document(&curve_document);
    cna_demo_check(
        &demo, releases_ok,
        "release every remaining curve, clip, byte cursor and document handle");
    return cna_demo_finish(&demo);
}
