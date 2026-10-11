// SPDX-License-Identifier: MS-PL
// Positional sound: AudioListener and AudioEmitter with SoundEffectInstance::Apply3D.
#include <chrono>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEmitter.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioListener.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

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
        SoundEffectInstance voice = tone.CreateInstance();
        voice.setIsLoopedProperty(true);
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
    }
}

CNA_DEMOS_CONSOLE(Apply3DDemo, {
    .id = "audio.sound_effect_instance.apply_3d",
    .title = "Positional (3D) sound",
    .description = "Moves an AudioEmitter from the listener's left to its right, calling SoundEffectInstance::Apply3D "
                   "each step; listen for the tone panning across.",
    .apis = {"SoundEffectInstance::Apply3D", "AudioListener::AudioListener", "AudioListener::Position",
             "AudioListener::Forward", "AudioListener::Up", "AudioEmitter::AudioEmitter", "AudioEmitter::Position",
             "SoundEffect::DistanceScale", "SoundEffect::DopplerScale", "SoundEffect::SpeedOfSound",
             "SoundEffect::CreateInstance", "SoundEffectInstance::Play"},
    .tags = {"audio", "3d", "positional", "pan"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_440.wav"},
})
