// SPDX-License-Identifier: MS-PL
// XACT categories: one volume slider, pause and stop for a whole group of sounds.
#include <any>
#include <chrono>
#include <memory>
#include <string>
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
        ctx.Check(music != effects && music == engine.GetCategory("Music"), "categories compare by name");
        const AudioCategory musicCopy = engine.GetCategory("Music");
        ctx.Check(music.Equals(musicCopy) && music.Equals(std::any(musicCopy)) &&
                      !music.Equals(std::any(effects)) && !music.Equals(std::any(std::string("Music"))) &&
                      music.GetHashCode() == musicCopy.GetHashCode() && music.ToString() == "Music",
                  "category typed and boxed identity, hash and text use its authored name");

        std::unique_ptr<Cue> tune(sounds.GetCue("LowTone"));   // in Music
        std::unique_ptr<Cue> beep(sounds.GetCue("HighTone"));  // in SFX
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

        effects.Pause();
        std::unique_ptr<Cue> later(sounds.GetCue("HighTone"));
        later->Play();
        engine.Update();
        ctx.Check(later->getIsPlayingProperty() && !later->getIsPausedProperty(),
                  "category Pause affects cues active at that call, not cues started afterward");
        later->Dispose();
        effects.Resume();

        // A separate authored hierarchy makes Master the parent of SFX. The original
        // independent Music/SFX categories above keep their original fixture and checks.
        AudioEngine hierarchy("assets/xact/hierarchy.xgs");
        WaveBank hierarchyWaves(&hierarchy, "assets/xact/tones.xwb");
        SoundBank hierarchySounds(&hierarchy, "assets/xact/tones.xsb");
        AudioCategory master = hierarchy.GetCategory("Master");
        AudioCategory child = hierarchy.GetCategory("SFX");
        ctx.Check(effects.Equals(child) && effects == child &&
                      effects.GetHashCode() == child.GetHashCode() && child.ToString() == "SFX",
                  "the same category name compares equal across different engines");
        std::unique_ptr<Cue> direct(hierarchySounds.GetCue("LowTone"));
        std::unique_ptr<Cue> descendant(hierarchySounds.GetCue("HighTone"));
        direct->Play();
        direct->Pause();
        descendant->Play();
        descendant->Pause();
        ctx.Check(direct->getIsPausedProperty() && descendant->getIsPausedProperty(),
                  "both hierarchy voices are live and paused before category controls");
        master.Resume();
        ctx.Check(direct->getIsPlayingProperty() && descendant->getIsPlayingProperty(),
                  "parent Resume reaches its own voice and the live descendant voice");
        master.Pause();
        ctx.Check(direct->getIsPausedProperty() && descendant->getIsPausedProperty(),
                  "parent Pause reaches both direct and descendant voices");
        child.Resume();
        ctx.Check(direct->getIsPausedProperty() && descendant->getIsPlayingProperty(),
                  "child Resume leaves the parent's own voice paused");
        child.Pause();
        const bool bothPausedBeforeStop = direct->getIsPausedProperty() && descendant->getIsPausedProperty();
        master.Stop(AudioStopOptions::Immediate);
        ctx.Check(bothPausedBeforeStop && direct->getIsStoppedProperty() && descendant->getIsStoppedProperty(),
                  "parent Stop releases its own voice and the paused descendant voice");
        direct->Dispose();
        descendant->Dispose();
    }
}

CNA_DEMOS_CONSOLE(Categories, {
    .id = "audio.xact.categories",
    .title = "XACT categories",
    .description = "Controls active Music/SFX cues, shows that Pause is punctual rather than persistent for later "
                   "cues, and contrasts independent groups with authored parent/child category control.",
    .apis = {"AudioEngine::GetCategory", "AudioCategory::Name", "AudioCategory::SetVolume", "AudioCategory::Pause",
             "AudioCategory::Resume", "AudioCategory::Stop", "AudioCategory::op_Equality",
             "AudioCategory::op_Inequality", "AudioCategory::Equals", "AudioCategory::GetHashCode",
             "AudioCategory::ToString"},
    .tags = {"audio", "xact", "category", "volume", "options"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb", "xact/hierarchy.xgs"},
})
