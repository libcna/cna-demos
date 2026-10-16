// SPDX-License-Identifier: MS-PL
// MediaPlayer visualization: the playing music's spectrum and waveform, every frame.
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Media/MediaPlayer.hpp"
#include "Microsoft/Xna/Framework/Media/MediaState.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include "Microsoft/Xna/Framework/Media/VisualizationData.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    class VisualizationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            const bool zeroFrequencies = std::all_of(data_.getFrequenciesProperty().begin(),
                                                     data_.getFrequenciesProperty().end(),
                                                     [](float value) { return value == 0.0f; });
            const bool zeroSamples = std::all_of(data_.getSamplesProperty().begin(),
                                                 data_.getSamplesProperty().end(),
                                                 [](float value) { return value == 0.0f; });
            Context().Check(data_.getFrequenciesProperty().size() == VisualizationData::Size &&
                                data_.getSamplesProperty().size() == VisualizationData::Size &&
                                zeroFrequencies && zeroSamples,
                            "new visualization data contains two zero-initialized 256-value buffers");

            data_.freq.fill(0.25f);
            data_.samp.fill(-0.5f);
            MediaPlayer::setIsVisualizationEnabledProperty(false);
            MediaPlayer::GetVisualizationData(data_);
            const bool disabledFrequenciesZero = std::all_of(data_.getFrequenciesProperty().begin(),
                                                             data_.getFrequenciesProperty().end(),
                                                             [](float value) { return value == 0.0f; });
            const bool disabledSamplesZero = std::all_of(data_.getSamplesProperty().begin(),
                                                         data_.getSamplesProperty().end(),
                                                         [](float value) { return value == 0.0f; });
            Context().Check(!MediaPlayer::getIsVisualizationEnabledProperty() &&
                                disabledFrequenciesZero && disabledSamplesZero,
                            "disabled visualization clears both caller buffers instead of exposing stale data");

            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            tone440_.reset(Song::FromUri("440 Hz", TitleLocation::Path() + "/assets/sounds/tone_440.wav"));
            tone660_.reset(Song::FromUri("660 Hz", TitleLocation::Path() + "/assets/sounds/tone_660.wav"));
            // Visualization data is captured only while enabled.
            MediaPlayer::setIsVisualizationEnabledProperty(true);
            MediaPlayer::setIsRepeatingProperty(true);
            MediaPlayer::Play(tone440_.get());
            phaseStarted_ = Clock::now();
        }

        void UnloadContent() override
        {
            MediaPlayer::Stop();
            MediaPlayer::setIsRepeatingProperty(false);
            MediaPlayer::setIsVisualizationEnabledProperty(false);
        }

        void Update(GameTime& gameTime) override
        {
            // 256 frequency bands (0..1, low to high) and 256 samples of the waveform (-1..1).
            MediaPlayer::GetVisualizationData(data_);
            float energy = 0.0f;
            const auto& frequencies = data_.getFrequenciesProperty();
            for (std::size_t i = 0; i < frequencies.size(); ++i)
            {
                const float band = frequencies[i];
                energy += band;
                loudestBand_ = std::max(loudestBand_, band);
                rangesValid_ = rangesValid_ && std::isfinite(band) && band >= 0.0f && band <= 1.0f;
                if (i > 0 && band > strongestMagnitude_[phase_])
                {
                    strongestMagnitude_[phase_] = band;
                    strongestBand_[phase_] = i;
                }
            }
            for (const float sample : data_.getSamplesProperty())
            {
                rangesValid_ = rangesValid_ && std::isfinite(sample) && sample >= -1.0f && sample <= 1.0f;
                sawPositiveSample_ = sawPositiveSample_ || sample > 0.1f;
                sawNegativeSample_ = sawNegativeSample_ || sample < -0.1f;
            }
            peak_ = std::max(peak_, energy);

            const double phaseSeconds =
                std::chrono::duration<double>(Clock::now() - phaseStarted_).count();
            const bool captureReady = strongestMagnitude_[phase_] > 0.0f;
            // Fixed-step GameTime may catch up faster than the audio callback under a loaded
            // parallel test run. Give the real mixer at least 750 ms and wait for its first
            // nonzero capture, with a finite wall-clock deadline that still exposes a broken
            // visualization path instead of silently reaching the automated frame limit.
            const bool finishPhase = phaseSeconds > 0.75 && (captureReady || phaseSeconds > 4.0);
            if (phase_ == 0 && finishPhase)
            {
                Context().Check(strongestMagnitude_[0] > 0.0f,
                                "the first playing tone produces a nonzero spectrum");
                MediaPlayer::Stop();
                MediaPlayer::setIsVisualizationEnabledProperty(false);
                MediaPlayer::Play(tone660_.get());
                MediaPlayer::setIsVisualizationEnabledProperty(true);
                phase_ = 1;
                phaseStarted_ = Clock::now();
            }
            else if (!checked_ && phase_ == 1 && finishPhase)
            {
                Context().Print("strongest spectrum so far: {:.2f} (sum of the 256 bands)", peak_);
                Context().Print("dominant tone bands: 440 Hz -> {}, 660 Hz -> {}",
                                strongestBand_[0], strongestBand_[1]);
                Context().Check(peak_ > 0.0f && strongestMagnitude_[1] > 0.0f,
                                "both playing tones produce spectrum energy");
                Context().Check(rangesValid_ && sawPositiveSample_ && sawNegativeSample_,
                                "frequency magnitudes and bipolar waveform samples stay in their documented ranges");
                Context().Check(strongestBand_[0] > 0 && strongestBand_[1] > strongestBand_[0] &&
                                    strongestBand_[1] <= strongestBand_[0] * 2 + 1,
                                "the 660 Hz tone peaks in a predictably higher band than the 440 Hz tone");
                checked_ = true;
                if (Context().Automated())
                    Exit();
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Frequencies", Vector2(20.0f, 20.0f), Color::White);
            const auto& bands = data_.getFrequenciesProperty();
            for (std::size_t i = 0; i < bands.size(); ++i)
            {
                // Scaled to the loudest band heard so far, so quiet music still shows.
                const int height = static_cast<int>(bands[i] / std::max(loudestBand_, 0.001f) * 180.0f);
                spriteBatch_->Draw(*pixel_, Rectangle(20 + static_cast<int>(i) * 3, 230 - height, 2, height),
                                   Color::Lerp(Color::LimeGreen, Color::OrangeRed, static_cast<float>(i) / 256.0f));
            }
            spriteBatch_->DrawString(*font_, "Samples", Vector2(20.0f, 260.0f), Color::White);
            const auto& samples = data_.getSamplesProperty();
            for (std::size_t i = 0; i < samples.size(); ++i)
                spriteBatch_->Draw(*pixel_, Rectangle(20 + static_cast<int>(i) * 3, 380 + static_cast<int>(samples[i] * 80.0f), 3, 3),
                                   Color::DeepSkyBlue);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        using Clock = std::chrono::steady_clock;

        std::unique_ptr<Song> tone440_;
        std::unique_ptr<Song> tone660_;
        VisualizationData data_;
        float peak_ = 0.0f;
        float loudestBand_ = 0.0f;
        std::array<float, 2> strongestMagnitude_{};
        std::array<std::size_t, 2> strongestBand_{};
        bool rangesValid_ = true;
        bool sawPositiveSample_ = false;
        bool sawNegativeSample_ = false;
        int phase_ = 0;
        Clock::time_point phaseStarted_{};
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(VisualizationDemo, {
    .id = "media.media_player.visualization",
    .title = "Music visualization",
    .description = "Checks VisualizationData's zero and disabled states, then captures repeating 440 Hz and 660 Hz "
                   "tones, verifies bounded bipolar samples and ordered dominant spectrum bands, and draws the live "
                   "spectrum and waveform.",
    .apis = {"MediaPlayer::IsVisualizationEnabled", "MediaPlayer::GetVisualizationData",
             "VisualizationData::VisualizationData", "VisualizationData::Frequencies",
             "VisualizationData::Samples", "MediaPlayer::IsRepeating"},
    .tags = {"media", "music", "spectrum", "visualizer", "fft"},
    .needs = CnaDemos::Need::Audio,
    .frames = 600,
    .assets = {"fonts/pixel_font.cnb", "sounds/tone_440.wav", "sounds/tone_660.wav"},
})
