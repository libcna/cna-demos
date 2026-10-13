// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.media.media_player",
    .title = "Background music in C",
    .description = "A C program plays retained songs as a queue, exercises transport and events, then contrasts "
                   "collection lifecycle with caller-owned queue copies, empty bounds and unchecked active indices.",
    .apis = {"cna_song_create", "cna_song_create_from_uri", "cna_song_create_with_duration",
             "cna_song_get_handle_text_size_ext", "cna_song_copy_handle_text_ext", "cna_song_equals",
             "cna_song_get_hash_code", "cna_song_get_is_protected", "cna_song_get_is_rated",
             "cna_song_get_play_count", "cna_song_set_play_count", "cna_song_get_rating",
             "cna_song_get_track_number", "cna_song_get_duration",
             "cna_song_set_duration", "cna_song_get_is_disposed", "cna_song_dispose",
             "cna_song_collection_create", "cna_media_player_play_songs",
             "cna_media_player_play_songs_from", "cna_media_player_get_queue",
             "cna_media_queue_get_active_song_index", "cna_media_player_move_next",
             "cna_media_player_move_previous",
             "cna_media_player_pause", "cna_media_player_resume", "cna_media_player_stop",
             "cna_media_player_get_is_muted", "cna_media_player_set_is_muted",
             "cna_media_player_get_is_repeating", "cna_media_player_set_is_repeating",
             "cna_media_player_get_is_shuffled", "cna_media_player_set_is_shuffled",
             "cna_media_player_get_volume", "cna_media_player_set_volume",
             "cna_media_player_get_state", "cna_media_player_get_play_position_ticks",
             "cna_media_player_get_game_has_control", "cna_media_player_subscribe_media_state_changed_ext",
             "cna_media_player_subscribe_active_song_changed_ext", "cna_song_collection_get_at",
             "cna_song_collection_get_count", "cna_song_collection_get_is_disposed", "cna_song_collection_dispose",
             "cna_media_player_raise_active_song_changed_ext", "cna_media_player_raise_media_state_changed_ext",
             "cna_media_player_unsubscribe_ext", "cna_media_player_detect_song_ended_by_elapsed_time_ext",
             "cna_media_queue_get_count", "cna_media_queue_get_at",
             "cna_media_queue_get_active_song", "cna_media_queue_add",
             "cna_media_queue_clear", "cna_media_queue_set_active_song_index", "cna_media_queue_destroy",
             "cna_song_get_name_size", "cna_song_copy_name", "cna_song_destroy",
             "cna_song_collection_destroy", "cna_title_location_copy_path"},
    .tags = {"c", "binding", "media", "music", "song", "queue"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 100,
    .assets = {"music/rising.wav", "music/falling.wav"},
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). Queue lifecycle and bounds checks run after Stop and never replay a "
                    "queue-owned Song. No audible-quality or unsafe active-Song replay validation.",
})
