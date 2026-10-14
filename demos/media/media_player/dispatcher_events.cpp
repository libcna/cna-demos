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
        bool canonicalEventPayloads = true;
        const auto stateToken = MediaPlayer::MediaStateChanged.Add([&](System::Object* sender, const System::EventArgs& args) {
            canonicalEventPayloads = canonicalEventPayloads && sender == nullptr && &args == &System::EventArgs::Empty;
            events.push_back(std::string("MediaStateChanged: ") +
                             (MediaPlayer::getStateProperty() == MediaState::Playing ? "Playing"
                              : MediaPlayer::getStateProperty() == MediaState::Paused ? "Paused"
                                                                                       : "Stopped"));
        });
        const auto songToken = MediaPlayer::ActiveSongChanged.Add(
            [&](System::Object* sender, const System::EventArgs& args) {
                canonicalEventPayloads = canonicalEventPayloads && sender == nullptr && &args == &System::EventArgs::Empty;
                events.push_back("ActiveSongChanged");
            });

        const std::unique_ptr<Song> song(Song::FromUri("Falling", TitleLocation::Path() + "/assets/music/falling.wav"));
        MediaPlayer::Play(song.get());

        // A Game calls FrameworkDispatcher::Update every frame. A program without one -- a tool,
        // or XNA on Windows Forms -- must call it itself: MediaPlayer's events are raised there.
        ctx.Check(events.empty(), "no event has been raised yet: they wait for FrameworkDispatcher::Update");
        FrameworkDispatcher::Update();
        const std::size_t afterPlay = events.size();
        ctx.Check(events == std::vector<std::string>{"ActiveSongChanged", "MediaStateChanged: Playing"} &&
                      canonicalEventPayloads,
                  "Play dispatches ActiveSongChanged before MediaStateChanged with canonical event payloads");

        MediaPlayer::Pause();
        MediaPlayer::Resume();
        const bool coalescedChangesStayedDeferred = events.size() == afterPlay;
        FrameworkDispatcher::Update();
        ctx.Check(coalescedChangesStayedDeferred && events.size() == afterPlay + 1 &&
                      events.back() == "MediaStateChanged: Playing",
                  "state changes before one dispatcher update coalesce into one current-state notification");

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        MediaPlayer::Pause();
        FrameworkDispatcher::Update();
        const std::size_t afterPauseEvent = events.size();
        MediaPlayer::Pause();
        FrameworkDispatcher::Update();
        const bool repeatedPauseStayedQuiet = events.size() == afterPauseEvent;
        const double pausedPosition = MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        const double pausedLater = MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty();
        ctx.Check(MediaPlayer::getStateProperty() == MediaState::Paused && pausedPosition > 0.05 &&
                      std::abs(pausedLater - pausedPosition) < 0.01,
                  "Pause freezes PlayPosition while wall-clock time continues");

        MediaPlayer::Resume();
        FrameworkDispatcher::Update();
        const std::size_t afterResumeEvent = events.size();
        MediaPlayer::Resume();
        FrameworkDispatcher::Update();
        const bool repeatedResumeStayedQuiet = events.size() == afterResumeEvent;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ctx.Check(MediaPlayer::getStateProperty() == MediaState::Playing &&
                      MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty() > pausedPosition + 0.05,
                  "Resume continues PlayPosition from the paused value");

        MediaPlayer::Stop();
        FrameworkDispatcher::Update();
        const std::size_t afterStopEvent = events.size();
        MediaPlayer::Stop();
        FrameworkDispatcher::Update();
        const bool repeatedStopStayedQuiet = events.size() == afterStopEvent;
        ctx.Check(MediaPlayer::getStateProperty() == MediaState::Stopped &&
                      MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty() == 0.0,
                  "Stop resets the state and PlayPosition");
        ctx.Check(repeatedPauseStayedQuiet && repeatedResumeStayedQuiet && repeatedStopStayedQuiet,
                  "repeated Pause, Resume and Stop calls matching the current state raise no event");

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
        const std::vector<std::string> expectedEvents{
            "ActiveSongChanged",
            "MediaStateChanged: Playing",
            "MediaStateChanged: Playing",
            "MediaStateChanged: Paused",
            "MediaStateChanged: Playing",
            "MediaStateChanged: Stopped",
            "ActiveSongChanged",
            "MediaStateChanged: Playing",
            "ActiveSongChanged",
            "MediaStateChanged: Stopped",
        };
        ctx.Check(events == expectedEvents && canonicalEventPayloads,
                  "deferred transport events preserve their exact active-song-before-state order");

        MediaPlayer::MediaStateChanged.Remove(stateToken);
        MediaPlayer::ActiveSongChanged.Remove(songToken);
        const std::size_t afterRemoval = events.size();
        MediaPlayer::Play(song.get());
        FrameworkDispatcher::Update();
        MediaPlayer::Stop();
        FrameworkDispatcher::Update();
        ctx.Check(events.size() == afterRemoval,
                  "removed event listeners receive neither later Play nor Stop notifications");
    }
}

CNA_DEMOS_CONSOLE(DispatcherEvents, {
    .id = "media.media_player.dispatcher_events",
    .title = "MediaPlayer events without a Game",
    .description = "Plays, pauses and resumes a song from a console program and shows deferred MediaStateChanged and "
                   "ActiveSongChanged ordering, coalescing and removal, while PlayPosition freezes, resumes and resets "
                   "with the transport.",
    .apis = {"FrameworkDispatcher::Update", "MediaPlayer::MediaStateChanged", "MediaPlayer::ActiveSongChanged",
             "MediaPlayer::Pause", "MediaPlayer::Resume", "MediaPlayer::Stop", "MediaPlayer::Play",
             "MediaPlayer::PlayPosition", "MediaPlayer::State", "Song::FromUri"},
    .tags = {"media", "music", "events", "dispatcher", "tools"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"music/falling.wav"},
})
