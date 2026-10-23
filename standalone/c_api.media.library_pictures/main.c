/* SPDX-License-Identifier: MS-PL */
#define _POSIX_C_SOURCE 200809L
/* Picture values, album hierarchy, token lookup, collection lifetime and both save routes in C. */
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
    char music[320];
    char pictures[320];
    char config[320];
    char data[320];
    char settings[352];
    char family[352];
    char vacation[352];
    char day_two[384];
    char portrait[384];
    char checker[416];
    char saved[352];
    char saved_preview[384];
    char saved_stream[384];
    char storage_app[384];
    char storage_container[416];
    char storage_player[448];
    char storage_file[480];
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
    char template_path[] = "/tmp/cna-demos-c-pictures-XXXXXX";
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
        !join_path(fixture->data, sizeof(fixture->data), root, "data") ||
        !join_path(fixture->settings, sizeof(fixture->settings), fixture->config, "user-dirs.dirs") ||
        !join_path(fixture->family, sizeof(fixture->family), fixture->pictures, "Family") ||
        !join_path(fixture->vacation, sizeof(fixture->vacation), fixture->pictures, "Vacation") ||
        !join_path(fixture->day_two, sizeof(fixture->day_two), fixture->vacation, "Day 2") ||
        !join_path(fixture->portrait, sizeof(fixture->portrait), fixture->family, "portrait.png") ||
        !join_path(fixture->checker, sizeof(fixture->checker), fixture->day_two, "checker.png") ||
        !join_path(fixture->saved, sizeof(fixture->saved), fixture->pictures, "Saved Pictures") ||
        !join_path(fixture->saved_preview, sizeof(fixture->saved_preview), fixture->saved, "Preview.png") ||
        !join_path(fixture->saved_stream, sizeof(fixture->saved_stream), fixture->saved, "Stream Import.png") ||
        !join_path(fixture->storage_app, sizeof(fixture->storage_app), fixture->data,
                   "cna-demos-c-media-pictures") ||
        !join_path(fixture->storage_container, sizeof(fixture->storage_container), fixture->storage_app,
                   "Picture Imports") ||
        !join_path(fixture->storage_player, sizeof(fixture->storage_player), fixture->storage_container,
                   "AllPlayers") ||
        !join_path(fixture->storage_file, sizeof(fixture->storage_file), fixture->storage_player,
                   "source.png") ||
        mkdir(fixture->music, 0700) != 0 || mkdir(fixture->pictures, 0700) != 0 ||
        mkdir(fixture->config, 0700) != 0 || mkdir(fixture->data, 0700) != 0 ||
        mkdir(fixture->family, 0700) != 0 || mkdir(fixture->vacation, 0700) != 0 ||
        mkdir(fixture->day_two, 0700) != 0) {
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
    if (fclose(file) != 0 || setenv("XDG_CONFIG_HOME", fixture->config, 1) != 0 ||
        setenv("XDG_DATA_HOME", fixture->data, 1) != 0) {
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
    (void)unlink(fixture->saved_preview);
    (void)unlink(fixture->saved_stream);
    (void)unlink(fixture->portrait);
    (void)unlink(fixture->checker);
    (void)unlink(fixture->settings);
    (void)unlink(fixture->storage_file);
    (void)rmdir(fixture->saved);
    (void)rmdir(fixture->day_two);
    (void)rmdir(fixture->vacation);
    (void)rmdir(fixture->family);
    (void)rmdir(fixture->pictures);
    (void)rmdir(fixture->music);
    (void)rmdir(fixture->config);
    (void)rmdir(fixture->storage_player);
    (void)rmdir(fixture->storage_container);
    (void)rmdir(fixture->storage_app);
    (void)rmdir(fixture->data);
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
#endif

static int name_is(
    CNA_Handle handle, NameSizeRoute size_route, NameCopyRoute copy_route,
    const char* expected)
{
    char text[96];
    char short_text[96];
    char short_before[96];
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(99);
    const uint64_t expected_bytes = (uint64_t)strlen(expected);
    memset(text, 0, sizeof(text));
    memset(short_text, 0xA5, sizeof(short_text));
    memset(short_before, 0xA5, sizeof(short_before));
    return expected_bytes > UINT64_C(0) && expected_bytes < sizeof(text) &&
        size_route(handle, &bytes) == CNA_RESULT_SUCCESS && bytes == expected_bytes &&
        copy_route(handle, text, sizeof(text), &copied) == CNA_RESULT_SUCCESS &&
        copied == bytes && memcmp(text, expected, (size_t)bytes) == 0 &&
        copy_route(handle, short_text, bytes - UINT64_C(1), &copied) ==
            CNA_RESULT_BUFFER_TOO_SMALL &&
        copied == bytes && memcmp(short_text, short_before, sizeof(short_text)) == 0;
}

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
    CNA_StorageDeviceHandle device = CNA_INVALID_HANDLE;
    CNA_StorageContainerHandle container = CNA_INVALID_HANDLE;
    CNA_StorageStreamHandle stream = CNA_INVALID_HANDLE;
    CNA_PictureCollectionHandle all_pictures = CNA_INVALID_HANDLE;
    CNA_PictureCollectionHandle saved_pictures = CNA_INVALID_HANDLE;
    CNA_PictureCollectionHandle root_pictures = CNA_INVALID_HANDLE;
    CNA_PictureCollectionHandle family_pictures = CNA_INVALID_HANDLE;
    CNA_PictureCollectionHandle day_two_pictures = CNA_INVALID_HANDLE;
    CNA_PictureCollectionHandle saved_album_pictures = CNA_INVALID_HANDLE;
    CNA_PictureAlbumCollectionHandle root_children = CNA_INVALID_HANDLE;
    CNA_PictureAlbumCollectionHandle vacation_children = CNA_INVALID_HANDLE;
    CNA_PictureAlbumCollectionHandle saved_children = CNA_INVALID_HANDLE;
    CNA_PictureHandle picture[2] = {CNA_INVALID_HANDLE, CNA_INVALID_HANDLE};
    CNA_PictureHandle portrait_alias = CNA_INVALID_HANDLE;
    CNA_PictureHandle checker_alias = CNA_INVALID_HANDLE;
    CNA_PictureHandle token_alias = CNA_INVALID_HANDLE;
    CNA_PictureHandle preview = CNA_INVALID_HANDLE;
    CNA_PictureHandle stream_import = CNA_INVALID_HANDLE;
    CNA_PictureHandle rejected_picture = CNA_INVALID_HANDLE;
    CNA_PictureAlbumHandle root_album = CNA_INVALID_HANDLE;
    CNA_PictureAlbumHandle family_album = CNA_INVALID_HANDLE;
    CNA_PictureAlbumHandle vacation_album = CNA_INVALID_HANDLE;
    CNA_PictureAlbumHandle day_two_album = CNA_INVALID_HANDLE;
    CNA_PictureAlbumHandle portrait_album = CNA_INVALID_HANDLE;
    CNA_PictureAlbumHandle family_parent = CNA_INVALID_HANDLE;
    CNA_PictureAlbumHandle day_two_parent = CNA_INVALID_HANDLE;
    CNA_PictureAlbumHandle saved_album = CNA_INVALID_HANDLE;
    CNA_PictureAlbumHandle saved_parent = CNA_INVALID_HANDLE;
    uint8_t* panorama = NULL;
    uint8_t* checker = NULL;
    uint8_t* copied_image = NULL;
    uint8_t* short_image = NULL;
    uint8_t* thumbnail = NULL;
    uint8_t* short_thumbnail = NULL;
    uint64_t panorama_bytes = UINT64_C(0);
    uint64_t checker_bytes = UINT64_C(0);
    uint64_t image_bytes = UINT64_C(0);
    uint64_t thumbnail_bytes = UINT64_C(0);
    uint64_t required = UINT64_C(0);
    uint64_t token_bytes = UINT64_C(0);
    char token[512];
    char short_token[512];
    char short_token_before[512];
    CNA_Bool available = CNA_FALSE;
    CNA_Bool predicate = CNA_FALSE;
    CNA_Bool disposed = CNA_FALSE;
    int32_t count = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t hash_a = 0;
    int32_t hash_b = 1;
    int64_t date_ticks = INT64_C(0);
    int64_t stream_position = INT64_C(-1);
    uint32_t thumbnail_width = 0U;
    uint32_t thumbnail_height = 0U;
    int index;
    int releases_ok = 1;

    if (!prepare_private_library(&fixture)) {
        return cna_demo_skip("could not create private Linux/XDG media and storage roots");
    }
    cna_demo_init(&demo, "c_api.media.library_pictures", argc, argv, 1);
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
        read_title_asset(game, "assets/media/library_panorama.png", &panorama, &panorama_bytes) &&
            read_title_asset(game, "assets/textures/checker.png", &checker, &checker_bytes) &&
            write_bytes(fixture.portrait, panorama, panorama_bytes) &&
            write_bytes(fixture.checker, checker, checker_bytes),
        "two original PNGs populate a nested private picture hierarchy");
    if (panorama == NULL || checker == NULL ||
        !cna_demo_check_result(
            &demo, cna_media_library_create(game, &library),
            "index the private picture hierarchy") ||
        !cna_demo_check_result(
            &demo, cna_media_library_get_pictures(library, &all_pictures),
            "get the complete picture collection") ||
        !cna_demo_check_result(
            &demo, cna_media_library_get_saved_pictures(library, &saved_pictures),
            "get the initially empty saved-picture collection") ||
        !cna_demo_check_result(
            &demo, cna_media_library_get_root_picture_album(
                library, &root_album, &available),
            "get the picture-album root")) {
        goto cleanup;
    }
    if (!cna_demo_check(
            &demo,
            available == CNA_TRUE &&
                cna_picture_collection_get_count(all_pictures, &count) == CNA_RESULT_SUCCESS &&
                count == 2 &&
                cna_picture_collection_get_count(saved_pictures, &count) == CNA_RESULT_SUCCESS &&
                count == 0,
            "the library starts with two indexed pictures and no application-saved pictures")) {
        goto cleanup;
    }

    available = CNA_TRUE;
    family_parent = UINT64_C(999999);
    cna_demo_check(
        &demo,
        name_is(root_album, cna_picture_album_get_name_size, cna_picture_album_copy_name,
                "pictures") &&
            cna_picture_album_get_parent(root_album, &family_parent, &available) ==
                CNA_RESULT_SUCCESS &&
            available == CNA_FALSE && family_parent == CNA_INVALID_HANDLE,
        "the named picture root terminates parent traversal with an unavailable handle");
    if (!cna_demo_check_result(
            &demo, cna_picture_album_get_albums(root_album, &root_children),
            "get the root's child albums") ||
        !cna_demo_check_result(
            &demo, cna_picture_album_get_pictures(root_album, &root_pictures),
            "get the root's direct pictures")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_picture_album_collection_get_count(root_children, &count) == CNA_RESULT_SUCCESS &&
            count == 2 &&
            cna_picture_collection_get_count(root_pictures, &count) == CNA_RESULT_SUCCESS &&
            count == 0 &&
            cna_picture_album_collection_get_at(root_children, 0, &family_album) ==
                CNA_RESULT_SUCCESS &&
            cna_picture_album_collection_get_at(root_children, 1, &vacation_album) ==
                CNA_RESULT_SUCCESS &&
            name_is(family_album, cna_picture_album_get_name_size,
                    cna_picture_album_copy_name, "Family") &&
            name_is(vacation_album, cna_picture_album_get_name_size,
                    cna_picture_album_copy_name, "Vacation"),
        "the root orders Family and Vacation albums and contains no direct pictures");
    if (!cna_demo_check_result(
            &demo, cna_picture_album_get_albums(vacation_album, &vacation_children),
            "get Vacation's child albums") ||
        !cna_demo_check(
            &demo,
            cna_picture_album_collection_get_count(vacation_children, &count) ==
                    CNA_RESULT_SUCCESS &&
                count == 1 &&
                cna_picture_album_collection_get_at(vacation_children, 0, &day_two_album) ==
                    CNA_RESULT_SUCCESS &&
                name_is(day_two_album, cna_picture_album_get_name_size,
                        cna_picture_album_copy_name, "Day 2"),
            "Vacation contains the single nested Day 2 album")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_picture_album_get_parent(family_album, &family_parent, &available) ==
                CNA_RESULT_SUCCESS &&
            available == CNA_TRUE &&
            cna_picture_album_equals(family_parent, root_album, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_picture_album_get_parent(day_two_album, &day_two_parent, &available) ==
                CNA_RESULT_SUCCESS &&
            available == CNA_TRUE &&
            cna_picture_album_equals(day_two_parent, vacation_album, &predicate) ==
                CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE,
        "child albums point back to their exact parents at both tree depths");
    cna_demo_check(
        &demo,
        cna_picture_album_equals(family_album, vacation_album, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_FALSE &&
            cna_picture_album_get_hash_code(family_album, &hash_a) == CNA_RESULT_SUCCESS &&
            cna_picture_album_get_hash_code(family_parent, &hash_b) == CNA_RESULT_SUCCESS &&
            hash_a != hash_b &&
            cna_picture_album_get_hash_code(day_two_album, &hash_a) == CNA_RESULT_SUCCESS &&
            cna_picture_album_get_hash_code(day_two_album, &hash_b) == CNA_RESULT_SUCCESS &&
            hash_a == hash_b,
        "picture-album equality and hashes distinguish paths and remain stable");

    if (!cna_demo_check_result(
            &demo, cna_picture_album_get_pictures(family_album, &family_pictures),
            "get Family's pictures") ||
        !cna_demo_check_result(
            &demo, cna_picture_album_get_pictures(day_two_album, &day_two_pictures),
            "get Day 2's pictures")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_picture_collection_get_count(family_pictures, &count) == CNA_RESULT_SUCCESS &&
            count == 1 &&
            cna_picture_collection_get_at(family_pictures, 0, &portrait_alias) ==
                CNA_RESULT_SUCCESS &&
            cna_picture_collection_get_count(day_two_pictures, &count) == CNA_RESULT_SUCCESS &&
            count == 1 &&
            cna_picture_collection_get_at(day_two_pictures, 0, &checker_alias) ==
                CNA_RESULT_SUCCESS,
        "each leaf album contains its one direct picture");
    if (!cna_demo_check(
            &demo,
            cna_picture_collection_get_at(all_pictures, 0, &picture[0]) == CNA_RESULT_SUCCESS &&
                cna_picture_collection_get_at(all_pictures, 1, &picture[1]) == CNA_RESULT_SUCCESS &&
                name_is(picture[0], cna_picture_get_name_size, cna_picture_copy_name,
                        "portrait") &&
                name_is(picture[1], cna_picture_get_name_size, cna_picture_copy_name,
                        "checker"),
            "the library collection follows deterministic depth-first picture order")) {
        goto cleanup;
    }
    rejected_picture = UINT64_C(999999);
    cna_demo_check(
        &demo,
        cna_picture_collection_get_at(all_pictures, 2, &rejected_picture) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected_picture == CNA_INVALID_HANDLE,
        "the picture collection refuses its upper bound without publishing a handle");
    cna_demo_check(
        &demo,
        cna_picture_get_width(picture[0], &width) == CNA_RESULT_SUCCESS && width == 640 &&
            cna_picture_get_height(picture[0], &height) == CNA_RESULT_SUCCESS && height == 320 &&
            cna_picture_get_date_unix_ticks(picture[0], &date_ticks) == CNA_RESULT_SUCCESS &&
            date_ticks > INT64_C(0) &&
            cna_picture_get_width(picture[1], &width) == CNA_RESULT_SUCCESS && width == 64 &&
            cna_picture_get_height(picture[1], &height) == CNA_RESULT_SUCCESS && height == 64,
        "picture values expose decoded dimensions and the indexed file timestamp");
    cna_demo_check(
        &demo,
        cna_picture_get_album(picture[0], &portrait_album, &available) == CNA_RESULT_SUCCESS &&
            available == CNA_TRUE &&
            cna_picture_album_equals(portrait_album, family_album, &predicate) ==
                CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_picture_equals(picture[0], portrait_alias, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_picture_equals(picture[0], checker_alias, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_FALSE,
        "a picture links to its containing album and aliases compare by resolved path");
    cna_demo_check(
        &demo,
        cna_picture_get_hash_code(picture[0], &hash_a) == CNA_RESULT_SUCCESS &&
            cna_picture_get_hash_code(portrait_alias, &hash_b) == CNA_RESULT_SUCCESS &&
            hash_a == hash_b,
        "equal picture aliases publish equal hashes");

    memset(token, 0, sizeof(token));
    memset(short_token, 0xA5, sizeof(short_token));
    memset(short_token_before, 0xA5, sizeof(short_token_before));
    if (!cna_demo_check(
            &demo,
            cna_picture_get_token_size_ext(picture[0], &token_bytes) == CNA_RESULT_SUCCESS &&
                token_bytes == strlen(fixture.portrait) && token_bytes < sizeof(token) &&
                cna_picture_copy_token_ext(
                    picture[0], token, sizeof(token), &required) == CNA_RESULT_SUCCESS &&
                required == token_bytes &&
                memcmp(token, fixture.portrait, (size_t)token_bytes) == 0 &&
                cna_picture_copy_token_ext(
                    picture[0], short_token, token_bytes - UINT64_C(1), &required) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                required == token_bytes &&
                memcmp(short_token, short_token_before, sizeof(short_token)) == 0,
            "the exact resolved-path token copies atomically")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_media_library_get_picture_from_token(
            library, (CNA_StringView){token, token_bytes}, &token_alias, &available) ==
                CNA_RESULT_SUCCESS &&
            available == CNA_TRUE &&
            cna_picture_equals(token_alias, picture[0], &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE,
        "the copied token resolves to an equal picture alias");
    rejected_picture = UINT64_C(999999);
    cna_demo_check(
        &demo,
        cna_media_library_get_picture_from_token(
            library, text_view("missing-picture-token"), &rejected_picture, &available) ==
                CNA_RESULT_SUCCESS &&
            available == CNA_FALSE && rejected_picture == CNA_INVALID_HANDLE,
        "an unknown token reports ordinary absence without a stale picture handle");

    if (!cna_demo_check(
            &demo,
            cna_picture_get_image_size(picture[0], &image_bytes) == CNA_RESULT_SUCCESS &&
                image_bytes == panorama_bytes && image_bytes > UINT64_C(1),
            "the indexed panorama reports its exact encoded length")) {
        goto cleanup;
    }
    copied_image = (uint8_t*)malloc((size_t)image_bytes);
    short_image = (uint8_t*)malloc((size_t)image_bytes);
    if (copied_image == NULL || short_image == NULL) {
        cna_demo_check(&demo, 0, "allocate picture-image transfer buffers");
        goto cleanup;
    }
    memset(short_image, 0xA5, (size_t)image_bytes);
    cna_demo_check(
        &demo,
        cna_picture_copy_image(
            picture[0], short_image, image_bytes - UINT64_C(1), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == image_bytes && short_image[0] == 0xA5U &&
            short_image[image_bytes - UINT64_C(1)] == 0xA5U,
        "a short full-image buffer reports its requirement without a partial write");
    cna_demo_check(
        &demo,
        cna_picture_copy_image(picture[0], copied_image, image_bytes, &required) ==
                CNA_RESULT_SUCCESS &&
            required == image_bytes && memcmp(copied_image, panorama, (size_t)image_bytes) == 0,
        "GetImage preserves every original panorama byte");
    if (!cna_demo_check(
            &demo,
            cna_picture_get_thumbnail_size(picture[0], &thumbnail_bytes) == CNA_RESULT_SUCCESS &&
                thumbnail_bytes > UINT64_C(1) && thumbnail_bytes <= UINT64_C(16 * 1024 * 1024),
            "the indexed panorama reports a nonempty bounded thumbnail")) {
        goto cleanup;
    }
    thumbnail = (uint8_t*)malloc((size_t)thumbnail_bytes);
    short_thumbnail = (uint8_t*)malloc((size_t)thumbnail_bytes);
    if (thumbnail == NULL || short_thumbnail == NULL) {
        cna_demo_check(&demo, 0, "allocate picture-thumbnail transfer buffers");
        goto cleanup;
    }
    memset(short_thumbnail, 0xA5, (size_t)thumbnail_bytes);
    cna_demo_check(
        &demo,
        cna_picture_copy_thumbnail(
            picture[0], short_thumbnail, thumbnail_bytes - UINT64_C(1), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == thumbnail_bytes && short_thumbnail[0] == 0xA5U &&
            short_thumbnail[thumbnail_bytes - UINT64_C(1)] == 0xA5U,
        "a short picture-thumbnail buffer is atomic");
    if (!cna_demo_check(
            &demo,
            cna_picture_copy_thumbnail(
                picture[0], thumbnail, thumbnail_bytes, &required) == CNA_RESULT_SUCCESS &&
                required == thumbnail_bytes &&
                png_dimensions(thumbnail, thumbnail_bytes, &thumbnail_width, &thumbnail_height) &&
                thumbnail_width > 0U && thumbnail_width < 640U &&
                thumbnail_width == 2U * thumbnail_height,
            "the copied picture thumbnail is a smaller PNG with the original aspect ratio")) {
        goto cleanup;
    }

    if (!cna_demo_check_result(
            &demo,
            cna_media_library_save_picture(
                library, text_view("Preview"), thumbnail, thumbnail_bytes, &preview),
            "save the generated thumbnail from a byte buffer")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        name_is(preview, cna_picture_get_name_size, cna_picture_copy_name, "Preview") &&
            cna_picture_get_width(preview, &width) == CNA_RESULT_SUCCESS &&
            width == (int32_t)thumbnail_width &&
            cna_picture_get_height(preview, &height) == CNA_RESULT_SUCCESS &&
            height == (int32_t)thumbnail_height &&
            cna_picture_collection_get_count(all_pictures, &count) == CNA_RESULT_SUCCESS &&
            count == 3 &&
            cna_picture_collection_get_count(saved_pictures, &count) == CNA_RESULT_SUCCESS &&
            count == 1,
        "buffer saving returns decoded values and grows both live library collections");
    if (!cna_demo_check_result(
            &demo, cna_picture_get_album(preview, &saved_album, &available),
            "get the saved picture's album") ||
        !cna_demo_check(
            &demo,
            available == CNA_TRUE &&
                name_is(saved_album, cna_picture_album_get_name_size,
                        cna_picture_album_copy_name, "Saved Pictures") &&
                cna_picture_album_get_parent(saved_album, &saved_parent, &available) ==
                    CNA_RESULT_SUCCESS &&
                available == CNA_TRUE &&
                cna_picture_album_equals(saved_parent, root_album, &predicate) ==
                    CNA_RESULT_SUCCESS &&
                predicate == CNA_TRUE &&
                cna_picture_album_collection_get_count(root_children, &count) ==
                    CNA_RESULT_SUCCESS &&
                count == 3,
            "saving creates the Saved Pictures album directly below the live root")) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo, cna_picture_album_get_pictures(saved_album, &saved_album_pictures),
            "get Saved Pictures members") ||
        !cna_demo_check_result(
            &demo, cna_picture_album_get_albums(saved_album, &saved_children),
            "get Saved Pictures child albums")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_picture_collection_get_count(saved_album_pictures, &count) == CNA_RESULT_SUCCESS &&
            count == 1 &&
            cna_picture_album_collection_get_count(saved_children, &count) == CNA_RESULT_SUCCESS &&
            count == 0,
        "the saved album contains the imported preview and no child albums");

    if (!cna_demo_check_result(
            &demo,
            cna_storage_set_app_name_ext(text_view("cna-demos-c-media-pictures")),
            "isolate the storage stream source") ||
        !cna_demo_check_result(
            &demo, cna_storage_device_show_selector(NULL, NULL, &device),
            "select the local storage device") ||
        !cna_demo_check_result(
            &demo,
            cna_storage_container_open(
                device, text_view("Picture Imports"), NULL, NULL, &container),
            "open the private picture-import container") ||
        !cna_demo_check_result(
            &demo,
            cna_storage_container_create_file(container, text_view("source.png"), &stream),
            "create the source storage stream") ||
        !cna_demo_check_result(
            &demo, cna_storage_stream_write(stream, checker, checker_bytes),
            "write the exact checker PNG into the storage stream") ||
        !cna_demo_check_result(
            &demo,
            cna_storage_stream_seek(
                stream, INT64_C(0), CNA_SEEK_ORIGIN_BEGIN, &stream_position),
            "rewind the storage stream before import") ||
        !cna_demo_check_result(
            &demo,
            cna_media_library_save_picture_from_stream(
                library, text_view("Stream Import"), stream, &stream_import),
            "save a picture from the remaining storage stream")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_storage_stream_get_position(stream, &stream_position) == CNA_RESULT_SUCCESS &&
            stream_position == (int64_t)checker_bytes &&
            name_is(stream_import, cna_picture_get_name_size, cna_picture_copy_name,
                    "Stream Import") &&
            cna_picture_get_width(stream_import, &width) == CNA_RESULT_SUCCESS && width == 64 &&
            cna_picture_get_height(stream_import, &height) == CNA_RESULT_SUCCESS && height == 64,
        "stream saving consumes the source and returns the checker's exact decoded dimensions");
    image_bytes = UINT64_C(0);
    if (cna_picture_get_image_size(stream_import, &image_bytes) == CNA_RESULT_SUCCESS &&
        image_bytes == checker_bytes) {
        uint8_t* imported = (uint8_t*)malloc((size_t)image_bytes);
        int exact = imported != NULL &&
            cna_picture_copy_image(stream_import, imported, image_bytes, &required) ==
                CNA_RESULT_SUCCESS &&
            required == image_bytes && memcmp(imported, checker, (size_t)image_bytes) == 0;
        cna_demo_check(
            &demo, exact,
            "the stream import preserves every encoded checker byte");
        free(imported);
    } else {
        cna_demo_check(&demo, 0, "the stream import preserves every encoded checker byte");
    }
    cna_demo_check(
        &demo,
        cna_picture_collection_get_count(all_pictures, &count) == CNA_RESULT_SUCCESS &&
            count == 4 &&
            cna_picture_collection_get_count(saved_pictures, &count) == CNA_RESULT_SUCCESS &&
            count == 2 &&
            cna_picture_collection_get_count(saved_album_pictures, &count) ==
                CNA_RESULT_SUCCESS &&
            count == 2,
        "both save routes update the all, saved and album collection views together");
    printf("  indexed: 640x320 and 64x64; thumbnail: %ux%u; saved: 2\n",
           (unsigned)thumbnail_width, (unsigned)thumbnail_height);

    /* XNA's Picture and PictureAlbum refuse every member but IsDisposed once disposed
       (CNA-DEMOS-ISSUE-098); the C routes answer CNA_RESULT_INVALID_STATE. */
    uint64_t disposed_bytes = 0;
    cna_demo_check(
        &demo,
        cna_picture_get_is_disposed(picture[0], &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE && cna_picture_dispose(portrait_alias) == CNA_RESULT_SUCCESS &&
            cna_picture_dispose(portrait_alias) == CNA_RESULT_SUCCESS &&
            cna_picture_get_is_disposed(picture[0], &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_picture_get_name_size(picture[0], &disposed_bytes) == CNA_RESULT_INVALID_STATE,
        "picture disposal is idempotent, shared by aliases and refuses value access");
    cna_demo_check(
        &demo,
        cna_picture_album_get_is_disposed(family_album, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE && cna_picture_album_dispose(portrait_album) == CNA_RESULT_SUCCESS &&
            cna_picture_album_dispose(portrait_album) == CNA_RESULT_SUCCESS &&
            cna_picture_album_get_is_disposed(family_album, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_picture_album_get_name_size(family_album, &disposed_bytes) ==
                CNA_RESULT_INVALID_STATE,
        "picture-album disposal is idempotent, shared by aliases and refuses value access");
    cna_demo_check(
        &demo,
        cna_picture_collection_get_is_disposed(saved_pictures, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_picture_collection_dispose(saved_pictures) == CNA_RESULT_SUCCESS &&
            cna_picture_collection_dispose(saved_pictures) == CNA_RESULT_SUCCESS &&
            cna_picture_collection_get_is_disposed(saved_pictures, &disposed) ==
                CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_picture_collection_get_count(saved_pictures, &count) == CNA_RESULT_INVALID_STATE,
        "picture-collection disposal is idempotent and refuses that collection view's count");
    cna_demo_check(
        &demo,
        cna_picture_album_collection_get_is_disposed(root_children, &disposed) ==
                CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_picture_album_collection_dispose(root_children) == CNA_RESULT_SUCCESS &&
            cna_picture_album_collection_dispose(root_children) == CNA_RESULT_SUCCESS &&
            cna_picture_album_collection_get_is_disposed(root_children, &disposed) ==
                CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE &&
            cna_picture_album_collection_get_count(root_children, &count) ==
                CNA_RESULT_INVALID_STATE,
        "picture-album collection disposal is idempotent and refuses that collection view's count");
    cna_demo_check_result(
        &demo, cna_media_library_destroy(library),
        "release the library while picture and album handles retain it");
    library = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_picture_get_width(stream_import, &width) == CNA_RESULT_SUCCESS && width == 64 &&
            cna_picture_album_get_parent(day_two_album, &day_two_parent, &available) ==
                CNA_RESULT_SUCCESS &&
            available == CNA_TRUE,
        "retained picture and album handles remain readable after library release");

cleanup:
    free(short_thumbnail);
    free(thumbnail);
    free(short_image);
    free(copied_image);
    free(checker);
    free(panorama);
    if (stream != CNA_INVALID_HANDLE) {
        releases_ok &= cna_storage_stream_close(stream) == CNA_RESULT_SUCCESS;
        stream = CNA_INVALID_HANDLE;
    }
    if (container != CNA_INVALID_HANDLE) {
        (void)cna_storage_container_delete_file(container, text_view("source.png"));
        releases_ok &= cna_storage_container_destroy(container) == CNA_RESULT_SUCCESS;
        container = CNA_INVALID_HANDLE;
    }
    releases_ok &= release_handle(&device, cna_storage_device_destroy);
    releases_ok &= release_handle(&rejected_picture, cna_picture_destroy);
    releases_ok &= release_handle(&stream_import, cna_picture_destroy);
    releases_ok &= release_handle(&preview, cna_picture_destroy);
    releases_ok &= release_handle(&token_alias, cna_picture_destroy);
    releases_ok &= release_handle(&checker_alias, cna_picture_destroy);
    releases_ok &= release_handle(&portrait_alias, cna_picture_destroy);
    for (index = 0; index < 2; ++index) {
        releases_ok &= release_handle(&picture[index], cna_picture_destroy);
    }
    releases_ok &= release_handle(&saved_parent, cna_picture_album_destroy);
    releases_ok &= release_handle(&saved_album, cna_picture_album_destroy);
    releases_ok &= release_handle(&day_two_parent, cna_picture_album_destroy);
    releases_ok &= release_handle(&family_parent, cna_picture_album_destroy);
    releases_ok &= release_handle(&portrait_album, cna_picture_album_destroy);
    releases_ok &= release_handle(&day_two_album, cna_picture_album_destroy);
    releases_ok &= release_handle(&vacation_album, cna_picture_album_destroy);
    releases_ok &= release_handle(&family_album, cna_picture_album_destroy);
    releases_ok &= release_handle(&root_album, cna_picture_album_destroy);
    releases_ok &= release_handle(&saved_children, cna_picture_album_collection_destroy);
    releases_ok &= release_handle(&vacation_children, cna_picture_album_collection_destroy);
    releases_ok &= release_handle(&root_children, cna_picture_album_collection_destroy);
    releases_ok &= release_handle(&saved_album_pictures, cna_picture_collection_destroy);
    releases_ok &= release_handle(&day_two_pictures, cna_picture_collection_destroy);
    releases_ok &= release_handle(&family_pictures, cna_picture_collection_destroy);
    releases_ok &= release_handle(&root_pictures, cna_picture_collection_destroy);
    releases_ok &= release_handle(&saved_pictures, cna_picture_collection_destroy);
    releases_ok &= release_handle(&all_pictures, cna_picture_collection_destroy);
    releases_ok &= release_handle(&library, cna_media_library_destroy);
    cna_demo_check(
        &demo, releases_ok,
        "close storage and release every retained picture-library handle exactly once");
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the fixture-owning game");
    }
    cleanup_private_library(&fixture);
    return cna_demo_finish(&demo);
#endif
}
