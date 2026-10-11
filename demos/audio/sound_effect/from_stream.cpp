// SPDX-License-Identifier: MS-PL
// SoundEffect::FromStream: a WAV file loaded at run time, without the content pipeline.
#include <chrono>
#include <fstream>
#include <memory>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    void FromStream(CnaDemos::DemoContext& ctx)
    {
        // A loose WAV -- user content, a mod's sounds, a file the game downloaded. XNA passes a
        // Stream (TitleContainer.OpenStream); CNA's FromStream takes a std::istream
        // (CNA-DEMOS-ISSUE-029), so the file is opened as one, from the title's folder.
        std::ifstream file(TitleLocation::Path() + "/assets/sounds/tone_440.wav", std::ios::binary);
        ctx.Check(file.is_open(), "the WAV file opens");
        // The caller owns the result.
        const std::unique_ptr<SoundEffect> tone(SoundEffect::FromStream(file));

        // 0.5 s of 22050 Hz mono 16-bit PCM, as tools/generate_assets.py wrote it.
        const double seconds = tone->getDurationProperty().getTotalSecondsProperty();
        ctx.Print("duration {:.3f} s", seconds);
        ctx.CheckNear(seconds, 0.5, 0.01, "the duration comes from the WAV header and data");
        ctx.Check(tone->getNameProperty().empty(), "a SoundEffect from a stream has no name");
        tone->setNameProperty("tone_440");
        ctx.Check(tone->getNameProperty() == "tone_440", "the game may name it");

        SoundEffectInstance instance = tone->CreateInstance();
        instance.Play();
        ctx.Check(instance.getStateProperty() == SoundState::Playing, "it plays like any other SoundEffect");
        // Audio completion follows the device worker, not a guaranteed wall-clock deadline.
        // Keep a finite timeout so a voice that never finishes remains a failed check.
        const auto waitStart = std::chrono::steady_clock::now();
        const auto deadline = waitStart + std::chrono::seconds(3);
        while (instance.getStateProperty() != SoundState::Stopped && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ctx.Print("waited {:.3f} s for natural completion (3 s limit)",
                  std::chrono::duration<double>(std::chrono::steady_clock::now() - waitStart).count());
        ctx.Check(instance.getStateProperty() == SoundState::Stopped, "and stops at its end");
    }
}

CNA_DEMOS_CONSOLE(FromStream, {
    .id = "audio.sound_effect.from_stream",
    .title = "Load a WAV at run time",
    .description = "Loads a loose WAV file with SoundEffect::FromStream -- no content pipeline -- checks its "
                   "Duration, names it, and plays it through a SoundEffectInstance.",
    .apis = {"SoundEffect::FromStream", "SoundEffect::Duration", "SoundEffect::Name", "SoundEffect::CreateInstance",
             "SoundEffectInstance::Play", "SoundEffectInstance::State"},
    .tags = {"audio", "wav", "loading", "runtime"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_440.wav"},
})
