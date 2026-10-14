// SPDX-License-Identifier: MS-PL
// DynamicSoundEffectInstance: stream generated audio, refilling buffers whenever BufferNeeded fires.
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioChannels.hpp"
#include "Microsoft/Xna/Framework/Audio/DynamicSoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "System/EventArgs.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    constexpr int SampleRate = 24000;

    class StreamingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            voice_ = std::make_unique<DynamicSoundEffectInstance>(SampleRate, AudioChannels::Mono);
            // 50 ms per buffer: small buffers mean low latency, but they must be refilled often.
            bufferBytes_ = voice_->GetSampleSizeInBytes(System::TimeSpan::FromMilliseconds(50));
            Context().Print("50 ms of 24 kHz mono 16-bit = {} bytes per buffer", bufferBytes_);

            Context().Check(bufferBytes_ == 2400, "50 ms contains 1200 mono 16-bit samples");
            Context().CheckNear(voice_->GetSampleDuration(bufferBytes_).getTotalMillisecondsProperty(), 50.0, 1e-6,
                                "the dynamic instance converts the byte count back to its duration");

            DynamicSoundEffectInstance recoveryProbe(SampleRate, AudioChannels::Mono);
            const std::vector<SharpRuntime::bytecs> recoveryBuffer(static_cast<std::size_t>(bufferBytes_), 0);
            int recoveryRequests = 0;
            int oneShotRequests = 0;
            bool recoveryPayloads = true;
            System::EventHandler<System::EventArgs>::Token oneShotToken = 0;
            oneShotToken = recoveryProbe.BufferNeeded.Add(
                [&](System::Object*, const System::EventArgs&) {
                    ++oneShotRequests;
                    recoveryProbe.BufferNeeded.Remove(oneShotToken);
                });
            const auto recoveryToken = recoveryProbe.BufferNeeded.Add(
                [&](System::Object* sender, const System::EventArgs& args) {
                    ++recoveryRequests;
                    recoveryPayloads = recoveryPayloads && sender == &recoveryProbe &&
                                       &args == &System::EventArgs::Empty;
                    recoveryProbe.SubmitBuffer(recoveryBuffer);
                });
            recoveryProbe.Play();
            recoveryProbe.Update();
            Context().Check(recoveryRequests == 3 && recoveryProbe.getPendingBufferCountProperty() == 3 &&
                                recoveryProbe.getStateProperty() == SoundState::Playing && recoveryPayloads,
                            "an empty playing stream requests exactly three buffers and recovers synchronously");
            Context().Check(oneShotRequests == 1 && recoveryRequests == 3 && recoveryProbe.BufferNeeded.Size() == 1,
                            "a listener can remove itself without suppressing the independent refill listener");
            recoveryProbe.BufferNeeded.Remove(recoveryToken);
            recoveryProbe.Stop();

            // Streams repeat by supplying more samples, not by looping an authored sound.
            voice_->setIsLoopedProperty(false);
            bool loopRejected = false;
            try { voice_->setIsLoopedProperty(true); }
            catch (const System::InvalidOperationException&) { loopRejected = true; }
            Context().Check(loopRejected && !voice_->getIsLoopedProperty(),
                            "a dynamic stream refuses IsLooped=true and remains non-looping");

            // Raised (from the game loop) when the voice is running out of queued audio.
            voice_->BufferNeeded += [this](System::Object*, const System::EventArgs&) {
                ++requests_;
                Submit();
            };
            Submit();
            Submit();
            Context().Check(voice_->getPendingBufferCountProperty() == 2,
                            "two submitted buffers are pending before playback starts");
            voice_->Play();
            // There is no authored release region to enter: Stop(false) is invalid once playing.
            bool releaseRejected = false;
            try { voice_->Stop(false); }
            catch (const System::InvalidOperationException&) { releaseRejected = true; }
            Context().Check(releaseRejected && voice_->getStateProperty() == SoundState::Playing,
                            "Stop(false) is rejected without interrupting the stream");
        }

        void Update(GameTime& gameTime) override
        {
            if (!playbackChecked_ && FramesDrawn() >= 5)
            {
                Context().Check(voice_->getStateProperty() == SoundState::Playing, "the stream is playing");
                playbackChecked_ = true; // fixed-step catch-up can run several Updates before one Draw
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateBlue);
            Game::Draw(gameTime);
        }

        void UnloadContent() override
        {
            Context().Print("BufferNeeded raised {} times; {} buffers submitted; {} still queued", requests_, submitted_,
                            voice_->getPendingBufferCountProperty());
            Context().Check(requests_ > 0, "the voice asked for more audio while playing");
            voice_->Stop();
            Context().Check(voice_->getStateProperty() == SoundState::Stopped &&
                            voice_->getPendingBufferCountProperty() == 0,
                            "immediate Stop stops playback and clears the queued buffers");
            voice_->Dispose();
            Context().Check(voice_->getIsDisposedProperty(), "Dispose marks the stream as disposed");
            bool pendingRejected = false;
            int disposedPending = -1;
            try { disposedPending = voice_->getPendingBufferCountProperty(); }
            catch (const System::ObjectDisposedException&) { pendingRejected = true; }
            if (!pendingRejected) Context().Print("Disposed PendingBufferCount returned {}", disposedPending);
            Context().Check(pendingRejected, "PendingBufferCount rejects a disposed stream (CNA-DEMOS-ISSUE-038)");
            bool loopRejected = false;
            bool disposedLooped = true;
            try { disposedLooped = voice_->getIsLoopedProperty(); }
            catch (const System::ObjectDisposedException&) { loopRejected = true; }
            if (!loopRejected) Context().Print("Disposed IsLooped returned {}", disposedLooped);
            Context().Check(loopRejected, "IsLooped rejects a disposed stream (CNA-DEMOS-ISSUE-038)");
            voice_.reset();
        }

    private:
        // A continuous 330 Hz tone, the phase carried from buffer to buffer so there are no clicks.
        void Submit()
        {
            std::vector<SharpRuntime::bytecs> pcm(static_cast<std::size_t>(bufferBytes_));
            for (std::size_t i = 0; i + 1 < pcm.size(); i += 2)
            {
                const auto value = static_cast<std::int16_t>(8000.0 * std::sin(phase_));
                phase_ += MathHelper::TwoPi * 330.0 / SampleRate;
                pcm[i] = static_cast<SharpRuntime::bytecs>(value & 0xFF);
                pcm[i + 1] = static_cast<SharpRuntime::bytecs>((value >> 8) & 0xFF);
            }
            voice_->SubmitBuffer(pcm);
            ++submitted_;
        }

        std::unique_ptr<DynamicSoundEffectInstance> voice_;
        int bufferBytes_ = 0;
        int requests_ = 0;
        int submitted_ = 0;
        double phase_ = 0.0;
        bool playbackChecked_ = false;
    };
}

CNA_DEMOS_GAME(StreamingDemo, {
    .id = "audio.dynamic_sound_effect_instance.streaming",
    .title = "Streaming generated audio",
    .description = "Recovers an empty DynamicSoundEffectInstance through independently mutable BufferNeeded listeners, "
                   "then plays a continuous tone and checks PCM duration, stream restrictions and disposal guards.",
    .apis = {"DynamicSoundEffectInstance::DynamicSoundEffectInstance", "DynamicSoundEffectInstance::BufferNeeded",
             "DynamicSoundEffectInstance::SubmitBuffer", "DynamicSoundEffectInstance::PendingBufferCount",
             "DynamicSoundEffectInstance::GetSampleSizeInBytes", "DynamicSoundEffectInstance::GetSampleDuration",
             "DynamicSoundEffectInstance::IsLooped", "DynamicSoundEffectInstance::Play",
             "DynamicSoundEffectInstance::Stop", "SoundEffectInstance::Dispose"},
    .tags = {"audio", "streaming", "synthesis", "procedural"},
    .needs = CnaDemos::Need::Audio,
    .frames = 60,
    .knownIssue = "CNA-DEMOS-ISSUE-038",
})
