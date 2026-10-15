// SPDX-License-Identifier: MS-PL
// Song properties: what a song created from a file can tell, and what only a media library knows.
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    template<typename Action>
    bool ThrowsObjectDisposed(Action&& action)
    {
        try
        {
            action();
        }
        catch (const System::ObjectDisposedException&)
        {
            return true;
        }
        catch (...)
        {
        }
        return false;
    }

    void Properties(CnaDemos::DemoContext& ctx)
    {
        // The name is the caller's; the file is any local audio file the platform can decode.
        const std::string risingPath = TitleLocation::Path() + "/assets/music/rising.wav";
        const std::unique_ptr<Song> song(Song::FromUri("Rising (demo tune)", risingPath));
        ctx.Print("Name \"{}\", Duration {} ms, TrackNumber {}, PlayCount {}, Rating {}", song->getNameProperty(),
                  song->getDurationProperty().getTotalMillisecondsProperty(), song->getTrackNumberProperty(),
                  song->getPlayCountProperty(), song->getRatingProperty());
        ctx.Check(song->getNameProperty() == "Rising (demo tune)", "Name is the name given to FromUri");

        const std::unique_ptr<Song> renamed(Song::FromUri("Renamed copy", risingPath));
        const std::unique_ptr<Song> namesake(
            Song::FromUri("Rising (demo tune)", TitleLocation::Path() + "/assets/sounds/tone_440.wav"));
        ctx.Check(song->Equals(renamed.get()) &&
                      song->Equals(static_cast<const System::Object*>(renamed.get())) &&
                      *song == *renamed && !(*song != *renamed) &&
                      song->GetHashCode() == renamed->GetHashCode(),
                  "songs for the same file have typed/object equality and equal hashes despite different names");
        ctx.Check(!song->Equals(namesake.get()) &&
                      !song->Equals(static_cast<const System::Object*>(nullptr)) && *song != *namesake &&
                      song->ToString() == "Rising (demo tune)" && renamed->ToString() == "Renamed copy" &&
                      namesake->ToString() == "Rising (demo tune)",
                  "different files remain unequal despite equal display names, which supply diagnostic text");

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
        song->Dispose();
        ctx.Check(song->getIsDisposedProperty() && !renamed->getIsDisposedProperty() &&
                      !namesake->getIsDisposedProperty(),
                  "Dispose is idempotent and leaves peer Song objects live");

        int disposalRefusals = 0;
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->getNameProperty(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->getAlbumProperty(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->getArtistProperty(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->getGenreProperty(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->getDurationProperty(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->getIsProtectedProperty(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->getIsRatedProperty(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->getPlayCountProperty(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->getRatingProperty(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->getTrackNumberProperty(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->Equals(renamed.get()); });
        disposalRefusals += ThrowsObjectDisposed([&]
        {
            (void)song->Equals(static_cast<const System::Object*>(renamed.get()));
        });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)(*song == *renamed); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)(*song != *namesake); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->GetHashCode(); });
        disposalRefusals += ThrowsObjectDisposed([&] { (void)song->ToString(); });
        ctx.Check(disposalRefusals == 16,
                  "a disposed Song refuses properties and value operations (CNA-DEMOS-ISSUE-098)");
        ctx.Print("Disposed Song refusals: {}/16", disposalRefusals);
    }
}

CNA_DEMOS_CONSOLE(Properties, {
    .id = "media.song.properties",
    .title = "Song properties",
    .description = "Creates a Song from a file with Song::FromUri; checks identity, metadata defaults and the disposed-object refusal contract.",
    .apis = {"Song::FromUri", "Song::Name", "Song::Duration", "Song::TrackNumber", "Song::PlayCount", "Song::Rating",
             "Song::Album", "Song::Artist", "Song::Genre", "Song::IsProtected", "Song::IsRated", "Song::IsDisposed",
             "Song::Dispose", "Song::Equals", "Song::GetHashCode", "Song::op_Equality", "Song::op_Inequality",
             "Song::ToString"},
    .tags = {"media", "music", "song", "metadata"},
    .assets = {"music/rising.wav", "sounds/tone_440.wav"},
    .restrictions = "Uses caller-created local-file songs; library grouping and playback are separate demos. "
                    "CNA-DEMOS-ISSUE-098: disposed Song operations remain usable instead of throwing ObjectDisposedException.",
    .knownIssue = "CNA-DEMOS-ISSUE-098",
})
