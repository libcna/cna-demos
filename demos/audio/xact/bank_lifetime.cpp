// SPDX-License-Identifier: MS-PL
// TQ-022/084 (3bcf9b354, 3274f9572): malformed banks fail construction; released-bank lookups retain XNA exception families.
// Bank disposal releases dependent cues; caller-owned C++ wrappers remain separately owned.
#include <memory>
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioCategory.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/Cue.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundBank.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/EventArgs.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework::Audio;
using Microsoft::Xna::Framework::TitleLocation;
namespace
{
    void BankLifetime(CnaDemos::DemoContext& ctx)
    {
        bool nullWaveRefused = false;
        bool nullSoundRefused = false;
        try { WaveBank unexpected(nullptr, "assets/xact/tones.xwb"); }
        catch (const System::ArgumentNullException&) { nullWaveRefused = true; }
        try { SoundBank unexpected(nullptr, "assets/xact/tones.xsb"); }
        catch (const System::ArgumentNullException&) { nullSoundRefused = true; }
        ctx.Check(nullWaveRefused && nullSoundRefused,
                  "both bank constructors reject a null owning engine");

        // Callback storage precedes every observed object, including during exception unwinding.
        int engineEvents = 0, waveEvents = 0, soundEvents = 0, preparedEvents = 0, activeEvents = 0;
        bool engineSender = true, waveSender = true, soundSender = true, cueSenders = true;
        AudioEngine engine("assets/xact/demo.xgs");
        bool emptyWaveRefused = false;
        bool emptySoundRefused = false;
        bool missingWaveRefused = false;
        bool missingSoundRefused = false;
        try { WaveBank unexpected(&engine, ""); }
        catch (const System::ArgumentNullException&) { emptyWaveRefused = true; }
        try { SoundBank unexpected(&engine, ""); }
        catch (const System::ArgumentNullException&) { emptySoundRefused = true; }
        try { WaveBank unexpected(&engine, TitleLocation::Path() + "/assets/xact/missing.xwb"); }
        catch (const System::IO::FileNotFoundException&) { missingWaveRefused = true; }
        try { SoundBank unexpected(&engine, "assets/xact/missing.xsb"); }
        catch (const System::IO::FileNotFoundException&) { missingSoundRefused = true; }
        ctx.Check(emptyWaveRefused && emptySoundRefused,
                  "both bank constructors reject an empty filename");
        ctx.Check(missingWaveRefused && missingSoundRefused,
                  "missing eager wave and sound banks are FileNotFoundException");

        bool corruptWaveRefused = false;
        try { WaveBank unexpected(&engine, TitleLocation::Path() + "/assets/xact/demo.xgs"); }
        catch (const System::ArgumentException&) { corruptWaveRefused = true; }
        bool corruptSoundRefused = false;
        try { SoundBank unexpected(&engine, "assets/xact/demo.xgs"); }
        catch (const System::ArgumentException&) { corruptSoundRefused = true; }
        ctx.Check(corruptWaveRefused,
                  "an existing non-XWB file is refused with ArgumentException before bank creation");
        ctx.Check(corruptSoundRefused,
                  "an existing non-XSB file is refused with ArgumentException before bank creation");

        bool missingStreamRefused = false;
        try { WaveBank unexpected(&engine, TitleLocation::Path() + "/assets/xact/missing-stream.xwb", 0, 16); }
        catch (const System::IO::FileNotFoundException&) { missingStreamRefused = true; }
        ctx.Check(missingStreamRefused,
                  "a missing streaming wave bank is refused with FileNotFoundException");
        engine.Disposing.Add([&](System::Object* sender, const System::EventArgs&) {
            ++engineEvents;
            engineSender = engineSender && sender == &engine;
        });
        {
            WaveBank waves(&engine, TitleLocation::Path() + "/assets/xact/tones.xwb");
            SoundBank sounds(&engine, "assets/xact/tones.xsb");
            std::unique_ptr<Cue> prepared(sounds.GetCue("MidTone"));
            std::unique_ptr<Cue> active(sounds.GetCue("MidTone"));
            waves.Disposing.Add([&](System::Object* sender, const System::EventArgs&) {
                ++waveEvents;
                waveSender = waveSender && sender == &waves;
            });
            sounds.Disposing.Add([&](System::Object* sender, const System::EventArgs&) {
                ++soundEvents;
                soundSender = soundSender && sender == &sounds;
            });
            prepared->Disposing.Add([&, expected = prepared.get()](System::Object* sender, const System::EventArgs&) {
                ++preparedEvents;
                cueSenders = cueSenders && sender == expected;
            });
            active->Disposing.Add([&, expected = active.get()](System::Object* sender, const System::EventArgs&) {
                ++activeEvents;
                cueSenders = cueSenders && sender == expected;
            });
            // XACT 3.6's owner boundary (CNA TQ-XACT-30/32): GetCue prepares the cue and resolves
            // its waves before anything plays. An undisposed cue handle keeps its sound bank in use,
            // and the sound bank's resolved references keep the wave bank in use.
            ctx.Check(prepared->getIsPreparedProperty() && waves.getIsInUseProperty() && sounds.getIsInUseProperty(),
                      "prepared cue handles put the sound bank in use, and their resolved waves the wave bank");
            active->Play();
            active->Pause();
            ctx.Check(active->getIsPausedProperty() && waves.getIsInUseProperty() && sounds.getIsInUseProperty(),
                      "a paused consuming cue keeps both banks in use");

            waves.Dispose();
            // A disposed cue reports no state at all: XNA's get_IsStopped (like the other state
            // getters) reads the released handle's native state, 0 (Cue.get_IsStopped IL_0000-0014).
            ctx.Check(waves.getIsDisposedProperty() && active->getIsDisposedProperty() &&
                      !active->getIsStoppedProperty() && !waves.getIsInUseProperty(),
                      "disposing the wave bank disposes its consuming cue");
            // The never-played prepared cue resolved the same waves, so it is released as well.
            ctx.Check(!sounds.getIsDisposedProperty() && prepared->getIsDisposedProperty() &&
                      !prepared->getIsStoppedProperty() && !sounds.getIsInUseProperty(),
                      "the sound bank survives wave-bank disposal, which also releases the never-played prepared cue");
            waves.Dispose();
            ctx.Check(waveEvents == 1 && waveSender && preparedEvents == 1 && activeEvents == 1 && cueSenders,
                      "wave-bank disposal identifies each released object once despite repetition");
            bool missingWavesRefused = false;
            try
            {
                const std::unique_ptr<Cue> unexpected(sounds.GetCue("MidTone"));
                static_cast<void>(unexpected);
            }
            catch (const System::InvalidOperationException&) { missingWavesRefused = true; }
            ctx.Check(missingWavesRefused, "GetCue refuses a cue whose wave bank is no longer loaded");

            sounds.Dispose();
            ctx.Check(sounds.getIsDisposedProperty() && !sounds.getIsInUseProperty() &&
                          prepared->getIsDisposedProperty() && active->getIsDisposedProperty(),
                      "the sound bank disposes after its wave bank without reviving either cue");
            bool getCueRefused = false;
            bool playCueRefused = false;
            try
            {
                const std::unique_ptr<Cue> unexpected(sounds.GetCue("MidTone"));
                static_cast<void>(unexpected);
            }
            catch (const System::ArgumentException&) { getCueRefused = true; }
            try { sounds.PlayCue("MidTone"); }
            catch (const System::InvalidOperationException&) { playCueRefused = true; }
            ctx.Check(getCueRefused && playCueRefused,
                      "a disposed sound bank refuses both controlled and fire-and-forget cue creation");
            sounds.Dispose();
            ctx.Check(soundEvents == 1 && soundSender && preparedEvents == 1 && activeEvents == 1 && cueSenders,
                      "sound-bank disposal notifies once and repeats none of the cue notifications");
            ctx.Check(!engine.getIsDisposedProperty(), "disposing the banks leaves their engine alive");
            // unique_ptr still deletes the caller-owned wrappers, already disposed by their banks.
        }
        ctx.Check(waveEvents == 1 && soundEvents == 1 && preparedEvents == 1 && activeEvents == 1,
                  "deleting the disposed bank and cue wrappers raises no extra notifications");

        // A sound bank's own disposal releases a never-played prepared cue, and with it the
        // last resolved reference that kept the wave bank in use.
        {
            WaveBank waves(&engine, TitleLocation::Path() + "/assets/xact/tones.xwb");
            SoundBank sounds(&engine, "assets/xact/tones.xsb");
            std::unique_ptr<Cue> prepared(sounds.GetCue("MidTone"));
            prepared.reset();
            ctx.Check(!sounds.getIsInUseProperty() && waves.getIsInUseProperty(),
                      "deleting the only cue handle frees the sound bank, whose resolved references keep the wave bank in use");
            prepared.reset(sounds.GetCue("MidTone"));
            sounds.Dispose();
            ctx.Check(prepared->getIsDisposedProperty() && !prepared->getIsStoppedProperty() &&
                          !waves.getIsDisposedProperty() && !waves.getIsInUseProperty(),
                      "disposing the sound bank disposes its never-played prepared cue and frees the wave bank");
        }

        // Fresh banks can register with the same live engine. Disposing the engine now releases
        // the entire graph, including both a paused consumer and a never-played prepared cue.
        int dependentWaveEvents = 0, dependentSoundEvents = 0;
        int dependentPreparedEvents = 0, dependentActiveEvents = 0;
        bool dependentSenders = true;
        WaveBank waves(&engine, TitleLocation::Path() + "/assets/xact/tones.xwb");
        SoundBank sounds(&engine, "assets/xact/tones.xsb");
        std::unique_ptr<Cue> prepared(sounds.GetCue("MidTone"));
        std::unique_ptr<Cue> active(sounds.GetCue("MidTone"));
        waves.Disposing.Add([&](System::Object* sender, const System::EventArgs&) {
            ++dependentWaveEvents;
            dependentSenders = dependentSenders && sender == &waves;
        });
        sounds.Disposing.Add([&](System::Object* sender, const System::EventArgs&) {
            ++dependentSoundEvents;
            dependentSenders = dependentSenders && sender == &sounds;
        });
        prepared->Disposing.Add([&, expected = prepared.get()](System::Object* sender, const System::EventArgs&) {
            ++dependentPreparedEvents;
            dependentSenders = dependentSenders && sender == expected;
        });
        active->Disposing.Add([&, expected = active.get()](System::Object* sender, const System::EventArgs&) {
            ++dependentActiveEvents;
            dependentSenders = dependentSenders && sender == expected;
        });
        active->Play();
        active->Pause();
        engine.Dispose();
        ctx.Check(engine.getIsDisposedProperty() && waves.getIsDisposedProperty() && sounds.getIsDisposedProperty() &&
                  prepared->getIsDisposedProperty() && active->getIsDisposedProperty(),
                  "engine disposal cascades to both banks and both caller-owned cue states");
        ctx.Check(engineEvents == 1 && engineSender && dependentWaveEvents == 1 && dependentSoundEvents == 1 &&
                  dependentPreparedEvents == 1 && dependentActiveEvents == 1 && dependentSenders,
                  "engine and dependent objects identify themselves with one disposal notification each");
        engine.Dispose();
        waves.Dispose();
        sounds.Dispose();
        prepared->Dispose();
        active->Dispose();
        ctx.Check(engineEvents == 1 && dependentWaveEvents == 1 && dependentSoundEvents == 1 &&
                  dependentPreparedEvents == 1 && dependentActiveEvents == 1,
                  "repeated explicit disposal across the released graph is idempotent");
        bool categoryRefused = false;
        bool getVariableRefused = false;
        bool setVariableRefused = false;
        bool updateCompleted = false;
        try { static_cast<void>(engine.GetCategory("SFX")); }
        catch (const System::InvalidOperationException&) { categoryRefused = true; }
        try { static_cast<void>(engine.GetGlobalVariable("SpeedOfSound")); }
        catch (const System::ArgumentException&) { getVariableRefused = true; }
        try { engine.SetGlobalVariable("SpeedOfSound", 500.0f); }
        catch (const System::ArgumentException&) { setVariableRefused = true; }
        try
        {
            engine.Update();
            updateCompleted = true;
        }
        catch (...) {}
        ctx.Check(engine.getRendererDetailsProperty().empty(),
                  "renderer records are released but remain safely queryable after engine disposal");
        ctx.Check(categoryRefused, "a disposed engine refuses category lookup");
        ctx.Check(getVariableRefused && setVariableRefused,
                  "a disposed engine refuses both global-variable access directions");
        ctx.Check(updateCompleted, "Update is a safe no-op after engine disposal");
        ctx.Print("A prepared cue depends on both banks: disposing either releases it, and engine disposal releases "
                  "the whole graph. Event counts are checked without requiring a global callback order.");
    }
}
CNA_DEMOS_CONSOLE(BankLifetime, {
    .id = "audio.xact.bank_lifetime",
    .title = "XACT bank and engine disposal",
    .description = "Checks XACT bank construction failures and stub states, then follows prepared and paused cues "
                   "and the banks' IsInUse through bank/engine disposal and verifies AudioEngine behavior afterward.",
    .apis = {"AudioEngine::Dispose", "AudioEngine::IsDisposed", "AudioEngine::Disposing", "AudioEngine::Update",
             "AudioEngine::RendererDetails", "AudioEngine::GetCategory", "AudioEngine::GetGlobalVariable",
             "AudioEngine::SetGlobalVariable",
             "WaveBank::Dispose", "WaveBank::IsDisposed", "WaveBank::IsInUse", "WaveBank::Disposing",
             "SoundBank::Dispose", "SoundBank::IsDisposed", "SoundBank::IsInUse", "SoundBank::Disposing",
             "SoundBank::GetCue", "SoundBank::PlayCue", "Cue::IsPrepared", "Cue::IsDisposed", "Cue::IsStopped",
             "Cue::Disposing"},
    .tags = {"audio", "xact", "lifetime", "ownership", "events"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb"},
})
