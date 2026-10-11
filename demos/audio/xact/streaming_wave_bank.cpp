// SPDX-License-Identifier: MS-PL
// A streaming WaveBank: music played from the file as it goes, instead of loaded into memory.
#include <chrono>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioStopOptions.hpp"
#include "Microsoft/Xna/Framework/Audio/Cue.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundBank.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    void StreamingWaveBank(CnaDemos::DemoContext& ctx)
    {
        AudioEngine engine("assets/xact/demo.xgs");
        // Short effects live in an in-memory bank; long music is authored as a streaming bank
        // and opened with the streaming constructor: offset 0 into the file, packets of 16
        // sectors (the minimum is 2). Only the bank's header is read here. (CNA opens a relative
        // streaming path from the working directory, unlike the other XACT files: the path is
        // built from TitleLocation, CNA-DEMOS-ISSUE-028.)
        WaveBank music(&engine, Microsoft::Xna::Framework::TitleLocation::Path() + "/assets/xact/music.xwb", 0, 16);
        SoundBank sounds(&engine, "assets/xact/music.xsb");

        // On XNA a streaming bank becomes prepared during AudioEngine::Update, once it has read
        // ahead: a game waits for IsPrepared before playing its cues. (CNA prepares it at once.)
        int updates = 0;
        while (!music.getIsPreparedProperty() && updates < 100)
        {
            engine.Update();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ++updates;
        }
        ctx.Print("the streaming bank was prepared after {} AudioEngine::Update call(s)", updates);
        ctx.Check(music.getIsPreparedProperty(), "the streaming wave bank becomes prepared");
        ctx.Check(!music.getIsInUseProperty(), "nothing plays from it yet");

        Cue* theme = sounds.GetCue("Theme");
        theme->Play();
        engine.Update();
        ctx.Check(theme->getIsPlayingProperty(), "the music cue plays from the streaming bank");
        ctx.Check(music.getIsInUseProperty(), "the bank is in use while its wave plays");
        for (int i = 0; i < 40; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            engine.Update();
        }

        // Music stops when the level ends; the bank is free again.
        theme->Stop(AudioStopOptions::Immediate);
        engine.Update();
        ctx.Check(theme->getIsStoppedProperty() && !music.getIsInUseProperty(),
                  "after Stop the cue is stopped and the bank no longer in use");
        theme->Dispose();
    }
}

CNA_DEMOS_CONSOLE(StreamingWaveBank, {
    .id = "audio.xact.streaming_wave_bank",
    .title = "Streaming wave bank (music)",
    .description = "Opens a streaming XACT wave bank with WaveBank's streaming constructor (offset, packet size), "
                   "waits for IsPrepared through AudioEngine::Update, and plays its music cue; IsInUse follows the "
                   "playback.",
    .apis = {"WaveBank::WaveBank", "WaveBank::IsPrepared", "WaveBank::IsInUse", "AudioEngine::Update",
             "SoundBank::GetCue", "Cue::Play", "Cue::Stop"},
    .tags = {"audio", "xact", "music", "streaming"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/music.xwb", "xact/music.xsb"},
})
