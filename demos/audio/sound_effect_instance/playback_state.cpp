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
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

using namespace Microsoft::Xna::Framework::Audio;
using Microsoft::Xna::Framework::Content::ContentManager;
using Microsoft::Xna::Framework::TitleLocation;

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
        voice.Pause();
        expect(SoundState::Stopped, "Pause before Play");
        voice.Resume();
        expect(SoundState::Playing, "Resume before Play");
        voice.Stop();
        voice.Pause();
        voice.Resume();
        expect(SoundState::Stopped, "Pause/Resume after Stop");

        voice.Play();
        expect(SoundState::Playing, "Play");
        voice.Pause();
        expect(SoundState::Paused, "Pause");
        voice.Resume();
        expect(SoundState::Playing, "Resume");
        voice.Stop();
        expect(SoundState::Stopped, "Stop");

        const SoundEffect longTone(TitleLocation::Path() + "/assets/music/rising.wav");
        SoundEffectInstance replayProbe = longTone.CreateInstance();
        // Half speed makes this two-second asset play for about four seconds.  Observe the voice
        // at its midpoint, call Play again, then give the original playback three seconds to
        // finish.  A restarted voice would still need another second at that deadline.  The
        // wider window also tolerates a delayed dummy-audio callback under a parallel CTest run.
        replayProbe.setPitchProperty(-1.0f);
        replayProbe.Play();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        ctx.Check(replayProbe.getStateProperty() == SoundState::Playing,
                  "the replay probe is still Playing before the repeated Play call");
        replayProbe.Play();
        const auto replayDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (replayProbe.getStateProperty() != SoundState::Stopped &&
               std::chrono::steady_clock::now() < replayDeadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ctx.Check(replayProbe.getStateProperty() == SoundState::Stopped,
                  "Play while already Playing leaves progress intact instead of restarting the track");

        // A non-looping voice stops by itself at the end of the sound.
        voice.Play();
        const auto waitStart = std::chrono::steady_clock::now();
        const auto deadline = waitStart + std::chrono::seconds(3);
        while (voice.getStateProperty() != SoundState::Stopped && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ctx.Print("waited {:.3f} s for natural completion (3 s limit)",
                  std::chrono::duration<double>(std::chrono::steady_clock::now() - waitStart).count());
        expect(SoundState::Stopped, "natural completion");
    }
}

CNA_DEMOS_CONSOLE(PlaybackState, {
    .id = "audio.sound_effect_instance.playback_state",
    .title = "Control a sound instance",
    .description = "Exercises SoundEffectInstance transport before first playback, after Stop and while already "
                   "Playing, then steps through Play, Pause, Resume and natural completion with exact SoundState.",
    .apis = {"SoundEffect::CreateInstance", "SoundEffectInstance::Play", "SoundEffectInstance::Pause",
             "SoundEffectInstance::Resume", "SoundEffectInstance::Stop", "SoundEffectInstance::State",
             "SoundState::Playing", "SoundState::Paused", "SoundState::Stopped"},
    .tags = {"audio", "instance", "state"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_660.wav", "music/rising.wav"},
})
