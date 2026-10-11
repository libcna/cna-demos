// SPDX-License-Identifier: MS-PL
// An XACT Cue object: prepared, playing, paused, stopped -- one Cue per playback.
#include <chrono>
#include <string>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioStopOptions.hpp"
#include "Microsoft/Xna/Framework/Audio/Cue.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundBank.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"

using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    std::string State(const Cue& cue)
    {
        return std::string(cue.getIsPreparedProperty() ? " prepared" : "") + (cue.getIsPlayingProperty() ? " playing" : "") +
               (cue.getIsPausedProperty() ? " paused" : "") + (cue.getIsStoppingProperty() ? " stopping" : "") +
               (cue.getIsStoppedProperty() ? " stopped" : "");
    }

    void CueStates(CnaDemos::DemoContext& ctx)
    {
        AudioEngine engine("assets/xact/demo.xgs");
        WaveBank waves(&engine, "assets/xact/tones.xwb");
        SoundBank sounds(&engine, "assets/xact/tones.xsb");

        // GetCue gives a Cue to control (PlayCue only starts one). Each Cue plays once.
        Cue* cue = sounds.GetCue("MidTone");
        ctx.Print("{}: {}", cue->getNameProperty(), State(*cue));
        ctx.Check(cue->getIsPreparedProperty() && !cue->getIsPlayingProperty(), "a new cue is prepared, not playing");

        cue->Play();
        engine.Update();
        ctx.Print("after Play: {}", State(*cue));
        ctx.Check(cue->getIsPlayingProperty(), "Play starts it");
        cue->Pause();
        engine.Update();
        ctx.Print("after Pause: {}", State(*cue));
        ctx.Check(cue->getIsPausedProperty(), "Pause holds it");
        cue->Resume();
        engine.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        cue->Stop(AudioStopOptions::Immediate);
        engine.Update();
        ctx.Print("after Stop(Immediate): {}", State(*cue));
        ctx.Check(cue->getIsStoppedProperty(), "Stop(Immediate) stops it at once (AsAuthored would let it fade)");

        // XACT's pattern: a Cue is one playback. To play the sound again, get a new Cue.
        cue->Dispose();
        ctx.Check(cue->getIsDisposedProperty(), "Dispose releases it");
        Cue* again = sounds.GetCue("MidTone");
        ctx.Check(again != cue && again->getIsPreparedProperty(), "GetCue returns a new, prepared cue");
        again->Play();
        engine.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        again->Stop(AudioStopOptions::AsAuthored);
        engine.Update();
        again->Dispose();
    }
}

CNA_DEMOS_CONSOLE(CueStates, {
    .id = "audio.xact.cue_states",
    .title = "XACT cue states",
    .description = "Gets a Cue with SoundBank::GetCue and walks it through Play, Pause, Resume and Stop, printing "
                   "IsPrepared/IsPlaying/IsPaused/IsStopped, then gets a fresh Cue to play the sound again.",
    .apis = {"SoundBank::GetCue", "Cue::Play", "Cue::Pause", "Cue::Resume", "Cue::Stop", "Cue::Name",
             "Cue::IsPrepared", "Cue::IsPlaying", "Cue::IsPaused", "Cue::IsStopping", "Cue::IsStopped",
             "Cue::IsDisposed", "Cue::Dispose", "AudioStopOptions"},
    .tags = {"audio", "xact", "cue", "state"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb"},
})
