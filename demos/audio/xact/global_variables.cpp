// SPDX-License-Identifier: MS-PL
// XACT global variables: values the sound designer's settings define and the game can set.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    void GlobalVariables(CnaDemos::DemoContext& ctx)
    {
        AudioEngine engine("assets/xact/demo.xgs");

        // SpeedOfSound drives XACT's 3D Doppler effect; demo.xgs defines it as public, 0..1000.
        ctx.Print("SpeedOfSound starts at {}", engine.GetGlobalVariable("SpeedOfSound"));
        ctx.CheckNear(engine.GetGlobalVariable("SpeedOfSound"), 343.0, 1e-3, "the initial value comes from the settings file");

        engine.SetGlobalVariable("SpeedOfSound", 500.0f);  // e.g. a slow-motion effect
        ctx.CheckNear(engine.GetGlobalVariable("SpeedOfSound"), 500.0, 1e-3, "SetGlobalVariable changes it");

        engine.SetGlobalVariable("SpeedOfSound", 5000.0f);
        ctx.Print("after setting 5000: {}", engine.GetGlobalVariable("SpeedOfSound"));
        ctx.CheckNear(engine.GetGlobalVariable("SpeedOfSound"), 1000.0, 1e-3, "values are clamped to the declared range");

        engine.SetGlobalVariable("SpeedOfSound", -500.0f);
        ctx.CheckNear(engine.GetGlobalVariable("SpeedOfSound"), 0.0, 1e-3,
                      "values also clamp at the declared lower bound");

        bool emptyGetRefused = false;
        bool emptySetRefused = false;
        bool unknownGetRefused = false;
        bool unknownSetRefused = false;
        try { static_cast<void>(engine.GetGlobalVariable("")); }
        catch (const System::ArgumentNullException&) { emptyGetRefused = true; }
        try { engine.SetGlobalVariable("", 1.0f); }
        catch (const System::ArgumentNullException&) { emptySetRefused = true; }
        try { static_cast<void>(engine.GetGlobalVariable("MissingVariable")); }
        catch (const System::InvalidOperationException&) { unknownGetRefused = true; }
        try { engine.SetGlobalVariable("MissingVariable", 1.0f); }
        catch (const System::InvalidOperationException&) { unknownSetRefused = true; }
        ctx.Check(emptyGetRefused && emptySetRefused,
                  "both global-variable access directions reject an empty name");
        ctx.Check(unknownGetRefused && unknownSetRefused,
                  "both global-variable access directions reject an unknown name");
    }
}

CNA_DEMOS_CONSOLE(GlobalVariables, {
    .id = "audio.xact.global_variables",
    .title = "XACT global variables",
    .description = "Reads and writes XACT SpeedOfSound with both clamp bounds, then checks empty and unknown names "
                   "through AudioEngine::GetGlobalVariable and SetGlobalVariable.",
    .apis = {"AudioEngine::GetGlobalVariable", "AudioEngine::SetGlobalVariable"},
    .tags = {"audio", "xact", "variables", "doppler"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs"},
})
