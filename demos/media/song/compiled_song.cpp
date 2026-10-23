// SPDX-License-Identifier: MS-PL
// A Song from the content pipeline: an Ogg Vorbis file compiled by the SongProcessor, as XNA did.
#include <chrono>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/FrameworkDispatcher.hpp"
#include "Microsoft/Xna/Framework/Media/MediaPlayer.hpp"
#include "Microsoft/Xna/Framework/Media/MediaState.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Media;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    void CompiledSong(CnaDemos::DemoContext& ctx)
    {
        // The media content project (content_media/) is built into assets/media; a ContentManager
        // rooted there loads it under the names it was compiled with.
        ContentManager content(nullptr, "assets/media");
        // content_media/music/theme.ogg went through CNA.SongProcessor at build time: the compiled
        // asset holds the song's metadata (its duration was measured then) and the audio stays
        // a compressed stream beside it, read while playing -- XNA's .xnb + .wma pair.
        Song theme = content.Load<Song>("music/theme");
        ctx.Print("Name \"{}\", Duration {} ms", theme.getNameProperty(), theme.getDurationProperty().getTotalMillisecondsProperty());
        ctx.Check(theme.getDurationProperty().getTotalSecondsProperty() > 3.9 &&
                      theme.getDurationProperty().getTotalSecondsProperty() < 4.1,
                  "the duration measured at build time is the tune's four seconds");
        // As in XNA, the stream is found beside the compiled asset wherever the build sits below
        // the manager's root (CNA-DEMOS-ISSUE-020).
        ContentManager parent(nullptr, "assets");
        const Song relocated = parent.Load<Song>("media/music/theme");
        ctx.Check(relocated.getDurationProperty() == theme.getDurationProperty(),
                  "a manager rooted above the media build loads the same song");

        MediaPlayer::Play(&theme);
        FrameworkDispatcher::Update();
        ctx.Check(MediaPlayer::getStateProperty() == MediaState::Playing, "it plays like a song from a file");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        ctx.Check(MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty() > 0.5, "PlayPosition advances");
        MediaPlayer::Stop();
        FrameworkDispatcher::Update();
    }
}

CNA_DEMOS_CONSOLE(CompiledSong, {
    .id = "media.song.compiled_song",
    .title = "A compiled song",
    .description = "Loads an Ogg Vorbis tune compiled by the content pipeline's SongProcessor as a Song (its duration "
                   "measured at build time) and plays it with MediaPlayer for a moment.",
    .apis = {"ContentManager::Load", "Song::Duration", "Song::Name", "MediaPlayer::Play", "MediaPlayer::State",
             "MediaPlayer::PlayPosition"},
    .tags = {"media", "music", "song", "content", "ogg"},
    .needs = CnaDemos::Need::Audio | CnaDemos::Need::Video,
    .assets = {"media/music/theme.cnb", "media/music/theme.ogg"},
    .restrictions = "Needs CNA's FFmpeg backend (Need::Video) only because the song is compiled with it.",
})
