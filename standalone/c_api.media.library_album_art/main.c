/* SPDX-License-Identifier: MS-PL */
#define _POSIX_C_SOURCE 200809L
/* Exact MediaLibrary album-art bytes, missing-art state and generated thumbnail through C. */
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

typedef struct PrivateLibrary {
    char root[256];
    char music[320];
    char pictures[320];
    char config[320];
    char settings[352];
    char covered[352];
    char bare[352];
    char covered_track[384];
    char bare_track[384];
    char cover[384];
    int ready;
} PrivateLibrary;

static CNA_StringView text_view(const char* data)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = (uint64_t)strlen(data);
    return value;
}

#if defined(__linux__)
static int join_path(char* output, size_t capacity, const char* parent, const char* child)
{
    const int count = snprintf(output, capacity, "%s/%s", parent, child);
    return count >= 0 && (size_t)count < capacity;
}

static int prepare_private_library(PrivateLibrary* fixture)
{
    char template_path[] = "/tmp/cna-demos-c-album-art-XXXXXX";
    char* root;
    FILE* file;
    memset(fixture, 0, sizeof(*fixture));
    root = mkdtemp(template_path);
    if (root == NULL || strlen(root) >= sizeof(fixture->root)) {
        return 0;
    }
    memcpy(fixture->root, root, strlen(root) + 1U);
    if (!join_path(fixture->music, sizeof(fixture->music), root, "music") ||
        !join_path(fixture->pictures, sizeof(fixture->pictures), root, "pictures") ||
        !join_path(fixture->config, sizeof(fixture->config), root, "config") ||
        !join_path(fixture->settings, sizeof(fixture->settings), fixture->config, "user-dirs.dirs") ||
        !join_path(fixture->covered, sizeof(fixture->covered), fixture->music, "covered") ||
        !join_path(fixture->bare, sizeof(fixture->bare), fixture->music, "bare") ||
        !join_path(fixture->covered_track, sizeof(fixture->covered_track), fixture->covered, "track_a.flac") ||
        !join_path(fixture->bare_track, sizeof(fixture->bare_track), fixture->bare, "track_c.flac") ||
        !join_path(fixture->cover, sizeof(fixture->cover), fixture->covered, "cover.png") ||
        mkdir(fixture->music, 0700) != 0 || mkdir(fixture->pictures, 0700) != 0 ||
        mkdir(fixture->config, 0700) != 0 || mkdir(fixture->covered, 0700) != 0 ||
        mkdir(fixture->bare, 0700) != 0) {
        return 0;
    }
    file = fopen(fixture->settings, "wb");
    if (file == NULL) {
        return 0;
    }
    if (fprintf(file, "XDG_MUSIC_DIR=\"%s\"\nXDG_PICTURES_DIR=\"%s\"\n",
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
    if (!fixture->ready) {
        return;
    }
    (void)unlink(fixture->cover);
    (void)unlink(fixture->covered_track);
    (void)unlink(fixture->bare_track);
    (void)unlink(fixture->settings);
    (void)rmdir(fixture->covered);
    (void)rmdir(fixture->bare);
    (void)rmdir(fixture->music);
    (void)rmdir(fixture->pictures);
    (void)rmdir(fixture->config);
    (void)rmdir(fixture->root);
}

static int read_title_asset(
    CNA_Handle game, const char* source, uint8_t** out_bytes, uint64_t* out_length)
{
    uint8_t* bytes;
    uint64_t required = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    CNA_Result result = cna_title_container_read_ext(
        game, text_view(source), NULL, UINT64_C(0), &required);
    if (result != CNA_RESULT_BUFFER_TOO_SMALL || required == UINT64_C(0) ||
        required > UINT64_C(16 * 1024 * 1024)) {
        return 0;
    }
    bytes = (uint8_t*)malloc((size_t)required);
    if (bytes == NULL || cna_title_container_read_ext(
            game, text_view(source), bytes, required, &copied) != CNA_RESULT_SUCCESS ||
        copied != required) {
        free(bytes);
        return 0;
    }
    *out_bytes = bytes;
    *out_length = required;
    return 1;
}

static int write_bytes(const char* destination, const uint8_t* bytes, uint64_t length)
{
    FILE* file = fopen(destination, "wb");
    int ok;
    if (file == NULL) {
        return 0;
    }
    ok = fwrite(bytes, 1U, (size_t)length, file) == (size_t)length;
    if (fclose(file) != 0) {
        ok = 0;
    }
    return ok;
}

static int copy_title_asset(CNA_Handle game, const char* source, const char* destination)
{
    uint8_t* bytes = NULL;
    uint64_t length = UINT64_C(0);
    int ok = read_title_asset(game, source, &bytes, &length) &&
        write_bytes(destination, bytes, length);
    free(bytes);
    return ok;
}
#endif

static uint32_t read_be32(const uint8_t* bytes)
{
    return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
        ((uint32_t)bytes[2] << 8) | (uint32_t)bytes[3];
}

static int png_dimensions(
    const uint8_t* bytes, uint64_t length, uint32_t* out_width, uint32_t* out_height)
{
    static const uint8_t Signature[8] = {0x89U, 'P', 'N', 'G', 0x0DU, 0x0AU, 0x1AU, 0x0AU};
    if (length < UINT64_C(24) || memcmp(bytes, Signature, sizeof(Signature)) != 0 ||
        memcmp(bytes + 12, "IHDR", 4U) != 0) {
        return 0;
    }
    *out_width = read_be32(bytes + 16);
    *out_height = read_be32(bytes + 20);
    return 1;
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
    CnaDemo demo;
    PrivateLibrary fixture;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_MediaLibraryHandle library = CNA_INVALID_HANDLE;
    CNA_AlbumCollectionHandle albums = CNA_INVALID_HANDLE;
    CNA_AlbumHandle album[2] = {CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    CNA_AlbumHandle with_art = CNA_INVALID_HANDLE;
    CNA_AlbumHandle without_art = CNA_INVALID_HANDLE;
    CNA_Bool has_art = CNA_FALSE;
    uint8_t* source_cover = NULL;
    uint8_t* art = NULL;
    uint8_t* art_short = NULL;
    uint8_t* thumbnail = NULL;
    uint8_t* thumbnail_short = NULL;
    uint64_t source_cover_bytes = UINT64_C(0);
    uint64_t art_bytes = UINT64_C(0);
    uint64_t thumbnail_bytes = UINT64_C(0);
    uint64_t required = UINT64_C(0);
    uint64_t untouched = UINT64_C(0x1122334455667788);
    uint32_t thumbnail_width = 0U;
    uint32_t thumbnail_height = 0U;
    int32_t count = 0;
    int index;
    int releases_ok = 1;

    if (!prepare_private_library(&fixture)) {
        return cna_demo_skip("could not create a private Linux/XDG media library");
    }
    cna_demo_init(&demo, "c_api.media.library_album_art", argc, argv, 1);
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
    cna_demo_check(
        &demo,
        copy_title_asset(game, "assets/media/library_music/track_a.flac", fixture.covered_track) &&
            copy_title_asset(game, "assets/media/library_music/track_c.flac", fixture.bare_track) &&
            read_title_asset(game, "assets/media/library_panorama.png", &source_cover, &source_cover_bytes) &&
            write_bytes(fixture.cover, source_cover, source_cover_bytes),
        "two tagged albums and one original cover are isolated from the host library");
    if (source_cover == NULL || !cna_demo_check_result(
            &demo, cna_media_library_create(game, &library),
            "index the private album-art fixture") ||
        !cna_demo_check_result(
            &demo, cna_media_library_get_albums(library, &albums),
            "get the two private albums")) {
        goto cleanup;
    }
    if (!cna_demo_check(
            &demo,
            cna_album_collection_get_count(albums, &count) == CNA_RESULT_SUCCESS && count == 2,
            "the two tagged folders form exactly two albums")) {
        goto cleanup;
    }
    for (index = 0; index < 2; ++index) {
        if (cna_album_collection_get_at(albums, index, &album[index]) != CNA_RESULT_SUCCESS ||
            cna_album_get_has_art(album[index], &has_art) != CNA_RESULT_SUCCESS) {
            goto cleanup;
        }
        if (has_art == CNA_TRUE) {
            with_art = album[index];
            album[index] = CNA_INVALID_HANDLE;
        } else {
            without_art = album[index];
            album[index] = CNA_INVALID_HANDLE;
        }
    }
    if (!cna_demo_check(
            &demo, with_art != CNA_INVALID_HANDLE && without_art != CNA_INVALID_HANDLE,
            "HasArt distinguishes the covered album from the bare album")) {
        goto cleanup;
    }
    required = untouched;
    cna_demo_check(
        &demo,
        cna_album_get_art_size(without_art, &required) == CNA_RESULT_INVALID_STATE &&
            required == untouched &&
            cna_album_get_thumbnail_size(without_art, &required) == CNA_RESULT_INVALID_STATE &&
            required == untouched,
        "both image-size routes refuse an album without art without changing output");

    if (!cna_demo_check(
            &demo,
            cna_album_get_art_size(with_art, &art_bytes) == CNA_RESULT_SUCCESS &&
                art_bytes == source_cover_bytes && art_bytes > UINT64_C(1),
            "the covered album reports the exact original image length")) {
        goto cleanup;
    }
    art = (uint8_t*)malloc((size_t)art_bytes);
    art_short = (uint8_t*)malloc((size_t)art_bytes);
    if (art == NULL || art_short == NULL) {
        cna_demo_check(&demo, 0, "allocate album-art transfer buffers");
        goto cleanup;
    }
    memset(art_short, 0xA5, (size_t)art_bytes);
    required = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_album_copy_art(with_art, art_short, art_bytes - UINT64_C(1), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == art_bytes && art_short[0] == 0xA5U &&
            art_short[art_bytes - UINT64_C(1)] == 0xA5U,
        "a short cover-art buffer reports its requirement without a partial write");
    cna_demo_check(
        &demo,
        cna_album_copy_art(with_art, art, art_bytes, &required) == CNA_RESULT_SUCCESS &&
            required == art_bytes && memcmp(art, source_cover, (size_t)art_bytes) == 0,
        "the album-art copy preserves every original encoded byte");

    if (!cna_demo_check(
            &demo,
            cna_album_get_thumbnail_size(with_art, &thumbnail_bytes) == CNA_RESULT_SUCCESS &&
                thumbnail_bytes > UINT64_C(1) && thumbnail_bytes <= UINT64_C(16 * 1024 * 1024),
            "the covered album reports a nonempty bounded thumbnail")) {
        goto cleanup;
    }
    thumbnail = (uint8_t*)malloc((size_t)thumbnail_bytes);
    thumbnail_short = (uint8_t*)malloc((size_t)thumbnail_bytes);
    if (thumbnail == NULL || thumbnail_short == NULL) {
        cna_demo_check(&demo, 0, "allocate album-thumbnail transfer buffers");
        goto cleanup;
    }
    memset(thumbnail_short, 0xA5, (size_t)thumbnail_bytes);
    required = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_album_copy_thumbnail(
            with_art, thumbnail_short, thumbnail_bytes - UINT64_C(1), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == thumbnail_bytes && thumbnail_short[0] == 0xA5U &&
            thumbnail_short[thumbnail_bytes - UINT64_C(1)] == 0xA5U,
        "a short thumbnail buffer reports its requirement without a partial write");
    cna_demo_check(
        &demo,
        cna_album_copy_thumbnail(with_art, thumbnail, thumbnail_bytes, &required) ==
                CNA_RESULT_SUCCESS &&
            required == thumbnail_bytes &&
            png_dimensions(thumbnail, thumbnail_bytes, &thumbnail_width, &thumbnail_height) &&
            thumbnail_width > 0U && thumbnail_width < 640U &&
            thumbnail_width == 2U * thumbnail_height &&
            (thumbnail_bytes != source_cover_bytes ||
             memcmp(thumbnail, source_cover, (size_t)thumbnail_bytes) != 0),
        "the copied thumbnail is a distinct smaller PNG with the original 2:1 aspect ratio");
    printf("  album art: %llu bytes; thumbnail: %ux%u, %llu bytes\n",
           (unsigned long long)art_bytes, (unsigned)thumbnail_width,
           (unsigned)thumbnail_height, (unsigned long long)thumbnail_bytes);

cleanup:
    free(thumbnail_short);
    free(thumbnail);
    free(art_short);
    free(art);
    free(source_cover);
    releases_ok &= release_handle(&with_art, cna_album_destroy);
    releases_ok &= release_handle(&without_art, cna_album_destroy);
    for (index = 0; index < 2; ++index) {
        releases_ok &= release_handle(&album[index], cna_album_destroy);
    }
    releases_ok &= release_handle(&albums, cna_album_collection_destroy);
    releases_ok &= release_handle(&library, cna_media_library_destroy);
    cna_demo_check(&demo, releases_ok, "release every album-art handle exactly once");
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the fixture-owning game");
    }
    cleanup_private_library(&fixture);
    return cna_demo_finish(&demo);
#endif
}
