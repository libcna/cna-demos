// SPDX-License-Identifier: MS-PL
// SoundEffect::FromStream: a WAV file loaded at run time, without the content pipeline.
#include <chrono>
#include <cmath>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/NotSupportedException.hpp"

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

        file.clear();
        file.seekg(0);
        const std::string wavBytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::unique_ptr<SoundEffect> positionedTone;
        {
            std::istringstream positioned(std::string("prefix") + wavBytes);
            positioned.seekg(6);
            positionedTone.reset(SoundEffect::FromStream(positioned));
        }
        ctx.Check(positionedTone &&
                      std::abs(positionedTone->getDurationProperty().getTotalSecondsProperty() - 0.5) < 0.01,
                  "FromStream starts at the current position and the result outlives its source stream");

        const auto unsupported = [](std::istream& stream) {
            try
            {
                const std::unique_ptr<SoundEffect> unexpected(SoundEffect::FromStream(stream));
                return false;
            }
            catch (const System::NotSupportedException&)
            {
                return true;
            }
        };
        std::istringstream empty;
        std::istringstream garbage("this is not audio data");
        std::istringstream truncated(wavBytes.substr(0, wavBytes.size() / 2));
        ctx.Check(unsupported(empty) && unsupported(garbage) && unsupported(truncated),
                  "empty, non-audio and truncated WAV streams are refused as unsupported");

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
    .description = "Loads a loose WAV from the current stream position, checks caller-owned lifetime and malformed "
                   "input refusals, then names and plays it through a SoundEffectInstance.",
    .apis = {"SoundEffect::FromStream", "SoundEffect::Duration", "SoundEffect::Name", "SoundEffect::CreateInstance",
             "SoundEffectInstance::Play", "SoundEffectInstance::State"},
    .tags = {"audio", "wav", "loading", "runtime"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_440.wav"},
})
