// SPDX-License-Identifier: MS-PL
// XACT categories: one volume slider, pause and stop for a whole group of sounds.
#include <chrono>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioCategory.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioStopOptions.hpp"
#include "Microsoft/Xna/Framework/Audio/Cue.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundBank.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    void Categories(CnaDemos::DemoContext& ctx)
    {
        AudioEngine engine("assets/xact/demo.xgs");
        WaveBank waves(&engine, "assets/xact/tones.xwb");
        SoundBank sounds(&engine, "assets/xact/tones.xsb");

        // The sound designer puts every sound in a category; the game adjusts categories, not sounds.
        AudioCategory music = engine.GetCategory("Music");
        AudioCategory effects = engine.GetCategory("SFX");
        ctx.Print("categories: {}, {}", music.getNameProperty(), effects.getNameProperty());
        ctx.Check(music != effects && music == engine.GetCategory("Music"), "categories compare by identity");

        Cue* tune = sounds.GetCue("LowTone");   // in Music
        Cue* beep = sounds.GetCue("HighTone");  // in SFX
        tune->Play();
        beep->Play();
        engine.Update();

        // An options-menu "music volume" slider: 0.2 for everything in Music.
        music.SetVolume(0.2f);
        // Pausing a category pauses its cues (e.g. effects while the pause menu is up).
        effects.Pause();
        engine.Update();
        ctx.Check(beep->getIsPausedProperty() && tune->getIsPlayingProperty(), "pausing SFX pauses only the SFX cue");
        effects.Resume();
        engine.Update();
        ctx.Check(beep->getIsPlayingProperty(), "Resume continues it");
        music.Stop(AudioStopOptions::Immediate);
        engine.Update();
        ctx.Check(tune->getIsStoppedProperty(), "stopping Music stops the music cue");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        bool unknown = false;
        try
        {
            (void)engine.GetCategory("Speech");
        }
        catch (const System::InvalidOperationException&)
        {
            unknown = true;
        }
        ctx.Check(unknown, "a category the settings file does not define is an InvalidOperationException");
        tune->Dispose();
        beep->Dispose();
    }
}

CNA_DEMOS_CONSOLE(Categories, {
    .id = "audio.xact.categories",
    .title = "XACT categories",
    .description = "Plays a Music cue and an SFX cue, then turns Music down with AudioCategory::SetVolume, pauses and "
                   "resumes SFX, and stops Music; an unknown category name throws.",
    .apis = {"AudioEngine::GetCategory", "AudioCategory::Name", "AudioCategory::SetVolume", "AudioCategory::Pause",
             "AudioCategory::Resume", "AudioCategory::Stop", "AudioCategory::op_Equality",
             "AudioCategory::op_Inequality"},
    .tags = {"audio", "xact", "category", "volume", "options"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb"},
})
