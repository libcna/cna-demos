// SPDX-License-Identifier: MS-PL
// MediaLibrary: the music and pictures the platform knows about, as collections.
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Media/AlbumCollection.hpp"
#include "Microsoft/Xna/Framework/Media/ArtistCollection.hpp"
#include "Microsoft/Xna/Framework/Media/GenreCollection.hpp"
#include "Microsoft/Xna/Framework/Media/MediaLibrary.hpp"
#include "Microsoft/Xna/Framework/Media/MediaSource.hpp"
#include "Microsoft/Xna/Framework/Media/MediaSourceType.hpp"
#include "Microsoft/Xna/Framework/Media/PictureCollection.hpp"
#include "Microsoft/Xna/Framework/Media/PlaylistCollection.hpp"
#include "Microsoft/Xna/Framework/Media/SongCollection.hpp"

using namespace Microsoft::Xna::Framework::Media;

namespace
{
    void Collections(CnaDemos::DemoContext& ctx)
    {
        // On a Zune or Windows Phone this was the device's music and photo library. On a desktop
        // CNA indexes the user's music and picture folders. The numbers depend on the machine,
        // so the demo only reports them (counts only -- nothing about the files themselves).
        const std::vector<std::shared_ptr<MediaSource>> sources = MediaSource::GetAvailableMediaSources();
        bool sourceRecordsAreNamed = !sources.empty();
        bool sourceRecordsAreLocal = !sources.empty();
        for (const auto& source : sources)
        {
            ctx.Print("media source: {} ({})", source->getNameProperty(),
                      source->getMediaSourceTypeProperty() == MediaSourceType::LocalDevice ? "LocalDevice"
                                                                                           : "WindowsMediaConnect");
            sourceRecordsAreNamed = sourceRecordsAreNamed && !source->getNameProperty().empty() &&
                source->ToString() == source->getNameProperty();
            sourceRecordsAreLocal = sourceRecordsAreLocal &&
                source->getMediaSourceTypeProperty() == MediaSourceType::LocalDevice;
        }
        ctx.Check(sourceRecordsAreNamed, "every returned media source has matching nonempty name and text");
        ctx.Check(sourceRecordsAreLocal, "every available CNA desktop media source is a LocalDevice");

        // XNA's MediaLibrary keeps the very MediaSource it is given and returns it from
        // MediaSource (CNA HND-TQ-39); CNA shares that object, so it outlives this list too.
        const MediaLibrary library(sources.front());
        const std::shared_ptr<MediaSource> selectedSource = library.getMediaSourceProperty();
        ctx.Check(selectedSource != nullptr && selectedSource == sources.front() &&
                      selectedSource->getNameProperty() == sources.front()->getNameProperty(),
                  "the library returns the very source it was opened on");
        ctx.Print("songs {}, albums {}, artists {}, genres {}, playlists {}, pictures {}",
                  library.getSongsProperty()->getCountProperty(), library.getAlbumsProperty()->getCountProperty(),
                  library.getArtistsProperty()->getCountProperty(), library.getGenresProperty()->getCountProperty(),
                  library.getPlaylistsProperty()->getCountProperty(), library.getPicturesProperty()->getCountProperty());
        ctx.Check(library.getSongsProperty()->getCountProperty() >= 0 && !library.getIsDisposedProperty(),
                  "the library opens and reports its collections");
    }
}

CNA_DEMOS_CONSOLE(Collections, {
    .id = "media.media_library.collections",
    .title = "Media library collections",
    .description = "Lists the available MediaSources and counts the songs, albums, artists, genres, playlists and "
                   "pictures of the default MediaLibrary (machine-dependent, so only reported).",
    .apis = {"MediaSource::GetAvailableMediaSources", "MediaSource::Name", "MediaLibrary::MediaLibrary",
             "MediaLibrary::Songs", "MediaLibrary::Albums", "MediaLibrary::Artists", "MediaLibrary::Genres",
             "MediaLibrary::Playlists", "MediaLibrary::Pictures", "MediaLibrary::IsDisposed",
             "MediaLibrary::MediaSource",
             "AlbumCollection::Count", "ArtistCollection::Count", "GenreCollection::Count", "PlaylistCollection::Count", "PictureCollection::Count",
             "MediaSource::MediaSourceType", "MediaSource::ToString", "MediaSourceType",
             "MediaSourceType::LocalDevice"},
    .tags = {"media", "library", "music", "pictures"},
    .automation = CnaDemos::Automation::SmokeOnly,
})
