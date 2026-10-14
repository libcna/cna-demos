/* SPDX-License-Identifier: MS-PL */
/* CNB Song and Video metadata, streaming references and document validation through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef CNA_Result (*TextSizeRoute)(CNA_CnbDocumentHandle, uint64_t*);
typedef CNA_Result (*TextCopyRoute)(CNA_CnbDocumentHandle, char*, uint64_t, uint64_t*);

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

static int text_is(
    CNA_CnbDocumentHandle document, TextSizeRoute size_route, TextCopyRoute copy_route,
    const char* expected)
{
    char text[64];
    char short_text[64];
    char short_before[64];
    uint64_t size = UINT64_C(0);
    uint64_t copied = UINT64_C(99);
    const uint64_t expected_size = (uint64_t)strlen(expected);

    memset(text, 0, sizeof(text));
    memset(short_text, 0xA5, sizeof(short_text));
    memcpy(short_before, short_text, sizeof(short_text));
    return expected_size > UINT64_C(0) &&
           size_route(document, &size) == CNA_RESULT_SUCCESS && size == expected_size &&
           copy_route(document, short_text, expected_size - UINT64_C(1), &copied) ==
               CNA_RESULT_BUFFER_TOO_SMALL &&
           copied == expected_size && memcmp(short_text, short_before, sizeof(short_text)) == 0 &&
           copy_route(document, text, sizeof(text), &copied) == CNA_RESULT_SUCCESS &&
           copied == expected_size && memcmp(text, expected, (size_t)expected_size) == 0;
}

static int external_reference_is(CNA_CnbDocumentHandle document, const char* expected)
{
    char text[64];
    uint64_t count = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    const uint64_t expected_size = (uint64_t)strlen(expected);
    return cna_cnb_document_get_external_reference_count(document, &count) ==
               CNA_RESULT_SUCCESS &&
           count == UINT64_C(1) &&
           cna_cnb_document_copy_external_reference_name(
               document, UINT64_C(0), text, sizeof(text), &copied) == CNA_RESULT_SUCCESS &&
           copied == expected_size && memcmp(text, expected, (size_t)expected_size) == 0;
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
    CNA_CnbDocumentHandle song = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle nameless = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle video = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle stale_song = CNA_INVALID_HANDLE;
    CNA_CnbVideoInfo video_info;
    CNA_CnbVideoInfo decoded_video;
    CNA_CnbVideoInfo broken_video;
    uint8_t song_image[16384];
    uint8_t song_repeat[16384];
    uint8_t video_image[16384];
    uint8_t short_image[16384];
    uint32_t asset_type = UINT32_C(0);
    uint32_t schema = UINT32_C(0);
    uint32_t duration = UINT32_C(0);
    uint64_t image_size = UINT64_C(0);
    uint64_t second_size = UINT64_C(0);
    uint64_t name_size = UINT64_C(99);
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_media_metadata", argc, argv, 1);
    cna_demo_check(
        &demo,
        CNA_CNB_MEDIA_SCHEMA_VERSION == UINT32_C(1) &&
            CNA_CNB_SONG_HEADER_FIXED_STRIDE == UINT32_C(8) &&
            CNA_CNB_VIDEO_HEADER_STRIDE == UINT32_C(24) &&
            CNA_CNB_MAX_VIDEO_DIMENSION == UINT32_C(65536),
        "the shared media schema publishes its fixed version, strides and dimension ceiling");

    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_song(
                text_view("Music/Theme.ogg"), text_view("Main Theme"), UINT32_C(195000),
                text_view("Music/Theme"), NULL, UINT64_C(0), &image_size) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                image_size > UINT64_C(0) && image_size <= sizeof(song_image),
            "size a named Song metadata document without embedding its stream")) {
        goto cleanup;
    }
    memset(short_image, 0xA5, sizeof(short_image));
    cna_demo_check(
        &demo,
        cna_cnb_encode_song(
            text_view("Music/Theme.ogg"), text_view("Main Theme"), UINT32_C(195000),
            text_view("Music/Theme"), short_image, image_size - UINT64_C(1), &second_size) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            second_size == image_size && bytes_are(short_image, sizeof(short_image), 0xA5U),
        "a short Song destination is sized without a partial write");
    cna_demo_check(
        &demo,
        cna_cnb_encode_song(
            text_view("Music/Theme.ogg"), text_view("Main Theme"), UINT32_C(195000),
            text_view("Music/Theme"), song_image, sizeof(song_image), &second_size) ==
                CNA_RESULT_SUCCESS &&
            second_size == image_size &&
            cna_cnb_encode_song(
                text_view("Music/Theme.ogg"), text_view("Main Theme"), UINT32_C(195000),
                text_view("Music/Theme"), song_repeat, sizeof(song_repeat), &second_size) ==
                CNA_RESULT_SUCCESS &&
            second_size == image_size &&
            memcmp(song_image, song_repeat, (size_t)image_size) == 0,
        "complete Song encoding is exact and repeatable");
    if (!cna_demo_check_result(
            &demo,
            cna_cnb_document_parse(
                song_image, image_size, text_view("song-memory"), NULL, &song),
            "parse the complete Song metadata document")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_document_get_asset_type_id(song, &asset_type) == CNA_RESULT_SUCCESS &&
            asset_type == CNA_CNB_ASSET_TYPE_SONG &&
            cna_cnb_document_get_asset_schema_version(song, &schema) == CNA_RESULT_SUCCESS &&
            schema == CNA_CNB_MEDIA_SCHEMA_VERSION &&
            cna_cnb_decode_song_duration_milliseconds(song, &duration) == CNA_RESULT_SUCCESS &&
            duration == UINT32_C(195000),
        "the Song document restores its identity, schema and millisecond duration");
    cna_demo_check(
        &demo,
        text_is(
            song, cna_cnb_decode_song_stream_reference_size,
            cna_cnb_decode_song_stream_reference, "Music/Theme.ogg") &&
            text_is(
                song, cna_cnb_decode_song_name_size, cna_cnb_decode_song_name, "Main Theme"),
        "Song stream and display names size and copy atomically");
    cna_demo_check(
        &demo, external_reference_is(song, "Music/Theme.ogg"),
        "the specialized Song stream reference equals its single XREF dependency");

    second_size = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_cnb_encode_song(
            text_view("Music/Sting.ogg"), text_view(""), UINT32_C(0), text_view(""),
            song_repeat, sizeof(song_repeat), &second_size) == CNA_RESULT_SUCCESS &&
            cna_cnb_document_parse(
                song_repeat, second_size, text_view("nameless-song"), NULL, &nameless) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_decode_song_name_size(nameless, &name_size) == CNA_RESULT_SUCCESS &&
            name_size == UINT64_C(0) &&
            cna_cnb_decode_song_name(
                nameless, NULL, UINT64_C(0), &name_size) == CNA_RESULT_SUCCESS &&
            name_size == UINT64_C(0) &&
            cna_cnb_decode_song_duration_milliseconds(nameless, &duration) ==
                CNA_RESULT_SUCCESS &&
            duration == UINT32_C(0),
        "an unknown-duration Song may carry an explicitly empty display name");
    second_size = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_cnb_encode_song(
            text_view(""), text_view("Invalid"), UINT32_C(0), text_view(""),
            song_repeat, sizeof(song_repeat), &second_size) == CNA_RESULT_IO,
        "a Song cannot encode without its external stream reference");

    memset(&video_info, 0, sizeof(video_info));
    video_info.struct_size = (uint32_t)sizeof(video_info);
    video_info.struct_version = CNA_CNB_VIDEO_INFO_STRUCT_VERSION;
    video_info.duration_milliseconds = UINT32_C(42000);
    video_info.width = UINT32_C(1280);
    video_info.height = UINT32_C(720);
    video_info.frames_per_second = 29.97F;
    video_info.soundtrack_type = CNA_VIDEO_SOUNDTRACK_TYPE_MUSIC_AND_DIALOG;
    image_size = UINT64_C(0);
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_video(
                text_view("Videos/Intro.mp4"), &video_info, text_view("Videos/Intro"),
                NULL, UINT64_C(0), &image_size) == CNA_RESULT_BUFFER_TOO_SMALL &&
                image_size > UINT64_C(0) && image_size <= sizeof(video_image),
            "size a Video metadata document without embedding its stream")) {
        goto cleanup;
    }
    memset(short_image, 0xA5, sizeof(short_image));
    cna_demo_check(
        &demo,
        cna_cnb_encode_video(
            text_view("Videos/Intro.mp4"), &video_info, text_view("Videos/Intro"),
            short_image, image_size - UINT64_C(1), &second_size) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            second_size == image_size && bytes_are(short_image, sizeof(short_image), 0xA5U),
        "a short Video destination is sized without a partial write");
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_video(
                text_view("Videos/Intro.mp4"), &video_info, text_view("Videos/Intro"),
                video_image, sizeof(video_image), &second_size) == CNA_RESULT_SUCCESS &&
                second_size == image_size &&
                cna_cnb_document_parse(
                    video_image, image_size, text_view("video-memory"), NULL, &video) ==
                    CNA_RESULT_SUCCESS,
            "encode and parse the complete Video metadata document")) {
        goto cleanup;
    }
    memset(&decoded_video, 0, sizeof(decoded_video));
    decoded_video.struct_size = (uint32_t)sizeof(decoded_video);
    decoded_video.struct_version = CNA_CNB_VIDEO_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_document_get_asset_type_id(video, &asset_type) == CNA_RESULT_SUCCESS &&
            asset_type == CNA_CNB_ASSET_TYPE_VIDEO &&
            cna_cnb_document_get_asset_schema_version(video, &schema) == CNA_RESULT_SUCCESS &&
            schema == CNA_CNB_MEDIA_SCHEMA_VERSION &&
            cna_cnb_decode_video(video, &decoded_video) == CNA_RESULT_SUCCESS &&
            decoded_video.duration_milliseconds == UINT32_C(42000) &&
            decoded_video.width == UINT32_C(1280) && decoded_video.height == UINT32_C(720) &&
            decoded_video.frames_per_second == 29.97F &&
            decoded_video.soundtrack_type == CNA_VIDEO_SOUNDTRACK_TYPE_MUSIC_AND_DIALOG &&
            decoded_video.reserved == UINT32_C(0),
        "the Video document restores its identity, schema and complete versioned metadata");
    cna_demo_check(
        &demo,
        text_is(
            video, cna_cnb_decode_video_stream_reference_size,
            cna_cnb_decode_video_stream_reference, "Videos/Intro.mp4") &&
            external_reference_is(video, "Videos/Intro.mp4"),
        "the atomic Video stream route equals its single XREF dependency");

    broken_video = video_info;
    broken_video.width = CNA_CNB_MAX_VIDEO_DIMENSION + UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_cnb_encode_video(
            text_view("Videos/Intro.mp4"), &broken_video, text_view(""),
            video_image, sizeof(video_image), &second_size) == CNA_RESULT_IO &&
            ((broken_video = video_info), broken_video.frames_per_second = 0.0F,
             cna_cnb_encode_video(
                 text_view("Videos/Intro.mp4"), &broken_video, text_view(""),
                 video_image, sizeof(video_image), &second_size)) == CNA_RESULT_IO &&
            ((broken_video = video_info),
             broken_video.soundtrack_type = CNA_VIDEO_SOUNDTRACK_TYPE_MAXIMUM + UINT32_C(1),
             cna_cnb_encode_video(
                 text_view("Videos/Intro.mp4"), &broken_video, text_view(""),
                 video_image, sizeof(video_image), &second_size)) == CNA_RESULT_IO,
        "Video encoding refuses an oversized frame, zero rate and unknown soundtrack type");
    duration = UINT32_C(0xA5A5A5A5);
    memset(&decoded_video, 0xA5, sizeof(decoded_video));
    decoded_video.struct_size = (uint32_t)sizeof(decoded_video);
    decoded_video.struct_version = CNA_CNB_VIDEO_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_decode_song_duration_milliseconds(video, &duration) == CNA_RESULT_IO &&
            duration == UINT32_C(0xA5A5A5A5) &&
            cna_cnb_decode_video(song, &decoded_video) == CNA_RESULT_IO,
        "Song and Video decoders reject each other's asset identity");

    stale_song = song;
    cna_demo_check_result(&demo, cna_cnb_document_destroy(song), "release the Song document");
    song = CNA_INVALID_HANDLE;
    duration = UINT32_C(0xA5A5A5A5);
    cna_demo_check(
        &demo,
        cna_cnb_decode_song_duration_milliseconds(stale_song, &duration) ==
                CNA_RESULT_INVALID_HANDLE &&
            duration == UINT32_C(0xA5A5A5A5),
        "a released Song document rejects stale schema access without changing output");

cleanup:
    releases_ok &= release_document(&video);
    releases_ok &= release_document(&nameless);
    releases_ok &= release_document(&song);
    cna_demo_check(&demo, releases_ok, "release every remaining Song and Video document handle");
    return cna_demo_finish(&demo);
}
