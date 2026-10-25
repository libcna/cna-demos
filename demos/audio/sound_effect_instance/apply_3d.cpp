// SPDX-License-Identifier: MS-PL
// Positional sound: AudioListener and AudioEmitter with SoundEffectInstance::Apply3D.
#include <array>
#include <chrono>
#include <limits>
#include <thread>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEmitter.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioListener.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "System/ArgumentNullException.hpp"
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
        emitter.setDopplerScaleProperty(2.5f);
        ctx.Check(emitter.getDopplerScaleProperty() == 2.5f,
                  "the emitter retains a positive authored DopplerScale");
        emitter.setDopplerScaleProperty(0.0f);
        bool emitterDopplerRejected = false;
        try { emitter.setDopplerScaleProperty(-0.1f); }
        catch (const System::ArgumentOutOfRangeException&) { emitterDopplerRejected = true; }
        ctx.Check(emitterDopplerRejected && emitter.getDopplerScaleProperty() == 0.0f,
                  "zero emitter DopplerScale is accepted and a negative write preserves it");
        emitter.setDopplerScaleProperty(1.0f);

        SoundEffectInstance overloadProbe = tone.CreateInstance();
        AudioListener farListener;
        farListener.setPositionProperty(Vector3(-100.0f, 0.0f, 0.0f));
        AudioListener nearListener;
        nearListener.setPositionProperty(Vector3(10.0f, 0.0f, 0.0f));
        const std::vector<AudioListener> listeners = {farListener, nearListener};
        bool collectionAccepted = true;
        bool pointerAccepted = true;
        try { overloadProbe.Apply3D(listeners, emitter); }
        catch (...) { collectionAccepted = false; }
        try { overloadProbe.Apply3D(listeners.data(), static_cast<int>(listeners.size()), emitter); }
        catch (...) { pointerAccepted = false; }
        ctx.Check(collectionAccepted && pointerAccepted && overloadProbe.getStateProperty() == SoundState::Stopped,
                  "collection and pointer/count Apply3D overloads accept multiple listeners while stopped");

        bool emptyRejected = false;
        bool nullRejected = false;
        bool zeroRejected = false;
        bool negativeRejected = false;
        try { overloadProbe.Apply3D(std::vector<AudioListener>{}, emitter); }
        catch (const System::ArgumentOutOfRangeException&) { emptyRejected = true; }
        try { overloadProbe.Apply3D(nullptr, 1, emitter); }
        catch (const System::ArgumentNullException&) { nullRejected = true; }
        try { overloadProbe.Apply3D(listeners.data(), 0, emitter); }
        catch (const System::ArgumentOutOfRangeException&) { zeroRejected = true; }
        try { overloadProbe.Apply3D(listeners.data(), -1, emitter); }
        catch (const System::ArgumentOutOfRangeException&) { negativeRejected = true; }
        ctx.Check(emptyRejected && nullRejected && zeroRejected && negativeRejected &&
                      overloadProbe.getStateProperty() == SoundState::Stopped &&
                      overloadProbe.getPanProperty() == 0.0f,
                  "invalid Apply3D listener inputs are refused without changing public state");

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
        // XNA set_Pan refuses the spatial mode before mutating Pan (ISSUE-073).
        // The previous expectation accidentally required CNA's failed-assignment bug.
        ctx.Check(voice.getPanProperty() == -.5f,
                  "the refused Pan setter preserves its prior public property value");
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

        // Position controls attenuation and pan; velocity controls Doppler pitch. Play three
        // short phases from the same position so a person can compare approaching, stationary
        // and receding motion. The derived pitch lives in the mixer -- Pitch remains the
        // authored property -- so this is honestly classified as an acoustic/manual check.
        emitter.setPositionProperty(Vector3(0.0f, 0.0f, -15.0f));
        const std::array velocities = {
            Vector3(0.0f, 0.0f, 45.0f), Vector3::Zero, Vector3(0.0f, 0.0f, -45.0f)};
        const std::array phaseNames = {"approaching", "stationary", "receding"};
        for (std::size_t phase = 0; phase < velocities.size(); ++phase)
        {
            emitter.setVelocityProperty(velocities[phase]);
            voice.Apply3D(listener, emitter);
            voice.Play();
            ctx.Print("Doppler phase: {} (emitter velocity {})", phaseNames[phase],
                      emitter.getVelocityProperty().ToString());
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            voice.Stop();
        }
        ctx.Check(emitter.getVelocityProperty() == velocities.back(),
                  "AudioEmitter retains the authored receding velocity used by the final Doppler phase");
        emitter.setVelocityProperty(Vector3::Zero);

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
    .description = "Moves an AudioEmitter across listeners, checks Apply3D overload/refusal and paused Pan/3D mode "
                   "behavior, plays approaching/stationary/receding velocity phases for an audible Doppler "
                   "comparison, then restores every process-global spatial value.",
    .apis = {"SoundEffectInstance::Apply3D", "AudioListener::AudioListener", "AudioListener::Position",
             "AudioListener::Forward", "AudioListener::Up", "AudioListener::Velocity", "AudioEmitter::AudioEmitter",
             "AudioEmitter::Position", "AudioEmitter::Velocity", "AudioEmitter::Up", "SoundEffect::DistanceScale",
             "SoundEffect::DopplerScale", "SoundEffect::SpeedOfSound",
             "SoundEffect::CreateInstance", "SoundEffectInstance::Play", "SoundEffectInstance::Pan",
             "SoundEffectInstance::Pause", "SoundEffectInstance::Stop", "SoundEffectInstance::State"},
    .tags = {"audio", "3d", "positional", "pan", "doppler", "velocity"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_440.wav"},
    .restrictions = "Automation checks state and ranges; comparing the three Doppler phases requires audible output.",
})
