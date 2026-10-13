// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.media.library_pictures",
    .title = "Picture-library hierarchy and imports in C",
    .description = "A C program walks a controlled picture-album tree, transfers image and thumbnail bytes, "
                   "resolves tokens, and saves pictures from a buffer and a storage stream.",
    .apis = {"cna_picture_get_name_size", "cna_picture_copy_name",
             "cna_picture_get_token_size_ext", "cna_picture_copy_token_ext",
             "cna_picture_get_album", "cna_picture_get_date_unix_ticks",
             "cna_picture_get_width", "cna_picture_get_height",
             "cna_picture_get_image_size", "cna_picture_copy_image",
             "cna_picture_get_thumbnail_size", "cna_picture_copy_thumbnail",
             "cna_picture_get_is_disposed", "cna_picture_dispose", "cna_picture_destroy",
             "cna_picture_equals", "cna_picture_get_hash_code",
             "cna_picture_album_get_name_size", "cna_picture_album_copy_name",
             "cna_picture_album_get_parent", "cna_picture_album_get_albums",
             "cna_picture_album_get_pictures", "cna_picture_album_get_is_disposed",
             "cna_picture_album_dispose", "cna_picture_album_destroy",
             "cna_picture_album_equals", "cna_picture_album_get_hash_code",
             "cna_media_library_get_pictures", "cna_media_library_get_saved_pictures",
             "cna_media_library_get_root_picture_album",
             "cna_media_library_get_picture_from_token", "cna_media_library_save_picture",
             "cna_media_library_save_picture_from_stream",
             "cna_picture_collection_get_count", "cna_picture_collection_get_at",
             "cna_picture_collection_get_is_disposed", "cna_picture_collection_dispose",
             "cna_picture_collection_destroy", "cna_picture_album_collection_get_count",
             "cna_picture_album_collection_get_at",
             "cna_picture_album_collection_get_is_disposed",
             "cna_picture_album_collection_dispose", "cna_picture_album_collection_destroy",
             "cna_media_library_create", "cna_media_library_destroy",
             "cna_title_container_read_ext", "cna_storage_set_app_name_ext",
             "cna_storage_device_show_selector", "cna_storage_container_open",
             "cna_storage_container_create_file", "cna_storage_stream_write",
             "cna_storage_stream_seek", "cna_storage_stream_get_position",
             "cna_storage_stream_close", "cna_storage_container_delete_file",
             "cna_storage_container_destroy", "cna_storage_device_destroy",
             "cna_game_create", "cna_game_destroy"},
    .tags = {"c", "binding", "media", "library", "picture", "album", "storage", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .assets = {"media/library_panorama.png", "textures/checker.png"},
    .restrictions = "Requires Linux/XDG user-folder resolution and skips before library access elsewhere. It "
                    "uses private temporary Music, Pictures and storage roots and original generated images. "
                    "Encoded bytes and PNG dimensions are checked programmatically; no rendering or visual "
                    "inspection is claimed.",
})
