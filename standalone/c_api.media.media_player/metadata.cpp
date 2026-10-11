// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.media.media_player",
    .title = "Background music in C",
    .description = "A C program creates two songs from files under the title path, plays them as a queue from a "
                   "song collection (which keeps the songs alive), then moves to the next song, pauses, resumes and "
                   "stops while counting the MediaPlayer's process-wide events.",
    .apis = {"cna_song_create_from_uri", "cna_song_collection_create", "cna_media_player_play_songs",
             "cna_media_player_get_queue", "cna_media_queue_get_active_song_index", "cna_media_player_move_next",
             "cna_media_player_pause", "cna_media_player_resume", "cna_media_player_stop",
             "cna_media_player_get_state", "cna_media_player_get_play_position_ticks",
             "cna_media_player_get_game_has_control", "cna_media_player_subscribe_media_state_changed_ext",
             "cna_media_player_subscribe_active_song_changed_ext", "cna_title_location_copy_path"},
    .tags = {"c", "binding", "media", "music", "song", "queue"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 100,
    .assets = {"music/rising.wav", "music/falling.wav"},
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
