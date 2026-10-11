// SPDX-License-Identifier: MS-PL
// XACT 3D audio: play a cue at a position relative to the listener.
#include <chrono>
#include <cmath>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEmitter.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioListener.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundBank.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    void PositionalCue(CnaDemos::DemoContext& ctx)
    {
        AudioEngine engine("assets/xact/demo.xgs");
        WaveBank waves(&engine, "assets/xact/tones.xwb");
        SoundBank sounds(&engine, "assets/xact/tones.xsb");

        // The listener stands at the origin looking along -Z; a beep goes round it once.
        AudioListener listener;
        AudioEmitter emitter;
        for (int step = 0; step < 8; ++step)
        {
            const float angle = MathHelper::TwoPi * static_cast<float>(step) / 8.0f;
            emitter.setPositionProperty(Vector3(std::sin(angle) * 5.0f, 0.0f, -std::cos(angle) * 5.0f));
            ctx.Print("beep at {:3.0f} degrees (x {:+.1f}, z {:+.1f})", MathHelper::ToDegrees(angle),
                      emitter.getPositionProperty().X, emitter.getPositionProperty().Z);
            // The listener/emitter overload positions a fire-and-forget cue; Cue::Apply3D does
            // the same for a Cue the game keeps.
            sounds.PlayCue("Beep", listener, emitter);
            for (int t = 0; t < 30; ++t)
            {
                engine.Update();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
}

CNA_DEMOS_CONSOLE(PositionalCue, {
    .id = "audio.xact.positional_cue",
    .title = "Positioned XACT cues",
    .description = "Plays an XACT cue eight times with SoundBank::PlayCue(name, listener, emitter) while the "
                   "AudioEmitter goes round the AudioListener; hearing the beep move is the check.",
    .apis = {"SoundBank::PlayCue", "AudioListener::AudioListener", "AudioEmitter::Position"},
    .tags = {"audio", "xact", "3d", "positional", "panning"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb"},
})
