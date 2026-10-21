/* SPDX-License-Identifier: MS-PL */
/* Video construction, metadata, paths, track selection and owned lifetime through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct State {
    CnaDemo* demo;
    int validated;
} State;

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static int video_path(CNA_Handle game, char* destination, size_t capacity)
{
    static const char Suffix[] = "/assets/media/videos/bounce.wmv";
    uint64_t bytes = 0U;
    uint64_t copied = 0U;
    if (cna_title_location_get_path_size(game, &bytes) != CNA_RESULT_SUCCESS ||
        bytes + sizeof(Suffix) > capacity ||
        cna_title_location_copy_path(game, destination, bytes, &copied) != CNA_RESULT_SUCCESS ||
        copied != bytes) {
        return 0;
    }
    memcpy(destination + bytes, Suffix, sizeof(Suffix));
    return 1;
}

static CNA_Result inspect_video(
    CNA_Handle game,
    const CNA_GameTime* game_time,
    void* context,
    CNA_CallbackError* out_error)
{
    static const char VideoType[] = "Microsoft.Xna.Framework.Media.Video";
    State* state = (State*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_VideoHandle video = CNA_INVALID_HANDLE;
    CNA_VideoHandle declared = CNA_INVALID_HANDLE;
    CNA_VideoHandle from_uri = CNA_INVALID_HANDLE;
    CNA_VideoHandle rejected = UINT64_C(99);
    CNA_VideoInfo info;
    CNA_VideoInfo invalid;
    CNA_VideoSoundtrackType soundtrack = UINT32_C(99);
    CNA_Bool bound = CNA_FALSE;
    int32_t width = 0;
    int32_t height = 0;
    float fps = 0.0F;
    int64_t ticks = 0;
    uint64_t bytes = 0U;
    char path[2048];
    char file_uri[2060];
    char text[2048];
    char before[2048];
    (void)game_time;
    (void)out_error;

    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "borrow the game graphics device") ||
        !cna_demo_check(state->demo, video_path(game, path, sizeof(path)),
                        "resolve the generated WMV beside the executable assets")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_video_create(device, view("definitely-missing-video.wmv"), &rejected) == CNA_RESULT_IO &&
            rejected == CNA_INVALID_HANDLE,
        "the probing constructor rejects a missing file without returning a handle");
    if (!cna_demo_check_result(state->demo, cna_video_create(device, view(path), &video),
                               "probe the generated WMV into an owned Video")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_video_get_width(video, &width) == CNA_RESULT_SUCCESS && width == 160 &&
            cna_video_get_height(video, &height) == CNA_RESULT_SUCCESS && height == 120 &&
            cna_video_get_frames_per_second(video, &fps) == CNA_RESULT_SUCCESS && fps == 15.0F,
        "the probing constructor reports the authored 160x120 frame size at 15 fps");
    cna_demo_check(
        state->demo,
        cna_video_get_duration(video, &ticks) == CNA_RESULT_SUCCESS && ticks > INT64_C(0) &&
            cna_video_get_has_graphics_device(video, &bound) == CNA_RESULT_SUCCESS &&
            bound == CNA_TRUE,
        "the probed video has positive duration and retains its device association");
    memset(&info, 0xA5, sizeof(info));
    cna_demo_check(
        state->demo,
        cna_video_info_init(&info) == CNA_RESULT_SUCCESS &&
            info.struct_size == sizeof(info) && info.struct_version == UINT32_C(1) &&
            info.width == 0 && info.height == 0 && info.fps == 0.0 &&
            cna_video_get_info(video, &info) == CNA_RESULT_SUCCESS &&
            info.width == width && info.height == height && info.fps == (double)fps,
        "VideoInfo initializes canonically and then receives the same probed metadata triple");
    invalid = info;
    invalid.struct_version = UINT32_C(2);
    cna_demo_check(
        state->demo,
        cna_video_get_info(video, &invalid) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_video_get_info(video, NULL) == CNA_RESULT_INVALID_ARGUMENT,
        "metadata filling rejects an unknown structure version and null output");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_video_get_file_name_size(video, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == strlen(path) && bytes < sizeof(text) &&
            cna_video_copy_file_name(video, text, bytes, &bytes) == CNA_RESULT_SUCCESS &&
            memcmp(text, path, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "the source path uses exact unterminated size and copy semantics");
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_video_copy_file_name(video, text, bytes - 1U, &bytes) == CNA_RESULT_BUFFER_TOO_SMALL &&
            memcmp(text, before, sizeof(text)) == 0,
        "a short source-path destination is rejected without a partial write");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_video_get_type_name_size(video, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == strlen(VideoType) &&
            cna_video_copy_type_name(video, text, bytes, &bytes) == CNA_RESULT_SUCCESS &&
            memcmp(text, VideoType, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "the Video reflection name uses exact unterminated size and copy semantics");
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_video_copy_type_name(video, text, bytes - 1U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            memcmp(text, before, sizeof(text)) == 0,
        "a short Video type-name destination is rejected without a partial write");

    cna_demo_check(
        state->demo,
        cna_video_create_with_metadata(
            device, view("missing-declared-video.wmv"), 2500, 640, 360, 29.97F,
            CNA_VIDEO_SOUNDTRACK_TYPE_MAXIMUM + UINT32_C(1), &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "declared construction rejects an undefined soundtrack identity atomically");
    if (cna_demo_check_result(
            state->demo,
            cna_video_create_with_metadata(
                device, view("missing-declared-video.wmv"), 2500, 640, 360, 29.97F,
                CNA_VIDEO_SOUNDTRACK_TYPE_DIALOG, &declared),
            "construct a metadata-only Video without probing its missing file")) {
        cna_demo_check(
            state->demo,
            cna_video_get_width(declared, &width) == CNA_RESULT_SUCCESS && width == 640 &&
                cna_video_get_height(declared, &height) == CNA_RESULT_SUCCESS && height == 360 &&
                cna_video_get_frames_per_second(declared, &fps) == CNA_RESULT_SUCCESS &&
                fps == 29.97F &&
                cna_video_get_soundtrack_type(declared, &soundtrack) == CNA_RESULT_SUCCESS &&
                soundtrack == CNA_VIDEO_SOUNDTRACK_TYPE_DIALOG &&
                cna_video_get_duration(declared, &ticks) == CNA_RESULT_SUCCESS &&
                ticks == INT64_C(25000000),
            "the metadata-only constructor preserves every declared value without touching the file");
        cna_demo_check(
            state->demo,
            cna_video_set_duration(declared, INT64_C(-99)) == CNA_RESULT_SUCCESS &&
                cna_video_get_duration(declared, &ticks) == CNA_RESULT_SUCCESS &&
                ticks == INT64_C(-99),
            "the canonical duration setter stores even a negative loader-supplied value");
        cna_demo_check(
            state->demo,
            cna_video_set_audio_track_ext(declared, 3) == CNA_RESULT_SUCCESS &&
                cna_video_set_video_track_ext(declared, -1) == CNA_RESULT_SUCCESS,
            "video track selectors store their indices for the next open without eager validation");
    }

    cna_demo_check(
        state->demo,
        cna_video_create_from_uri_ext(device, view(path), &from_uri) == CNA_RESULT_SUCCESS &&
            from_uri != CNA_INVALID_HANDLE,
        "the URI extension factory accepts an ordinary existing path");
    cna_demo_check(
        state->demo,
        cna_video_create_from_uri_ext(
            device, view("file:///definitely/missing/video.wmv"), &rejected) == CNA_RESULT_IO &&
            rejected == CNA_INVALID_HANDLE,
        "the URI extension factory rejects a missing resolved file without publishing a handle");
    cna_demo_check(
        state->demo,
        cna_video_create_from_uri_ext(device, view("https://example.invalid/video.wmv"),
                                     &rejected) == CNA_RESULT_INVALID_STATE &&
            rejected == CNA_INVALID_HANDLE,
        "the URI factory refuses non-file absolute URIs before any network request");
    /* File URI spelling differs only for an absolute DOS drive path. */
    strcpy(file_uri, path[0] == '/' ? "file://" : "file:///");
    strcat(file_uri, path);
    {
        CNA_VideoHandle resolved = CNA_INVALID_HANDLE;
        cna_demo_check(
            state->demo,
            cna_video_create_from_uri_ext(device, view(file_uri), &resolved) == CNA_RESULT_SUCCESS &&
                cna_video_get_width(resolved, &width) == CNA_RESULT_SUCCESS && width == 160 &&
                cna_video_get_height(resolved, &height) == CNA_RESULT_SUCCESS && height == 120,
            "an absolute file URI resolves the real authored video");
        if (resolved != CNA_INVALID_HANDLE) (void)cna_video_destroy(resolved);
        resolved = CNA_INVALID_HANDLE;
        cna_demo_check(
            state->demo,
            cna_video_create_from_uri_ext(device, view("assets/media/videos/bounce.wmv"),
                                         &resolved) == CNA_RESULT_SUCCESS &&
                cna_video_get_width(resolved, &width) == CNA_RESULT_SUCCESS && width == 160,
            "a relative video URI resolves against TitleLocation");
        if (resolved != CNA_INVALID_HANDLE) (void)cna_video_destroy(resolved);
    }
    if (declared != CNA_INVALID_HANDLE) {
        CNA_VideoPlayerHandle player = CNA_INVALID_HANDLE;
        CNA_VideoHandle active = UINT64_C(99);
        CNA_Bool available = CNA_TRUE;
        CNA_MediaState playback = CNA_MEDIA_STATE_PLAYING;
        if (cna_demo_check_result(state->demo, cna_video_player_create(game, &player),
                                 "create a player for the deferred missing-file contract")) {
            cna_demo_check(
                state->demo,
                cna_video_player_play(player, declared) == CNA_RESULT_INVALID_ARGUMENT &&
                    cna_video_player_get_state(player, &playback) == CNA_RESULT_SUCCESS &&
                    playback == CNA_MEDIA_STATE_STOPPED &&
                    cna_video_player_get_video(player, &active, &available) == CNA_RESULT_SUCCESS &&
                    available == CNA_FALSE && active == CNA_INVALID_HANDLE,
                "deferred missing-file Play reports XNA INVALID_ARGUMENT and remains closed");
            cna_demo_check_result(state->demo, cna_video_player_destroy(player),
                                 "destroy the deferred-file player independently");
        }
    }
    if (from_uri != CNA_INVALID_HANDLE) {
        cna_demo_check_result(state->demo, cna_video_destroy(from_uri),
                              "destroy the URI-factory Video independently");
    }
    if (declared != CNA_INVALID_HANDLE) {
        cna_demo_check_result(state->demo, cna_video_destroy(declared),
                              "destroy the metadata-only Video independently");
    }
    cna_demo_check(
        state->demo,
        cna_video_destroy(video) == CNA_RESULT_SUCCESS &&
            cna_video_destroy(video) == CNA_RESULT_INVALID_HANDLE &&
            cna_video_get_width(video, &width) == CNA_RESULT_INVALID_HANDLE,
        "the probed Video releases once and then rejects member access");
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    State state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.media.video_asset", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = inspect_video;
    callbacks.context = &state;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.callbacks = &callbacks;

    if (cna_demo_check_result(&demo, cna_game_create(&info, &game),
                              "create the Video metadata game")) {
        cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                              "run its Video inspection callback");
        cna_demo_check(&demo, state.validated == 1, "the Video metadata audit ran exactly once");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the Video metadata game");
    }
    return cna_demo_finish(&demo);
}
