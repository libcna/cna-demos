// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "media.media_library.album_art",
    .title = "Album covers and owned thumbnail streams",
    .description = "Indexes four private tagged albums, contrasts missing, FLAC/MP3 embedded and ordered folder art, and checks "
                   "candidate-family/extension priority, folder-over-embedded selection, thumbnail paths and stream ownership.",
    .apis = {"MediaLibrary::MediaLibrary", "MediaLibrary::Albums", "Album::HasArt", "Album::GetAlbumArt",
             "Album::GetThumbnail", "Album::Artist", "Artist::Name", "MediaLibrary::SavePicture",
             "Picture::Width", "Picture::Height", "AlbumCollection::Count"},
    .tags = {"media", "library", "album", "cover", "thumbnail", "streams", "ownership"},
    .deterministic = true,
    .assets = {"media/library_music/track_a.flac", "media/library_music/track_c.flac",
               "media/library_music/track_embedded.flac", "media/library_music/track_embedded_mp3.mp3",
               "media/library_panorama.png",
               "textures/checker.png", "textures/arrow.png", "textures/gradient.png"},
    .restrictions = "Standalone process verifies private Linux XDG music/picture roots before library access. "
                    "Returned image streams are caller-owned; Album/Picture pointers are borrowed. "
                    "FLAC PICTURE and MP3 APIC extraction, folder-over-embedded and filename-family/extension priority, large/small thumbnail branches "
                    "and corrupt-file fallback are exercised; other platforms remain separate. No playback or visual-quality validation.",
})
