/* SPDX-License-Identifier: MS-PL */
/*
 * Background music from C: two songs are created from files under the title's path, put in a song
 * collection (which keeps them alive, so the program releases its own song handles at once) and
 * played as a queue. The program moves to the second song, pauses, resumes and stops, and counts
 * the MediaPlayer's two events -- which are process-wide, so their subscriptions take no game.
 * Bars show the position in each song.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define TICKS_PER_SECOND INT64_C(10000000)
#define SONG_TICKS (2 * TICKS_PER_SECOND) /* both tunes last 2 s (tools/generate_assets.py) */

typedef struct PlayerState {
    CnaDemo* demo;
    CNA_Handle white;        /* owned 1x1 texture */
    CNA_Handle sprite_batch; /* owned */
    CNA_SongCollectionHandle songs;
    CNA_MediaQueueHandle queue; /* a view of the process-wide queue; released like any handle */
    CNA_MediaPlayerEventRegistrationHandle state_changed;
    CNA_MediaPlayerEventRegistrationHandle song_changed;
    int state_changes;
    int song_changes;
    int song_changes_before_next;
    int step;
    int frames;
} PlayerState;

static CNA_StringView view(const char* s, uint64_t length)
{
    CNA_StringView v;
    v.data = s;
    v.byte_length = length;
    return v;
}

static void on_state_changed(void* context)
{
    ++((PlayerState*)context)->state_changes;
}

static void on_song_changed(void* context)
{
    ++((PlayerState*)context)->song_changes;
}

static CNA_MediaState media_state(CNA_Handle game)
{
    CNA_MediaState state = CNA_MEDIA_STATE_MAXIMUM + 1U;
    (void)cna_media_player_get_state(game, &state); /* an unknown value fails every check below */
    return state;
}

static int32_t active_index(const PlayerState* state)
{
    int32_t index = -2;
    (void)cna_media_queue_get_active_song_index(state->queue, &index);
    return index;
}

/* A Song from a file under the title's path (relative paths resolve against the working
   directory, CNA-DEMOS-ISSUE-015). */
static CNA_Result create_song(CNA_Handle game, const char* name, const char* file, CNA_SongHandle* out_song)
{
    char path[1024];
    uint64_t length = 0U;
    CNA_Result result = cna_title_location_copy_path(game, path, sizeof(path) - 64U, &length);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    length += (uint64_t)snprintf(path + length, sizeof(path) - (size_t)length, "/assets/music/%s", file);
    return cna_song_create_from_uri(game, view(name, strlen(name)), view(path, length), out_song);
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    PlayerState* state = (PlayerState*)context;
    static const CNA_Color white = {255, 255, 255, 255};
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo info;
    CNA_SongHandle songs[2] = {CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    CNA_Bool has_control = CNA_FALSE;
    int32_t count = 0;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    result = cna_game_get_graphics_device(game, &graphics_device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.width = 1U;
    info.height = 1U;
    info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(graphics_device, &info, &state->white);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_texture2d_set_data_rgba8(state->white, &white, 1U);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_sprite_batch_create(graphics_device, &state->sprite_batch);
    }
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }

    /* Two songs in one collection. The collection keeps its songs alive, so the program's own
       handles can go right away. */
    if (cna_demo_check_result(state->demo, create_song(game, "Rising", "rising.wav", &songs[0]),
                              "cna_song_create_from_uri(\"Rising\")") &&
        cna_demo_check_result(state->demo, create_song(game, "Falling", "falling.wav", &songs[1]),
                              "cna_song_create_from_uri(\"Falling\")")) {
        cna_demo_check_result(state->demo, cna_song_collection_create(game, songs, 2U, &state->songs),
                              "cna_song_collection_create");
    }
    if (songs[0] != CNA_INVALID_HANDLE) {
        cna_demo_check_result(state->demo, cna_song_destroy(songs[0]), "cna_song_destroy (the collection keeps it)");
    }
    if (songs[1] != CNA_INVALID_HANDLE) {
        cna_demo_check_result(state->demo, cna_song_destroy(songs[1]), "cna_song_destroy (the collection keeps it)");
    }
    if (state->songs == CNA_INVALID_HANDLE) {
        return CNA_RESULT_SUCCESS; /* the failed checks above already report it */
    }

    /* The MediaPlayer's events are static: the subscriptions belong to the process. */
    cna_demo_check_result(state->demo,
                          cna_media_player_subscribe_media_state_changed_ext(on_state_changed, state,
                                                                             &state->state_changed),
                          "cna_media_player_subscribe_media_state_changed_ext");
    cna_demo_check_result(state->demo,
                          cna_media_player_subscribe_active_song_changed_ext(on_song_changed, state,
                                                                             &state->song_changed),
                          "cna_media_player_subscribe_active_song_changed_ext");
    cna_demo_check(state->demo,
                   cna_media_player_get_game_has_control(game, &has_control) == CNA_RESULT_SUCCESS &&
                       has_control == CNA_TRUE,
                   "the game has control of the music (always on a desktop)");
    cna_demo_check(state->demo, media_state(game) == CNA_MEDIA_STATE_STOPPED, "the player starts out Stopped");

    if (cna_demo_check_result(state->demo, cna_media_player_play_songs(game, state->songs),
                              "cna_media_player_play_songs") &&
        cna_demo_check_result(state->demo, cna_media_player_get_queue(game, &state->queue),
                              "cna_media_player_get_queue")) {
        (void)cna_media_queue_get_count(state->queue, &count);
        cna_demo_check(state->demo, media_state(game) == CNA_MEDIA_STATE_PLAYING && count == 2 && active_index(state) == 0,
                       "play_songs queues both and plays the first");
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    PlayerState* state = (PlayerState*)context;
    const int64_t now = game_time->total_game_time_ticks;
    int64_t position = 0;
    (void)out_error;
    if (state->queue == CNA_INVALID_HANDLE) {
        return state->demo->automated ? cna_game_request_exit(game) : CNA_RESULT_SUCCESS;
    }
    /* A short tour of the transport, one step at a time (an interactive run keeps playing). */
    if (state->step == 0 && now >= TICKS_PER_SECOND / 2) {
        state->step = 1;
        state->song_changes_before_next = state->song_changes;
        (void)cna_media_player_get_play_position_ticks(game, &position);
        printf("  after 0.5 s: position %.2f s\n", (double)position / (double)TICKS_PER_SECOND);
        cna_demo_check(state->demo, position > TICKS_PER_SECOND / 5, "the position advances while a song plays");
        if (cna_demo_check_result(state->demo, cna_media_player_move_next(game), "cna_media_player_move_next")) {
            cna_demo_check(state->demo, active_index(state) == 1, "move_next: the second song is active");
        }
    } else if (state->step == 1 && now >= TICKS_PER_SECOND) {
        state->step = 2;
        /* The events are raised by the game loop's FrameworkDispatcher update, after the call. */
        printf("  active-song-changed raised %d time(s) since move_next\n",
               state->song_changes - state->song_changes_before_next);
        cna_demo_check(state->demo, state->song_changes == state->song_changes_before_next + 1,
                       "active-song-changed was raised once for move_next, by a later update");
        cna_demo_check(state->demo,
                       cna_media_player_pause(game) == CNA_RESULT_SUCCESS && media_state(game) == CNA_MEDIA_STATE_PAUSED,
                       "pause: Paused");
    } else if (state->step == 2 && now >= TICKS_PER_SECOND * 6 / 5) {
        state->step = 3;
        cna_demo_check(state->demo,
                       cna_media_player_resume(game) == CNA_RESULT_SUCCESS &&
                           media_state(game) == CNA_MEDIA_STATE_PLAYING,
                       "resume: Playing again");
    } else if (state->step == 3 && now >= TICKS_PER_SECOND * 7 / 5 && state->demo->automated) {
        state->step = 4;
        cna_demo_check(state->demo,
                       cna_media_player_stop(game) == CNA_RESULT_SUCCESS && media_state(game) == CNA_MEDIA_STATE_STOPPED,
                       "stop: Stopped");
        printf("  media-state-changed raised %d times\n", state->state_changes);
        cna_demo_check(state->demo, state->state_changes >= 4,
                       "media-state-changed followed Playing, Paused, Playing, Stopped");
    }
    if (state->demo->frames > 0 && state->frames >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_SpriteCommand bar(CNA_Handle texture, int32_t x, int32_t y, int32_t width, CNA_Color color)
{
    CNA_SpriteCommand c;
    memset(&c, 0, sizeof(c));
    c.struct_size = (uint32_t)sizeof(c);
    c.struct_version = UINT32_C(1);
    c.texture = texture;
    c.destination.x = x;
    c.destination.y = y;
    c.destination.width = width;
    c.destination.height = 24;
    c.source.width = 1;
    c.source.height = 1;
    c.color = color;
    c.effects = CNA_SPRITE_EFFECT_NONE;
    return c;
}

static CNA_Result draw(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    static const CNA_Color background = {24, 26, 36, 255};
    static const CNA_Color track = {60, 64, 80, 255};
    static const CNA_Color playing = {240, 150, 50, 255};
    static const CNA_Color paused = {150, 150, 170, 255};
    PlayerState* state = (PlayerState*)context;
    CNA_SpriteCommand commands[4];
    CNA_SpriteBatchBeginInfo begin_info;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    const CNA_MediaState now = media_state(game);
    int64_t position = 0;
    int32_t active = state->queue != CNA_INVALID_HANDLE ? active_index(state) : -1;
    uint32_t n = 0U;
    int32_t i;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->frames;
    (void)cna_media_player_get_play_position_ticks(game, &position);
    if (position > SONG_TICKS) {
        position = SONG_TICKS;
    }
    result = cna_game_clear(game, background);

    /* One bar per song; the active one shows the position, orange while playing. */
    for (i = 0; i < 2; ++i) {
        commands[n++] = bar(state->white, 100, 180 + 80 * i, 600, track);
        if (i == active && now != CNA_MEDIA_STATE_STOPPED) {
            commands[n++] = bar(state->white, 100, 180 + 80 * i, (int32_t)(600 * position / SONG_TICKS),
                                now == CNA_MEDIA_STATE_PLAYING ? playing : paused);
        }
    }
    if (result == CNA_RESULT_SUCCESS) {
        memset(&begin_info, 0, sizeof(begin_info));
        begin_info.struct_size = (uint32_t)sizeof(begin_info);
        begin_info.struct_version = UINT32_C(1);
        begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
        result = cna_sprite_batch_begin(state->sprite_batch, &begin_info);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_sprite_batch_submit_many(state->sprite_batch, commands, n);
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_sprite_batch_end(state->sprite_batch);
        } else {
            (void)cna_sprite_batch_end(state->sprite_batch); /* end the batch even after a failure */
        }
    }
    if (result == CNA_RESULT_SUCCESS && state->demo->frames > 0 && state->frames == state->demo->frames &&
        cna_game_get_graphics_device(game, &graphics_device) == CNA_RESULT_SUCCESS) {
        cna_demo_capture(state->demo, graphics_device);
    }
    return result;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    PlayerState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    /* 100 frames at 60 per second cover the 1.4 s tour. */
    cna_demo_init(&demo, "c_api.media.media_player", argc, argv, 100);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.white = CNA_INVALID_HANDLE;
    state.sprite_batch = CNA_INVALID_HANDLE;
    state.songs = CNA_INVALID_HANDLE;
    state.queue = CNA_INVALID_HANDLE;
    state.state_changed = CNA_INVALID_HANDLE;
    state.song_changed = CNA_INVALID_HANDLE;
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
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    printf("two songs play as a queue: next, pause, resume, stop\n");
    cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager), "HiDef profile (back-buffer read-back)");
    cna_demo_check_result(&demo, cna_game_run(game), "cna_game_run");
    if (demo.automated) {
        cna_demo_check(&demo, state.step == 4, "the whole tour ran");
    }

    /* Event registrations, the queue view and the collection go before the game. */
    if (state.state_changed != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_media_player_unsubscribe_ext(state.state_changed),
                              "cna_media_player_unsubscribe_ext");
    }
    if (state.song_changed != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_media_player_unsubscribe_ext(state.song_changed),
                              "cna_media_player_unsubscribe_ext");
    }
    if (state.queue != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_media_queue_destroy(state.queue), "cna_media_queue_destroy (the view)");
    }
    if (state.songs != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_song_collection_destroy(state.songs), "cna_song_collection_destroy");
    }
    if (state.sprite_batch != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_sprite_batch_destroy(state.sprite_batch), "cna_sprite_batch_destroy");
    }
    if (state.white != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_texture2d_destroy(state.white), "cna_texture2d_destroy");
    }
    cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager), "cna_graphics_device_manager_destroy");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
