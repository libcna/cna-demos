// SPDX-License-Identifier: MS-PL
// A WAV file compiled by the content pipeline into a SoundEffect asset, next to a loose WAV.
#include <chrono>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework::Audio;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    void CompiledWav(CnaDemos::DemoContext& ctx)
    {
        ContentManager content(nullptr, "assets");
        // content/sounds/chime.wav went through CNA.SoundEffectProcessor at build time
        // (sounds/chime.cnb); assets/sounds/tone_660.wav is read as it is. The game code is the same.
        SoundEffect chime = content.Load<SoundEffect>("sounds/chime");
        SoundEffect tone = content.Load<SoundEffect>("sounds/tone_660");
        ctx.Print("chime: {} ms, tone_660: {} ms", chime.getDurationProperty().getTotalMillisecondsProperty(),
                  tone.getDurationProperty().getTotalMillisecondsProperty());
        ctx.CheckNear(chime.getDurationProperty().getTotalSecondsProperty(), 0.3, 0.001,
                      "the compiled chime keeps its 0.3 s length");
        ctx.CheckNear(tone.getDurationProperty().getTotalSecondsProperty(), 0.25, 0.001, "the loose tone is 0.25 s");

        chime.Play();
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
        tone.Play();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

CNA_DEMOS_CONSOLE(CompiledWav, {
    .id = "content.sound_effect.compiled_wav",
    .title = "Compiled sound effect",
    .description = "Loads a WAV compiled by the content pipeline (CNA.SoundEffectProcessor, .cnb) and a loose WAV with "
                   "the same ContentManager::Load<SoundEffect>, checks their durations and plays both.",
    .apis = {"ContentManager::Load", "SoundEffect::Duration", "SoundEffect::Play"},
    .tags = {"content", "pipeline", "audio", "wav", "sound"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/chime.cnb", "sounds/tone_660.wav"},
})
