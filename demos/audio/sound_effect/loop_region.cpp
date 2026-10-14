// SPDX-License-Identifier: MS-PL
// A SoundEffect with a loop region: an intro that plays once, then a section that repeats.
#include <chrono>
#include <cmath>
#include <cstdint>
#include <thread>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioChannels.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "Microsoft/Xna/Framework/FrameworkDispatcher.hpp"
#include "System/ArgumentException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    constexpr int Rate = 22050;
    constexpr int IntroSamples = Rate / 4; // 0.25 s rising chirp
    constexpr int LoopSamples = Rate / 4;  // 0.25 s steady hum, repeated

    // 16-bit mono PCM: a chirp from 300 to 600 Hz, then exactly 50 cycles of 200 Hz, which loops
    // without a click because it ends where it starts.
    std::vector<std::uint8_t> MakeEngineSound()
    {
        std::vector<std::uint8_t> bytes;
        double phase = 0.0;
        for (int i = 0; i < IntroSamples + LoopSamples; ++i)
        {
            const double frequency = i < IntroSamples ? 300.0 + 300.0 * i / IntroSamples : 200.0;
            if (i == IntroSamples)
                phase = 0.0;
            phase += 2.0 * 3.14159265358979323846 * frequency / Rate;
            const auto sample = static_cast<std::int16_t>(0.3 * std::sin(phase) * 32767.0);
            bytes.push_back(static_cast<std::uint8_t>(static_cast<std::uint16_t>(sample) & 0xFF));
            bytes.push_back(static_cast<std::uint8_t>(static_cast<std::uint16_t>(sample) >> 8));
        }
        return bytes;
    }

    void LoopRegion(CnaDemos::DemoContext& ctx)
    {
        const std::vector<std::uint8_t> pcm = MakeEngineSound();
        // loopStart and loopLength count samples (per channel), not bytes.
        SoundEffect engine(pcm, 0, static_cast<int>(pcm.size()), Rate, AudioChannels::Mono, IntroSamples, LoopSamples);
        ctx.Print("Duration {} ms: a {} ms intro, then a {} ms loop region", engine.getDurationProperty().getTotalMillisecondsProperty(),
                  IntroSamples * 1000 / Rate, LoopSamples * 1000 / Rate);
        ctx.Check(std::abs(engine.getDurationProperty().getTotalSecondsProperty() - 0.5) < 0.001,
                  "the effect lasts the whole buffer, intro and loop");

        // A looped instance plays the intro once and then repeats only the region.
        SoundEffectInstance voice = engine.CreateInstance();
        voice.setIsLoopedProperty(true);
        voice.Play();
        FrameworkDispatcher::Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        FrameworkDispatcher::Update();
        ctx.Check(voice.getStateProperty() == SoundState::Playing,
                  "after more than twice the buffer it is still playing (the region loops)");
        voice.Stop();
        ctx.Check(voice.getStateProperty() == SoundState::Stopped, "Stop ends the loop");

        // A region ending exactly at the final sample is valid above; negative values and any
        // region reaching farther are rejected before they reach the audio backend.
        const auto loopRegionRejected = [&](int start, int length) {
            bool refused = false;
            try
            {
                SoundEffect bad(pcm, 0, static_cast<int>(pcm.size()), Rate, AudioChannels::Mono, start, length);
            }
            catch (const System::ArgumentException&)
            {
                refused = true;
            }
            return refused;
        };
        ctx.Check(loopRegionRejected(IntroSamples, LoopSamples + 1) && loopRegionRejected(-1, LoopSamples) &&
                      loopRegionRejected(IntroSamples, -1),
                  "past-end, negative-start and negative-length loop regions are refused");
    }
}

CNA_DEMOS_CONSOLE(LoopRegion, {
    .id = "audio.sound_effect.loop_region",
    .title = "A loop region",
    .description = "Builds a SoundEffect from PCM with an exact-end loop region, plays its intro and repeating "
                   "section, and refuses past-end or negative loop coordinates.",
    .apis = {"SoundEffect::SoundEffect", "SoundEffect::Duration", "SoundEffectInstance::IsLooped",
             "SoundEffectInstance::Play", "SoundEffectInstance::State", "SoundEffectInstance::Stop",
             "ArgumentException", "SoundEffect::CreateInstance"},
    .tags = {"audio", "sound", "loop", "engine"},
    .needs = CnaDemos::Need::Audio,
})
