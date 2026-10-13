// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "media.media_library.music_index",
    .title = "Tagged music library and shared collection views",
    .description = "Imports three original tagged FLAC tracks and an M3U into a private Linux library, "
                   "then checks album, artist and genre membership, Song back-references, playlist ordering and optional positive duration aggregation.",
    .apis = {"MediaLibrary::MediaLibrary", "MediaLibrary::Songs", "MediaLibrary::Albums", "MediaLibrary::Artists",
             "MediaLibrary::Genres", "MediaLibrary::Playlists", "Song::Name", "Song::TrackNumber",
             "Song::Album", "Song::Artist", "Song::Genre", "Album::Name", "Album::Artist", "Album::Genre",
             "Album::Songs", "Album::Equals", "Artist::Name", "Artist::Songs", "Artist::Albums", "Genre::Name", "Genre::Songs",
             "Genre::Albums", "Playlist::Name", "Playlist::Songs", "Playlist::Duration", "Album::Duration", "Song::Duration", "SongCollection::Count",
             "AlbumCollection::Count", "ArtistCollection::Count", "GenreCollection::Count", "PlaylistCollection::Count"},
    .tags = {"media", "library", "metadata", "album", "artist", "genre", "playlist", "ownership"},
    .deterministic = true,
    .assets = {"media/library_music/track_a.flac", "media/library_music/track_b.flac", "media/library_music/track_c.flac"},
    .restrictions = "Standalone process selects private XDG roots before first CNA/SDL use. Linux/XDG resolution is required; "
                    "otherwise skipped before accessing a library. All collection entries are library-owned. "
                    "Positive duration checks require CNA_FFMPEG_AVAILABLE; other builds retain the relationship checks. No playback or album-art validation.",
})
