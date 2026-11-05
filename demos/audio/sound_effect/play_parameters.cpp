// SPDX-License-Identifier: MS-PL
// TQ-005 (3d4e3f3a3): Refresh range-checks pitch and throws on disposed SoundEffect playback.
// SoundEffect::Play(volume, pitch, pan) and the master volume: fire-and-forget sounds with settings.
#include <limits>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;
using Microsoft::Xna::Framework::Content::ContentManager;
using Microsoft::Xna::Framework::TitleLocation;

namespace
{
    void PlayParameters(CnaDemos::DemoContext& ctx)
    {
        ContentManager content;
        content.setRootDirectoryProperty("assets");
        SoundEffect tone = content.Load<SoundEffect>("sounds/tone_440");

        // volume 0..1, pitch -1..1 (one octave down to one octave up), pan -1 (left) .. 1 (right).
        ctx.Check(tone.Play(1.0f, 0.0f, -1.0f), "full volume, original pitch, left");
        ctx.Check(tone.Play(0.5f, 1.0f, 0.0f), "half volume, an octave up, centre");
        ctx.Check(tone.Play(0.25f, -1.0f, 1.0f), "quarter volume, an octave down, right");
        bool highPitchRejected = false, lowPitchRejected = false;
        try { (void)tone.Play(1.0f, 5.0f, 0.0f); }
        catch (const System::ArgumentOutOfRangeException&) { highPitchRejected = true; }
        try { (void)tone.Play(1.0f, -5.0f, 0.0f); }
        catch (const System::ArgumentOutOfRangeException&) { lowPitchRejected = true; }
        ctx.Check(highPitchRejected && lowPitchRejected, "both pitch outliers are refused instead of clamped");
        bool highPanRejected = false;
        bool lowPanRejected = false;
        try { (void)tone.Play(1.0f, 0.0f, 1.5f); }
        catch (const System::ArgumentOutOfRangeException&) { highPanRejected = true; }
        try { (void)tone.Play(1.0f, 0.0f, -1.5f); }
        catch (const System::ArgumentOutOfRangeException&) { lowPanRejected = true; }
        ctx.Check(highPanRejected && lowPanRejected,
                  "pan outside either boundary is rejected instead of clamped");

        // MasterVolume scales every SoundEffect in the program (1 by default).
        const float previous = SoundEffect::getMasterVolumeProperty();
        ctx.Check(previous == 1.0f, "MasterVolume starts at 1");
        SoundEffect::setMasterVolumeProperty(0.5f);
        ctx.Check(SoundEffect::getMasterVolumeProperty() == 0.5f, "MasterVolume can be lowered for all sounds");
        SoundEffect::setMasterVolumeProperty(0.0f);
        ctx.Check(SoundEffect::getMasterVolumeProperty() == 0.0f, "MasterVolume accepts the silent lower boundary");
        SoundEffect::setMasterVolumeProperty(1.0f);
        ctx.Check(SoundEffect::getMasterVolumeProperty() == 1.0f, "MasterVolume accepts the full upper boundary");

        SoundEffect::setMasterVolumeProperty(0.5f);
        bool highRejected = false;
        bool negativeRejected = false;
        bool nanRejected = false;
        try { SoundEffect::setMasterVolumeProperty(1.5f); }
        catch (const System::ArgumentOutOfRangeException&) { highRejected = true; }
        try { SoundEffect::setMasterVolumeProperty(-0.1f); }
        catch (const System::ArgumentOutOfRangeException&) { negativeRejected = true; }
        try { SoundEffect::setMasterVolumeProperty(std::numeric_limits<float>::quiet_NaN()); }
        catch (const System::ArgumentOutOfRangeException&) { nanRejected = true; }
        ctx.Check(highRejected && negativeRejected && nanRejected &&
                      SoundEffect::getMasterVolumeProperty() == 0.5f,
                  "finite and NaN range failures preserve the prior MasterVolume");
        SoundEffect::setMasterVolumeProperty(previous);
        ctx.Check(SoundEffect::getMasterVolumeProperty() == previous,
                  "the demo restores the original process-global MasterVolume");

        SoundEffect disposedProbe(TitleLocation::Path() + "/assets/sounds/tone_440.wav");
        disposedProbe.Dispose();
        bool createRejected = false;
        try { (void)disposedProbe.CreateInstance(); }
        catch (const System::ObjectDisposedException&) { createRejected = true; }
        bool playRejected = false, configuredPlayRejected = false;
        try { (void)disposedProbe.Play(); }
        catch (const System::ObjectDisposedException&) { playRejected = true; }
        try { (void)disposedProbe.Play(1.0f, 0.0f, 0.0f); }
        catch (const System::ObjectDisposedException&) { configuredPlayRejected = true; }
        ctx.Check(disposedProbe.getIsDisposedProperty() && playRejected && configuredPlayRejected && createRejected,
                  "a disposed effect refuses both Play overloads and CreateInstance");
    }
}

CNA_DEMOS_CONSOLE(PlayParameters, {
    .id = "audio.sound_effect.play_parameters",
    .title = "Play with volume, pitch and pan",
    .description = "Exercises fire-and-forget Play parameter clamps, refusals and disposal, then checks exact "
                   "MasterVolume boundaries, atomic failures and process-global restoration.",
    .apis = {"SoundEffect::Play", "SoundEffect::CreateInstance", "SoundEffect::Dispose", "SoundEffect::IsDisposed",
             "SoundEffect::MasterVolume", "ContentManager::ContentManager"},
    .tags = {"audio", "volume", "pitch", "pan"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_440.wav"},
})
