/* SPDX-License-Identifier: MS-PL */
/* VideoPlayer transport, decoded-frame borrowing and lifetime through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef enum Phase {
    PHASE_PAUSED,
    PHASE_WAITING_FOR_LOOP,
    PHASE_DONE
} Phase;

typedef struct State {
    CnaDemo* demo;
    CNA_VideoHandle video;
    CNA_VideoPlayerHandle player;
    Phase phase;
    int updates;
    int paused_updates;
    int64_t paused_ticks;
    int64_t previous_ticks;
    int64_t maximum_ticks;
    uint64_t last_generation;
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

static void frame_init(CNA_VideoFrameEXT* frame)
{
    memset(frame, 0, sizeof(*frame));
    frame->struct_size = (uint32_t)sizeof(*frame);
    frame->struct_version = CNA_VIDEO_FRAME_EXT_STRUCT_VERSION;
}

static CNA_Result finish_player(State* state, CNA_Handle game)
{
    CNA_VideoFrameEXT stopped;
    CNA_VideoFrameEXT replayed;
    CNA_MediaState media_state = CNA_MEDIA_STATE_PLAYING;
    CNA_Bool available = CNA_TRUE;
    CNA_Bool disposed = CNA_FALSE;
    CNA_VideoHandle current = UINT64_C(99);
    CNA_Handle texture = UINT64_C(99);
    const uint64_t before_replay = state->last_generation;

    frame_init(&stopped);
    frame_init(&replayed);
    cna_demo_check(
        state->demo,
        cna_video_player_set_is_looped(state->player, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_video_player_stop(state->player) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_state(state->player, &media_state) == CNA_RESULT_SUCCESS &&
            media_state == CNA_MEDIA_STATE_STOPPED &&
            cna_video_player_get_play_position_ticks(state->player, &state->previous_ticks) ==
                CNA_RESULT_SUCCESS &&
            state->previous_ticks == INT64_C(0) &&
            cna_video_player_get_video(state->player, &current, &available) == CNA_RESULT_SUCCESS &&
            available == CNA_FALSE && current == CNA_INVALID_HANDLE,
        "Stop clears the active Video and resets state and position");
    cna_demo_check(
        state->demo,
        cna_video_player_get_frame_ext(state->player, &stopped) == CNA_RESULT_SUCCESS &&
            stopped.available == CNA_FALSE && stopped.texture == CNA_INVALID_HANDLE &&
            stopped.generation >= before_replay && stopped.presentation_time < 0.0,
        "the stopped frame is absent without restarting the lifetime generation");
    cna_demo_check(
        state->demo,
        cna_video_player_play(state->player, state->video) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_frame_ext(state->player, &replayed) == CNA_RESULT_SUCCESS &&
            replayed.available == CNA_TRUE && replayed.texture != CNA_INVALID_HANDLE &&
            replayed.generation > before_replay,
        "replaying decodes a fresh frame with a strictly newer generation");
    cna_demo_check_result(state->demo, cna_video_player_stop(state->player),
                          "stop the replay before releasing the caller's Video handle");
    cna_demo_check(
        state->demo,
        cna_video_destroy(state->video) == CNA_RESULT_SUCCESS &&
            cna_video_destroy(state->video) == CNA_RESULT_INVALID_HANDLE,
        "the stopped caller-owned Video handle releases exactly once");
    state->video = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        cna_video_player_dispose(state->player) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_is_disposed(state->player, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_video_player_stop(state->player) == CNA_RESULT_INVALID_STATE &&
            cna_video_player_get_texture(state->player, &texture, &available) ==
                CNA_RESULT_INVALID_STATE,
        "Dispose remains observable and playback routes reject the disposed player");
    cna_demo_check(
        state->demo,
        cna_video_player_destroy(state->player) == CNA_RESULT_SUCCESS &&
            cna_video_player_destroy(state->player) == CNA_RESULT_INVALID_HANDLE,
        "the disposed VideoPlayer handle releases exactly once");
    state->player = CNA_INVALID_HANDLE;
    state->phase = PHASE_DONE;
    return cna_game_request_exit(game);
}

static CNA_Result load_content(
    CNA_Handle game,
    const CNA_GameTime* game_time,
    void* context,
    CNA_CallbackError* out_error)
{
    static const char PlayerType[] = "Microsoft.Xna.Framework.Media.VideoPlayer";
    static const char TextureType[] = "Microsoft.Xna.Framework.Graphics.Texture2D";
    State* state = (State*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle first_texture = CNA_INVALID_HANDLE;
    CNA_Handle second_texture = CNA_INVALID_HANDLE;
    CNA_VideoHandle current = UINT64_C(99);
    CNA_VideoFrameEXT empty;
    CNA_VideoFrameEXT first;
    CNA_Bool flag = UINT8_C(77);
    CNA_MediaState media_state = CNA_MEDIA_STATE_PLAYING;
    float volume = -1.0F;
    uint64_t bytes = 0U;
    uint64_t copied = 0U;
    char path[2048];
    char text[64];
    char before[64];
    (void)game_time;
    (void)out_error;

    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "borrow the game graphics device") ||
        !cna_demo_check(state->demo, video_path(game, path, sizeof(path)),
                        "resolve the generated WMV beside the executable assets") ||
        !cna_demo_check_result(state->demo, cna_video_create(device, view(path), &state->video),
                               "probe the generated WMV for playback") ||
        !cna_demo_check_result(state->demo, cna_video_player_create(game, &state->player),
                               "create an owned VideoPlayer")) {
        return cna_game_request_exit(game);
    }

    cna_demo_check(
        state->demo,
        cna_video_player_get_is_disposed(state->player, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_video_player_get_state(state->player, &media_state) == CNA_RESULT_SUCCESS &&
            media_state == CNA_MEDIA_STATE_STOPPED &&
            cna_video_player_get_play_position_ticks(state->player, &state->paused_ticks) ==
                CNA_RESULT_SUCCESS &&
            state->paused_ticks == INT64_C(0) &&
            cna_video_player_get_video(state->player, &current, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && current == CNA_INVALID_HANDLE,
        "a fresh player is live, stopped at zero and has no active Video");
    frame_init(&empty);
    cna_demo_check(
        state->demo,
        cna_video_player_get_frame_ext(state->player, &empty) == CNA_RESULT_SUCCESS &&
            empty.available == CNA_FALSE && empty.texture == CNA_INVALID_HANDLE &&
            empty.generation == UINT64_C(0) && empty.presentation_time < 0.0,
        "a fresh player reports an ordinary absent frame with generation zero");
    cna_demo_check(
        state->demo,
        cna_video_player_get_texture(state->player, &first_texture, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && first_texture == CNA_INVALID_HANDLE,
        "GetTexture before Play reports no frame without failing");

    cna_demo_check(
        state->demo,
        cna_video_player_get_is_looped(state->player, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_video_player_set_is_looped(state->player, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_is_looped(state->player, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "looping changes from its false default to true");
    cna_demo_check(
        state->demo,
        cna_video_player_get_is_muted(state->player, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_video_player_set_is_muted(state->player, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_is_muted(state->player, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_video_player_set_is_muted(state->player, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_is_muted(state->player, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "muting round-trips and is restored before playback");
    cna_demo_check(
        state->demo,
        cna_video_player_set_volume(state->player, -0.5F) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_volume(state->player, &volume) == CNA_RESULT_SUCCESS &&
            volume == 0.0F &&
            cna_video_player_set_volume(state->player, 1.5F) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_volume(state->player, &volume) == CNA_RESULT_SUCCESS &&
            volume == 1.0F &&
            cna_video_player_set_volume(state->player, 0.5F) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_volume(state->player, &volume) == CNA_RESULT_SUCCESS &&
            volume == 0.5F,
        "volume clamps below and above the public range and preserves an interior value");
    cna_demo_check(
        state->demo,
        cna_video_player_set_audio_track_ext(state->player, 0) == CNA_RESULT_SUCCESS &&
            cna_video_player_set_video_track_ext(state->player, 0) == CNA_RESULT_SUCCESS,
        "player track preferences accept the authored audio and video streams");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_video_player_get_type_name_size(state->player, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(PlayerType) - 1U &&
            cna_video_player_copy_type_name(state->player, text, bytes, &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, PlayerType, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "the player reflection name uses exact unterminated size and copy semantics");
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_video_player_copy_type_name(state->player, text, bytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == bytes && memcmp(text, before, sizeof(text)) == 0,
        "a short player type-name destination is rejected atomically");

    if (!cna_demo_check_result(state->demo, cna_video_player_play(state->player, state->video),
                               "start decoding the generated WMV")) {
        return cna_game_request_exit(game);
    }
    cna_demo_check(
        state->demo,
        cna_video_player_get_state(state->player, &media_state) == CNA_RESULT_SUCCESS &&
            media_state == CNA_MEDIA_STATE_PLAYING &&
            cna_video_player_get_video(state->player, &current, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && current == state->video,
        "Play enters Playing and publishes the same caller-owned Video handle");

    frame_init(&first);
    cna_demo_check(
        state->demo,
        cna_video_player_get_frame_ext(state->player, &first) == CNA_RESULT_SUCCESS &&
            first.available == CNA_TRUE && first.texture != CNA_INVALID_HANDLE &&
            first.generation > UINT64_C(0) && first.presentation_time >= 0.0 &&
            first.reserved[0] == 0U && first.reserved[1] == 0U && first.reserved[2] == 0U,
        "the first real decode publishes a versioned frame and nonzero generation");
    first_texture = first.texture;
    state->last_generation = first.generation;
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_texture2d_get_type_name_byte_count(first_texture, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(TextureType) - 1U &&
            cna_texture2d_copy_type_name(first_texture, text, bytes, &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, TextureType, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "the borrowed video frame reports the exact unterminated Texture2D type name");
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_texture2d_copy_type_name(first_texture, text, bytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == bytes && memcmp(text, before, sizeof(text)) == 0,
        "a short frame type-name destination is rejected atomically");
    cna_demo_check(
        state->demo,
        cna_texture2d_get_type_name_byte_count(first_texture, &bytes) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_texture(state->player, &second_texture, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && second_texture != CNA_INVALID_HANDLE &&
            cna_texture2d_get_type_name_byte_count(first_texture, &bytes) ==
                CNA_RESULT_INVALID_HANDLE &&
            cna_texture2d_get_type_name_byte_count(second_texture, &bytes) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_state(state->player, &media_state) == CNA_RESULT_SUCCESS &&
            cna_texture2d_get_type_name_byte_count(second_texture, &bytes) ==
                CNA_RESULT_INVALID_HANDLE,
        "each player call invalidates its previously borrowed frame texture handle");

    cna_demo_check(
        state->demo,
        cna_video_player_pause(state->player) == CNA_RESULT_SUCCESS &&
            cna_video_player_get_state(state->player, &media_state) == CNA_RESULT_SUCCESS &&
            media_state == CNA_MEDIA_STATE_PAUSED &&
            cna_video_player_get_play_position_ticks(state->player, &state->paused_ticks) ==
                CNA_RESULT_SUCCESS &&
            state->paused_ticks >= INT64_C(0),
        "Pause enters Paused and captures a nonnegative position");
    state->previous_ticks = state->paused_ticks;
    state->maximum_ticks = state->paused_ticks;
    state->phase = PHASE_PAUSED;
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(
    CNA_Handle game,
    const CNA_GameTime* game_time,
    void* context,
    CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    CNA_VideoFrameEXT frame;
    CNA_MediaState media_state = CNA_MEDIA_STATE_STOPPED;
    int64_t ticks = INT64_C(0);
    int limit;
    (void)game_time;
    (void)out_error;

    ++state->updates;
    limit = state->demo->frames > 0 ? state->demo->frames : 600;
    if (state->player == CNA_INVALID_HANDLE || state->phase == PHASE_DONE) {
        return CNA_RESULT_SUCCESS;
    }
    if (state->phase == PHASE_PAUSED) {
        ++state->paused_updates;
        if (state->paused_updates < 10) {
            return CNA_RESULT_SUCCESS;
        }
        cna_demo_check(
            state->demo,
            cna_video_player_get_play_position_ticks(state->player, &ticks) == CNA_RESULT_SUCCESS &&
                ticks == state->paused_ticks,
            "the playback clock remains fixed while paused across multiple game updates");
        cna_demo_check(
            state->demo,
            cna_video_player_resume(state->player) == CNA_RESULT_SUCCESS &&
                cna_video_player_get_state(state->player, &media_state) == CNA_RESULT_SUCCESS &&
                media_state == CNA_MEDIA_STATE_PLAYING,
            "Resume returns the paused player to Playing");
        state->phase = PHASE_WAITING_FOR_LOOP;
        state->previous_ticks = state->paused_ticks;
        return CNA_RESULT_SUCCESS;
    }

    frame_init(&frame);
    if (cna_video_player_get_frame_ext(state->player, &frame) != CNA_RESULT_SUCCESS ||
        cna_video_player_get_play_position_ticks(state->player, &ticks) != CNA_RESULT_SUCCESS) {
        cna_demo_check(state->demo, 0, "poll the decoded frame and playback clock while looping");
        return finish_player(state, game);
    }
    if (frame.available == CNA_TRUE && frame.generation > state->last_generation) {
        state->last_generation = frame.generation;
    }
    if (ticks > state->maximum_ticks) {
        state->maximum_ticks = ticks;
    }
    if (state->maximum_ticks > INT64_C(15000000) && ticks + INT64_C(10000000) < state->previous_ticks) {
        cna_demo_check(
            state->demo,
            frame.available == CNA_TRUE && frame.texture != CNA_INVALID_HANDLE &&
                frame.generation > UINT64_C(1) &&
                cna_video_player_get_state(state->player, &media_state) == CNA_RESULT_SUCCESS &&
                media_state == CNA_MEDIA_STATE_PLAYING,
            "the authored end loops to an earlier clock position while decoded playback stays active");
        return finish_player(state, game);
    }
    state->previous_ticks = ticks;
    if (state->updates >= limit) {
        cna_demo_check(state->demo, 0,
                       "the authored video reaches its loop point within the bounded run");
        return finish_player(state, game);
    }
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    State state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.media.video_player", argc, argv, 360);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.video = CNA_INVALID_HANDLE;
    state.player = CNA_INVALID_HANDLE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.context = &state;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.callbacks = &callbacks;

    if (cna_demo_check_result(&demo, cna_game_create(&info, &game),
                              "create the VideoPlayer game")) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through pause, loop, replay and disposal");
    }
    if (state.player != CNA_INVALID_HANDLE) {
        (void)cna_video_player_destroy(state.player);
    }
    if (state.video != CNA_INVALID_HANDLE) {
        (void)cna_video_destroy(state.video);
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the VideoPlayer game");
    }
    return cna_demo_finish(&demo);
}
