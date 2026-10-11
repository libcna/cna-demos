// SPDX-License-Identifier: MS-PL
// SoundEffectInstance Volume, Pitch and Pan: change a playing voice, within XNA's ranges.
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using namespace Microsoft::Xna::Framework::Audio;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    void VolumePitchPan(CnaDemos::DemoContext& ctx)
    {
        ContentManager content(nullptr, "assets");
        const SoundEffect tone = content.Load<SoundEffect>("sounds/tone_440");
        SoundEffectInstance voice = tone.CreateInstance();
        ctx.Check(voice.getVolumeProperty() == 1.0f && voice.getPitchProperty() == 0.0f && voice.getPanProperty() == 0.0f,
                  "a new instance: Volume 1, Pitch 0, Pan 0");

        voice.Play();
        // Properties can change while the voice plays.
        voice.setVolumeProperty(0.4f);
        voice.setPitchProperty(0.5f);  // half an octave up
        voice.setPanProperty(-0.75f);  // mostly left
        ctx.Print("Volume {}, Pitch {}, Pan {}", voice.getVolumeProperty(), voice.getPitchProperty(),
                  voice.getPanProperty());
        ctx.Check(voice.getVolumeProperty() == 0.4f && voice.getPitchProperty() == 0.5f &&
                      voice.getPanProperty() == -0.75f,
                  "the new values are kept");

        // XNA documents the ranges (Volume 0..1, Pitch and Pan -1..1); CNA rejects values outside them.
        const auto rejects = [&](const char* what, auto set) {
            try
            {
                set();
                ctx.Check(false, std::string(what) + " is rejected");
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                ctx.Check(true, std::string(what) + " is rejected");
            }
        };
        rejects("Volume 1.5", [&] { voice.setVolumeProperty(1.5f); });
        rejects("Pitch -2", [&] { voice.setPitchProperty(-2.0f); });
        rejects("Pan 1.1", [&] { voice.setPanProperty(1.1f); });
        voice.Stop();
    }
}

CNA_DEMOS_CONSOLE(VolumePitchPan, {
    .id = "audio.sound_effect_instance.volume_pitch_pan",
    .title = "Volume, pitch and pan of a voice",
    .description = "Changes SoundEffectInstance::Volume, Pitch and Pan while the voice plays and shows the "
                   "ArgumentOutOfRangeException for values outside 0..1 and -1..1.",
    .apis = {"SoundEffectInstance::Volume", "SoundEffectInstance::Pitch", "SoundEffectInstance::Pan",
             "SoundEffectInstance::Play"},
    .tags = {"audio", "volume", "pitch", "pan", "instance"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_440.wav"},
})
