// SPDX-License-Identifier: MS-PL
// Song properties: what a song created from a file can tell, and what only a media library knows.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    void Properties(CnaDemos::DemoContext& ctx)
    {
        // The name is the caller's; the file is any local audio file the platform can decode.
        const std::unique_ptr<Song> song(Song::FromUri("Rising (demo tune)", TitleLocation::Path() + "/assets/music/rising.wav"));
        ctx.Print("Name \"{}\", Duration {} ms, TrackNumber {}, PlayCount {}, Rating {}", song->getNameProperty(),
                  song->getDurationProperty().getTotalMillisecondsProperty(), song->getTrackNumberProperty(),
                  song->getPlayCountProperty(), song->getRatingProperty());
        ctx.Check(song->getNameProperty() == "Rising (demo tune)", "Name is the name given to FromUri");

        // Album, artist, genre, protection and rating come from a media library (a music
        // collection the platform indexes); a song made from a file has none of them. Duration
        // is filled in when CNA can probe the file (builds with the FFmpeg media module); without
        // a probe it is zero, and playback still runs to the end.
        ctx.Check(song->getAlbumProperty() == nullptr && song->getArtistProperty() == nullptr &&
                      song->getGenreProperty() == nullptr,
                  "a song from a file has no album, artist or genre");
        ctx.Check(!song->getIsProtectedProperty() && !song->getIsRatedProperty(), "it is neither DRM-protected nor rated");
        ctx.Check(!song->getIsDisposedProperty(), "IsDisposed is false until Dispose");
        song->Dispose();
        ctx.Check(song->getIsDisposedProperty(), "Dispose sets IsDisposed");
    }
}

CNA_DEMOS_CONSOLE(Properties, {
    .id = "media.song.properties",
    .title = "Song properties",
    .description = "Creates a Song from a file with Song::FromUri and prints its properties: Name, Duration, track "
                   "number, play count, rating; album, artist and genre exist only for media-library songs.",
    .apis = {"Song::FromUri", "Song::Name", "Song::Duration", "Song::TrackNumber", "Song::PlayCount", "Song::Rating",
             "Song::Album", "Song::Artist", "Song::Genre", "Song::IsProtected", "Song::IsRated", "Song::IsDisposed",
             "Song::Dispose"},
    .tags = {"media", "music", "song", "metadata"},
    .assets = {"music/rising.wav"},
})
