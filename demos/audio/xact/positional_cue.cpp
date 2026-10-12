// SPDX-License-Identifier: MS-PL
// XACT 3D audio: play a cue at a position relative to the listener.
#include <chrono>
#include <cmath>
#include <memory>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEmitter.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioStopOptions.hpp"
#include "Microsoft/Xna/Framework/Audio/Cue.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
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

        // A caller-owned cue lets the game update positioning. Its public spatial variables
        // expose the last Apply3D calculation; they are not audio-output measurements.
        {
            AudioListener fixedListener;
            fixedListener.setPositionProperty(Vector3::Zero);
            fixedListener.setVelocityProperty(Vector3::Zero);
            AudioEmitter movingEmitter;
            movingEmitter.setPositionProperty(Vector3(0, 0, -5));
            movingEmitter.setForwardProperty(Vector3(0, 0, 1));
            movingEmitter.setVelocityProperty(Vector3::Zero);
            movingEmitter.setDopplerScaleProperty(1);
            std::unique_ptr<Cue> controlled(sounds.GetCue("MidTone"));
            controlled->Apply3D(fixedListener, movingEmitter);
            ctx.CheckNear(controlled->GetVariable("Distance"), 5.0f, 1e-4f,
                          "Apply3D publishes distance before the owned cue plays");
            ctx.CheckNear(controlled->GetVariable("OrientationAngle"), 0.0f, 1e-4f,
                          "an emitter aimed at the listener has zero orientation angle");
            ctx.CheckNear(controlled->GetVariable("DopplerPitchScalar"), 1.0f, 1e-4f,
                          "stationary listener and emitter have a neutral cue Doppler scalar");
            movingEmitter.setForwardProperty(Vector3(1, 0, 0));
            controlled->Apply3D(fixedListener, movingEmitter);
            ctx.CheckNear(controlled->GetVariable("OrientationAngle"), 90.0f, 1e-4f,
                          "turning the emitter sideways updates the cue angle in degrees");
            movingEmitter.setForwardProperty(Vector3(0, 0, 1));
            controlled->Apply3D(fixedListener, movingEmitter);
            controlled->Play();
            controlled->Pause();
            movingEmitter.setPositionProperty(Vector3(0, 0, -10));
            movingEmitter.setVelocityProperty(Vector3(0, 0, SoundEffect::getSpeedOfSoundProperty() * .1f));
            controlled->Apply3D(fixedListener, movingEmitter);
            ctx.Check(controlled->getIsPausedProperty() &&
                      std::abs(controlled->GetVariable("Distance") - 10.0f) < 1e-4f,
                      "a paused owned cue accepts a new position without resuming playback");
            // Approaching at one tenth of the speed of sound gives a raw cue ratio 1/(1-.1).
            ctx.CheckNear(controlled->GetVariable("DopplerPitchScalar"), 10.0f / 9.0f, 1e-4f,
                          "approaching velocity updates the public cue Doppler scalar");
            ctx.Print("Owned cue: distance {}, angle {}, raw Doppler {} (not measured audio output)",
                      controlled->GetVariable("Distance"), controlled->GetVariable("OrientationAngle"),
                      controlled->GetVariable("DopplerPitchScalar"));
            controlled->Stop(AudioStopOptions::Immediate);
            controlled->Dispose();
        }

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
                   "AudioEmitter goes round the AudioListener, and checks an owned Cue's Apply3D spatial variables before and during paused playback.",
    .apis = {"SoundBank::PlayCue", "AudioListener::AudioListener", "AudioEmitter::Position",
             "Cue::Apply3D", "Cue::GetVariable", "AudioEmitter::Forward", "AudioEmitter::Velocity",
             "AudioEmitter::DopplerScale", "SoundEffect::SpeedOfSound"},
    .tags = {"audio", "xact", "3d", "positional", "panning"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb"},
})
