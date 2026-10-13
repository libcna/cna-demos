// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.media.library_collections",
    .title = "Media library collection ownership in C",
    .description = "A C program opens the default and explicit local-device media libraries, reads every music "
                   "collection, then demonstrates child-retained lifetime and canonical disposal.",
    .apis = {"cna_media_library_create", "cna_media_library_create_from_source",
             "cna_media_library_get_is_disposed", "cna_media_library_dispose",
             "cna_media_library_destroy", "cna_media_library_get_media_source_type",
             "cna_media_library_get_media_source_name_size",
             "cna_media_library_copy_media_source_name", "cna_media_library_get_songs",
             "cna_media_library_get_albums", "cna_media_library_get_artists",
             "cna_media_library_get_genres", "cna_media_library_get_playlists",
             "cna_album_collection_get_count", "cna_album_collection_get_at",
             "cna_album_collection_get_is_disposed", "cna_album_collection_dispose",
             "cna_album_collection_destroy", "cna_artist_collection_get_count",
             "cna_artist_collection_get_at", "cna_artist_collection_get_is_disposed",
             "cna_artist_collection_dispose", "cna_artist_collection_destroy",
             "cna_genre_collection_get_count", "cna_genre_collection_get_at",
             "cna_genre_collection_get_is_disposed", "cna_genre_collection_dispose",
             "cna_genre_collection_destroy", "cna_playlist_collection_get_count",
             "cna_playlist_collection_get_at", "cna_playlist_collection_get_is_disposed",
             "cna_playlist_collection_dispose", "cna_playlist_collection_destroy",
             "cna_media_source_get_available_count", "cna_song_collection_get_count",
             "cna_song_collection_get_is_disposed", "cna_song_collection_dispose",
             "cna_song_collection_destroy", "cna_game_create", "cna_game_destroy"},
    .tags = {"c", "binding", "media", "library", "collections", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Reads the host's media-library roots but neither prints nor requires their contents. This "
                    "batch validates empty-safe collection and ownership contracts; positive album, artist, "
                    "genre and playlist values are covered separately with controlled media fixtures.",
})
