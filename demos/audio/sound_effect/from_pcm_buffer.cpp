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
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
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

        std::vector<SharpRuntime::bytecs> padded(pcm.size() + 4, 0x55);
        std::copy(pcm.begin(), pcm.end(), padded.begin() + 2);
        SoundEffect window(padded, 2, static_cast<int>(pcm.size()), sampleRate, AudioChannels::Mono, 0, 0);
        ctx.CheckNear(window.getDurationProperty().getTotalSecondsProperty(), seconds, 1e-6,
                      "the range constructor consumes only its aligned middle PCM window");

        // The same arithmetic as static helpers, e.g. for sizing streaming buffers.
        const double helper = SoundEffect::GetSampleDuration(static_cast<int>(pcm.size()), sampleRate, AudioChannels::Mono)
                                  .getTotalSecondsProperty();
        ctx.CheckNear(helper, seconds, 1e-6, "GetSampleDuration agrees");
        const int oneSecondStereo =
            SoundEffect::GetSampleSizeInBytes(System::TimeSpan::FromSeconds(1.0), 44100, AudioChannels::Stereo);
        ctx.Print("one second of 44.1 kHz 16-bit stereo = {} bytes", oneSecondStereo);
        ctx.Check(oneSecondStereo == 44100 * 2 * 2, "GetSampleSizeInBytes: rate x channels x 2 bytes");
        ctx.Check(SoundEffect::GetSampleDuration(4000, 44100, AudioChannels::Stereo).getTicksProperty() ==
                          23 * System::TimeSpan::TicksPerMillisecond &&
                      SoundEffect::GetSampleSizeInBytes(System::TimeSpan::FromMilliseconds(83), 40622,
                                                        AudioChannels::Mono) == 6742 &&
                      SoundEffect::GetSampleSizeInBytes(System::TimeSpan::FromSeconds(1), 44100,
                                                        AudioChannels::Mono) == 88198,
                  "sample helpers use XNA millisecond rounding and whole-frame float arithmetic");
        ctx.Check(SoundEffect::GetSampleDuration(-1, sampleRate, AudioChannels::Mono) == System::TimeSpan::Zero &&
                      SoundEffect::GetSampleDuration(1000, 0, AudioChannels::Stereo) == System::TimeSpan::Zero &&
                      SoundEffect::GetSampleDuration(1000, sampleRate, static_cast<AudioChannels>(0)) ==
                          System::TimeSpan::Zero &&
                      SoundEffect::GetSampleSizeInBytes(System::TimeSpan::FromSeconds(1), 0,
                                                        AudioChannels::Stereo) == 0 &&
                      SoundEffect::GetSampleSizeInBytes(System::TimeSpan::FromSeconds(1), sampleRate,
                                                        static_cast<AudioChannels>(0)) == 0,
                  "sample helpers return zero for nonpositive sizes, rates or channel counts");

        bool offsetRejected = false;
        bool countRejected = false;
        bool rateRejected = false;
        bool channelsRejected = false;
        try { SoundEffect bad(padded, -2, static_cast<int>(pcm.size()), sampleRate, AudioChannels::Mono, 0, 0); }
        catch (const System::ArgumentException&) { offsetRejected = true; }
        try { SoundEffect bad(padded, 2, static_cast<int>(pcm.size()) - 1, sampleRate, AudioChannels::Mono, 0, 0); }
        catch (const System::ArgumentException&) { countRejected = true; }
        try { SoundEffect bad(pcm, 7999, AudioChannels::Mono); }
        catch (const System::ArgumentOutOfRangeException&) { rateRejected = true; }
        try { SoundEffect bad(pcm, sampleRate, static_cast<AudioChannels>(3)); }
        catch (const System::ArgumentOutOfRangeException&) { channelsRejected = true; }
        ctx.Check(offsetRejected && countRejected && rateRejected && channelsRejected,
                  "PCM construction distinguishes invalid windows from unsupported formats");

        ctx.Check(tone.Play(), "the synthesised sound plays");
    }
}

CNA_DEMOS_CONSOLE(FromPcmBuffer, {
    .id = "audio.sound_effect.from_pcm_buffer",
    .title = "Sound from raw PCM",
    .description = "Synthesises 16-bit PCM, exercises whole-buffer and aligned-window construction, and checks "
                   "SoundEffect sample-helper rounding, invalid inputs, duration and playback.",
    .apis = {"SoundEffect::SoundEffect", "SoundEffect::Duration", "SoundEffect::GetSampleDuration",
             "SoundEffect::GetSampleSizeInBytes", "SoundEffect::Play", "AudioChannels::Mono", "AudioChannels::Stereo"},
    .tags = {"audio", "pcm", "synthesis", "procedural"},
    .needs = CnaDemos::Need::Audio,
})
