// SPDX-License-Identifier: MS-PL
// Video and VideoPlayer: playing a video and drawing its frames as a texture.
#include <memory>
#include <optional>
#include <string>
#include <vector>

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
#include "Microsoft/Xna/Framework/Media/VideoSoundtrackType.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    class PlayVideoDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            // A .wmv in the media content project, compiled by the VideoProcessor (an XNA content
            // project did the same): the metadata is read without decoding a frame. That project
            // is built into assets/media and gets a ContentManager rooted there, because the
            // stream reference in a compiled video is relative to the root of its content build.
            media_ = std::make_unique<Microsoft::Xna::Framework::Content::ContentManager>(&getServicesProperty(),
                                                                                          "assets/media");
            video_.emplace(media_->Load<Video>("videos/bounce"));
            const VideoSoundtrackType soundtrack = video_->getVideoSoundtrackTypeProperty();
            ctx.Print("video: {}x{}, {:.1f} fps, {:.2f} s, soundtrack {}", video_->getWidthProperty(),
                      video_->getHeightProperty(), video_->getFramesPerSecondProperty(),
                      video_->getDurationProperty().getTotalSecondsProperty(),
                      soundtrack == VideoSoundtrackType::Music    ? "Music"
                      : soundtrack == VideoSoundtrackType::Dialog ? "Dialog"
                                                                  : "MusicAndDialog");
            ctx.Check(video_->getWidthProperty() == 160 && video_->getHeightProperty() == 120 &&
                          video_->getFramesPerSecondProperty() == 15.0f,
                      "the metadata describes the 160x120, 15 fps clip");

            player_ = std::make_unique<VideoPlayer>();
            player_->Play(&*video_);
            ctx.Check(player_->getStateProperty() == MediaState::Playing && player_->getVideoProperty() == &*video_,
                      "Play starts it");
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
            if (!sampled_ && position > 1.0)
            {
                // The current frame, as a texture the game draws like any other.
                Texture2D* frame = player_->GetTexture();
                std::vector<Color> pixels(static_cast<std::size_t>(frame->getWidthProperty() * frame->getHeightProperty()));
                frame->GetData(pixels.data(), static_cast<int>(pixels.size()));
                const Color corner = pixels[static_cast<std::size_t>(5 * frame->getWidthProperty() + 5)];
                ctx.Print("frame at {:.2f} s: top-left pixel {}", position, corner.ToString());
                ctx.Check(frame->getWidthProperty() == 160 && corner.getBProperty() > 60,
                          "GetTexture returns the current frame (the clip's blue-ish gradient)");
                sampled_ = true;
            }
            // Not looped: the player stops by itself at the end of the clip.
            if (player_->getStateProperty() == MediaState::Stopped && !ended_)
            {
                ended_ = true;
                ctx.Check(sampled_, "the clip played to its end");
                if (ctx.Automated())
                {
                    Exit();
                }
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            // Read before GetTexture: decoding past the last frame is what stops the player.
            const MediaState state = player_->getStateProperty();
            const double position = player_->getPlayPositionProperty().getTotalSecondsProperty();
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::LinearClamp, nullptr, nullptr);
            if (state != MediaState::Stopped)
            {
                if (Texture2D* frame = player_->GetTexture())
                {
                    spriteBatch_->Draw(*frame, Rectangle(160, 60, 480, 360), Color::White);
                }
            }
            const char* stateName = state == MediaState::Playing ? "Playing"
                                    : state == MediaState::Paused ? "Paused"
                                                                  : "Stopped";
            spriteBatch_->DrawString(*font_,
                                     std::string("VideoPlayer: ") + stateName + ", " +
                                         std::to_string(position).substr(0, 4) + " s",
                                     Vector2(160.0f, 20.0f), Color::White);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<Microsoft::Xna::Framework::Content::ContentManager> media_;
        std::optional<Video> video_;
        std::unique_ptr<VideoPlayer> player_;
        bool sampled_ = false;
        bool ended_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(PlayVideoDemo, {
    .id = "media.video_player.play_video",
    .title = "Playing a video",
    .description = "Loads a WMV compiled by the content pipeline as a Video (size, frame rate, duration), plays it "
                   "with a VideoPlayer and draws VideoPlayer::GetTexture each frame until it stops at the end.",
    .apis = {"Video::Width", "Video::Height", "Video::FramesPerSecond", "Video::Duration", "VideoPlayer::VideoPlayer",
             "VideoPlayer::Play", "VideoPlayer::GetTexture", "VideoPlayer::State", "VideoPlayer::PlayPosition",
             "VideoPlayer::Video", "VideoPlayer::Stop", "MediaState",
             "Video::VideoSoundtrackType", "VideoSoundtrackType"},
    .tags = {"media", "video", "cutscene", "texture"},
    .needs = CnaDemos::Need::Video | CnaDemos::Need::Audio,
    .frames = 600,
    .assets = {"fonts/pixel_font.cnb", "media/videos/bounce.wmv"},
})
