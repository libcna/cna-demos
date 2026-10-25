// SPDX-License-Identifier: MS-PL
// Current Song/MediaPlayer behavior for a valid Opus file that the CNA mixer cannot decode.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Media/MediaPlayer.hpp"
#include "Microsoft/Xna/Framework/Media/MediaState.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"

using namespace Microsoft::Xna::Framework::Media;

namespace
{
    void UnsupportedOpus(CnaDemos::DemoContext& context)
    {
        std::unique_ptr<Song> song(Song::FromUri(
            "Valid Opus fixture", "assets/media/library_formats/format_opus.opus"));
        context.Check(song != nullptr && !song->getIsDisposedProperty(),
                      "a valid local .opus file constructs a Song value");

        MediaPlayer::Stop();
        context.Check(MediaPlayer::getStateProperty() == MediaState::Stopped,
                      "the transport is stopped before probing decoder support");
        MediaPlayer::Play(song.get());

        // CNA's media catalog and tag reader understand Opus, but this build's mixer deliberately
        // has its Opus decoder disabled. MediaPlayer has no error-result channel, so Play returns
        // with its normal private copy selected in the queue while transport stays stopped.
        Song* queued = MediaPlayer::getQueueProperty().getActiveSongProperty();
        context.Check(queued != nullptr && queued != song.get() &&
                          queued->getNameProperty() == song->getNameProperty(),
                      "Play selects a private copy of the Opus Song in MediaPlayer's queue");
        context.Check(MediaPlayer::getStateProperty() == MediaState::Stopped &&
                          MediaPlayer::getPlayPositionProperty().getTicksProperty() == 0,
                      "the current mixer refusal leaves MediaPlayer stopped at position zero");
        MediaPlayer::Stop();
    }
}

CNA_DEMOS_CONSOLE(UnsupportedOpus, {
    .id = "media.media_player.unsupported_opus",
    .title = "Unsupported Opus playback",
    .description = "Constructs a Song from a valid Ogg Opus fixture and records current CNA behavior: the media "
                   "catalog accepts it, but MediaPlayer remains stopped because the present mixer has no Opus decoder.",
    .apis = {"Song::FromUri", "Song::IsDisposed", "MediaPlayer::Play", "MediaPlayer::Queue",
             "MediaQueue::ActiveSong", "MediaPlayer::State", "MediaPlayer::PlayPosition", "MediaPlayer::Stop"},
    .tags = {"media", "song", "opus", "unsupported-format", "error-handling"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"media/library_formats/format_opus.opus"},
    .restrictions = "This pins current mixer support, not a timeless format claim; update the demo when CNA gains Opus decoding.",
})
