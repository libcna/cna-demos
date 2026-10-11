// SPDX-License-Identifier: MS-PL
// XACT cue variables: values each Cue keeps for itself (unlike the engine's global variables).
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioStopOptions.hpp"
#include "Microsoft/Xna/Framework/Audio/Cue.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundBank.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include "System/InvalidOperationException.hpp"

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
        Cue* first = sounds.GetCue("LowTone");
        Cue* second = sounds.GetCue("LowTone");
        ctx.Check(first->GetVariable("Intensity") == 50.0f && second->GetVariable("Intensity") == 50.0f,
                  "each new cue starts with the declared initial value, 50");

        first->SetVariable("Intensity", 80.0f);
        ctx.Print("first cue: Intensity {}, second cue: Intensity {}", first->GetVariable("Intensity"),
                  second->GetVariable("Intensity"));
        ctx.Check(first->GetVariable("Intensity") == 80.0f && second->GetVariable("Intensity") == 50.0f,
                  "setting one cue's variable leaves the other cue's alone");

        second->SetVariable("Intensity", 250.0f);
        ctx.Check(second->GetVariable("Intensity") == 100.0f, "values are clamped to the declared range (0..100)");

        // Cue variables and global variables are separate: each API sees only its own kind.
        ctx.Check(Refused([&] { (void)first->GetVariable("SpeedOfSound"); }),
                  "a cue does not see the global variable SpeedOfSound");
        ctx.Check(Refused([&] { (void)engine.GetGlobalVariable("Intensity"); }),
                  "the engine does not see the cue variable Intensity");

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
    }
}

CNA_DEMOS_CONSOLE(CueVariables, {
    .id = "audio.xact.cue_variables",
    .title = "XACT cue variables",
    .description = "Sets and reads the cue variable Intensity with Cue::SetVariable and Cue::GetVariable: each Cue "
                   "keeps its own value, clamped to the declared range, and cue and global variables are separate "
                   "namespaces.",
    .apis = {"Cue::SetVariable", "Cue::GetVariable", "AudioEngine::GetGlobalVariable", "SoundBank::GetCue"},
    .tags = {"audio", "xact", "variables", "cue"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb"},
})
