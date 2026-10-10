// SPDX-License-Identifier: MS-PL
// SoundEffect from raw PCM: synthesise samples in code, wrap them in a SoundEffect, play them.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioChannels.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    void FromPcmBuffer(CnaDemos::DemoContext& ctx)
    {
        // 16-bit signed little-endian PCM, mono, 24,000 samples per second: 0.25 s of a 523 Hz tone.
        constexpr int sampleRate = 24000;
        constexpr int samples = sampleRate / 4;
        std::vector<SharpRuntime::bytecs> pcm;
        pcm.reserve(samples * 2);
        for (int i = 0; i < samples; ++i)
        {
            const float fade = std::min(1.0f, std::min(i, samples - 1 - i) / 200.0f);  // no clicks
            const auto value = static_cast<std::int16_t>(
                12000.0f * fade * std::sin(MathHelper::TwoPi * 523.25f * static_cast<float>(i) / sampleRate));
            pcm.push_back(static_cast<SharpRuntime::bytecs>(value & 0xFF));
            pcm.push_back(static_cast<SharpRuntime::bytecs>((value >> 8) & 0xFF));
        }

        SoundEffect tone(pcm, sampleRate, AudioChannels::Mono);
        const double seconds = tone.getDurationProperty().getTotalSecondsProperty();
        ctx.Print("{} bytes of PCM -> Duration {:.4f} s", pcm.size(), seconds);
        ctx.CheckNear(seconds, 0.25, 1e-3, "Duration follows from bytes, sample rate and channels");

        // The same arithmetic as static helpers, e.g. for sizing streaming buffers.
        const double helper = SoundEffect::GetSampleDuration(static_cast<int>(pcm.size()), sampleRate, AudioChannels::Mono)
                                  .getTotalSecondsProperty();
        ctx.CheckNear(helper, seconds, 1e-6, "GetSampleDuration agrees");
        const int oneSecondStereo =
            SoundEffect::GetSampleSizeInBytes(System::TimeSpan::FromSeconds(1.0), 44100, AudioChannels::Stereo);
        ctx.Print("one second of 44.1 kHz 16-bit stereo = {} bytes", oneSecondStereo);
        ctx.Check(oneSecondStereo == 44100 * 2 * 2, "GetSampleSizeInBytes: rate x channels x 2 bytes");

        ctx.Check(tone.Play(), "the synthesised sound plays");
    }
}

CNA_DEMOS_CONSOLE(FromPcmBuffer, {
    .id = "audio.sound_effect.from_pcm_buffer",
    .title = "Sound from raw PCM",
    .description = "Synthesises a quarter second of 16-bit PCM, creates a SoundEffect from the buffer, and checks "
                   "Duration against SoundEffect::GetSampleDuration and GetSampleSizeInBytes.",
    .apis = {"SoundEffect::SoundEffect", "SoundEffect::Duration", "SoundEffect::GetSampleDuration",
             "SoundEffect::GetSampleSizeInBytes", "SoundEffect::Play", "AudioChannels::Mono", "AudioChannels::Stereo"},
    .tags = {"audio", "pcm", "synthesis", "procedural"},
    .needs = CnaDemos::Need::Audio,
})
