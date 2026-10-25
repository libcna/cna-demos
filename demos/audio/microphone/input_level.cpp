// SPDX-License-Identifier: MS-PL
// Microphone: explicit, owned capture with a removable BufferReady subscription.
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/Microphone.hpp"
#include "Microsoft/Xna/Framework/Audio/MicrophoneState.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    struct CaptureCallbackState
    {
        void Read()
        {
            std::scoped_lock lock(mutex);
            if (!active || microphone == nullptr)
                return;

            const int bytes = microphone->GetData(buffer);
            double sum = 0.0;
            for (int i = 0; i + 1 < bytes; i += 2)
            {
                const auto sample = static_cast<std::int16_t>(
                    static_cast<std::uint8_t>(buffer[static_cast<std::size_t>(i)]) |
                    (static_cast<std::uint8_t>(buffer[static_cast<std::size_t>(i) + 1]) << 8));
                sum += static_cast<double>(sample) * sample;
            }
            const double rms = bytes > 1 ? std::sqrt(sum / (bytes / 2)) / 32768.0 : 0.0;
            level.store(std::min(1.0, rms * 4.0), std::memory_order_relaxed);
            callbacks.fetch_add(1, std::memory_order_relaxed);
        }

        std::mutex mutex;
        Microphone* microphone = nullptr;
        std::vector<SharpRuntime::bytecs> buffer;
        bool active = false;
        std::atomic<double> level{0.0};
        std::atomic<int> callbacks{0};
    };

    class InputLevelDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            const auto& microphones = Microphone::getAllProperty();
            microphone_ = Microphone::getDefaultProperty();
            Context().Check((microphone_ == nullptr) == microphones.empty(),
                            "Default is absent exactly when microphone enumeration is empty");
            if (microphone_ == nullptr)
            {
                status_ = "No microphone is connected; safe enumeration checks still ran.";
                return;
            }

            Context().Check(microphones.front() == microphone_, "Default is the first device enumerated by All");
            Context().Check(!microphone_->Name.empty(), "the enumerated microphone exposes a nonempty device name");
            isHeadset_ = microphone_->getIsHeadsetProperty();

            constexpr double bufferMilliseconds = 100.0;
            const System::TimeSpan requested = System::TimeSpan::FromMilliseconds(bufferMilliseconds);
            const int sampleBytes = microphone_->GetSampleSizeInBytes(requested);
            Context().Check(sampleBytes > 0 && microphone_->GetSampleDuration(sampleBytes) == requested,
                            "GetSampleDuration reverses the 100 ms mono capture buffer size without opening hardware");

            callbackState_ = std::make_shared<CaptureCallbackState>();
            callbackState_->microphone = microphone_;
            callbackState_->buffer.resize(static_cast<std::size_t>(sampleBytes));
            subscribersBefore_ = microphone_->BufferReady.Size();
            const std::weak_ptr<CaptureCallbackState> weak = callbackState_;
            subscription_.emplace(microphone_->BufferReady.Add(
                [weak](System::Object*, const System::EventArgs&)
                {
                    if (const auto state = weak.lock())
                        state->Read();
                }));
            Context().Check(microphone_->BufferReady.Size() == subscribersBefore_ + 1,
                            "BufferReady subscription has an owned removal token");

            previousKeyboard_ = Keyboard::GetState();
            status_ = microphone_->getStateProperty() == MicrophoneState::Stopped
                ? "Space: start capture (a second press stops it)."
                : "The shared microphone is already active; this demo will not take ownership.";
            Context().Print("found {} at {} Hz ({})", microphone_->Name, microphone_->getSampleRateProperty(),
                            isHeadset_ ? "headset" : "not a headset");
        }

        void UnloadContent() override
        {
            StopOwnedCapture();
            if (microphone_ != nullptr && subscription_)
            {
                microphone_->BufferReady.Remove(*subscription_);
                subscription_.reset();
                Context().Check(microphone_->BufferReady.Size() == subscribersBefore_,
                                "BufferReady handler is unregistered during shutdown");
            }
            callbackState_.reset();
        }

        void Update(GameTime& gameTime) override
        {
            const KeyboardState keyboard = Keyboard::GetState();
            const bool activate = !Context().Automated() && keyboard.IsKeyDown(Keys::Space) &&
                previousKeyboard_.IsKeyUp(Keys::Space);
            previousKeyboard_ = keyboard;

            if (activate && microphone_ != nullptr)
            {
                if (ownsCapture_)
                    StopOwnedCapture();
                else
                    StartCapture();
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, status_, Vector2(40.0f, 40.0f), Color::White);
            const double level = callbackState_ ? callbackState_->level.load(std::memory_order_relaxed) : 0.0;
            spriteBatch_->Draw(*pixel_, Rectangle(40, 200, static_cast<int>(720.0 * level), 60), Color::LimeGreen);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void StartCapture()
        {
            if (microphone_->getStateProperty() != MicrophoneState::Stopped)
            {
                status_ = "Microphone is already active; ownership was not taken.";
                return;
            }

            savedDuration_ = microphone_->getBufferDurationProperty();
            microphone_->setBufferDurationProperty(System::TimeSpan::FromMilliseconds(100.0));
            {
                std::scoped_lock lock(callbackState_->mutex);
                callbackState_->active = true;
            }
            try
            {
                microphone_->Start();
            }
            catch (const std::exception& exception)
            {
                {
                    std::scoped_lock lock(callbackState_->mutex);
                    callbackState_->active = false;
                }
                if (microphone_->getStateProperty() == MicrophoneState::Started)
                    microphone_->Stop();
                microphone_->setBufferDurationProperty(savedDuration_);
                status_ = std::string("Capture start failed safely: ") + exception.what();
                return;
            }
            ownsCapture_ = microphone_->getStateProperty() == MicrophoneState::Started;
            if (!ownsCapture_)
            {
                std::scoped_lock lock(callbackState_->mutex);
                callbackState_->active = false;
                microphone_->setBufferDurationProperty(savedDuration_);
                status_ = "Capture did not start.";
                return;
            }
            status_ = "Capturing: speak into the microphone. Space stops capture.";
        }

        void StopOwnedCapture()
        {
            if (!ownsCapture_)
                return;
            {
                std::scoped_lock lock(callbackState_->mutex);
                callbackState_->active = false;
            }
            if (microphone_->getStateProperty() == MicrophoneState::Started)
                microphone_->Stop();
            microphone_->setBufferDurationProperty(savedDuration_);
            ownsCapture_ = false;
            status_ = "Capture stopped and the prior buffer duration restored. Space restarts it.";
        }

        Microphone* microphone_ = nullptr;
        bool isHeadset_ = false;
        bool ownsCapture_ = false;
        std::size_t subscribersBefore_ = 0;
        std::optional<System::EventHandler<System::EventArgs>::Token> subscription_;
        System::TimeSpan savedDuration_{};
        KeyboardState previousKeyboard_;
        std::shared_ptr<CaptureCallbackState> callbackState_;
        std::string status_ = "Checking microphone capability...";
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(InputLevelDemo, {
    .id = "audio.microphone.input_level",
    .title = "Microphone input level",
    .description = "Enumerates microphones and validates sample conversion without opening hardware. In an attended "
                   "run, a fresh Space press starts owned capture; another press or shutdown stops it, restores the "
                   "prior buffer duration and removes the token-owned BufferReady handler.",
    .apis = {"Microphone::All", "Microphone::Default", "Microphone::BufferDuration", "Microphone::BufferReady",
             "Microphone::GetData", "Microphone::GetSampleDuration", "Microphone::GetSampleSizeInBytes",
             "Microphone::IsHeadset", "Microphone::Name", "Microphone::SampleRate", "Microphone::Start",
             "Microphone::Stop", "Microphone::State", "MicrophoneState"},
    .tags = {"audio", "microphone", "recording", "voice", "explicit-consent", "owned-lifetime"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = {"Real capture requires a fresh Space press in an attended run; unattended runs only enumerate "
                     "and validate value conversions."},
})
