// SPDX-License-Identifier: MS-PL
// A playlist: MediaPlayer::Play(SongCollection), the Queue, MoveNext/MovePrevious and IsRepeating.
#include <memory>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/FrameworkDispatcher.hpp"
#include "Microsoft/Xna/Framework/Media/MediaPlayer.hpp"
#include "Microsoft/Xna/Framework/Media/MediaQueue.hpp"
#include "Microsoft/Xna/Framework/Media/MediaState.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include "Microsoft/Xna/Framework/Media/SongCollection.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    std::string Active()
    {
        const Song* song = MediaPlayer::getQueueProperty().getActiveSongProperty();
        return song != nullptr ? song->getNameProperty() : "(none)";
    }

    void QueueAndRepeat(CnaDemos::DemoContext& ctx)
    {
        Song* queuedRising = nullptr;
        Song* queuedFalling = nullptr;
        {
            const std::unique_ptr<Song> rising(
                Song::FromUri("Rising", TitleLocation::Path() + "/assets/music/rising.wav"));
            const std::unique_ptr<Song> falling(
                Song::FromUri("Falling", TitleLocation::Path() + "/assets/music/falling.wav"));
            const SongCollection playlist({rising.get(), falling.get()});

            // Playing a collection fills MediaPlayer's Queue with its own Song records.
            MediaPlayer::Play(playlist);
            const MediaQueue& startedQueue = MediaPlayer::getQueueProperty();
            queuedRising = startedQueue[0];
            queuedFalling = startedQueue[1];
            ctx.Check(queuedRising != rising.get() && queuedFalling != falling.get() &&
                          queuedRising->getNameProperty() == "Rising" &&
                          queuedFalling->getNameProperty() == "Falling",
                      "Play copies the ordered caller songs into queue-owned records");
        }

        const MediaQueue& queue = MediaPlayer::getQueueProperty();
        ctx.Print("queue: {} songs, active #{} {}", queue.getCountProperty(), queue.getActiveSongIndexProperty(), Active());
        ctx.Check(queue.getCountProperty() == 2 && Active() == "Rising", "the queue holds the playlist, starting at Rising");
        ctx.Check(queue[0] == queuedRising && queue[1] == queuedFalling &&
                      queue.getActiveSongProperty() == queue[0],
                  "queue-owned items survive caller-song destruction and identify the active entry");
        bool negativeRefused = false;
        bool pastEndRefused = false;
        try { static_cast<void>(queue[-1]); }
        catch (const System::ArgumentOutOfRangeException&) { negativeRefused = true; }
        try { static_cast<void>(queue[queue.getCountProperty()]); }
        catch (const System::ArgumentOutOfRangeException&) { pastEndRefused = true; }
        ctx.Check(negativeRefused && pastEndRefused && queue[0] == queuedRising && queue[1] == queuedFalling,
                  "the queue indexer rejects negative and count indices without changing its items");

        MediaPlayer::MoveNext();
        ctx.Print("MoveNext: #{} {}", queue.getActiveSongIndexProperty(), Active());
        ctx.Check(Active() == "Falling" && MediaPlayer::getStateProperty() == MediaState::Playing,
                  "MoveNext plays the next song");

        // At the end of the queue: with IsRepeating it wraps around to the first song.
        MediaPlayer::setIsRepeatingProperty(true);
        MediaPlayer::MoveNext();
        ctx.Print("MoveNext past the end, IsRepeating: #{} {}", queue.getActiveSongIndexProperty(), Active());
        ctx.Check(Active() == "Rising", "with IsRepeating the queue wraps around");

        // MovePrevious steps back; on the first song it stays there (no backward wrap).
        MediaPlayer::MovePrevious();
        ctx.Check(Active() == "Rising", "MovePrevious on the first song stays on it");
        MediaPlayer::MoveNext();
        MediaPlayer::MovePrevious();
        ctx.Print("MoveNext, MovePrevious: #{} {}", queue.getActiveSongIndexProperty(), Active());
        ctx.Check(Active() == "Rising", "MovePrevious returns to the song before");

        // IsShuffled makes the next song a random one from the queue.
        MediaPlayer::setIsShuffledProperty(true);
        ctx.Check(MediaPlayer::getIsShuffledProperty(), "IsShuffled reads back");

        MediaPlayer::setIsShuffledProperty(false);
        MediaPlayer::setIsRepeatingProperty(false);
        {
            const std::unique_ptr<Song> alternateFalling(
                Song::FromUri("Alternate Falling", TitleLocation::Path() + "/assets/music/falling.wav"));
            const std::unique_ptr<Song> alternateRising(
                Song::FromUri("Alternate Rising", TitleLocation::Path() + "/assets/music/rising.wav"));
            const SongCollection replacement({alternateFalling.get(), alternateRising.get()});

            MediaPlayer::Play(replacement, 1);
            ctx.Check(queue.getCountProperty() == 2 && queue[0]->getNameProperty() == "Alternate Falling" &&
                          queue[1]->getNameProperty() == "Alternate Rising",
                      "playing a new collection replaces rather than appends to the active queue");
            ctx.Check(queue.getActiveSongIndexProperty() == 1 && Active() == "Alternate Rising" &&
                          MediaPlayer::getStateProperty() == MediaState::Playing,
                      "the indexed collection overload starts playback at the requested nonzero entry");
        }
        ctx.Check(queue.getCountProperty() == 2 && queue.getActiveSongIndexProperty() == 1 &&
                      Active() == "Alternate Rising",
                  "the replacement queue keeps its copied entries after the caller collection is destroyed");

        MediaPlayer::Stop();
        FrameworkDispatcher::Update();
    }
}

CNA_DEMOS_CONSOLE(QueueAndRepeat, {
    .id = "media.media_player.queue_and_repeat",
    .title = "Playlists and repeat",
    .description = "Plays and replaces SongCollections from default and nonzero starting entries, walks the MediaPlayer "
                   "Queue with MoveNext and MovePrevious, and shows IsRepeating wrapping around the ends "
                   "(IsShuffled for random order).",
    .apis = {"MediaPlayer::Play", "MediaPlayer::Queue", "MediaPlayer::MoveNext", "MediaPlayer::MovePrevious",
             "MediaPlayer::IsRepeating", "MediaPlayer::IsShuffled", "MediaQueue::ActiveSong",
             "MediaQueue::ActiveSongIndex", "MediaQueue::Count", "MediaQueue::Item", "SongCollection"},
    .tags = {"media", "music", "playlist", "queue", "repeat", "shuffle"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"music/rising.wav", "music/falling.wav"},
})
