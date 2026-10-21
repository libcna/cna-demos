// SPDX-License-Identifier: MS-PL
// VideoPlayer controls: Pause/Resume, IsLooped, Volume and IsMuted.
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Media/MediaState.hpp"
#include "Microsoft/Xna/Framework/Media/Video/Video.hpp"
#include "Microsoft/Xna/Framework/Media/Video/VideoPlayer.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    template<typename TException>
    bool ThrowsExactly(const std::function<void()>& action)
    {
        try
        {
            action();
        }
        catch (const TException&)
        {
            return true;
        }
        catch (...)
        {
        }
        return false;
    }

    // What the demo does next, driven by the player's own PlayPosition.
    enum class Step
    {
        PlayUntilHalfASecond,
        Paused,
        PlayUntilItLoops,
        AwaitLoopFrame,
        Done
    };

    class PauseAndLoopDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            // The media content build lives in assets/media (CNA-DEMOS-ISSUE-020).
            media_ = std::make_unique<Microsoft::Xna::Framework::Content::ContentManager>(&getServicesProperty(),
                                                                                          "assets/media");
            video_.emplace(media_->Load<Video>("videos/bounce"));

            player_ = std::make_unique<VideoPlayer>();
            ctx.Check(!player_->getIsDisposedProperty() && !player_->getIsLoopedProperty() &&
                          !player_->getIsMutedProperty() && player_->getVolumeProperty() == 1.0f &&
                          player_->getStateProperty() == MediaState::Stopped &&
                          player_->getVideoProperty() == nullptr &&
                          player_->getPlayPositionProperty() == System::TimeSpan::Zero,
                      "a fresh player is live, stopped at zero and has XNA's default configuration");
            ctx.Check(ThrowsExactly<System::InvalidOperationException>([&] {
                          static_cast<void>(player_->GetTexture());
                      }),
                      "GetTexture before Play throws InvalidOperationException (CNA-DEMOS-ISSUE-099)");
            ctx.Check(ThrowsExactly<System::ArgumentNullException>([&] { player_->Play(nullptr); }),
                      "Play rejects a null Video with ArgumentNullException (CNA-DEMOS-ISSUE-099)");
            const bool lowVolumeRefused = ThrowsExactly<System::ArgumentOutOfRangeException>(
                [&] { player_->setVolumeProperty(-0.25f); });
            const bool lowVolumeUnchanged = player_->getVolumeProperty() == 1.0f;
            const bool highVolumeRefused = ThrowsExactly<System::ArgumentOutOfRangeException>(
                [&] { player_->setVolumeProperty(1.25f); });
            ctx.Check(lowVolumeRefused && lowVolumeUnchanged && highVolumeRefused &&
                          player_->getVolumeProperty() == 1.0f,
                      "Volume refuses values outside [0,1] without mutation (CNA-DEMOS-ISSUE-099)");
            // Settings that belong to the player, not to the video: set before or during playback.
            player_->setIsLoopedProperty(true);
            player_->setVolumeProperty(0.5f);
            player_->setIsMutedProperty(true);
            ctx.Check(player_->getIsLoopedProperty() && player_->getVolumeProperty() == 0.5f && player_->getIsMutedProperty(),
                      "IsLooped, Volume and IsMuted keep their values");
            player_->setIsMutedProperty(false);
            player_->Play(&*video_);
            firstGeneration_ = player_->GetFrameGenerationEXT();
            ctx.Check(firstGeneration_ > 0 && player_->GetFramePresentationTimeEXT() >= 0.0,
                      "Play decodes a first frame with a nonzero generation and timestamp");
        }

        void UnloadContent() override
        {
            if (!player_->getIsDisposedProperty())
                player_->Stop();
            player_->Dispose();
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            auto& ctx = Context();
            const double position = player_->getPlayPositionProperty().getTotalSecondsProperty();
            switch (step_)
            {
                case Step::PlayUntilHalfASecond:
                    if (position > 0.5)
                    {
                        player_->Pause();
                        pausedAt_ = player_->getPlayPositionProperty().getTotalSecondsProperty();
                        pausedFrames_ = 0;
                        ctx.Check(player_->getStateProperty() == MediaState::Paused, "Pause: State is Paused");
                        step_ = Step::Paused;
                    }
                    break;
                case Step::Paused:
                    // The clock stands still while paused, however many frames pass.
                    if (++pausedFrames_ == 20)
                    {
                        ctx.Print("paused at {:.2f} s, still {:.2f} s twenty frames later", pausedAt_, position);
                        ctx.Check(position == pausedAt_, "PlayPosition does not move while paused");
                        player_->Resume();
                        ctx.Check(player_->getStateProperty() == MediaState::Playing, "Resume: State is Playing");
                        step_ = Step::PlayUntilItLoops;
                    }
                    break;
                case Step::PlayUntilItLoops:
                    // Looped: at the end of the clip the position starts again instead of stopping.
                    if (position + 1.0 < lastPosition_)
                    {
                        ctx.Print("wrapped from {:.2f} s to {:.2f} s", lastPosition_, position);
                        ctx.Check(player_->getStateProperty() == MediaState::Playing,
                                  "a looped video starts again and keeps playing");
                        generationAtLoop_ = player_->GetFrameGenerationEXT();
                        step_ = Step::AwaitLoopFrame;
                    }
                    break;
                case Step::AwaitLoopFrame:
                    // The EOF-detecting GetTexture resets the presentation timestamp during Draw;
                    // the following Draw decodes the first frame of the next loop.
                    if (player_->GetFrameGenerationEXT() > generationAtLoop_)
                    {
                        ctx.Check(player_->GetFrameGenerationEXT() > firstGeneration_ &&
                                      player_->GetFramePresentationTimeEXT() >= 0.0,
                                  "decoded frame generation remains monotonic across the loop");
                        step_ = Step::Done;
                        if (ctx.Automated())
                        {
                            Exit();
                        }
                    }
                    break;
                case Step::Done:
                    break;
            }
            lastPosition_ = position;
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            const MediaState state = player_->getStateProperty();
            const double position = player_->getPlayPositionProperty().getTotalSecondsProperty();
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::LinearClamp, nullptr, nullptr);
            if (state != MediaState::Stopped)
            {
                // While paused GetTexture keeps returning the frame the clock stopped on.
                if (Texture2D* frame = player_->GetTexture())
                {
                    spriteBatch_->Draw(*frame, Rectangle(160, 60, 480, 360), Color::White);
                }
            }
            const char* stateName = state == MediaState::Playing ? "Playing" : state == MediaState::Paused ? "Paused" : "Stopped";
            spriteBatch_->DrawString(*font_,
                                     std::string("VideoPlayer: ") + stateName + ", " + std::to_string(position).substr(0, 4) +
                                         " s, looped",
                                     Vector2(160.0f, 20.0f), Color::White);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

        void EndRun() override
        {
            if (Context().Automated())
            {
                Context().Check(step_ == Step::Done, "paused, resumed and looped during the run");
                Context().Check(!player_->getIsDisposedProperty() &&
                                    player_->getStateProperty() == MediaState::Playing,
                                "the looped player is still live before explicit disposal");

                std::unique_ptr<Video> rawVideo(
                    Video::FromUriEXT(video_->getFileNameProperty(), &getGraphicsDeviceProperty()));
                Context().Check(rawVideo && rawVideo->getWidthProperty() == video_->getWidthProperty() &&
                                    rawVideo->getHeightProperty() == video_->getHeightProperty() &&
                                    rawVideo->getFramesPerSecondProperty() == video_->getFramesPerSecondProperty() &&
                                    rawVideo->getDurationProperty().getTicksProperty() > 0 &&
                                    rawVideo->getFileNameProperty() == video_->getFileNameProperty() &&
                                    rawVideo->getGraphicsDeviceProperty() == &getGraphicsDeviceProperty() &&
                                    rawVideo->GetTypeName() == "Microsoft.Xna.Framework.Media.Video",
                                "FromUriEXT probes the same local video and retains its path and device");
                VideoPlayer stoppedPlayer;
                stoppedPlayer.Play(rawVideo.get());
                stoppedPlayer.Stop();
                Context().Check(stoppedPlayer.getStateProperty() == MediaState::Stopped &&
                                    stoppedPlayer.getPlayPositionProperty() == System::TimeSpan::Zero &&
                                    stoppedPlayer.getVideoProperty() == rawVideo.get(),
                                "Stop resets transport but retains the last Video identity (CNA-DEMOS-ISSUE-099)");

                auto shortLivedVideo = std::make_unique<Video>(
                    video_->getFileNameProperty(), &getGraphicsDeviceProperty());
                VideoPlayer dependentPlayer;
                dependentPlayer.Play(shortLivedVideo.get());
                shortLivedVideo.reset();
                Context().Check(dependentPlayer.getStateProperty() == MediaState::Stopped &&
                                    dependentPlayer.getVideoProperty() == nullptr &&
                                    dependentPlayer.getPlayPositionProperty() == System::TimeSpan::Zero &&
                                    dependentPlayer.GetTexture() == nullptr,
                                "destroying an active C++ Video safely stops its dependent player");

                player_->Dispose();
                Context().Check(player_->getIsDisposedProperty(),
                                "Dispose marks an active player and closes its playback resources");
                Context().Check(player_->getVideoProperty() == &*video_,
                                "Dispose retains the last Video identity (CNA-DEMOS-ISSUE-099)");
                const bool disposedPropertiesRefused =
                    ThrowsExactly<System::ObjectDisposedException>(
                        [&] { static_cast<void>(player_->getStateProperty()); }) &&
                    ThrowsExactly<System::ObjectDisposedException>(
                        [&] { static_cast<void>(player_->getPlayPositionProperty()); }) &&
                    ThrowsExactly<System::ObjectDisposedException>(
                        [&] { player_->setIsLoopedProperty(false); }) &&
                    ThrowsExactly<System::ObjectDisposedException>(
                        [&] { player_->setIsMutedProperty(true); }) &&
                    ThrowsExactly<System::ObjectDisposedException>(
                        [&] { player_->setVolumeProperty(0.25f); });
                Context().Check(disposedPropertiesRefused,
                                "disposed state and configuration accessors refuse use (CNA-DEMOS-ISSUE-099)");
                player_->Dispose();
                Context().Check(player_->getIsDisposedProperty(), "a second Dispose is safely idempotent");
                ExpectDisposed("Play after Dispose", [&] { player_->Play(&*video_); });
                ExpectDisposed("Stop after Dispose", [&] { player_->Stop(); });
                ExpectDisposed("Pause after Dispose", [&] { player_->Pause(); });
                ExpectDisposed("Resume after Dispose", [&] { player_->Resume(); });
                ExpectDisposed("GetTexture after Dispose", [&] { static_cast<void>(player_->GetTexture()); });
                ExpectDisposed("SetAudioTrackEXT after Dispose", [&] { player_->SetAudioTrackEXT(0); });
                ExpectDisposed("SetVideoTrackEXT after Dispose", [&] { player_->SetVideoTrackEXT(0); });
            }
            DemoGame::EndRun();
        }

    private:
        void ExpectDisposed(std::string_view what, const std::function<void()>& action)
        {
            try
            {
                action();
                Context().Check(false, std::string(what) + " throws ObjectDisposedException");
            }
            catch (const System::ObjectDisposedException& e)
            {
                Context().Print("{} -> ObjectDisposedException: {}", what, e.what());
                Context().Check(true, std::string(what) + " throws ObjectDisposedException");
            }
        }

        std::unique_ptr<Microsoft::Xna::Framework::Content::ContentManager> media_;
        std::optional<Video> video_;
        std::unique_ptr<VideoPlayer> player_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Step step_ = Step::PlayUntilHalfASecond;
        double pausedAt_ = 0.0;
        double lastPosition_ = 0.0;
        std::uint64_t firstGeneration_ = 0;
        std::uint64_t generationAtLoop_ = 0;
        int pausedFrames_ = 0;
    };
}

CNA_DEMOS_GAME(PauseAndLoopDemo, {
    .id = "media.video_player.pause_and_loop",
    .title = "Pausing and looping a video",
    .description = "Audits VideoPlayer defaults and validation, then plays a compiled video with looping, pause/resume, "
                   "frame generations, raw-video ownership and complete disposal guards.",
    .apis = {"VideoPlayer::IsLooped", "VideoPlayer::Pause", "VideoPlayer::Resume", "VideoPlayer::Volume",
             "VideoPlayer::IsMuted", "VideoPlayer::PlayPosition", "VideoPlayer::State",
             "MediaState::Stopped", "MediaState::Playing", "MediaState::Paused",
             "VideoPlayer::Play", "VideoPlayer::GetTexture", "VideoPlayer::Video", "VideoPlayer::Dispose",
             "VideoPlayer::IsDisposed", "Video::FromUriEXT", "Video::getFileNameProperty",
             "Video::getGraphicsDeviceProperty", "Video::GetTypeName", "VideoPlayer::GetFrameGenerationEXT",
             "VideoPlayer::GetFramePresentationTimeEXT", "VideoPlayer::SetAudioTrackEXT",
             "VideoPlayer::SetVideoTrackEXT"},
    .tags = {"media", "video", "pause", "loop"},
    .needs = CnaDemos::Need::Video | CnaDemos::Need::Audio,
    .frames = 900,
    .assets = {"fonts/pixel_font.cnb", "media/videos/bounce.wmv"},
    .restrictions = "Uses Windows XNA 4.0 VideoPlayer IL for exception, retained-Video and disposal contracts. "
                    "Frame pixels are drawn but only transport/state and an existing programmatic texture path are checked; "
                    "no audible or manual visual-quality result is claimed.",
})
