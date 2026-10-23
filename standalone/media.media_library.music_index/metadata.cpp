// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "media.media_library.music_index",
    .title = "Tagged music library and shared collection views",
    .description = "Imports three original tagged FLAC tracks and an M3U into a private Linux library; checks its shared relationships, "
                   "optional durations and disposed-object refusals across entity, collection and library views.",
    .apis = {"MediaLibrary::MediaLibrary", "MediaLibrary::Songs", "MediaLibrary::Albums", "MediaLibrary::Artists",
             "MediaLibrary::Genres", "MediaLibrary::Playlists", "MediaLibrary::MediaSource", "MediaLibrary::Pictures",
             "MediaLibrary::SavedPictures", "MediaLibrary::RootPictureAlbum", "MediaLibrary::GetPictureFromToken",
             "MediaLibrary::SavePicture", "Song::Name", "Song::TrackNumber",
             "Song::Album", "Song::Artist", "Song::Genre", "Album::Name", "Album::Artist", "Album::Genre",
             "Album::Songs", "Album::Equals", "Artist::Name", "Artist::Songs", "Artist::Albums", "Genre::Name", "Genre::Songs",
             "Genre::Albums", "Playlist::Name", "Playlist::Songs", "Playlist::Duration", "Album::Duration", "Song::Duration", "SongCollection::Count",
             "AlbumCollection::Count", "ArtistCollection::Count", "GenreCollection::Count", "PlaylistCollection::Count",
             "SongCollection::Item", "AlbumCollection::Item", "ArtistCollection::Item", "GenreCollection::Item",
             "PlaylistCollection::Item",
             "Album::GetHashCode", "Album::op_Equality", "Album::op_Inequality", "Album::ToString",
             "Artist::Equals", "Artist::GetHashCode", "Artist::op_Equality", "Artist::op_Inequality", "Artist::ToString",
             "Genre::Equals", "Genre::GetHashCode", "Genre::op_Equality", "Genre::op_Inequality", "Genre::ToString",
             "Playlist::Equals", "Playlist::GetHashCode", "Playlist::op_Equality", "Playlist::op_Inequality",
             "Playlist::ToString",
             "Album::Dispose", "Album::IsDisposed", "AlbumCollection::Dispose", "AlbumCollection::IsDisposed",
             "Artist::Dispose", "Artist::IsDisposed", "ArtistCollection::Dispose", "ArtistCollection::IsDisposed",
             "Genre::Dispose", "Genre::IsDisposed", "GenreCollection::Dispose", "GenreCollection::IsDisposed",
             "Playlist::Dispose", "Playlist::IsDisposed", "PlaylistCollection::Dispose", "PlaylistCollection::IsDisposed",
             "SongCollection::Dispose", "SongCollection::IsDisposed", "MediaLibrary::Dispose", "MediaLibrary::IsDisposed"},
    .tags = {"media", "library", "metadata", "album", "artist", "genre", "playlist", "ownership"},
    .deterministic = true,
    .assets = {"media/library_music/track_a.flac", "media/library_music/track_b.flac", "media/library_music/track_c.flac"},
    .restrictions = "Standalone process selects private XDG roots before first CNA/SDL use. Linux/XDG resolution is required; "
                    "otherwise skipped before accessing a library. All collection entries are library-owned; disposal is observed before destruction. "
                    "Positive duration checks require CNA_FFMPEG_AVAILABLE; other builds retain the relationship checks. No playback or album-art validation.",
})
