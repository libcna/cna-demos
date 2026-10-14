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

        // A cue definition can impose its own limit, independently of category capacity.
        // demo.xgs leaves categories unlimited; cue_limits.xsb limits LowTone to one / Fail.
        AudioEngine cueEngine("assets/xact/demo.xgs");
        WaveBank cueWaves(&cueEngine, "assets/xact/tones.xwb");
        SoundBank cueSounds(&cueEngine, "assets/xact/cue_limits.xsb");
        std::unique_ptr<Cue> first(cueSounds.GetCue("LowTone"));
        first->Play();
        first->Pause();
        ctx.Check(first->getIsPausedProperty(), "the first instance occupies its authored cue-definition slot");
        std::unique_ptr<Cue> duplicate(cueSounds.GetCue("LowTone"));
        duplicate->Play();
        ctx.Check(duplicate->getIsStoppedProperty() && first->getIsPausedProperty(),
                  "the same definition refuses a second instance even with spare category capacity");
        std::unique_ptr<Cue> otherName(cueSounds.GetCue("MidTone"));
        otherName->Play();
        otherName->Pause();
        ctx.Check(otherName->getIsPausedProperty() && first->getIsPausedProperty(),
                  "another definition in the same Music category has independent capacity");
        SoundBank otherBank(&cueEngine, "assets/xact/cue_limits.xsb");
        std::unique_ptr<Cue> otherCopy(otherBank.GetCue("LowTone"));
        otherCopy->Play();
        otherCopy->Pause();
        ctx.Check(otherCopy->getIsPausedProperty() && first->getIsPausedProperty(),
                  "loading the same sound-bank file again creates a separate cue-definition allowance");
        first->Stop(AudioStopOptions::Immediate);
        std::unique_ptr<Cue> replacement(cueSounds.GetCue("LowTone"));
        replacement->Play();
        replacement->Pause();
        ctx.Check(replacement->getIsPausedProperty() && otherName->getIsPausedProperty() &&
                  otherCopy->getIsPausedProperty(),
                  "stopping frees that definition's slot without disturbing other definitions or banks");
        first->Dispose();
        duplicate->Dispose();
        otherName->Dispose();
        otherCopy->Dispose();
        replacement->Dispose();

        // FACT counts a cue-level limit by definition, but ReplaceOldest searches the whole
        // SoundBank for its victim. cue_replace.xsb gives MidTone a one/ReplaceOldest limit.
        AudioEngine replaceEngine("assets/xact/demo.xgs");
        WaveBank replaceWaves(&replaceEngine, "assets/xact/tones.xwb");
        SoundBank replaceSounds(&replaceEngine, "assets/xact/cue_replace.xsb");
        std::unique_ptr<Cue> bankOldest(replaceSounds.GetCue("LowTone"));
        std::unique_ptr<Cue> triggerA(replaceSounds.GetCue("MidTone"));
        bankOldest->Play();
        bankOldest->Pause();
        triggerA->Play();
        triggerA->Pause();
        ctx.Check(bankOldest->getIsPausedProperty() && triggerA->getIsPausedProperty(),
                  "an unrelated bank cue does not consume MidTone's definition-local slot");
        std::unique_ptr<Cue> triggerB(replaceSounds.GetCue("MidTone"));
        triggerB->Play();
        ctx.Check(bankOldest->getIsStoppedProperty(),
                  "cue-level ReplaceOldest evicts the oldest playing cue bank-wide");
        ctx.Check(triggerA->getIsPausedProperty() && triggerB->getIsPlayingProperty(),
                  "the older same-definition cue survives while the replacement starts");
        bankOldest->Dispose();
        triggerA->Dispose();
        triggerB->Dispose();
    }
}

CNA_DEMOS_CONSOLE(InstanceLimits, {
    .id = "audio.xact.instance_limits",
    .title = "Authored XACT category and cue instance limits",
    .description = "Contrasts category and cue-definition limits, including Fail, category ReplaceOldest and FACT's "
                   "bank-wide victim search for cue-level ReplaceOldest.",
    .apis = {"AudioEngine::AudioEngine", "SoundBank::GetCue", "Cue::Play", "Cue::Stop",
             "Cue::IsPlaying", "Cue::IsPaused", "Cue::Pause", "Cue::IsStopped", "Cue::Dispose", "AudioStopOptions::Immediate"},
    .tags = {"audio", "xact", "category", "instance-limit", "voice"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/limits.xgs", "xact/tones.xwb", "xact/tones.xsb", "xact/demo.xgs", "xact/cue_limits.xsb",
               "xact/cue_replace.xsb"},
})
