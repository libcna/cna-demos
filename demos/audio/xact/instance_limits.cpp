// SPDX-License-Identifier: MS-PL
// XACT category instance limits are authored in settings, not set on individual cues.
#include <memory>
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioStopOptions.hpp"
#include "Microsoft/Xna/Framework/Audio/Cue.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundBank.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"

using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    void InstanceLimits(CnaDemos::DemoContext& ctx)
    {
        // tools/xact_files.py authors Music with limit 1 / Fail and SFX with
        // limit 1 / ReplaceOldest, both with zero fade time. The banks are unchanged.
        AudioEngine engine("assets/xact/limits.xgs");
        WaveBank waves(&engine, "assets/xact/tones.xwb");
        SoundBank sounds(&engine, "assets/xact/tones.xsb");

        std::unique_ptr<Cue> music(sounds.GetCue("LowTone"));
        std::unique_ptr<Cue> refused(sounds.GetCue("MidTone"));
        music->Play();
        ctx.Check(music->getIsPlayingProperty(), "the first Music cue starts");
        // Pausing keeps the category slot occupied and avoids the short tone ending mid-example.
        music->Pause();
        refused->Play();
        ctx.Check(refused->getIsStoppedProperty() && music->getIsPausedProperty(),
                  "a paused Music cue still occupies its slot, so a second cue is refused");

        std::unique_ptr<Cue> older(sounds.GetCue("HighTone"));
        std::unique_ptr<Cue> newer(sounds.GetCue("Beep"));
        older->Play();
        ctx.Check(older->getIsPlayingProperty(), "SFX has its own independent instance allowance");
        older->Pause();
        ctx.Check(older->getIsPausedProperty(), "the old SFX voice remains live until replacement");
        newer->Play();
        ctx.Check(older->getIsStoppedProperty() && newer->getIsPlayingProperty(),
                  "SFX's ReplaceOldest policy stops its old cue and starts the new one");
        ctx.Check(music->getIsPausedProperty(), "replacing an SFX cue leaves the paused Music voice alone");
        ctx.Print("Music (Fail): first paused, second refused. SFX (ReplaceOldest): newest playing.");

        music->Stop(AudioStopOptions::Immediate);
        std::unique_ptr<Cue> next(sounds.GetCue("MidTone"));
        next->Play();
        ctx.Check(next->getIsPlayingProperty(), "stopping the old Music cue frees its category slot");
        // GetCue returns caller-owned objects. Dispose releases playback; unique_ptr frees them.
        music->Dispose();
        refused->Dispose();
        older->Dispose();
        newer->Dispose();
        next->Dispose();
    }
}

CNA_DEMOS_CONSOLE(InstanceLimits, {
    .id = "audio.xact.instance_limits",
    .title = "Authored XACT category instance limits",
    .description = "Uses two categories limited to one cue: Music refuses the second voice, while SFX replaces "
                   "the oldest. Checks category isolation and reusing a freed slot through public cue states.",
    .apis = {"AudioEngine::AudioEngine", "SoundBank::GetCue", "Cue::Play", "Cue::Stop",
             "Cue::IsPlaying", "Cue::IsPaused", "Cue::Pause", "Cue::IsStopped", "Cue::Dispose", "AudioStopOptions::Immediate"},
    .tags = {"audio", "xact", "category", "instance-limit", "voice"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/limits.xgs", "xact/tones.xwb", "xact/tones.xsb"},
})
