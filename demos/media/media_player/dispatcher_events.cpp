// SPDX-License-Identifier: MS-PL
// MediaPlayer events outside a Game: FrameworkDispatcher::Update raises them.
#include <chrono>
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

        MediaPlayer::Pause();
        FrameworkDispatcher::Update();
        MediaPlayer::Resume();
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
                   "ActiveSongChanged are raised only by FrameworkDispatcher::Update, which a Game calls every frame.",
    .apis = {"FrameworkDispatcher::Update", "MediaPlayer::MediaStateChanged", "MediaPlayer::ActiveSongChanged",
             "MediaPlayer::Pause", "MediaPlayer::Resume", "MediaPlayer::Play", "Song::FromUri"},
    .tags = {"media", "music", "events", "dispatcher", "tools"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"music/falling.wav"},
})
