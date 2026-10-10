// SPDX-License-Identifier: MS-PL
// Microphone: record from the default microphone and show the input level as a bar.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
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
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/EventArgs.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class InputLevelDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            microphone_ = Microphone::getDefaultProperty();
            if (microphone_ == nullptr)
                Context().Skip("no microphone is connected");

            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // Audio arrives in buffers of BufferDuration; BufferReady says one is waiting.
            microphone_->setBufferDurationProperty(System::TimeSpan::FromMilliseconds(100));
            buffer_.resize(static_cast<std::size_t>(microphone_->GetSampleSizeInBytes(microphone_->getBufferDurationProperty())));
            microphone_->BufferReady += [this](System::Object*, const System::EventArgs&) { ReadBuffer(); };
            microphone_->Start();
            Context().Print("recording at {} Hz", microphone_->getSampleRateProperty());
        }

        void UnloadContent() override
        {
            if (microphone_ != nullptr && microphone_->getStateProperty() == MicrophoneState::Started)
                microphone_->Stop();
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Speak into the microphone (Escape to quit)", Vector2(40.0f, 40.0f),
                                     Color::White);
            spriteBatch_->Draw(*pixel_, Rectangle(40, 200, static_cast<int>(720.0 * level_), 60), Color::LimeGreen);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        // The level of the latest buffer: root mean square of the 16-bit samples, 0..1.
        void ReadBuffer()
        {
            const int bytes = microphone_->GetData(buffer_);
            double sum = 0.0;
            for (int i = 0; i + 1 < bytes; i += 2)
            {
                const auto sample = static_cast<std::int16_t>(static_cast<std::uint8_t>(buffer_[static_cast<std::size_t>(i)]) |
                                                              (static_cast<std::uint8_t>(buffer_[static_cast<std::size_t>(i) + 1]) << 8));
                sum += static_cast<double>(sample) * sample;
            }
            const double rms = bytes > 0 ? std::sqrt(sum / (bytes / 2)) / 32768.0 : 0.0;
            level_ = std::min(1.0, rms * 4.0);
        }

        Microphone* microphone_ = nullptr;
        std::vector<SharpRuntime::bytecs> buffer_;
        double level_ = 0.0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(InputLevelDemo, {
    .id = "audio.microphone.input_level",
    .title = "Microphone input level",
    .description = "Records from Microphone::Default in 100 ms buffers, reads each with GetData in the BufferReady "
                   "event and shows the input level as a bar.",
    .apis = {"Microphone::Default", "Microphone::BufferDuration", "Microphone::BufferReady", "Microphone::GetData",
             "Microphone::GetSampleSizeInBytes", "Microphone::SampleRate", "Microphone::Start", "Microphone::Stop",
             "Microphone::State"},
    .tags = {"audio", "microphone", "recording", "voice"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Microphone,
    .assets = {"fonts/pixel_font.cnb"},
})
