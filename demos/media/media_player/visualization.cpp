// SPDX-License-Identifier: MS-PL
// MediaPlayer visualization: the playing music's spectrum and waveform, every frame.
#include <algorithm>
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

            song_.reset(Song::FromUri("Rising", TitleLocation::Path() + "/assets/music/rising.wav"));
            // Visualization data is captured only while enabled.
            MediaPlayer::setIsVisualizationEnabledProperty(true);
            MediaPlayer::setIsRepeatingProperty(true);
            MediaPlayer::Play(song_.get());
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
            for (const float band : data_.getFrequenciesProperty())
            {
                energy += band;
                loudestBand_ = std::max(loudestBand_, band);
            }
            peak_ = std::max(peak_, energy);
            if (!checked_ && gameTime.getTotalGameTimeProperty().getTotalSecondsProperty() > 1.0)
            {
                Context().Print("strongest spectrum so far: {:.2f} (sum of the 256 bands)", peak_);
                Context().Check(peak_ > 0.0f, "the spectrum shows the playing music");
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
        std::unique_ptr<Song> song_;
        VisualizationData data_;
        float peak_ = 0.0f;
        float loudestBand_ = 0.0f;
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(VisualizationDemo, {
    .id = "media.media_player.visualization",
    .title = "Music visualization",
    .description = "Checks VisualizationData's two zero-initialized 256-value buffers, proves disabled capture clears "
                   "caller data, then enables MediaPlayer visualization while a tune repeats and draws its spectrum "
                   "and waveform.",
    .apis = {"MediaPlayer::IsVisualizationEnabled", "MediaPlayer::GetVisualizationData",
             "VisualizationData::VisualizationData", "VisualizationData::Frequencies",
             "VisualizationData::Samples", "MediaPlayer::IsRepeating"},
    .tags = {"media", "music", "spectrum", "visualizer", "fft"},
    .needs = CnaDemos::Need::Audio,
    .frames = 120,
    .assets = {"fonts/pixel_font.cnb", "music/rising.wav"},
})
