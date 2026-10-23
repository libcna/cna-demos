/* SPDX-License-Identifier: MS-PL */
/* MediaLibrary construction, music collections, disposal and retained ownership through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* data, uint64_t byte_length)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = byte_length;
    return value;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_MediaLibraryHandle library = CNA_INVALID_HANDLE;
    CNA_MediaLibraryHandle from_source = CNA_INVALID_HANDLE;
    CNA_MediaLibraryHandle invalid_library = UINT64_C(99);
    CNA_SongCollectionHandle songs = CNA_INVALID_HANDLE;
    CNA_AlbumCollectionHandle albums = CNA_INVALID_HANDLE;
    CNA_ArtistCollectionHandle artists = CNA_INVALID_HANDLE;
    CNA_GenreCollectionHandle genres = CNA_INVALID_HANDLE;
    CNA_PlaylistCollectionHandle playlists = CNA_INVALID_HANDLE;
    CNA_Handle stale;
    CNA_Handle element;
    CNA_MediaSourceType source_type = CNA_MEDIA_SOURCE_TYPE_WINDOWS_MEDIA_CONNECT;
    CNA_Bool disposed = CNA_TRUE;
    uint32_t source_count = UINT32_C(99);
    int32_t song_count = -1;
    int32_t album_count = -1;
    int32_t artist_count = -1;
    int32_t genre_count = -1;
    int32_t playlist_count = -1;
    uint64_t name_bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    char source_name[32];
    char short_name[32];
    char short_before[32];

    cna_demo_init(&demo, "c_api.media.library_collections", argc, argv, 1);
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.window_title = text_view(
        "C media library collections", sizeof("C media library collections") - 1U);
    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game),
            "create the owner game without opening a window")) {
        return cna_demo_finish(&demo);
    }

    cna_demo_check(
        &demo,
        cna_media_source_get_available_count(game, &source_count) == CNA_RESULT_SUCCESS &&
            source_count == 1U &&
            cna_media_library_create_from_source(
                game, source_count, &invalid_library) == CNA_RESULT_INVALID_ARGUMENT &&
            invalid_library == CNA_INVALID_HANDLE,
        "the sole desktop source count is also an atomic constructor bound");
    if (!cna_demo_check_result(
            &demo, cna_media_library_create(game, &library),
            "open the default device media library") ||
        !cna_demo_check_result(
            &demo, cna_media_library_create_from_source(game, 0U, &from_source),
            "open the same library from the enumerated local-device source")) {
        goto cleanup;
    }

    memset(source_name, 0, sizeof(source_name));
    cna_demo_check(
        &demo,
        cna_media_library_get_media_source_type(library, &source_type) ==
                CNA_RESULT_SUCCESS &&
            source_type == CNA_MEDIA_SOURCE_TYPE_LOCAL_DEVICE &&
            cna_media_library_get_media_source_name_size(library, &name_bytes) ==
                CNA_RESULT_SUCCESS &&
            name_bytes == sizeof("Local Device") - 1U &&
            cna_media_library_copy_media_source_name(
                library, source_name, sizeof(source_name), &copied) == CNA_RESULT_SUCCESS &&
            copied == name_bytes &&
            memcmp(source_name, "Local Device", sizeof("Local Device") - 1U) == 0,
        "the default library reports its exact local-device source");
    memset(short_name, 0xA5, sizeof(short_name));
    memset(short_before, 0xA5, sizeof(short_before));
    copied = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_media_library_copy_media_source_name(
            library, short_name, name_bytes - UINT64_C(1), &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == name_bytes &&
            memcmp(short_name, short_before, sizeof(short_name)) == 0,
        "an undersized media-source name copy reports its size without a partial write");
    source_type = CNA_MEDIA_SOURCE_TYPE_WINDOWS_MEDIA_CONNECT;
    cna_demo_check(
        &demo,
        cna_media_library_get_media_source_type(from_source, &source_type) ==
                CNA_RESULT_SUCCESS &&
            source_type == CNA_MEDIA_SOURCE_TYPE_LOCAL_DEVICE,
        "the explicit-source constructor copies the selected local-device identity");
    cna_demo_check(
        &demo,
        cna_media_library_get_is_disposed(library, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE,
        "a newly opened media library is live");

    if (!cna_demo_check_result(
            &demo, cna_media_library_get_songs(library, &songs),
            "borrow the library's song collection") ||
        !cna_demo_check_result(
            &demo, cna_media_library_get_albums(library, &albums),
            "borrow the library's album collection") ||
        !cna_demo_check_result(
            &demo, cna_media_library_get_artists(library, &artists),
            "borrow the library's artist collection") ||
        !cna_demo_check_result(
            &demo, cna_media_library_get_genres(library, &genres),
            "borrow the library's genre collection") ||
        !cna_demo_check_result(
            &demo, cna_media_library_get_playlists(library, &playlists),
            "borrow the library's playlist collection")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_song_collection_get_count(songs, &song_count) == CNA_RESULT_SUCCESS &&
            song_count >= 0 &&
            cna_album_collection_get_count(albums, &album_count) == CNA_RESULT_SUCCESS &&
            album_count >= 0 &&
            cna_artist_collection_get_count(artists, &artist_count) == CNA_RESULT_SUCCESS &&
            artist_count >= 0 &&
            cna_genre_collection_get_count(genres, &genre_count) == CNA_RESULT_SUCCESS &&
            genre_count >= 0 &&
            cna_playlist_collection_get_count(playlists, &playlist_count) == CNA_RESULT_SUCCESS &&
            playlist_count >= 0,
        "all five music collections report nonnegative host-dependent counts");

    element = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_album_collection_get_at(albums, album_count, &element) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            element == CNA_INVALID_HANDLE,
        "the album collection upper bound is refused without publishing an element");
    element = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_artist_collection_get_at(artists, artist_count, &element) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            element == CNA_INVALID_HANDLE,
        "the artist collection upper bound is refused without publishing an element");
    element = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_genre_collection_get_at(genres, genre_count, &element) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            element == CNA_INVALID_HANDLE,
        "the genre collection upper bound is refused without publishing an element");
    element = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_playlist_collection_get_at(playlists, playlist_count, &element) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            element == CNA_INVALID_HANDLE,
        "the playlist collection upper bound is refused without publishing an element");

    cna_demo_check(
        &demo,
        cna_media_library_dispose(library) == CNA_RESULT_SUCCESS &&
            cna_media_library_dispose(library) == CNA_RESULT_SUCCESS &&
            cna_media_library_get_is_disposed(library, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_album_collection_get_count(albums, &album_count) == CNA_RESULT_SUCCESS &&
            cna_artist_collection_get_count(artists, &artist_count) == CNA_RESULT_SUCCESS &&
            cna_genre_collection_get_count(genres, &genre_count) == CNA_RESULT_SUCCESS &&
            cna_playlist_collection_get_count(playlists, &playlist_count) == CNA_RESULT_SUCCESS,
        "library disposal is idempotent and leaves its catalog collections readable");
    stale = library;
    cna_demo_check_result(
        &demo, cna_media_library_destroy(library),
        "release the library handle while its collection handles retain the catalog");
    library = CNA_INVALID_HANDLE;
    disposed = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_media_library_get_is_disposed(stale, &disposed) == CNA_RESULT_INVALID_HANDLE &&
            disposed == CNA_FALSE &&
            cna_song_collection_get_count(songs, &song_count) == CNA_RESULT_SUCCESS &&
            cna_album_collection_get_count(albums, &album_count) == CNA_RESULT_SUCCESS &&
            cna_artist_collection_get_count(artists, &artist_count) == CNA_RESULT_SUCCESS &&
            cna_genre_collection_get_count(genres, &genre_count) == CNA_RESULT_SUCCESS &&
            cna_playlist_collection_get_count(playlists, &playlist_count) == CNA_RESULT_SUCCESS,
        "released library inspection fails atomically while every child keeps it alive");

    disposed = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_album_collection_get_is_disposed(albums, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_album_collection_dispose(albums) == CNA_RESULT_SUCCESS &&
            cna_album_collection_dispose(albums) == CNA_RESULT_SUCCESS &&
            cna_album_collection_get_is_disposed(albums, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_album_collection_get_count(albums, &album_count) == CNA_RESULT_INVALID_STATE,
        "disposing the album collection twice refuses its count and records disposal");
    disposed = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_artist_collection_get_is_disposed(artists, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_artist_collection_dispose(artists) == CNA_RESULT_SUCCESS &&
            cna_artist_collection_dispose(artists) == CNA_RESULT_SUCCESS &&
            cna_artist_collection_get_is_disposed(artists, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_artist_collection_get_count(artists, &artist_count) == CNA_RESULT_INVALID_STATE,
        "disposing the artist collection twice refuses its count and records disposal");
    disposed = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_genre_collection_get_is_disposed(genres, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_genre_collection_dispose(genres) == CNA_RESULT_SUCCESS &&
            cna_genre_collection_dispose(genres) == CNA_RESULT_SUCCESS &&
            cna_genre_collection_get_is_disposed(genres, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_genre_collection_get_count(genres, &genre_count) == CNA_RESULT_INVALID_STATE,
        "disposing the genre collection twice refuses its count and records disposal");
    disposed = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_playlist_collection_get_is_disposed(playlists, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_playlist_collection_dispose(playlists) == CNA_RESULT_SUCCESS &&
            cna_playlist_collection_dispose(playlists) == CNA_RESULT_SUCCESS &&
            cna_playlist_collection_get_is_disposed(playlists, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_playlist_collection_get_count(playlists, &playlist_count) == CNA_RESULT_INVALID_STATE,
        "disposing the playlist collection twice refuses its count and records disposal");
    disposed = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_song_collection_get_is_disposed(songs, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_song_collection_dispose(songs) == CNA_RESULT_SUCCESS &&
            cna_song_collection_dispose(songs) == CNA_RESULT_SUCCESS &&
            cna_song_collection_get_is_disposed(songs, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_song_collection_get_count(songs, &song_count) == CNA_RESULT_INVALID_STATE,
        "disposing the library-owned song collection also refuses its count idempotently");

cleanup:
    if (songs != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_song_collection_destroy(songs), "release the song collection view");
    }
    if (albums != CNA_INVALID_HANDLE) {
        stale = albums;
        cna_demo_check_result(&demo, cna_album_collection_destroy(albums), "release the album collection view");
        album_count = 99;
        cna_demo_check(
            &demo,
            cna_album_collection_get_count(stale, &album_count) == CNA_RESULT_INVALID_HANDLE &&
                album_count == 99 &&
                cna_album_collection_destroy(stale) == CNA_RESULT_INVALID_HANDLE,
            "a released collection rejects stale reads and repeated destruction atomically");
    }
    if (artists != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_artist_collection_destroy(artists), "release the artist collection view");
    }
    if (genres != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_genre_collection_destroy(genres), "release the genre collection view");
    }
    if (playlists != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_playlist_collection_destroy(playlists),
            "release the playlist collection view");
    }
    if (library != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_media_library_destroy(library), "release the default media library");
    }
    if (from_source != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_media_library_destroy(from_source),
            "release the explicit-source media library");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    }
    return cna_demo_finish(&demo);
}
