// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "media.media_library.format_index",
    .title = "Music formats, tags and fallback indexing",
    .description = "Indexes Ogg/OGA, MP3, WAV, FLAC and Opus recursively in a private library; checks full/partial tags, "
                   "rating and track propagation, path fallback, grouping, ignored formats and symlink policies.",
    .apis = {"MediaLibrary::MediaLibrary", "MediaLibrary::Songs", "MediaLibrary::Albums",
             "MediaLibrary::Artists", "MediaLibrary::Genres", "Song::Name", "Song::TrackNumber",
             "Song::Rating", "Song::IsRated", "Song::Duration", "Song::Album", "Song::Artist",
             "Song::Genre", "SongCollection::Count", "SongCollection::Item",
             "AlbumCollection::Count", "ArtistCollection::Count", "GenreCollection::Count"},
    .tags = {"media", "library", "formats", "tags", "rating", "fallback", "filesystem"},
    .deterministic = true,
    .assets = {"media/library_formats/format_vorbis.ogg", "media/library_formats/format_alias.oga",
               "media/library_formats/format_id3.mp3", "media/library_formats/format_flac.flac",
               "media/library_formats/format_opus.opus", "media/library_formats/format_partial.ogg",
               "media/library_music/track_c.flac", "music/rising.wav"},
    .restrictions = "Standalone process verifies private Linux XDG media roots before library access. "
                    "The six supported extension routes, full/partial tags, filename fallback, recursive discovery and symlink policies are public-state checks. "
                    "Positive durations require CNA_FFMPEG_AVAILABLE. No playback, listening, malformed-tag or other-platform validation.",
})
