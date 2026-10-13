// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "media.media_library.album_art",
    .title = "Album covers and owned thumbnail streams",
    .description = "Indexes two private tagged albums, contrasts folder cover art with missing art, and checks "
                   "original bytes, a decoded smaller thumbnail and stream ownership beyond library lifetime.",
    .apis = {"MediaLibrary::MediaLibrary", "MediaLibrary::Albums", "Album::HasArt", "Album::GetAlbumArt",
             "Album::GetThumbnail", "Album::Artist", "Artist::Name", "MediaLibrary::SavePicture",
             "Picture::Width", "Picture::Height", "AlbumCollection::Count"},
    .tags = {"media", "library", "album", "cover", "thumbnail", "streams", "ownership"},
    .deterministic = true,
    .assets = {"media/library_music/track_a.flac", "media/library_music/track_c.flac", "media/library_panorama.png"},
    .restrictions = "Standalone process verifies private Linux XDG music/picture roots before library access. "
                    "Returned image streams are caller-owned; Album/Picture pointers are borrowed. "
                    "Only folder PNG art is exercised; embedded art, small-image fallback, corrupt images "
                    "and other platforms remain separate. No playback or visual-quality validation.",
})
