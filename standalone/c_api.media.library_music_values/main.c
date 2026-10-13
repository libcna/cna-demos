/* SPDX-License-Identifier: MS-PL */
#define _POSIX_C_SOURCE 200809L
/* Tagged MediaLibrary album, artist, genre, playlist and Song relationships through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "CnaDemos/CApiDemo.h"

typedef CNA_Result (*NameSizeRoute)(CNA_Handle, uint64_t*);
typedef CNA_Result (*NameCopyRoute)(CNA_Handle, char*, uint64_t, uint64_t*);

typedef struct PrivateLibrary {
    char root[256];
    char music[288];
    char pictures[288];
    char config[288];
    char settings[320];
    char tracks[3][320];
    char playlist[320];
    int ready;
} PrivateLibrary;

static CNA_StringView text_view(const char* data, uint64_t byte_length)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = byte_length;
    return value;
}

#if defined(__linux__)
static int prepare_private_library(PrivateLibrary* fixture)
{
    char template_path[] = "/tmp/cna-demos-c-media-values-XXXXXX";
    FILE* file;
    char* root;
    memset(fixture, 0, sizeof(*fixture));
    root = mkdtemp(template_path);
    if (root == NULL || strlen(root) >= sizeof(fixture->root)) {
        return 0;
    }
    memcpy(fixture->root, root, strlen(root) + 1U);
    if (snprintf(fixture->music, sizeof(fixture->music), "%s/music", root) < 0 ||
        snprintf(fixture->pictures, sizeof(fixture->pictures), "%s/pictures", root) < 0 ||
        snprintf(fixture->config, sizeof(fixture->config), "%s/config", root) < 0 ||
        snprintf(fixture->settings, sizeof(fixture->settings), "%s/user-dirs.dirs", fixture->config) < 0 ||
        snprintf(fixture->tracks[0], sizeof(fixture->tracks[0]), "%s/track_a.flac", fixture->music) < 0 ||
        snprintf(fixture->tracks[1], sizeof(fixture->tracks[1]), "%s/track_b.flac", fixture->music) < 0 ||
        snprintf(fixture->tracks[2], sizeof(fixture->tracks[2]), "%s/track_c.flac", fixture->music) < 0 ||
        snprintf(fixture->playlist, sizeof(fixture->playlist), "%s/Selected.m3u", fixture->music) < 0 ||
        mkdir(fixture->music, 0700) != 0 || mkdir(fixture->pictures, 0700) != 0 ||
        mkdir(fixture->config, 0700) != 0) {
        return 0;
    }
    file = fopen(fixture->settings, "wb");
    if (file == NULL) {
        return 0;
    }
    if (fprintf(
            file, "XDG_MUSIC_DIR=\"%s\"\nXDG_PICTURES_DIR=\"%s\"\n",
            fixture->music, fixture->pictures) < 0) {
        (void)fclose(file);
        return 0;
    }
    if (fclose(file) != 0 || setenv("XDG_CONFIG_HOME", fixture->config, 1) != 0) {
        return 0;
    }
    fixture->ready = 1;
    return 1;
}

static void cleanup_private_library(PrivateLibrary* fixture)
{
    int index;
    if (!fixture->ready) {
        return;
    }
    for (index = 0; index < 3; ++index) {
        (void)unlink(fixture->tracks[index]);
    }
    (void)unlink(fixture->playlist);
    (void)unlink(fixture->settings);
    (void)rmdir(fixture->music);
    (void)rmdir(fixture->pictures);
    (void)rmdir(fixture->config);
    (void)rmdir(fixture->root);
}

static int copy_title_asset(
    CNA_Handle game, const char* source, const char* destination)
{
    uint8_t* bytes = NULL;
    uint64_t length = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    FILE* file;
    CNA_Result result = cna_title_container_read_ext(
        game, text_view(source, (uint64_t)strlen(source)), NULL, UINT64_C(0), &length);
    if (result != CNA_RESULT_BUFFER_TOO_SMALL || length == UINT64_C(0) ||
        length > UINT64_C(1024 * 1024)) {
        return 0;
    }
    bytes = (uint8_t*)malloc((size_t)length);
    if (bytes == NULL ||
        cna_title_container_read_ext(
            game, text_view(source, (uint64_t)strlen(source)), bytes, length, &copied) !=
            CNA_RESULT_SUCCESS ||
        copied != length) {
        free(bytes);
        return 0;
    }
    file = fopen(destination, "wb");
    if (file == NULL) {
        free(bytes);
        return 0;
    }
    if (fwrite(bytes, 1U, (size_t)length, file) != (size_t)length) {
        (void)fclose(file);
        free(bytes);
        return 0;
    }
    if (fclose(file) != 0) {
        free(bytes);
        return 0;
    }
    free(bytes);
    return 1;
}
#endif

static int name_is(
    CNA_Handle handle, NameSizeRoute size_route, NameCopyRoute copy_route,
    const char* expected)
{
    char text[64];
    char short_text[64];
    char short_before[64];
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(99);
    const uint64_t expected_bytes = (uint64_t)strlen(expected);
    memset(text, 0, sizeof(text));
    memset(short_text, 0xA5, sizeof(short_text));
    memset(short_before, 0xA5, sizeof(short_before));
    return size_route(handle, &bytes) == CNA_RESULT_SUCCESS && bytes == expected_bytes &&
        copy_route(handle, text, sizeof(text), &copied) == CNA_RESULT_SUCCESS &&
        copied == bytes && memcmp(text, expected, (size_t)bytes) == 0 &&
        copy_route(handle, short_text, bytes - UINT64_C(1), &copied) ==
            CNA_RESULT_BUFFER_TOO_SMALL &&
        copied == bytes && memcmp(short_text, short_before, sizeof(short_text)) == 0;
}

static int release_handle(CNA_Handle* handle, CNA_Result (*destroy)(CNA_Handle))
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
#if !defined(__linux__)
    (void)argc;
    (void)argv;
    return cna_demo_skip("requires Linux XDG user-folder resolution");
#else
    static const char* const Sources[3] = {
        "assets/media/library_music/track_a.flac",
        "assets/media/library_music/track_b.flac",
        "assets/media/library_music/track_c.flac"};
    CnaDemo demo;
    PrivateLibrary fixture;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_MediaLibraryHandle library = CNA_INVALID_HANDLE;
    CNA_SongCollectionHandle songs = CNA_INVALID_HANDLE;
    CNA_AlbumCollectionHandle albums = CNA_INVALID_HANDLE;
    CNA_ArtistCollectionHandle artists = CNA_INVALID_HANDLE;
    CNA_GenreCollectionHandle genres = CNA_INVALID_HANDLE;
    CNA_PlaylistCollectionHandle playlists = CNA_INVALID_HANDLE;
    CNA_SongHandle song[3] = {CNA_INVALID_HANDLE, CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    CNA_AlbumHandle album[3] = {CNA_INVALID_HANDLE, CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    CNA_ArtistHandle artist[2] = {CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    CNA_GenreHandle genre[2] = {CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    CNA_PlaylistHandle playlist[2] = {CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    CNA_ArtistHandle album_artist = CNA_INVALID_HANDLE;
    CNA_GenreHandle album_genre = CNA_INVALID_HANDLE;
    CNA_AlbumHandle song_album = CNA_INVALID_HANDLE;
    CNA_ArtistHandle song_artist = CNA_INVALID_HANDLE;
    CNA_GenreHandle song_genre = CNA_INVALID_HANDLE;
    CNA_SongCollectionHandle album_songs = CNA_INVALID_HANDLE;
    CNA_SongCollectionHandle artist_songs = CNA_INVALID_HANDLE;
    CNA_AlbumCollectionHandle artist_albums = CNA_INVALID_HANDLE;
    CNA_SongCollectionHandle genre_songs = CNA_INVALID_HANDLE;
    CNA_AlbumCollectionHandle genre_albums = CNA_INVALID_HANDLE;
    CNA_SongCollectionHandle playlist_songs = CNA_INVALID_HANDLE;
    CNA_SongHandle selected[2] = {CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    CNA_Bool predicate = CNA_FALSE;
    CNA_Bool available = CNA_FALSE;
    CNA_Bool disposed = CNA_FALSE;
    CNA_Bool has_art = CNA_TRUE;
    int32_t count = 0;
    int32_t hash_a = 0;
    int32_t hash_b = 1;
    int32_t track = 0;
    int64_t song_ticks[3] = {INT64_C(0), INT64_C(0), INT64_C(0)};
    int64_t album_ticks[2] = {INT64_C(0), INT64_C(0)};
    int64_t playlist_ticks = INT64_C(0);
    FILE* file;
    int index;
    int releases_ok = 1;

    if (!prepare_private_library(&fixture)) {
        return cna_demo_skip("could not create a private Linux/XDG media library");
    }
    cna_demo_init(&demo, "c_api.media.library_music_values", argc, argv, 1);
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game),
            "create the fixture-owning game")) {
        goto cleanup;
    }
    for (index = 0; index < 3; ++index) {
        if (!copy_title_asset(game, Sources[index], fixture.tracks[index])) {
            cna_demo_check(&demo, 0, "copy each tagged FLAC into the private Music root");
            goto cleanup;
        }
    }
    file = fopen(fixture.playlist, "wb");
    if (file == NULL) {
        cna_demo_check(&demo, 0, "write the private Selected playlist");
        goto cleanup;
    }
    {
        static const char PlaylistText[] = "#EXTM3U\ntrack_b.flac\ntrack_a.flac\n";
        if (fwrite(PlaylistText, 1U, sizeof(PlaylistText) - 1U, file) !=
                sizeof(PlaylistText) - 1U) {
            (void)fclose(file);
            cna_demo_check(&demo, 0, "write the private Selected playlist");
            goto cleanup;
        }
        if (fclose(file) != 0) {
            cna_demo_check(&demo, 0, "write the private Selected playlist");
            goto cleanup;
        }
    }
    cna_demo_check(
        &demo, 1,
        "three tagged FLAC fixtures and one M3U are isolated from the host library");
    if (!cna_demo_check_result(
            &demo, cna_media_library_create(game, &library),
            "index the private tagged music library") ||
        !cna_demo_check_result(&demo, cna_media_library_get_songs(library, &songs), "get all indexed songs") ||
        !cna_demo_check_result(&demo, cna_media_library_get_albums(library, &albums), "get all indexed albums") ||
        !cna_demo_check_result(&demo, cna_media_library_get_artists(library, &artists), "get all indexed artists") ||
        !cna_demo_check_result(&demo, cna_media_library_get_genres(library, &genres), "get all indexed genres") ||
        !cna_demo_check_result(&demo, cna_media_library_get_playlists(library, &playlists), "get all indexed playlists")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_song_collection_get_count(songs, &count) == CNA_RESULT_SUCCESS && count == 3 &&
            cna_album_collection_get_count(albums, &count) == CNA_RESULT_SUCCESS && count == 2 &&
            cna_artist_collection_get_count(artists, &count) == CNA_RESULT_SUCCESS && count == 2 &&
            cna_genre_collection_get_count(genres, &count) == CNA_RESULT_SUCCESS && count == 1 &&
            cna_playlist_collection_get_count(playlists, &count) == CNA_RESULT_SUCCESS && count == 1,
        "tags form three songs, two albums, two artists, one genre and one playlist");
    for (index = 0; index < 3; ++index) {
        if (cna_song_collection_get_at(songs, index, &song[index]) != CNA_RESULT_SUCCESS) goto cleanup;
    }
    for (index = 0; index < 2; ++index) {
        if (cna_album_collection_get_at(albums, index, &album[index]) != CNA_RESULT_SUCCESS ||
            cna_artist_collection_get_at(artists, index, &artist[index]) != CNA_RESULT_SUCCESS) goto cleanup;
    }
    if (cna_album_collection_get_at(albums, 0, &album[2]) != CNA_RESULT_SUCCESS ||
        cna_genre_collection_get_at(genres, 0, &genre[0]) != CNA_RESULT_SUCCESS ||
        cna_genre_collection_get_at(genres, 0, &genre[1]) != CNA_RESULT_SUCCESS ||
        cna_playlist_collection_get_at(playlists, 0, &playlist[0]) != CNA_RESULT_SUCCESS ||
        cna_playlist_collection_get_at(playlists, 0, &playlist[1]) != CNA_RESULT_SUCCESS) goto cleanup;
    cna_demo_check(
        &demo,
        name_is(song[0], cna_song_get_name_size, cna_song_copy_name, "Dawn") &&
            name_is(song[1], cna_song_get_name_size, cna_song_copy_name, "Dusk") &&
            name_is(song[2], cna_song_get_name_size, cna_song_copy_name, "Echo") &&
            cna_song_get_track_number(song[0], &track) == CNA_RESULT_SUCCESS && track == 1 &&
            cna_song_get_track_number(song[1], &track) == CNA_RESULT_SUCCESS && track == 2 &&
            cna_song_get_track_number(song[2], &track) == CNA_RESULT_SUCCESS && track == 1,
        "song names and track numbers come from the authored tags");
    cna_demo_check(
        &demo,
        name_is(album[0], cna_album_get_name_size, cna_album_copy_name, "Field Notes") &&
            name_is(album[1], cna_album_get_name_size, cna_album_copy_name, "Field Notes") &&
            name_is(artist[0], cna_artist_get_name_size, cna_artist_copy_name, "Demo Ensemble") &&
            name_is(artist[1], cna_artist_get_name_size, cna_artist_copy_name, "Second Ensemble") &&
            name_is(genre[0], cna_genre_get_name_size, cna_genre_copy_name, "Instrumental") &&
            name_is(playlist[0], cna_playlist_get_name_size, cna_playlist_copy_name, "Selected"),
        "album, artist, genre and playlist strings are exact and copy atomically");
    cna_demo_check(
        &demo,
        cna_album_equals(album[0], album[2], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_album_get_hash_code(album[0], &hash_a) == CNA_RESULT_SUCCESS &&
            cna_album_get_hash_code(album[2], &hash_b) == CNA_RESULT_SUCCESS && hash_a == hash_b &&
            cna_album_equals(album[0], album[1], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_FALSE,
        "equal album handles share a hash while equal titles from different artists stay distinct");
    cna_demo_check(
        &demo,
        cna_artist_equals(artist[0], artist[0], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_artist_equals(artist[0], artist[1], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_FALSE &&
            cna_artist_get_hash_code(artist[0], &hash_a) == CNA_RESULT_SUCCESS &&
            cna_artist_get_hash_code(artist[0], &hash_b) == CNA_RESULT_SUCCESS && hash_a == hash_b,
        "artist equality and hashes distinguish the two authored performers");
    cna_demo_check(
        &demo,
        cna_genre_equals(genre[0], genre[1], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_genre_get_hash_code(genre[0], &hash_a) == CNA_RESULT_SUCCESS &&
            cna_genre_get_hash_code(genre[1], &hash_b) == CNA_RESULT_SUCCESS && hash_a == hash_b &&
            cna_playlist_equals(playlist[0], playlist[1], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_playlist_get_hash_code(playlist[0], &hash_a) == CNA_RESULT_SUCCESS &&
            cna_playlist_get_hash_code(playlist[1], &hash_b) == CNA_RESULT_SUCCESS && hash_a == hash_b,
        "genre and playlist aliases retain value equality and equal hashes");

    cna_demo_check_result(&demo, cna_song_collection_destroy(songs), "release the top-level song collection");
    songs = CNA_INVALID_HANDLE;
    cna_demo_check_result(&demo, cna_album_collection_destroy(albums), "release the top-level album collection");
    albums = CNA_INVALID_HANDLE;
    cna_demo_check_result(&demo, cna_artist_collection_destroy(artists), "release the top-level artist collection");
    artists = CNA_INVALID_HANDLE;
    cna_demo_check_result(&demo, cna_genre_collection_destroy(genres), "release the top-level genre collection");
    genres = CNA_INVALID_HANDLE;
    cna_demo_check_result(&demo, cna_playlist_collection_destroy(playlists), "release the top-level playlist collection");
    playlists = CNA_INVALID_HANDLE;
    cna_demo_check_result(&demo, cna_media_library_destroy(library), "release the library while entity handles retain it");
    library = CNA_INVALID_HANDLE;

    cna_demo_check(
        &demo,
        cna_album_get_artist(album[0], &album_artist, &available) == CNA_RESULT_SUCCESS &&
            available == CNA_TRUE && cna_artist_equals(album_artist, artist[0], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_album_get_genre(album[0], &album_genre, &available) == CNA_RESULT_SUCCESS &&
            available == CNA_TRUE && cna_genre_equals(album_genre, genre[0], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE,
        "an album retained past the library exposes its matching artist and shared genre");
    cna_demo_check(
        &demo,
        cna_song_get_album(song[0], &song_album, &available) == CNA_RESULT_SUCCESS &&
            available == CNA_TRUE && cna_album_equals(song_album, album[0], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_song_get_artist(song[0], &song_artist, &available) == CNA_RESULT_SUCCESS &&
            available == CNA_TRUE && cna_artist_equals(song_artist, artist[0], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_song_get_genre(song[0], &song_genre, &available) == CNA_RESULT_SUCCESS &&
            available == CNA_TRUE && cna_genre_equals(song_genre, genre[0], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE,
        "a retained Song's album, artist and genre agree with every indexed group");
    cna_demo_check(
        &demo,
        cna_album_get_songs(album[0], &album_songs) == CNA_RESULT_SUCCESS &&
            cna_song_collection_get_count(album_songs, &count) == CNA_RESULT_SUCCESS && count == 2 &&
            cna_artist_get_songs(artist[0], &artist_songs) == CNA_RESULT_SUCCESS &&
            cna_song_collection_get_count(artist_songs, &count) == CNA_RESULT_SUCCESS && count == 2 &&
            cna_artist_get_albums(artist[0], &artist_albums) == CNA_RESULT_SUCCESS &&
            cna_album_collection_get_count(artist_albums, &count) == CNA_RESULT_SUCCESS && count == 1,
        "album and artist views retain exactly the first performer's two songs and one album");
    cna_demo_check(
        &demo,
        cna_genre_get_songs(genre[0], &genre_songs) == CNA_RESULT_SUCCESS &&
            cna_song_collection_get_count(genre_songs, &count) == CNA_RESULT_SUCCESS && count == 3 &&
            cna_genre_get_albums(genre[0], &genre_albums) == CNA_RESULT_SUCCESS &&
            cna_album_collection_get_count(genre_albums, &count) == CNA_RESULT_SUCCESS && count == 2,
        "the shared Instrumental genre retains all three songs and both artist-specific albums");
    cna_demo_check(
        &demo,
        cna_playlist_get_songs(playlist[0], &playlist_songs) == CNA_RESULT_SUCCESS &&
            cna_song_collection_get_count(playlist_songs, &count) == CNA_RESULT_SUCCESS && count == 2 &&
            cna_song_collection_get_at(playlist_songs, 0, &selected[0]) == CNA_RESULT_SUCCESS &&
            cna_song_collection_get_at(playlist_songs, 1, &selected[1]) == CNA_RESULT_SUCCESS &&
            name_is(selected[0], cna_song_get_name_size, cna_song_copy_name, "Dusk") &&
            name_is(selected[1], cna_song_get_name_size, cna_song_copy_name, "Dawn"),
        "the M3U preserves its reversed two-song order without duplicating Song values");
    cna_demo_check(
        &demo,
        cna_song_get_duration(song[0], &song_ticks[0]) == CNA_RESULT_SUCCESS &&
            cna_song_get_duration(song[1], &song_ticks[1]) == CNA_RESULT_SUCCESS &&
            cna_song_get_duration(song[2], &song_ticks[2]) == CNA_RESULT_SUCCESS &&
            cna_album_get_duration(album[0], &album_ticks[0]) == CNA_RESULT_SUCCESS &&
            cna_album_get_duration(album[1], &album_ticks[1]) == CNA_RESULT_SUCCESS &&
            cna_playlist_get_duration(playlist[0], &playlist_ticks) == CNA_RESULT_SUCCESS &&
            album_ticks[0] == song_ticks[0] + song_ticks[1] &&
            album_ticks[1] == song_ticks[2] &&
            playlist_ticks == song_ticks[1] + song_ticks[0],
        "album and playlist durations exactly aggregate only their member songs");
    cna_demo_check(
        &demo,
        cna_album_get_has_art(album[0], &has_art) == CNA_RESULT_SUCCESS && has_art == CNA_FALSE &&
            cna_album_get_has_art(album[1], &has_art) == CNA_RESULT_SUCCESS && has_art == CNA_FALSE,
        "both albums honestly report no art in the art-free music fixture");

    cna_demo_check(
        &demo,
        cna_album_get_is_disposed(album[0], &disposed) == CNA_RESULT_SUCCESS && disposed == CNA_FALSE &&
            cna_album_dispose(album[2]) == CNA_RESULT_SUCCESS &&
            cna_album_dispose(album[2]) == CNA_RESULT_SUCCESS &&
            cna_album_get_is_disposed(album[0], &disposed) == CNA_RESULT_SUCCESS && disposed == CNA_TRUE,
        "album disposal is idempotent and shared by aliases");
    cna_demo_check(
        &demo,
        cna_artist_get_is_disposed(artist[0], &disposed) == CNA_RESULT_SUCCESS && disposed == CNA_FALSE &&
            cna_artist_dispose(album_artist) == CNA_RESULT_SUCCESS &&
            cna_artist_dispose(album_artist) == CNA_RESULT_SUCCESS &&
            cna_artist_get_is_disposed(artist[0], &disposed) == CNA_RESULT_SUCCESS && disposed == CNA_TRUE,
        "artist disposal is idempotent and shared by relationship handles");
    cna_demo_check(
        &demo,
        cna_genre_get_is_disposed(genre[0], &disposed) == CNA_RESULT_SUCCESS && disposed == CNA_FALSE &&
            cna_genre_dispose(album_genre) == CNA_RESULT_SUCCESS &&
            cna_genre_dispose(album_genre) == CNA_RESULT_SUCCESS &&
            cna_genre_get_is_disposed(genre[0], &disposed) == CNA_RESULT_SUCCESS && disposed == CNA_TRUE,
        "genre disposal is idempotent and shared by relationship handles");
    cna_demo_check(
        &demo,
        cna_playlist_get_is_disposed(playlist[0], &disposed) == CNA_RESULT_SUCCESS && disposed == CNA_FALSE &&
            cna_playlist_dispose(playlist[1]) == CNA_RESULT_SUCCESS &&
            cna_playlist_dispose(playlist[1]) == CNA_RESULT_SUCCESS &&
            cna_playlist_get_is_disposed(playlist[0], &disposed) == CNA_RESULT_SUCCESS && disposed == CNA_TRUE,
        "playlist disposal is idempotent and shared by aliases");

cleanup:
    for (index = 0; index < 2; ++index) {
        releases_ok &= release_handle(&selected[index], cna_song_destroy);
    }
    releases_ok &= release_handle(&playlist_songs, cna_song_collection_destroy);
    releases_ok &= release_handle(&genre_albums, cna_album_collection_destroy);
    releases_ok &= release_handle(&genre_songs, cna_song_collection_destroy);
    releases_ok &= release_handle(&artist_albums, cna_album_collection_destroy);
    releases_ok &= release_handle(&artist_songs, cna_song_collection_destroy);
    releases_ok &= release_handle(&album_songs, cna_song_collection_destroy);
    releases_ok &= release_handle(&song_genre, cna_genre_destroy);
    releases_ok &= release_handle(&song_artist, cna_artist_destroy);
    releases_ok &= release_handle(&song_album, cna_album_destroy);
    releases_ok &= release_handle(&album_genre, cna_genre_destroy);
    releases_ok &= release_handle(&album_artist, cna_artist_destroy);
    for (index = 0; index < 2; ++index) {
        releases_ok &= release_handle(&playlist[index], cna_playlist_destroy);
        releases_ok &= release_handle(&genre[index], cna_genre_destroy);
        releases_ok &= release_handle(&artist[index], cna_artist_destroy);
    }
    for (index = 0; index < 3; ++index) {
        releases_ok &= release_handle(&album[index], cna_album_destroy);
        releases_ok &= release_handle(&song[index], cna_song_destroy);
    }
    releases_ok &= release_handle(&playlists, cna_playlist_collection_destroy);
    releases_ok &= release_handle(&genres, cna_genre_collection_destroy);
    releases_ok &= release_handle(&artists, cna_artist_collection_destroy);
    releases_ok &= release_handle(&albums, cna_album_collection_destroy);
    releases_ok &= release_handle(&songs, cna_song_collection_destroy);
    releases_ok &= release_handle(&library, cna_media_library_destroy);
    cna_demo_check(
        &demo, releases_ok,
        "release every retained entity and relationship handle exactly once");
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the fixture-owning game");
    }
    cleanup_private_library(&fixture);
    return cna_demo_finish(&demo);
#endif
}
