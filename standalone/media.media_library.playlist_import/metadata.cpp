// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "media.media_library.playlist_import",
    .title = "M3U playlist import boundaries",
    .description = "Imports private M3U/M3U8 playlists and checks ordered shared Song views, duplicate entries, UTF-8, "
                   "ignored comments and missing files, path containment, supported extensions and scan depth.",
    .apis = {"MediaLibrary::MediaLibrary", "MediaLibrary::Playlists",
             "Playlist::Name", "Playlist::Songs", "PlaylistCollection::Count", "PlaylistCollection::Item",
             "Song::Name", "SongCollection::Count", "SongCollection::Item"},
    .tags = {"media", "library", "playlist", "m3u", "utf-8", "filesystem", "security"},
    .deterministic = true,
    .assets = {"media/library_music/track_a.flac", "media/library_music/track_b.flac",
               "media/library_music/track_c.flac"},
    .restrictions = "Standalone process verifies private Linux XDG media roots before library access. "
                    "M3U and M3U8 are exercised through public MediaLibrary views; PLS, nested playlist discovery, "
                    "absolute entries and symlink escapes are deliberately unsupported. No playback, listening or other-platform validation.",
})
