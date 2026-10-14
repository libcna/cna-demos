// SPDX-License-Identifier: MS-PL
// MediaPlayer events outside a Game: FrameworkDispatcher::Update raises them.
#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/FrameworkDispatcher.hpp"
#include "Microsoft/Xna/Framework/Media/MediaPlayer.hpp"
#include "Microsoft/Xna/Framework/Media/MediaState.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/EventArgs.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    void DispatcherEvents(CnaDemos::DemoContext& ctx)
    {
        std::vector<std::string> events;
        const auto stateToken = MediaPlayer::MediaStateChanged.Add([&events](System::Object*, const System::EventArgs&) {
            events.push_back(std::string("MediaStateChanged: ") +
                             (MediaPlayer::getStateProperty() == MediaState::Playing ? "Playing"
                              : MediaPlayer::getStateProperty() == MediaState::Paused ? "Paused"
                                                                                       : "Stopped"));
        });
        const auto songToken = MediaPlayer::ActiveSongChanged.Add(
            [&events](System::Object*, const System::EventArgs&) { events.push_back("ActiveSongChanged"); });

        const std::unique_ptr<Song> song(Song::FromUri("Falling", TitleLocation::Path() + "/assets/music/falling.wav"));
        MediaPlayer::Play(song.get());

        // A Game calls FrameworkDispatcher::Update every frame. A program without one -- a tool,
        // or XNA on Windows Forms -- must call it itself: MediaPlayer's events are raised there.
        ctx.Check(events.empty(), "no event has been raised yet: they wait for FrameworkDispatcher::Update");
        FrameworkDispatcher::Update();
        const std::size_t afterPlay = events.size();
        ctx.Check(afterPlay >= 1, "FrameworkDispatcher::Update raises the events of Play");

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        MediaPlayer::Pause();
        FrameworkDispatcher::Update();
        const double pausedPosition = MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        const double pausedLater = MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty();
        ctx.Check(MediaPlayer::getStateProperty() == MediaState::Paused && pausedPosition > 0.05 &&
                      std::abs(pausedLater - pausedPosition) < 0.01,
                  "Pause freezes PlayPosition while wall-clock time continues");

        MediaPlayer::Resume();
        FrameworkDispatcher::Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ctx.Check(MediaPlayer::getStateProperty() == MediaState::Playing &&
                      MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty() > pausedPosition + 0.05,
                  "Resume continues PlayPosition from the paused value");

        MediaPlayer::Stop();
        FrameworkDispatcher::Update();
        ctx.Check(MediaPlayer::getStateProperty() == MediaState::Stopped &&
                      MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty() == 0.0,
                  "Stop resets the state and PlayPosition");

        MediaPlayer::Play(song.get());
        FrameworkDispatcher::Update();
        // The song ends after 2 s; keep pumping, as a game loop would.
        for (int i = 0; i < 300 && MediaPlayer::getStateProperty() != MediaState::Stopped; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            FrameworkDispatcher::Update();
        }
        for (const std::string& event : events)
            ctx.Print("{}", event);
        ctx.Check(MediaPlayer::getStateProperty() == MediaState::Stopped, "the song played to its end");
        ctx.Check(events.size() >= afterPlay + 3, "Pause, Resume and the end each raised MediaStateChanged");

        MediaPlayer::MediaStateChanged.Remove(stateToken);
        MediaPlayer::ActiveSongChanged.Remove(songToken);
    }
}

CNA_DEMOS_CONSOLE(DispatcherEvents, {
    .id = "media.media_player.dispatcher_events",
    .title = "MediaPlayer events without a Game",
    .description = "Plays, pauses and resumes a song from a console program and shows that MediaStateChanged and "
                   "ActiveSongChanged are raised only by FrameworkDispatcher::Update, while PlayPosition freezes, "
                   "resumes and resets with the transport.",
    .apis = {"FrameworkDispatcher::Update", "MediaPlayer::MediaStateChanged", "MediaPlayer::ActiveSongChanged",
             "MediaPlayer::Pause", "MediaPlayer::Resume", "MediaPlayer::Stop", "MediaPlayer::Play",
             "MediaPlayer::PlayPosition", "MediaPlayer::State", "Song::FromUri"},
    .tags = {"media", "music", "events", "dispatcher", "tools"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"music/falling.wav"},
})
