// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.media.library_music_values",
    .title = "Tagged media-library relationships in C",
    .description = "A C program indexes three original tagged FLAC tones and an M3U in a private library, then "
                   "follows shared Song, Album, Artist, Genre and Playlist relationships.",
    .apis = {"cna_album_get_name_size", "cna_album_copy_name", "cna_album_get_is_disposed",
             "cna_album_dispose", "cna_album_destroy", "cna_album_get_hash_code",
             "cna_album_get_songs", "cna_album_equals", "cna_album_get_artist",
             "cna_album_get_genre", "cna_album_get_duration", "cna_album_get_has_art",
             "cna_artist_get_name_size", "cna_artist_copy_name", "cna_artist_get_is_disposed",
             "cna_artist_dispose", "cna_artist_destroy", "cna_artist_get_hash_code",
             "cna_artist_get_songs", "cna_artist_get_albums", "cna_artist_equals",
             "cna_genre_get_name_size", "cna_genre_copy_name", "cna_genre_get_is_disposed",
             "cna_genre_dispose", "cna_genre_destroy", "cna_genre_get_hash_code",
             "cna_genre_get_songs", "cna_genre_get_albums", "cna_genre_equals",
             "cna_playlist_get_name_size", "cna_playlist_copy_name",
             "cna_playlist_get_is_disposed", "cna_playlist_dispose", "cna_playlist_destroy",
             "cna_playlist_get_hash_code", "cna_playlist_get_songs", "cna_playlist_equals",
             "cna_playlist_get_duration", "cna_song_get_album", "cna_song_get_artist",
             "cna_song_get_genre", "cna_media_library_create", "cna_media_library_get_songs",
             "cna_media_library_get_albums", "cna_media_library_get_artists",
             "cna_media_library_get_genres", "cna_media_library_get_playlists",
             "cna_song_collection_get_at", "cna_song_collection_get_count",
             "cna_song_get_name_size", "cna_song_copy_name", "cna_song_get_duration",
             "cna_song_get_track_number", "cna_title_container_read_ext"},
    .tags = {"c", "binding", "media", "library", "album", "artist", "genre", "playlist"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .assets = {"media/library_music/track_a.flac", "media/library_music/track_b.flac",
               "media/library_music/track_c.flac"},
    .restrictions = "Requires Linux/XDG user-folder resolution and skips before library access elsewhere. It "
                    "uses private temporary Music/Pictures roots and original generated audio fixtures. Duration "
                    "aggregation remains valid when a build has no FFmpeg metadata probe; no playback or audible "
                    "quality is claimed.",
})
