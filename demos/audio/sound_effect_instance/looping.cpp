// SPDX-License-Identifier: MS-PL
// SoundEffectInstance::IsLooped: a voice that repeats until stopped -- set before the first Play.
#include <chrono>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework::Audio;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    void Looping(CnaDemos::DemoContext& ctx)
    {
        ContentManager content(nullptr, "assets");
        const SoundEffect tone = content.Load<SoundEffect>("sounds/tone_660");  // 0.25 s
        SoundEffectInstance loop = tone.CreateInstance();
        loop.setIsLoopedProperty(true);
        loop.setVolumeProperty(0.3f);
        loop.Play();

        // XNA fixes looping once an instance has played.
        bool refused = false;
        try
        {
            loop.setIsLoopedProperty(false);
        }
        catch (const System::InvalidOperationException& e)
        {
            refused = true;
            ctx.Print("IsLooped after Play: {}", e.what());
        }
        ctx.Check(refused, "IsLooped cannot be changed after the instance has played");

        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        ctx.Check(loop.getStateProperty() == SoundState::Playing, "after almost three lengths it is still playing");
        loop.Stop();
        ctx.Check(loop.getStateProperty() == SoundState::Stopped, "Stop ends it");

        // Release a paused loop: Stop(false) removes repetition without unpausing or
        // discarding the queued sound. Keeping it paused makes this state check independent
        // of the audio callback reaching the end between Stop(false) and observation.
        loop.Play();
        loop.Pause();
        loop.Stop(false);
        ctx.Check(loop.getStateProperty() == SoundState::Paused && loop.getIsLoopedProperty(),
                  "releasing a paused loop preserves pause and its configured IsLooped property");
        loop.Resume();
        ctx.Check(loop.getStateProperty() == SoundState::Playing,
                  "a released paused loop can resume its remaining sound");
        const auto waitStart = std::chrono::steady_clock::now();
        const auto deadline = waitStart + std::chrono::seconds(3);
        while (loop.getStateProperty() != SoundState::Stopped && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ctx.Print("Released loop completed after {:.3f} s of resumed playback (3 s limit)",
                  std::chrono::duration<double>(std::chrono::steady_clock::now() - waitStart).count());
        ctx.Check(loop.getStateProperty() == SoundState::Stopped,
                  "Stop(false) lets the released sound finish instead of repeating forever");
    }
}

CNA_DEMOS_CONSOLE(Looping, {
    .id = "audio.sound_effect_instance.looping",
    .title = "Looping sound",
    .description = "Plays a 0.25 s tone with SoundEffectInstance::IsLooped set: it is still playing 0.7 s later "
                   "until Stop; changing IsLooped after Play is refused. A second pass releases a paused loop with Stop(false), then resumes it to natural completion.",
    .apis = {"SoundEffectInstance::IsLooped", "SoundEffectInstance::Volume", "SoundEffectInstance::Play",
             "SoundEffectInstance::Stop", "SoundEffectInstance::Pause", "SoundEffectInstance::Resume",
             "SoundEffectInstance::State"},
    .tags = {"audio", "loop", "music", "instance"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_660.wav"},
})
