// SPDX-License-Identifier: MS-PL
// MediaPlayer::Volume and IsMuted: music loudness, separate from SoundEffect::MasterVolume.
#include <chrono>
#include <memory>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/FrameworkDispatcher.hpp"
#include "Microsoft/Xna/Framework/Media/MediaPlayer.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Media;
using Microsoft::Xna::Framework::Audio::SoundEffect;

namespace
{
    void VolumeAndMute(CnaDemos::DemoContext& ctx)
    {
        const std::unique_ptr<Song> song(Song::FromUri("Rising", TitleLocation::Path() + "/assets/music/rising.wav"));
        const float masterBefore = SoundEffect::getMasterVolumeProperty();

        // Music has its own volume, 0 (silent) to 1 (full); typically an options-menu slider.
        MediaPlayer::setVolumeProperty(-0.25f);
        const float belowRange = MediaPlayer::getVolumeProperty();
        MediaPlayer::setVolumeProperty(1.25f);
        const float aboveRange = MediaPlayer::getVolumeProperty();
        ctx.Check(belowRange == 0.0f && aboveRange == 1.0f,
                  "Volume clamps settings below zero and above one to its public range");
        MediaPlayer::setVolumeProperty(0.5f);
        MediaPlayer::Play(song.get());
        ctx.Check(MediaPlayer::getVolumeProperty() == 0.5f, "Volume reads back 0.5");
        ctx.Check(SoundEffect::getMasterVolumeProperty() == masterBefore, "it does not touch SoundEffect::MasterVolume");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Muting keeps the volume setting: unmuting returns to it.
        MediaPlayer::setIsMutedProperty(true);
        ctx.Check(MediaPlayer::getIsMutedProperty() && MediaPlayer::getVolumeProperty() == 0.5f,
                  "IsMuted reads true without changing the stored Volume value");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        MediaPlayer::setIsMutedProperty(false);
        MediaPlayer::setVolumeProperty(1.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        MediaPlayer::Stop();
        FrameworkDispatcher::Update();
    }
}

CNA_DEMOS_CONSOLE(VolumeAndMute, {
    .id = "media.media_player.volume_and_mute",
    .title = "Music volume and mute",
    .description = "Clamps MediaPlayer::Volume to its 0..1 range, plays at 0.5, preserves that setting while IsMuted "
                   "is true, and keeps SoundEffect::MasterVolume independent.",
    .apis = {"MediaPlayer::Volume", "MediaPlayer::IsMuted", "SoundEffect::MasterVolume"},
    .tags = {"media", "music", "volume", "mute", "options"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"music/rising.wav"},
})
