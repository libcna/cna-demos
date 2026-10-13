/* SPDX-License-Identifier: MS-PL */
/*
 * Background music from C: two songs are created from files under the title's path, put in a song
 * collection (which keeps them alive, so the program releases its own song handles at once) and
 * played as a queue. The program samples C visualization data, moves to the second song, pauses,
 * resumes and stops, and counts the MediaPlayer's two events -- which are process-wide, so their
 * subscriptions take no game.
 * After stopping, it contrasts a collection-shared Song handle with owned queue copies across
 * collection/view destruction and a safe queue Clear. Bars show the position in each song.
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
    CNA_VisualizationData visualization;
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

static int visualization_is_zero(const CNA_VisualizationData* data)
{
    uint32_t i;
    for (i = 0U; i < CNA_VISUALIZATION_DATA_SIZE; ++i) {
        if (data->frequencies[i] != 0.0f || data->samples[i] != 0.0f) {
            return 0;
        }
    }
    return 1;
}

static int visualization_has_signal(const CNA_VisualizationData* data)
{
    float frequency_energy = 0.0f;
    float sample_peak = 0.0f;
    uint32_t i;
    for (i = 0U; i < CNA_VISUALIZATION_DATA_SIZE; ++i) {
        const float sample = data->samples[i] < 0.0f ? -data->samples[i] : data->samples[i];
        frequency_energy += data->frequencies[i];
        if (sample > sample_peak) {
            sample_peak = sample;
        }
    }
    return frequency_energy > 0.0f && sample_peak > 0.0f;
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

static CNA_Result create_song_with_duration(CNA_Handle game, const char* name, const char* file,
                                            int32_t duration_milliseconds, CNA_SongHandle* out_song)
{
    char path[1024];
    uint64_t length = 0U;
    CNA_Result result = cna_title_location_copy_path(game, path, sizeof(path) - 64U, &length);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    length += (uint64_t)snprintf(path + length, sizeof(path) - (size_t)length, "/assets/music/%s", file);
    return cna_song_create_with_duration(game, view(path, length), view(name, strlen(name)), duration_milliseconds,
                                         out_song);
}

static CNA_Result build_song_path(CNA_Handle game, const char* file, char* path, size_t capacity,
                                  uint64_t* out_length)
{
    uint64_t length = 0U;
    CNA_Result result = cna_title_location_copy_path(game, path, (uint64_t)capacity - 64U, &length);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    length += (uint64_t)snprintf(path + length, capacity - (size_t)length, "/assets/music/%s", file);
    *out_length = length;
    return CNA_RESULT_SUCCESS;
}

static int song_has_name(CNA_SongHandle song, const char* expected)
{
    char actual[64];
    uint64_t required = 0U;
    uint64_t copied = 0U;
    const uint64_t expected_length = (uint64_t)strlen(expected);
    memset(actual, 0, sizeof(actual));
    return cna_song_get_name_size(song, &required) == CNA_RESULT_SUCCESS && required == expected_length &&
           cna_song_copy_name(song, actual, sizeof(actual), &copied) == CNA_RESULT_SUCCESS &&
           copied == expected_length && memcmp(actual, expected, (size_t)expected_length) == 0;
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
    CNA_Bool visualization_enabled = CNA_FALSE;
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
    cna_demo_check(state->demo,
                   cna_visualization_data_init(&state->visualization) == CNA_RESULT_SUCCESS &&
                       state->visualization.struct_size == (uint32_t)sizeof(state->visualization) &&
                       state->visualization.struct_version == UINT32_C(1) &&
                       visualization_is_zero(&state->visualization),
                   "visualization data initializes as a versioned zero buffer");
    cna_demo_check(state->demo,
                   cna_media_player_set_is_visualization_enabled(game, CNA_TRUE) == CNA_RESULT_SUCCESS &&
                       cna_media_player_get_is_visualization_enabled(game, &visualization_enabled) ==
                           CNA_RESULT_SUCCESS &&
                       visualization_enabled == CNA_TRUE,
                   "visualization capture can be enabled and read back");

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
        cna_demo_check(state->demo,
                       cna_media_player_get_visualization_data(game, &state->visualization) == CNA_RESULT_SUCCESS &&
                           visualization_has_signal(&state->visualization),
                       "playing music produces nonzero frequency and sample data");
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
    CNA_SongHandle collection_song = CNA_INVALID_HANDLE;
    CNA_SongHandle queue_first = CNA_INVALID_HANDLE;
    CNA_SongHandle active_copy = CNA_INVALID_HANDLE;
    CNA_SongHandle appended = CNA_INVALID_HANDLE;
    CNA_SongHandle appended_copy = CNA_INVALID_HANDLE;
    CNA_SongHandle duration_song = CNA_INVALID_HANDLE;
    CNA_SongHandle identity_song = CNA_INVALID_HANDLE;
    CNA_SongHandle same_path_song = CNA_INVALID_HANDLE;
    CNA_SongHandle other_path_song = CNA_INVALID_HANDLE;
    CNA_SongHandle empty_song = CNA_INVALID_HANDLE;
    CNA_Bool active_available = CNA_FALSE;
    CNA_Bool collection_disposed = CNA_TRUE;
    CNA_Bool empty_available = CNA_TRUE;
    int32_t collection_count = -1;
    int32_t queue_count = -1;

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
    if (media_state(game) != CNA_MEDIA_STATE_STOPPED) {
        (void)cna_media_player_stop(game); /* keep the ownership exercise independent of live playback */
    }

    /* Disabled visualization returns its canonical zero buffer even after live data was captured.
       This is a state/data contract only; it does not measure audible output or FFT accuracy. */
    {
        CNA_Bool visualization_enabled = CNA_TRUE;
        uint32_t i;
        for (i = 0U; i < CNA_VISUALIZATION_DATA_SIZE; ++i) {
            state.visualization.frequencies[i] = 1.0f;
            state.visualization.samples[i] = -1.0f;
        }
        cna_demo_check(&demo,
                       cna_media_player_set_is_visualization_enabled(game, CNA_FALSE) == CNA_RESULT_SUCCESS &&
                           cna_media_player_get_is_visualization_enabled(game, &visualization_enabled) ==
                               CNA_RESULT_SUCCESS &&
                           visualization_enabled == CNA_FALSE &&
                           cna_media_player_get_visualization_data(game, &state.visualization) ==
                               CNA_RESULT_SUCCESS &&
                           visualization_is_zero(&state.visualization),
                       "disabled visualization reports false and replaces prior data with zeros");
    }

    /* Process-wide playback settings remain directly observable while stopped. Restore their
       ordinary values before the ownership and lifecycle checks below. */
    {
        CNA_Bool setting = CNA_FALSE;
        float volume = -1.0f;
        cna_demo_check(&demo,
                       cna_media_player_set_is_muted(game, CNA_TRUE) == CNA_RESULT_SUCCESS &&
                           cna_media_player_get_is_muted(game, &setting) == CNA_RESULT_SUCCESS &&
                           setting == CNA_TRUE &&
                           cna_media_player_set_is_muted(game, CNA_FALSE) == CNA_RESULT_SUCCESS &&
                           cna_media_player_get_is_muted(game, &setting) == CNA_RESULT_SUCCESS &&
                           setting == CNA_FALSE,
                       "mute can be enabled, read back and restored while stopped");
        cna_demo_check(&demo,
                       cna_media_player_set_is_repeating(game, CNA_TRUE) == CNA_RESULT_SUCCESS &&
                           cna_media_player_get_is_repeating(game, &setting) == CNA_RESULT_SUCCESS &&
                           setting == CNA_TRUE &&
                           cna_media_player_set_is_repeating(game, CNA_FALSE) == CNA_RESULT_SUCCESS &&
                           cna_media_player_get_is_repeating(game, &setting) == CNA_RESULT_SUCCESS &&
                           setting == CNA_FALSE,
                       "repeat can be enabled, read back and restored while stopped");
        cna_demo_check(&demo,
                       cna_media_player_set_is_shuffled(game, CNA_TRUE) == CNA_RESULT_SUCCESS &&
                           cna_media_player_get_is_shuffled(game, &setting) == CNA_RESULT_SUCCESS &&
                           setting == CNA_TRUE &&
                           cna_media_player_set_is_shuffled(game, CNA_FALSE) == CNA_RESULT_SUCCESS &&
                           cna_media_player_get_is_shuffled(game, &setting) == CNA_RESULT_SUCCESS &&
                           setting == CNA_FALSE,
                       "shuffle can be enabled, read back and restored while stopped");
        cna_demo_check(&demo,
                       cna_media_player_set_volume(game, -0.25f) == CNA_RESULT_SUCCESS &&
                           cna_media_player_get_volume(game, &volume) == CNA_RESULT_SUCCESS && volume == 0.0f &&
                           cna_media_player_set_volume(game, 1.25f) == CNA_RESULT_SUCCESS &&
                           cna_media_player_get_volume(game, &volume) == CNA_RESULT_SUCCESS && volume == 1.0f &&
                           cna_media_player_set_volume(game, 1.0f) == CNA_RESULT_SUCCESS,
                       "volume clamps below zero and above one before full volume is restored");
    }

    /* Rebuild the queue from the still-live collection at a chosen start index, navigate backward,
       then restore the second-song stopped state expected by the ownership checks. */
    cna_demo_check(&demo,
                   cna_media_player_play_songs_from(game, state.songs, 1) == CNA_RESULT_SUCCESS &&
                       media_state(game) == CNA_MEDIA_STATE_PLAYING && active_index(&state) == 1 &&
                       cna_media_queue_get_count(state.queue, &queue_count) == CNA_RESULT_SUCCESS &&
                       queue_count == 2,
                   "play_songs_from starts the live collection at its second Song");
    cna_demo_check(&demo,
                   cna_media_player_move_previous(game) == CNA_RESULT_SUCCESS && active_index(&state) == 0,
                   "move_previous returns from the second queued Song to the first");
    cna_demo_check(&demo,
                   cna_media_player_move_next(game) == CNA_RESULT_SUCCESS && active_index(&state) == 1 &&
                       cna_media_player_stop(game) == CNA_RESULT_SUCCESS &&
                       media_state(game) == CNA_MEDIA_STATE_STOPPED,
                   "move_next and stop restore the prior stopped queue state");

    /* The collection shares its retained Song, while queue accessors publish independent copies.
       Keep those handles across view/collection destruction and a safe stopped-queue Clear. */
    cna_demo_check(&demo,
                   cna_song_collection_get_count(state.songs, &collection_count) == CNA_RESULT_SUCCESS &&
                       collection_count == 2 &&
                       cna_song_collection_get_is_disposed(state.songs, &collection_disposed) == CNA_RESULT_SUCCESS &&
                       collection_disposed == CNA_FALSE,
                   "the SongCollection starts live with two songs");
    cna_demo_check_result(&demo, cna_song_collection_get_at(state.songs, 0, &collection_song),
                          "collection_get_at returns a shared retained Song handle");
    cna_demo_check(&demo,
                   cna_song_collection_dispose(state.songs) == CNA_RESULT_SUCCESS &&
                       cna_song_collection_get_count(state.songs, &collection_count) == CNA_RESULT_SUCCESS &&
                       collection_count == 0 &&
                       cna_song_collection_get_is_disposed(state.songs, &collection_disposed) == CNA_RESULT_SUCCESS &&
                       collection_disposed == CNA_TRUE && song_has_name(collection_song, "Rising"),
                   "disposing the collection empties it but keeps the shared Song alive");
    cna_demo_check_result(&demo, cna_song_collection_dispose(state.songs),
                          "disposing the SongCollection twice is a successful no-op");
    cna_demo_check_result(&demo, cna_media_queue_get_at(state.queue, 0, &queue_first),
                          "queue_get_at returns an owned copy of Rising");
    cna_demo_check(&demo,
                   cna_media_queue_get_active_song(state.queue, &active_copy, &active_available) == CNA_RESULT_SUCCESS &&
                       active_available == CNA_TRUE,
                   "queue_get_active_song returns an owned copy of the stopped active song");
    cna_demo_check_result(&demo, create_song(game, "Appended", "rising.wav", &appended),
                          "create the caller-owned Song used by queue_add");
    cna_demo_check_result(&demo, cna_media_queue_add(state.queue, appended),
                          "queue_add copies the caller-owned Song");
    cna_demo_check_result(&demo, cna_song_destroy(appended),
                          "destroy the queue_add input handle");
    appended = CNA_INVALID_HANDLE;
    cna_demo_check(&demo,
                   cna_media_queue_get_count(state.queue, &queue_count) == CNA_RESULT_SUCCESS && queue_count == 3 &&
                       cna_media_queue_get_at(state.queue, 2, &appended_copy) == CNA_RESULT_SUCCESS,
                   "the queue keeps three entries and publishes a copy of Appended");

    /* Explicit raises run subscribed process-wide handlers synchronously. Releasing both
       registrations then makes the same raises harmless while the game remains active. */
    {
        const int song_changes_before_raise = state.song_changes;
        const int state_changes_before_raise = state.state_changes;
        const CNA_Result song_raise = cna_media_player_raise_active_song_changed_ext(game);
        const CNA_Result state_raise = cna_media_player_raise_media_state_changed_ext(game);
        cna_demo_check(&demo,
                       song_raise == CNA_RESULT_SUCCESS && state_raise == CNA_RESULT_SUCCESS &&
                           state.song_changes == song_changes_before_raise + 1 &&
                           state.state_changes == state_changes_before_raise + 1,
                       "manual media-player raises invoke both subscribed handlers synchronously");
    }

    /* Event registrations, the queue view and the collection go before the game. */
    if (state.state_changed != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_media_player_unsubscribe_ext(state.state_changed),
                              "cna_media_player_unsubscribe_ext");
        state.state_changed = CNA_INVALID_HANDLE;
    }
    if (state.song_changed != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_media_player_unsubscribe_ext(state.song_changed),
                              "cna_media_player_unsubscribe_ext");
        state.song_changed = CNA_INVALID_HANDLE;
    }
    {
        const int song_changes_after_unsubscribe = state.song_changes;
        const int state_changes_after_unsubscribe = state.state_changes;
        const CNA_Result song_raise = cna_media_player_raise_active_song_changed_ext(game);
        const CNA_Result state_raise = cna_media_player_raise_media_state_changed_ext(game);
        cna_demo_check(&demo,
                       song_raise == CNA_RESULT_SUCCESS && state_raise == CNA_RESULT_SUCCESS &&
                           state.song_changes == song_changes_after_unsubscribe &&
                           state.state_changes == state_changes_after_unsubscribe,
                       "unsubscribed media-player handlers ignore later manual raises");
    }
    if (state.queue != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_media_queue_destroy(state.queue), "cna_media_queue_destroy (the view)");
        state.queue = CNA_INVALID_HANDLE;
    }
    if (state.songs != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_song_collection_destroy(state.songs), "cna_song_collection_destroy");
        state.songs = CNA_INVALID_HANDLE;
    }
    cna_demo_check(&demo, song_has_name(collection_song, "Rising"),
                   "the shared collection Song survives collection destruction");
    cna_demo_check(&demo,
                   cna_media_player_get_queue(game, &state.queue) == CNA_RESULT_SUCCESS &&
                       cna_media_queue_get_count(state.queue, &queue_count) == CNA_RESULT_SUCCESS && queue_count == 3,
                   "a new queue view sees all entries after the first view is destroyed");
    cna_demo_check_result(&demo, cna_media_queue_clear(state.queue),
                          "queue_clear releases its entries while the player is stopped");
    cna_demo_check(&demo,
                   cna_media_queue_get_count(state.queue, &queue_count) == CNA_RESULT_SUCCESS && queue_count == 0 &&
                       active_index(&state) == -1,
                   "queue_clear empties the queue and resets the active index");
    empty_song = collection_song;
    empty_available = CNA_TRUE;
    cna_demo_check(&demo,
                   cna_media_queue_get_active_song(state.queue, &empty_song, &empty_available) == CNA_RESULT_SUCCESS &&
                       empty_available == CNA_FALSE && empty_song == CNA_INVALID_HANDLE,
                   "an empty active-song lookup reports unavailable and initializes its output");
    empty_song = collection_song;
    cna_demo_check(&demo,
                   cna_media_queue_get_at(state.queue, 0, &empty_song) == CNA_RESULT_INVALID_ARGUMENT &&
                       empty_song == CNA_INVALID_HANDLE,
                   "queue_get_at rejects an empty index and initializes its output");
    empty_song = collection_song;
    empty_available = CNA_TRUE;
    cna_demo_check(&demo,
                   cna_media_queue_set_active_song_index(state.queue, 7) == CNA_RESULT_SUCCESS &&
                       active_index(&state) == 7 &&
                       cna_media_queue_get_active_song(state.queue, &empty_song, &empty_available) == CNA_RESULT_SUCCESS &&
                       empty_available == CNA_FALSE && empty_song == CNA_INVALID_HANDLE,
                   "the unchecked active index is stored but has no active Song");
    cna_demo_check(&demo,
                   cna_media_queue_set_active_song_index(state.queue, -1) == CNA_RESULT_SUCCESS &&
                       active_index(&state) == -1,
                   "the active index can be restored after the bounds check");
    cna_demo_check(&demo,
                   song_has_name(queue_first, "Rising") && song_has_name(active_copy, "Falling") &&
                       song_has_name(appended_copy, "Appended"),
                   "owned queue copies remain readable after queue_clear");
    {
        const CNA_Result collection_release = cna_song_destroy(collection_song);
        const CNA_Result first_release = cna_song_destroy(queue_first);
        const CNA_Result active_release = cna_song_destroy(active_copy);
        const CNA_Result appended_release = cna_song_destroy(appended_copy);
        cna_demo_check(&demo,
                       collection_release == CNA_RESULT_SUCCESS && first_release == CNA_RESULT_SUCCESS &&
                           active_release == CNA_RESULT_SUCCESS && appended_release == CNA_RESULT_SUCCESS,
                       "all retained and copied Song handles release independently");
    }
    cna_demo_check_result(&demo, cna_media_queue_destroy(state.queue),
                          "destroy the reacquired queue view");
    state.queue = CNA_INVALID_HANDLE;

    /* An explicit duration is caller metadata, independent of the file's real two-second length.
       The fallback end detector uses that metadata and treats zero as unknown rather than ended. */
    if (cna_demo_check_result(&demo, create_song_with_duration(game, "Timed", "rising.wav", 750, &duration_song),
                              "create a Song with an explicit 750 ms duration")) {
        const int64_t explicit_ticks = TICKS_PER_SECOND * 3 / 4;
        int64_t duration_ticks = -1;
        CNA_Bool ended_below = CNA_TRUE;
        CNA_Bool ended_at = CNA_FALSE;
        CNA_Bool ended_without_duration = CNA_TRUE;
        CNA_Bool song_disposed = CNA_TRUE;
        const CNA_Result duration_result = cna_song_get_duration(duration_song, &duration_ticks);
        const CNA_Result below_result = cna_media_player_detect_song_ended_by_elapsed_time_ext(
            duration_song, explicit_ticks - 1, &ended_below);
        const CNA_Result at_result = cna_media_player_detect_song_ended_by_elapsed_time_ext(
            duration_song, explicit_ticks, &ended_at);
        cna_demo_check(&demo,
                       duration_result == CNA_RESULT_SUCCESS && duration_ticks == explicit_ticks &&
                           below_result == CNA_RESULT_SUCCESS && ended_below == CNA_FALSE &&
                           at_result == CNA_RESULT_SUCCESS && ended_at == CNA_TRUE,
                       "the explicit duration controls the elapsed-time end boundary exactly");
        cna_demo_check(&demo,
                       cna_song_set_duration(duration_song, 0) == CNA_RESULT_SUCCESS &&
                           cna_song_get_duration(duration_song, &duration_ticks) == CNA_RESULT_SUCCESS &&
                           duration_ticks == 0 &&
                           cna_media_player_detect_song_ended_by_elapsed_time_ext(
                               duration_song, TICKS_PER_SECOND * 10, &ended_without_duration) == CNA_RESULT_SUCCESS &&
                           ended_without_duration == CNA_FALSE,
                       "a zero Song duration never reports elapsed-time completion");
        cna_demo_check(&demo,
                       cna_song_get_is_disposed(duration_song, &song_disposed) == CNA_RESULT_SUCCESS &&
                           song_disposed == CNA_FALSE,
                       "the explicit-duration Song starts live");
        cna_demo_check(&demo,
                       cna_song_dispose(duration_song) == CNA_RESULT_SUCCESS &&
                           cna_song_dispose(duration_song) == CNA_RESULT_SUCCESS &&
                           cna_song_get_is_disposed(duration_song, &song_disposed) == CNA_RESULT_SUCCESS &&
                           song_disposed == CNA_TRUE &&
                           cna_song_get_duration(duration_song, &duration_ticks) == CNA_RESULT_SUCCESS &&
                           duration_ticks == 0,
                       "Song Dispose is idempotent and leaves Duration readable");
        cna_demo_check_result(&demo, cna_song_destroy(duration_song),
                              "destroy the explicit-duration Song handle");
        duration_song = CNA_INVALID_HANDLE;
    }

    /* Song identity is the stored file path, independent of the handle and display name. These
       caller-owned helpers are never queued or played. */
    {
        char rising_path[1024];
        char falling_path[1024];
        char handle_text[1024];
        uint64_t rising_length = 0U;
        uint64_t falling_length = 0U;
        uint64_t required = 0U;
        uint64_t copied = 0U;
        CNA_Bool equal_same_path = CNA_FALSE;
        CNA_Bool equal_other_path = CNA_TRUE;
        CNA_Bool is_protected = CNA_TRUE;
        CNA_Bool is_rated = CNA_TRUE;
        int32_t identity_hash = 0;
        int32_t same_path_hash = 1;
        int32_t play_count = -1;
        int32_t rating = -1;
        int32_t track_number = -1;
        CNA_Result result = build_song_path(game, "rising.wav", rising_path, sizeof(rising_path), &rising_length);
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_song_create(game, view(rising_path, rising_length), view("Identity", 8U), &identity_song);
        }
        cna_demo_check_result(&demo, result, "create a caller-owned Song from a plain file path");

        result = cna_song_create(game, view(rising_path, rising_length), view("Different name", 14U),
                                 &same_path_song);
        cna_demo_check_result(&demo, result, "create another named Song over the same file path");

        result = build_song_path(game, "falling.wav", falling_path, sizeof(falling_path), &falling_length);
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_song_create(game, view(falling_path, falling_length), view("", 0U), &other_path_song);
        }
        cna_demo_check_result(&demo, result, "create an empty-name Song over a different file path");

        memset(handle_text, 0, sizeof(handle_text));
        cna_demo_check(&demo,
                       cna_song_get_handle_text_size_ext(identity_song, &required) == CNA_RESULT_SUCCESS &&
                           required == rising_length &&
                           cna_song_copy_handle_text_ext(identity_song, handle_text, sizeof(handle_text), &copied) ==
                               CNA_RESULT_SUCCESS &&
                           copied == rising_length && memcmp(handle_text, rising_path, (size_t)rising_length) == 0,
                       "the Song handle text is the exact stored file path");
        cna_demo_check(&demo,
                       song_has_name(identity_song, "Identity") && song_has_name(same_path_song, "Different name") &&
                           cna_song_equals(identity_song, same_path_song, &equal_same_path) == CNA_RESULT_SUCCESS &&
                           equal_same_path == CNA_TRUE &&
                           cna_song_equals(identity_song, other_path_song, &equal_other_path) == CNA_RESULT_SUCCESS &&
                           equal_other_path == CNA_FALSE,
                       "Song equality uses the path rather than handle or display-name identity");
        cna_demo_check(&demo,
                       cna_song_get_hash_code(identity_song, &identity_hash) == CNA_RESULT_SUCCESS &&
                           cna_song_get_hash_code(same_path_song, &same_path_hash) == CNA_RESULT_SUCCESS &&
                           identity_hash == same_path_hash,
                       "equal Songs have equal path-based hash codes");
        cna_demo_check(&demo, song_has_name(other_path_song, ""),
                       "the plain-path constructor preserves an empty display name");
        cna_demo_check(&demo,
                       cna_song_get_is_protected(identity_song, &is_protected) == CNA_RESULT_SUCCESS &&
                           is_protected == CNA_FALSE &&
                           cna_song_get_is_rated(identity_song, &is_rated) == CNA_RESULT_SUCCESS &&
                           is_rated == CNA_FALSE &&
                           cna_song_get_rating(identity_song, &rating) == CNA_RESULT_SUCCESS && rating == 0 &&
                           cna_song_get_track_number(identity_song, &track_number) == CNA_RESULT_SUCCESS &&
                           track_number == 0 &&
                           cna_song_get_play_count(identity_song, &play_count) == CNA_RESULT_SUCCESS &&
                           play_count == 0,
                       "a plain-path Song starts unprotected, unrated and without library counters");
        cna_demo_check(&demo,
                       cna_song_set_play_count(identity_song, 42) == CNA_RESULT_SUCCESS &&
                           cna_song_get_play_count(identity_song, &play_count) == CNA_RESULT_SUCCESS &&
                           play_count == 42,
                       "the caller can update and read back the Song play count");
        {
            const CNA_Result identity_release = identity_song != CNA_INVALID_HANDLE
                                                    ? cna_song_destroy(identity_song)
                                                    : CNA_RESULT_SUCCESS;
            const CNA_Result same_path_release = same_path_song != CNA_INVALID_HANDLE
                                                     ? cna_song_destroy(same_path_song)
                                                     : CNA_RESULT_SUCCESS;
            const CNA_Result other_path_release = other_path_song != CNA_INVALID_HANDLE
                                                      ? cna_song_destroy(other_path_song)
                                                      : CNA_RESULT_SUCCESS;
            cna_demo_check(&demo,
                           identity_release == CNA_RESULT_SUCCESS && same_path_release == CNA_RESULT_SUCCESS &&
                               other_path_release == CNA_RESULT_SUCCESS,
                           "destroy all caller-owned Song identity handles independently");
        }
        identity_song = CNA_INVALID_HANDLE;
        same_path_song = CNA_INVALID_HANDLE;
        other_path_song = CNA_INVALID_HANDLE;
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
