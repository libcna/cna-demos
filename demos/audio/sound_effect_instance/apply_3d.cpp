// SPDX-License-Identifier: MS-PL
// Positional sound: AudioListener and AudioEmitter with SoundEffectInstance::Apply3D.
#include <chrono>
#include <limits>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEmitter.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioListener.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    void Apply3DDemo(CnaDemos::DemoContext& ctx)
    {
        ContentManager content(nullptr, "assets");
        const SoundEffect tone = content.Load<SoundEffect>("sounds/tone_440");

        // The listener (the player's ears) at the origin, facing -Z with +Y up -- XNA's defaults.
        AudioListener listener;
        listener.setPositionProperty(Vector3::Zero);
        listener.setForwardProperty(Vector3::Forward);
        listener.setUpProperty(Vector3::Up);

        // The emitter (the sound source) sweeps from the far left to the far right.
        AudioEmitter emitter;
        ctx.Check(listener.getPositionProperty() == Vector3::Zero &&
                      listener.getForwardProperty() == Vector3::Forward &&
                      listener.getUpProperty() == Vector3::Up &&
                      listener.getVelocityProperty() == Vector3::Zero &&
                      emitter.getPositionProperty() == Vector3::Zero &&
                      emitter.getForwardProperty() == Vector3::Forward &&
                      emitter.getUpProperty() == Vector3::Up &&
                      emitter.getVelocityProperty() == Vector3::Zero &&
                      emitter.getDopplerScaleProperty() == 1.0f,
                  "listener and emitter expose the exact default spatial state");
        SoundEffectInstance voice = tone.CreateInstance();
        voice.setIsLoopedProperty(true);

        // A started voice retains its spatial mode even while paused. Pause keeps these
        // refusal checks independent of the mixer finishing a short non-looping sound.
        voice.setPanProperty(-.5f);
        voice.Play();
        voice.Pause();
        bool refused = false;
        try { voice.Apply3D(listener, emitter); }
        catch (const System::InvalidOperationException&) { refused = true; }
        ctx.Check(refused && voice.getStateProperty() == SoundState::Paused && voice.getPanProperty() == -.5f,
                  "a paused pan voice refuses Apply3D while retaining its state and stored pan");

        voice.Stop();
        listener.setVelocityProperty(Vector3(3.0f, -2.0f, 1.0f));
        emitter.setUpProperty(Vector3::Backward);
        ctx.Check(listener.getVelocityProperty() == Vector3(3.0f, -2.0f, 1.0f) &&
                      emitter.getUpProperty() == Vector3::Backward,
                  "listener velocity and emitter up retain independently authored vectors");
        voice.Apply3D(listener, emitter);
        ctx.Check(voice.getStateProperty() == SoundState::Stopped,
                  "Apply3D accepts the authored spatial state before playback");
        listener.setVelocityProperty(Vector3::Zero);
        emitter.setUpProperty(Vector3::Up);
        voice.Apply3D(listener, emitter); // Stop lets the next submission choose 3D instead.
        voice.Play();
        voice.Pause();
        ctx.Check(voice.getStateProperty() == SoundState::Paused,
                  "after Stop the same voice can start and pause in 3D mode");
        refused = false;
        try { voice.setPanProperty(.75f); }
        catch (const System::InvalidOperationException&) { refused = true; }
        ctx.Check(refused && voice.getStateProperty() == SoundState::Paused,
                  "a paused 3D voice refuses Pan without resuming or stopping");
        // CNA stores the public Pan value before its mode gate. This getter is not an
        // output-meter reading and does not imply that refused Pan changed the spatial mix.
        ctx.Check(voice.getPanProperty() == .75f,
                  "the refused Pan setter still exposes its assigned public property value");
        voice.Apply3D(listener, emitter);
        ctx.Check(voice.getStateProperty() == SoundState::Paused,
                  "a 3D voice can be re-aimed while paused after a refused Pan change");

        voice.Stop();
        voice.setPanProperty(-.25f); // Stop also permits switching back from 3D to pan.
        voice.Play();
        voice.Pause();
        refused = false;
        try { voice.Apply3D(listener, emitter); }
        catch (const System::InvalidOperationException&) { refused = true; }
        ctx.Check(refused && voice.getStateProperty() == SoundState::Paused && voice.getPanProperty() == -.25f,
                  "after Stop switching back to pan restores the Apply3D refusal");
        voice.Stop();
        // Restore 3D before the original audible sweep; its first Play selects this mode.
        voice.Apply3D(listener, emitter);
        for (int step = 0; step <= 8; ++step)
        {
            emitter.setPositionProperty(Vector3(-20.0f + 5.0f * static_cast<float>(step), 0.0f, -5.0f));
            // Apply3D sets the voice's pan, volume (distance attenuation) and Doppler pitch from the
            // two positions; call it before Play and whenever either moves.
            voice.Apply3D(listener, emitter);
            if (step == 0)
                voice.Play();
            std::this_thread::sleep_for(std::chrono::milliseconds(120));
        }
        voice.Stop();

        // How world units map to audio: DistanceScale for attenuation, DopplerScale and SpeedOfSound
        // for the pitch shift of moving emitters.
        ctx.Print("DistanceScale {}, DopplerScale {}, SpeedOfSound {}", SoundEffect::getDistanceScaleProperty(),
                  SoundEffect::getDopplerScaleProperty(), SoundEffect::getSpeedOfSoundProperty());
        ctx.Check(SoundEffect::getSpeedOfSoundProperty() > 300.0f, "SpeedOfSound defaults to air (343.5 units/s)");

        const float originalDistance = SoundEffect::getDistanceScaleProperty();
        const float originalDoppler = SoundEffect::getDopplerScaleProperty();
        const float originalSpeed = SoundEffect::getSpeedOfSoundProperty();
        SoundEffect::setDistanceScaleProperty(2.0f);
        SoundEffect::setDopplerScaleProperty(0.0f);
        SoundEffect::setSpeedOfSoundProperty(300.0f);
        ctx.Check(SoundEffect::getDistanceScaleProperty() == 2.0f &&
                      SoundEffect::getDopplerScaleProperty() == 0.0f &&
                      SoundEffect::getSpeedOfSoundProperty() == 300.0f,
                  "the global spatial scales retain valid authored values");
        SoundEffect::setDistanceScaleProperty(0.0f);
        ctx.Check(SoundEffect::getDistanceScaleProperty() == std::numeric_limits<float>::denorm_min(),
                  "zero DistanceScale is lifted to the smallest positive float");

        bool distanceRejected = false;
        bool dopplerNegativeRejected = false;
        bool dopplerNanRejected = false;
        bool speedZeroRejected = false;
        bool speedNanRejected = false;
        const float nan = std::numeric_limits<float>::quiet_NaN();
        try { SoundEffect::setDistanceScaleProperty(-1.0f); }
        catch (const System::ArgumentOutOfRangeException&) { distanceRejected = true; }
        try { SoundEffect::setDopplerScaleProperty(-0.1f); }
        catch (const System::ArgumentOutOfRangeException&) { dopplerNegativeRejected = true; }
        try { SoundEffect::setDopplerScaleProperty(nan); }
        catch (const System::ArgumentOutOfRangeException&) { dopplerNanRejected = true; }
        try { SoundEffect::setSpeedOfSoundProperty(0.0f); }
        catch (const System::ArgumentOutOfRangeException&) { speedZeroRejected = true; }
        try { SoundEffect::setSpeedOfSoundProperty(nan); }
        catch (const System::ArgumentOutOfRangeException&) { speedNanRejected = true; }
        ctx.Check(distanceRejected && dopplerNegativeRejected && dopplerNanRejected && speedZeroRejected &&
                      speedNanRejected &&
                      SoundEffect::getDistanceScaleProperty() == std::numeric_limits<float>::denorm_min() &&
                      SoundEffect::getDopplerScaleProperty() == 0.0f &&
                      SoundEffect::getSpeedOfSoundProperty() == 300.0f,
                  "invalid global spatial scales are rejected without changing valid state");

        SoundEffect::setDistanceScaleProperty(originalDistance);
        SoundEffect::setDopplerScaleProperty(originalDoppler);
        SoundEffect::setSpeedOfSoundProperty(originalSpeed);
        ctx.Check(SoundEffect::getDistanceScaleProperty() == originalDistance &&
                      SoundEffect::getDopplerScaleProperty() == originalDoppler &&
                      SoundEffect::getSpeedOfSoundProperty() == originalSpeed,
                  "the demo restores every process-global spatial scale");
    }
}

CNA_DEMOS_CONSOLE(Apply3DDemo, {
    .id = "audio.sound_effect_instance.apply_3d",
    .title = "Positional (3D) sound",
    .description = "Moves an AudioEmitter across a listener, checks paused Pan/3D mode changes, and exercises the "
                   "SoundEffect spatial-scale ranges while restoring every process-global value.",
    .apis = {"SoundEffectInstance::Apply3D", "AudioListener::AudioListener", "AudioListener::Position",
             "AudioListener::Forward", "AudioListener::Up", "AudioListener::Velocity", "AudioEmitter::AudioEmitter",
             "AudioEmitter::Position", "AudioEmitter::Up", "SoundEffect::DistanceScale",
             "SoundEffect::DopplerScale", "SoundEffect::SpeedOfSound",
             "SoundEffect::CreateInstance", "SoundEffectInstance::Play", "SoundEffectInstance::Pan",
             "SoundEffectInstance::Pause", "SoundEffectInstance::Stop", "SoundEffectInstance::State"},
    .tags = {"audio", "3d", "positional", "pan"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_440.wav"},
})
