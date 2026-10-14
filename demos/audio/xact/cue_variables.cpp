// SPDX-License-Identifier: MS-PL
// XACT cue variables: values each Cue keeps for itself (unlike the engine's global variables).
#include <memory>
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioStopOptions.hpp"
#include "Microsoft/Xna/Framework/Audio/Cue.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundBank.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    // Whether calling f is refused, as XNA refuses a variable name of the wrong kind.
    template <typename F>
    bool Refused(F&& f)
    {
        try
        {
            f();
        }
        catch (const System::InvalidOperationException&)
        {
            return true;
        }
        return false;
    }

    void CueVariables(CnaDemos::DemoContext& ctx)
    {
        AudioEngine engine("assets/xact/demo.xgs");
        WaveBank waves(&engine, "assets/xact/tones.xwb");
        SoundBank sounds(&engine, "assets/xact/tones.xsb");

        // demo.xgs declares Intensity as a cue variable (0..100, initially 50): every Cue gets
        // its own copy -- two engine sounds playing at different revs, say. The sound designer
        // drives volume or pitch from it with RPC curves.
        std::unique_ptr<Cue> first(sounds.GetCue("LowTone"));
        std::unique_ptr<Cue> second(sounds.GetCue("LowTone"));
        ctx.Check(first->GetVariable("Intensity") == 50.0f && second->GetVariable("Intensity") == 50.0f,
                  "each new cue starts with the declared initial value, 50");

        first->SetVariable("Intensity", 80.0f);
        ctx.Print("first cue: Intensity {}, second cue: Intensity {}", first->GetVariable("Intensity"),
                  second->GetVariable("Intensity"));
        ctx.Check(first->GetVariable("Intensity") == 80.0f && second->GetVariable("Intensity") == 50.0f,
                  "setting one cue's variable leaves the other cue's alone");

        second->SetVariable("Intensity", 250.0f);
        ctx.Check(second->GetVariable("Intensity") == 100.0f, "values are clamped to the declared range (0..100)");
        second->SetVariable("Intensity", -25.0f);
        ctx.Check(second->GetVariable("Intensity") == 0.0f, "cue variables also clamp at the declared lower bound");

        // Cue variables and global variables are separate: each API sees only its own kind.
        ctx.Check(Refused([&] { (void)first->GetVariable("SpeedOfSound"); }),
                  "a cue does not see the global variable SpeedOfSound");
        ctx.Check(Refused([&] { (void)engine.GetGlobalVariable("Intensity"); }),
                  "the engine does not see the cue variable Intensity");
        ctx.Check(Refused([&] { first->SetVariable("SpeedOfSound", 500.0f); }),
                  "a cue cannot write the engine-global namespace");
        ctx.Check(Refused([&] { engine.SetGlobalVariable("Intensity", 25.0f); }),
                  "the engine cannot write the cue-local namespace");

        bool emptyGetRefused = false;
        bool emptySetRefused = false;
        try { static_cast<void>(first->GetVariable("")); }
        catch (const System::ArgumentNullException&) { emptyGetRefused = true; }
        try { first->SetVariable("", 1.0f); }
        catch (const System::ArgumentNullException&) { emptySetRefused = true; }
        ctx.Check(emptyGetRefused && emptySetRefused,
                  "both cue-variable access directions reject an empty name");
        ctx.Check(Refused([&] { static_cast<void>(first->GetVariable("MissingVariable")); }) &&
                      Refused([&] { first->SetVariable("MissingVariable", 1.0f); }),
                  "both cue-variable access directions reject an unknown name");

        // Variables can change while a cue plays.
        first->Play();
        engine.Update();
        first->SetVariable("Intensity", 20.0f);
        engine.Update();
        ctx.Check(first->getIsPlayingProperty() && first->GetVariable("Intensity") == 20.0f,
                  "a playing cue takes new values");
        first->Stop(AudioStopOptions::Immediate);
        engine.Update();
        first->Dispose();
        second->Dispose();
        bool disposedGetRefused = false;
        bool disposedSetRefused = false;
        try { static_cast<void>(first->GetVariable("Intensity")); }
        catch (const System::ObjectDisposedException&) { disposedGetRefused = true; }
        try { first->SetVariable("Intensity", 50.0f); }
        catch (const System::ObjectDisposedException&) { disposedSetRefused = true; }
        ctx.Check(disposedGetRefused && disposedSetRefused,
                  "a disposed cue refuses both variable access directions");
    }
}

CNA_DEMOS_CONSOLE(CueVariables, {
    .id = "audio.xact.cue_variables",
    .title = "XACT cue variables",
    .description = "Reads and writes per-cue Intensity values with clamping, validates cue/global namespace and name "
                   "boundaries, changes a live cue, and checks both access directions after disposal.",
    .apis = {"Cue::SetVariable", "Cue::GetVariable", "AudioEngine::GetGlobalVariable", "SoundBank::GetCue",
             "Cue::Play"},
    .tags = {"audio", "xact", "variables", "cue"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb"},
})
