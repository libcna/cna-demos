// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "media.media_library.save_picture",
    .title = "Save pictures and read image and thumbnail streams",
    .description = "Uses an isolated Linux media library to save an original panorama, check picture/album views, "
                   "read the owned image streams, save and reindex a smaller thumbnail, and expose incomplete short reads.",
    .apis = {"MediaLibrary::SavePicture", "MediaLibrary::Pictures", "MediaLibrary::SavedPictures",
             "MediaLibrary::RootPictureAlbum", "Picture::Name", "Picture::Width", "Picture::Height",
             "Picture::Album", "Picture::GetImage", "Picture::GetThumbnail", "PictureAlbum::Name",
             "PictureAlbum::Parent", "PictureAlbum::Pictures", "PictureAlbum::Albums", "PictureAlbumCollection::Count"},
    .tags = {"media", "pictures", "library", "streams", "ownership"},
    .deterministic = true,
    .assets = {"media/library_panorama.png"},
    .restrictions = "Standalone program initializes private XDG paths before CNA/SDL first use. Linux/XDG resolution is required; "
                    "otherwise skipped before accessing a library. "
                    "Private generated files only. Short reads expose CNA-DEMOS-ISSUE-041; other platforms are untested. "
                    "GetThumbnail's no-resize comment is stale (CNA-DEMOS-ISSUE-040); the normal thumbnail checks pass.",
    .knownIssue = "CNA-DEMOS-ISSUE-041",
})
