// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "media.media_library.album_art",
    .title = "Album covers and owned thumbnail streams",
    .description = "Indexes three private tagged albums, contrasts missing, embedded and ordered folder art, and checks "
                   "folder-over-embedded priority, original bytes, large/small/corrupt thumbnail paths and stream ownership.",
    .apis = {"MediaLibrary::MediaLibrary", "MediaLibrary::Albums", "Album::HasArt", "Album::GetAlbumArt",
             "Album::GetThumbnail", "Album::Artist", "Artist::Name", "MediaLibrary::SavePicture",
             "Picture::Width", "Picture::Height", "AlbumCollection::Count"},
    .tags = {"media", "library", "album", "cover", "thumbnail", "streams", "ownership"},
    .deterministic = true,
    .assets = {"media/library_music/track_a.flac", "media/library_music/track_c.flac",
               "media/library_music/track_embedded.flac", "media/library_panorama.png",
               "textures/checker.png", "textures/arrow.png", "textures/gradient.png"},
    .restrictions = "Standalone process verifies private Linux XDG music/picture roots before library access. "
                    "Returned image streams are caller-owned; Album/Picture pointers are borrowed. "
                    "Embedded PNG extraction, folder-over-embedded and folder-name priority, large/small thumbnail branches "
                    "and corrupt-file fallback are exercised; other platforms remain separate. No playback or visual-quality validation.",
})
