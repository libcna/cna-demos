// SPDX-License-Identifier: MS-PL
// SoundEffectInstance: a controllable voice -- Play, Pause, Resume, Stop and its State.
#include <chrono>
#include <string>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"

using namespace Microsoft::Xna::Framework::Audio;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    const char* Name(SoundState state)
    {
        switch (state)
        {
        case SoundState::Playing: return "Playing";
        case SoundState::Paused: return "Paused";
        case SoundState::Stopped: return "Stopped";
        }
        return "?";
    }

    void PlaybackState(CnaDemos::DemoContext& ctx)
    {
        ContentManager content(nullptr, "assets");
        const SoundEffect tone = content.Load<SoundEffect>("sounds/tone_660");  // 0.25 s
        SoundEffectInstance voice = tone.CreateInstance();

        const auto expect = [&](SoundState wanted, const char* after) {
            ctx.Print("{:<16} -> {}", after, Name(voice.getStateProperty()));
            ctx.Check(voice.getStateProperty() == wanted, std::string(after) + " leaves it " + Name(wanted));
        };
        expect(SoundState::Stopped, "CreateInstance");
        voice.Play();
        expect(SoundState::Playing, "Play");
        voice.Pause();
        expect(SoundState::Paused, "Pause");
        voice.Resume();
        expect(SoundState::Playing, "Resume");
        voice.Stop();
        expect(SoundState::Stopped, "Stop");

        // A non-looping voice stops by itself at the end of the sound.
        voice.Play();
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        expect(SoundState::Stopped, "0.6 s after Play");
    }
}

CNA_DEMOS_CONSOLE(PlaybackState, {
    .id = "audio.sound_effect_instance.playback_state",
    .title = "Control a sound instance",
    .description = "Creates a SoundEffectInstance and steps it through Play, Pause, Resume and Stop, checking "
                   "SoundState after each; a non-looping instance stops by itself at the end.",
    .apis = {"SoundEffect::CreateInstance", "SoundEffectInstance::Play", "SoundEffectInstance::Pause",
             "SoundEffectInstance::Resume", "SoundEffectInstance::Stop", "SoundEffectInstance::State",
             "SoundState::Playing", "SoundState::Paused", "SoundState::Stopped"},
    .tags = {"audio", "instance", "state"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_660.wav"},
})
