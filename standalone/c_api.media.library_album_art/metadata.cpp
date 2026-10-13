// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.media.library_album_art",
    .title = "Media-library album art in C",
    .description = "A C program builds two private tagged albums, reads exact cover bytes and the generated "
                   "thumbnail, and contrasts the album without art.",
    .apis = {"cna_album_get_art_size", "cna_album_copy_art",
             "cna_album_get_thumbnail_size", "cna_album_copy_thumbnail",
             "cna_album_get_has_art", "cna_media_library_create",
             "cna_media_library_get_albums", "cna_album_collection_get_count",
             "cna_album_collection_get_at", "cna_title_container_read_ext",
             "cna_album_destroy", "cna_album_collection_destroy",
             "cna_media_library_destroy", "cna_game_create", "cna_game_destroy"},
    .tags = {"c", "binding", "media", "library", "album", "art", "thumbnail"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .assets = {"media/library_music/track_a.flac", "media/library_music/track_c.flac",
               "media/library_panorama.png"},
    .restrictions = "Requires Linux/XDG user-folder resolution and skips before library access elsewhere. It "
                    "uses private temporary Music/Pictures roots and original generated fixtures. Image bytes "
                    "and PNG dimensions are checked programmatically; no rendering or visual inspection is claimed.",
})
