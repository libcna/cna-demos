// SPDX-License-Identifier: MS-PL
// SoundEffect::Play(volume, pitch, pan) and the master volume: fire-and-forget sounds with settings.
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    void PlayParameters(CnaDemos::DemoContext& ctx)
    {
        ContentManager content(nullptr, "assets");
        SoundEffect tone = content.Load<SoundEffect>("sounds/tone_440");

        // volume 0..1, pitch -1..1 (one octave down to one octave up), pan -1 (left) .. 1 (right).
        ctx.Check(tone.Play(1.0f, 0.0f, -1.0f), "full volume, original pitch, left");
        ctx.Check(tone.Play(0.5f, 1.0f, 0.0f), "half volume, an octave up, centre");
        ctx.Check(tone.Play(0.25f, -1.0f, 1.0f), "quarter volume, an octave down, right");

        // MasterVolume scales every SoundEffect in the program (1 by default).
        const float previous = SoundEffect::getMasterVolumeProperty();
        ctx.Check(previous == 1.0f, "MasterVolume starts at 1");
        SoundEffect::setMasterVolumeProperty(0.5f);
        ctx.Check(SoundEffect::getMasterVolumeProperty() == 0.5f, "MasterVolume can be lowered for all sounds");
        SoundEffect::setMasterVolumeProperty(previous);
    }
}

CNA_DEMOS_CONSOLE(PlayParameters, {
    .id = "audio.sound_effect.play_parameters",
    .title = "Play with volume, pitch and pan",
    .description = "Plays one sound three times with SoundEffect::Play(volume, pitch, pan) -- left, centre and "
                   "right, at three pitches -- and changes SoundEffect::MasterVolume.",
    .apis = {"SoundEffect::Play", "SoundEffect::MasterVolume", "ContentManager::ContentManager"},
    .tags = {"audio", "volume", "pitch", "pan"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_440.wav"},
})
