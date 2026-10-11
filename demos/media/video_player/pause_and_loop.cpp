// SPDX-License-Identifier: MS-PL
// VideoPlayer controls: Pause/Resume, IsLooped, Volume and IsMuted.
#include <memory>
#include <optional>
#include <string>

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

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    // What the demo does next, driven by the player's own PlayPosition.
    enum class Step
    {
        PlayUntilHalfASecond,
        Paused,
        PlayUntilItLoops,
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
            // Settings that belong to the player, not to the video: set before or during playback.
            player_->setIsLoopedProperty(true);
            player_->setVolumeProperty(0.5f);
            player_->setIsMutedProperty(true);
            ctx.Check(player_->getIsLoopedProperty() && player_->getVolumeProperty() == 0.5f && player_->getIsMutedProperty(),
                      "IsLooped, Volume and IsMuted keep their values");
            player_->setIsMutedProperty(false);
            player_->Play(&*video_);
        }

        void UnloadContent() override
        {
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
            }
            DemoGame::EndRun();
        }

    private:
        std::unique_ptr<Microsoft::Xna::Framework::Content::ContentManager> media_;
        std::optional<Video> video_;
        std::unique_ptr<VideoPlayer> player_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Step step_ = Step::PlayUntilHalfASecond;
        double pausedAt_ = 0.0;
        double lastPosition_ = 0.0;
        int pausedFrames_ = 0;
    };
}

CNA_DEMOS_GAME(PauseAndLoopDemo, {
    .id = "media.video_player.pause_and_loop",
    .title = "Pausing and looping a video",
    .description = "Plays a compiled video with a VideoPlayer set to IsLooped (Volume and IsMuted set too), pauses "
                   "it after half a second -- PlayPosition stands still -- resumes it, and sees it start again "
                   "from the beginning at the end of the clip instead of stopping.",
    .apis = {"VideoPlayer::IsLooped", "VideoPlayer::Pause", "VideoPlayer::Resume", "VideoPlayer::Volume",
             "VideoPlayer::IsMuted", "VideoPlayer::PlayPosition", "VideoPlayer::State", "MediaState::Paused"},
    .tags = {"media", "video", "pause", "loop"},
    .needs = CnaDemos::Need::Video | CnaDemos::Need::Audio,
    .frames = 900,
    .assets = {"fonts/pixel_font.cnb", "media/videos/bounce.wmv"},
})
