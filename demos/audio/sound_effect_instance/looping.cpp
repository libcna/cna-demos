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
    }
}

CNA_DEMOS_CONSOLE(Looping, {
    .id = "audio.sound_effect_instance.looping",
    .title = "Looping sound",
    .description = "Plays a 0.25 s tone with SoundEffectInstance::IsLooped set: it is still playing 0.7 s later "
                   "until Stop; changing IsLooped after Play is refused.",
    .apis = {"SoundEffectInstance::IsLooped", "SoundEffectInstance::Volume", "SoundEffectInstance::Play",
             "SoundEffectInstance::Stop"},
    .tags = {"audio", "loop", "music", "instance"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_660.wav"},
})
