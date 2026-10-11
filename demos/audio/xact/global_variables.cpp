// SPDX-License-Identifier: MS-PL
// XACT global variables: values the sound designer's settings define and the game can set.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"

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
    }
}

CNA_DEMOS_CONSOLE(GlobalVariables, {
    .id = "audio.xact.global_variables",
    .title = "XACT global variables",
    .description = "Reads and sets the XACT global variable SpeedOfSound with AudioEngine::GetGlobalVariable and "
                   "SetGlobalVariable, including clamping to the range the settings file declares.",
    .apis = {"AudioEngine::GetGlobalVariable", "AudioEngine::SetGlobalVariable"},
    .tags = {"audio", "xact", "variables", "doppler"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs"},
})
