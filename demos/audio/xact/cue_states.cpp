// SPDX-License-Identifier: MS-PL
// An XACT Cue object: prepared, playing, paused, stopped -- one Cue per playback.
#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioStopOptions.hpp"
#include "Microsoft/Xna/Framework/Audio/Cue.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundBank.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include "System/EventArgs.hpp"
#include "System/ObjectDisposedException.hpp"

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

        // GetCue returns a caller-owned object: Dispose releases playback, unique_ptr deletes it.
        // PlayCue owns its fire-and-forget cues internally. Each Cue plays once.
        // Callback state precedes the cue so it also survives destruction during exception unwinding.
        int disposals = 0;
        System::Object* disposedSender = nullptr;
        std::unique_ptr<Cue> cue(sounds.GetCue("MidTone"));
        cue->Disposing.Add([&](System::Object* sender, const System::EventArgs&) {
            ++disposals;
            disposedSender = sender;
        });
        ctx.Print("{}: {}", cue->getNameProperty(), State(*cue));
        ctx.Check(!cue->getIsCreatedProperty() && !cue->getIsPreparingProperty() &&
                      cue->getIsPreparedProperty() && !cue->getIsPlayingProperty(),
                  "synchronous GetCue skips Created/Preparing and returns a prepared, idle cue");

        cue->Play();
        engine.Update();
        ctx.Print("after Play: {}", State(*cue));
        ctx.Check(cue->getIsPlayingProperty(), "Play starts it");
        cue->Play();
        ctx.Check(cue->getIsPlayingProperty() && !cue->getIsPausedProperty(),
                  "Play while already playing is a state-preserving no-op");
        cue->Pause();
        engine.Update();
        ctx.Print("after Pause: {}", State(*cue));
        ctx.Check(cue->getIsPausedProperty(), "Pause holds it");
        cue->Play();
        ctx.Check(cue->getIsPlayingProperty() && cue->getIsPausedProperty(),
                  "Play while paused does not resume or duplicate the cue");
        cue->Resume();
        engine.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        cue->Stop(AudioStopOptions::Immediate);
        engine.Update();
        ctx.Print("after Stop(Immediate): {}", State(*cue));
        ctx.Check(cue->getIsStoppedProperty(), "Stop(Immediate) stops it at once");
        cue->Play();
        ctx.Check(cue->getIsStoppedProperty() && !cue->getIsPlayingProperty(),
                  "a stopped Cue is single-use and Play cannot restart it");

        // XACT's pattern: a Cue is one playback. To play the sound again, get a new Cue.
        cue->Dispose();
        ctx.Check(cue->getIsDisposedProperty(), "Dispose releases it");
        bool disposedPlayRefused = false;
        try { cue->Play(); }
        catch (const System::ObjectDisposedException&) { disposedPlayRefused = true; }
        ctx.Check(disposedPlayRefused, "Play refuses a disposed cue with ObjectDisposedException");
        bool disposedCleanupSafe = false;
        try
        {
            cue->Pause();
            cue->Resume();
            cue->Stop(AudioStopOptions::Immediate);
            disposedCleanupSafe = true;
        }
        catch (...) {}
        ctx.Check(disposedCleanupSafe && cue->getIsDisposedProperty() && cue->getIsStoppedProperty(),
                  "Pause, Resume and Stop remain safe cleanup no-ops after disposal");
        ctx.Check(disposals == 1 && disposedSender == cue.get(), "Disposing identifies the cue being released");
        cue->Dispose();
        ctx.Check(disposals == 1, "repeated Dispose does not raise Disposing again");
        std::unique_ptr<Cue> again(sounds.GetCue("MidTone"));
        ctx.Check(again != cue && again->getIsPreparedProperty(), "GetCue returns a new, prepared cue");
        cue.reset();
        ctx.Check(disposals == 1, "deleting an already disposed cue does not repeat its notification");
        again->Play();
        engine.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        again->Stop(AudioStopOptions::AsAuthored);
        engine.Update();
        // This simple cue has no authored fade or release RPC: AsAuthored stops it immediately.
        ctx.Check(again->getIsStoppedProperty() && !again->getIsStoppingProperty(),
                  "AsAuthored stops a cue with no authored release immediately");
        again->Dispose();

        // release.xsb has a complex single-sound cue with a 500 ms fade-out, backed by
        // a three-second original tone. The fade is authored in the bank, not by this loop.
        WaveBank releaseWaves(&engine, "assets/xact/release.xwb");
        SoundBank releaseSounds(&engine, "assets/xact/release.xsb");
        std::unique_ptr<Cue> release(releaseSounds.GetCue("ReleaseTone"));
        release->Play();
        engine.Update();
        ctx.Check(release->getIsPlayingProperty(), "the authored-release cue starts playing");
        release->Stop(AudioStopOptions::AsAuthored);
        const auto started = std::chrono::steady_clock::now();
        ctx.Check(release->getIsStoppingProperty() && !release->getIsPlayingProperty() &&
                  !release->getIsStoppedProperty(), "an authored fade enters Stopping before Stopped");
        const auto deadline = started + std::chrono::seconds(3);
        while (release->getIsStoppingProperty() && std::chrono::steady_clock::now() < deadline)
        {
            engine.Update();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        ctx.Check(release->getIsStoppedProperty() && !release->getIsStoppingProperty(),
                  "the authored release reaches Stopped while the engine is pumped");
        ctx.Print("Observed authored-release state wait: {} ms (not output-sample timing)",
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - started).count());
        release->Dispose();

        std::unique_ptr<Cue> interrupted(releaseSounds.GetCue("ReleaseTone"));
        interrupted->Play();
        interrupted->Stop(AudioStopOptions::AsAuthored);
        ctx.Check(interrupted->getIsStoppingProperty(), "a fresh cue has its own authored release");
        interrupted->Stop(AudioStopOptions::Immediate);
        ctx.Check(interrupted->getIsStoppedProperty() && !interrupted->getIsStoppingProperty(),
                  "Immediate interrupts an authored release without waiting for its deadline");
        interrupted->Dispose();

        // A separate settings bank binds a volume RPC to XACT's built-in ReleaseTime variable.
        // This simple cue has no fadeOut field, so its Stopping tail comes only from the RPC's
        // final 150 ms input point rather than the authored cue fade exercised above.
        AudioEngine rpcEngine("assets/xact/release_rpc.xgs");
        WaveBank rpcWaves(&rpcEngine, "assets/xact/release.xwb");
        SoundBank rpcSounds(&rpcEngine, "assets/xact/release_rpc.xsb");
        std::unique_ptr<Cue> rpcRelease(rpcSounds.GetCue("ReleaseTimeCue"));
        rpcRelease->Play();
        rpcEngine.Update();
        ctx.Check(rpcRelease->getIsPlayingProperty(), "the ReleaseTime-RPC cue starts playing");
        rpcRelease->Stop(AudioStopOptions::AsAuthored);
        const auto rpcStarted = std::chrono::steady_clock::now();
        ctx.Check(rpcRelease->getIsStoppingProperty() && !rpcRelease->getIsStoppedProperty(),
                  "a ReleaseTime RPC gives a simple cue an observable Stopping phase");
        const auto rpcDeadline = rpcStarted + std::chrono::seconds(2);
        while (rpcRelease->getIsStoppingProperty() && std::chrono::steady_clock::now() < rpcDeadline)
        {
            rpcEngine.Update();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        ctx.Check(rpcRelease->getIsStoppedProperty() && !rpcRelease->getIsStoppingProperty(),
                  "the RPC-only release reaches Stopped while the engine is pumped");
        ctx.Print("Observed RPC-only release state wait: {} ms (not output-sample timing)",
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - rpcStarted).count());
        rpcRelease->Dispose();
    }
}

CNA_DEMOS_CONSOLE(CueStates, {
    .id = "audio.xact.cue_states",
    .title = "XACT cue states",
    .description = "Walks a one-shot Cue through Play/Pause/Resume/Stop and disposal, checks repeated transport "
                   "calls in each state, then contrasts immediate, authored-fade and ReleaseTime-RPC releases.",
    .apis = {"SoundBank::GetCue", "Cue::Play", "Cue::Pause", "Cue::Resume", "Cue::Stop", "Cue::Name",
             "Cue::IsCreated", "Cue::IsPreparing", "Cue::IsPrepared", "Cue::IsPlaying", "Cue::IsPaused",
             "Cue::IsStopping", "Cue::IsStopped", "Cue::IsDisposed", "Cue::Dispose", "Cue::Disposing",
             "AudioStopOptions"},
    .tags = {"audio", "xact", "cue", "state"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb", "xact/release.xwb", "xact/release.xsb",
               "xact/release_rpc.xgs", "xact/release_rpc.xsb"},
})
