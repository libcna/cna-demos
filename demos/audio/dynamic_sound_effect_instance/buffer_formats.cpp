// SPDX-License-Identifier: MS-PL
// Stereo stream buffers: byte/sample units, frame alignment and stop-before-format-change.
#include <cstdint>
#include <limits>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioChannels.hpp"
#include "Microsoft/Xna/Framework/Audio/DynamicSoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    void BufferFormats(CnaDemos::DemoContext& ctx)
    {
        constexpr int sampleRate = 24000;
        constexpr int frames = 1200; // 50 ms, two interleaved channels per frame
        constexpr int pcmBytes = frames * 2 * 2;
        constexpr int floatSamples = frames * 2;
        // One whole guard frame at each end; submit only the middle 50 ms.
        std::vector<SharpRuntime::bytecs> pcm(pcmBytes + 8, 0);
        std::vector<float> floats(floatSamples + 4, 0.0f);
        DynamicSoundEffectInstance voice(sampleRate, AudioChannels::Stereo);
        ctx.Check(voice.GetSampleSizeInBytes(System::TimeSpan::FromMilliseconds(50)) == pcmBytes,
                  "50 ms of stereo 16-bit PCM is 4800 bytes, including both channels");
        voice.SubmitBuffer(pcm, 4, pcmBytes);
        ctx.Check(voice.getPendingBufferCountProperty() == 1,
                  "a byte range starting after one stereo frame queues one integer buffer");

        bool refused = false;
        try { voice.SubmitBuffer(pcm, 2, pcmBytes); }
        catch (const System::ArgumentException&) { refused = true; }
        ctx.Check(refused && voice.getPendingBufferCountProperty() == 1,
                  "an offset on a sample but between stereo frames is refused without adding a buffer");
        refused = false;
        try { voice.SubmitBuffer(pcm, 4, pcmBytes + 2); }
        catch (const System::ArgumentException&) { refused = true; }
        ctx.Check(refused && voice.getPendingBufferCountProperty() == 1,
                  "a byte count ending between stereo frames is refused without changing the queue");

        // Float ranges use sample bounds, independently of the integer frame rules above.
        // A refused float request must not select float mode before the integer stream starts.
        refused = false;
        try { voice.SubmitFloatBufferEXT(floats, -1, 2); }
        catch (const System::ArgumentOutOfRangeException&) { refused = true; }
        ctx.Check(refused && voice.getStateProperty() == SoundState::Stopped &&
                  voice.getPendingBufferCountProperty() == 1,
                  "a negative float sample offset preserves the staged integer buffer");
        refused = false;
        try { voice.SubmitFloatBufferEXT(floats, 2, std::numeric_limits<SharpRuntime::intcs>::max()); }
        catch (const System::ArgumentOutOfRangeException&) { refused = true; }
        ctx.Check(refused && voice.getStateProperty() == SoundState::Stopped &&
                  voice.getPendingBufferCountProperty() == 1,
                  "an oversized float count is refused without changing the staged integer buffer");

        voice.Play();
        ctx.Check(voice.getStateProperty() == SoundState::Playing, "the integer stream starts playing");
        voice.Pause();
        ctx.Check(voice.getStateProperty() == SoundState::Paused, "pausing retains the live integer stream");
        const int pausedIntegerBuffers = voice.getPendingBufferCountProperty();
        refused = false;
        try { voice.SubmitFloatBufferEXT(floats, 2, floatSamples); }
        catch (const System::InvalidOperationException&) { refused = true; }
        ctx.Check(refused && voice.getStateProperty() == SoundState::Paused &&
                  voice.getPendingBufferCountProperty() == pausedIntegerBuffers,
                  "a float submission cannot change a paused integer stream or its pending buffers");
        // Discard queued audio while retaining the paused voice and its sample format.
        // ClearBuffers is not Stop: a float submission is still an invalid format switch.
        voice.ClearBuffers();
        ctx.Check(voice.getStateProperty() == SoundState::Paused && voice.getPendingBufferCountProperty() == 0,
                  "clearing a paused integer stream discards its buffers without stopping it");
        refused = false;
        try { voice.SubmitFloatBufferEXT(floats, 2, floatSamples); }
        catch (const System::InvalidOperationException&) { refused = true; }
        ctx.Check(refused && voice.getStateProperty() == SoundState::Paused &&
                  voice.getPendingBufferCountProperty() == 0,
                  "an empty paused stream still refuses a different sample format");
        voice.SubmitBuffer(pcm, 4, pcmBytes);
        voice.QueueInitialBuffers(); // explicitly submit the refill while the voice is paused
        // PendingBufferCount includes both staged and submitted buffers. It confirms the
        // retained queue/state here, not independent proof of the native transfer or samples.
        ctx.Check(voice.getStateProperty() == SoundState::Paused && voice.getPendingBufferCountProperty() == 1,
                  "the paused stream accepts a same-format refill before resuming");
        voice.Resume();
        ctx.Check(voice.getStateProperty() == SoundState::Playing,
                  "the original integer stream resumes after the refused format change");
        voice.Stop();
        ctx.Check(voice.getStateProperty() == SoundState::Stopped && voice.getPendingBufferCountProperty() == 0,
                  "stopping the started integer stream clears old data before switching formats");

        // Float offset/count are samples, not bytes. Select both channels of 1200 frames.
        voice.SubmitFloatBufferEXT(floats, 2, floatSamples);
        ctx.Check(voice.getPendingBufferCountProperty() == 1,
                  "the stopped stream accepts one 2400-sample stereo float range");
        refused = false;
        try { voice.SubmitFloatBufferEXT(floats, 2, -2); }
        catch (const System::ArgumentOutOfRangeException&) { refused = true; }
        ctx.Check(refused && voice.getStateProperty() == SoundState::Stopped &&
                  voice.getPendingBufferCountProperty() == 1,
                  "a negative float sample count preserves the staged float buffer");
        refused = false;
        try { voice.SubmitFloatBufferEXT(floats, static_cast<SharpRuntime::intcs>(floats.size()), 2); }
        catch (const System::ArgumentOutOfRangeException&) { refused = true; }
        ctx.Check(refused && voice.getStateProperty() == SoundState::Stopped &&
                  voice.getPendingBufferCountProperty() == 1,
                  "a nonempty float range beginning at the vector end leaves the queue unchanged");
        // These XNA helpers deliberately retain their 16-bit meaning in CNA's float extension.
        ctx.CheckNear(voice.GetSampleDuration(pcmBytes).getTotalMillisecondsProperty(), 50.0, 1e-6,
                      "the XNA duration helper still interprets its byte count as 16-bit PCM");
        voice.Play();
        ctx.Check(voice.getStateProperty() == SoundState::Playing, "the replacement float stream starts playing");
        voice.Pause();
        ctx.Check(voice.getStateProperty() == SoundState::Paused, "pausing retains the live float stream");
        const int pausedFloatBuffers = voice.getPendingBufferCountProperty();
        refused = false;
        try { voice.SubmitBuffer(pcm, 4, pcmBytes); }
        catch (const System::InvalidOperationException&) { refused = true; }
        ctx.Check(refused && voice.getStateProperty() == SoundState::Paused &&
                  voice.getPendingBufferCountProperty() == pausedFloatBuffers,
                  "an integer submission cannot change a paused float stream or its pending buffers");
        voice.Stop();
        ctx.Check(voice.getStateProperty() == SoundState::Stopped && voice.getPendingBufferCountProperty() == 0,
                  "stopping the float stream clears its data before returning to integer PCM");
        voice.SubmitBuffer(pcm, 4, pcmBytes);
        voice.Play();
        ctx.Check(voice.getStateProperty() == SoundState::Playing,
                  "the same instance plays integer PCM again after the stopped format transition");
        voice.Stop();

        // Use long independent buffers so the real playing-state guard is observed without a
        // race against the short format-transition fixtures reaching their natural end.
        constexpr int liveFrames = sampleRate * 2;
        const std::vector<SharpRuntime::bytecs> livePcm(liveFrames * 2 * 2, 0);
        const std::vector<float> liveFloats(liveFrames * 2, 0.0f);
        {
            DynamicSoundEffectInstance liveInteger(sampleRate, AudioChannels::Stereo);
            liveInteger.SubmitBuffer(livePcm);
            liveInteger.Play();
            const int pending = liveInteger.getPendingBufferCountProperty();
            refused = false;
            try { liveInteger.SubmitFloatBufferEXT(liveFloats); }
            catch (const System::InvalidOperationException&) { refused = true; }
            ctx.Check(refused && liveInteger.getStateProperty() == SoundState::Playing &&
                          liveInteger.getPendingBufferCountProperty() == pending,
                      "a playing integer stream refuses float data without changing its queue");
            liveInteger.Stop();
        }
        {
            DynamicSoundEffectInstance liveFloat(sampleRate, AudioChannels::Stereo);
            liveFloat.SubmitFloatBufferEXT(liveFloats);
            liveFloat.Play();
            const int pending = liveFloat.getPendingBufferCountProperty();
            refused = false;
            try { liveFloat.SubmitBuffer(livePcm); }
            catch (const System::InvalidOperationException&) { refused = true; }
            ctx.Check(refused && liveFloat.getStateProperty() == SoundState::Playing &&
                          liveFloat.getPendingBufferCountProperty() == pending,
                      "a playing float stream refuses integer data without changing its queue");
            liveFloat.Stop();
        }
        ctx.Print("Buffer units and stream state checked; silent fixtures do not measure output samples or audio quality.");
    }
}

CNA_DEMOS_CONSOLE(BufferFormats, {
    .id = "audio.dynamic_sound_effect_instance.buffer_formats",
    .title = "Stereo buffers and stopped format transitions",
    .description = "Submits stereo PCM byte ranges and float sample ranges, rejects partial integer frames and out-of-bounds float ranges, "
                   "refuses live format changes, clears and refills a paused queue, and demonstrates stopping before changing format.",
    .apis = {"DynamicSoundEffectInstance::DynamicSoundEffectInstance", "DynamicSoundEffectInstance::SubmitBuffer",
             "Microsoft::Xna::Framework::Audio::DynamicSoundEffectInstance::SubmitFloatBufferEXT",
             "Microsoft::Xna::Framework::Audio::DynamicSoundEffectInstance::ClearBuffers",
             "Microsoft::Xna::Framework::Audio::DynamicSoundEffectInstance::QueueInitialBuffers",
             "DynamicSoundEffectInstance::PendingBufferCount", "DynamicSoundEffectInstance::GetSampleSizeInBytes",
             "DynamicSoundEffectInstance::GetSampleDuration", "DynamicSoundEffectInstance::Play",
             "DynamicSoundEffectInstance::Stop", "SoundEffectInstance::Pause", "SoundEffectInstance::Resume",
             "SoundEffectInstance::State"},
    .tags = {"audio", "streaming", "stereo", "pcm", "cnaext"},
    .needs = CnaDemos::Need::Audio,
    .restrictions = "Silent stereo fixtures establish accepted ranges, refusal preservation and playback states. "
                    "No sample readback, audible quality, partial-float-frame output behavior "
                    "or concurrent producer behavior is inferred. Pending counts combine staged and submitted buffers; "
                    "unchanged counts do not independently prove native transfer.",
})
