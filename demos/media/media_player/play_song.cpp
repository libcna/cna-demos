// SPDX-License-Identifier: MS-PL
// MediaPlayer: play a Song as background music, follow PlayPosition and see it end.
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

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
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    std::string StateName(MediaState state)
    {
        return state == MediaState::Playing ? "Playing" : state == MediaState::Paused ? "Paused" : "Stopped";
    }

    class PlaySongDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // A Song is streamed music: one plays at a time, through MediaPlayer, beside any
            // number of SoundEffects. A relative FromUri path is rooted at the title directory,
            // independently of the process working directory.
            song_.reset(Song::FromUri("Rising", "assets/music/rising.wav"));
            Context().Check(std::filesystem::equivalent(song_->getHandle(),
                                std::filesystem::path(TitleLocation::Path()) / "assets/music/rising.wav"),
                            "relative Song URI selects the file in the title directory");
            Context().Check(MediaPlayer::getStateProperty() == MediaState::Stopped, "MediaPlayer starts out Stopped");
            // On a phone the user's own music may be playing: a game plays its soundtrack only
            // when GameHasControl (always true on a desktop).
            Context().Check(MediaPlayer::getGameHasControlProperty(), "the game has control of the music");
            MediaPlayer::Play(song_.get());
            Context().Check(MediaPlayer::getStateProperty() == MediaState::Playing, "Play makes it Playing at once");
        }

        void UnloadContent() override { MediaPlayer::Stop(); }

        void Update(GameTime& gameTime) override
        {
            // MediaPlayer is advanced by the game loop (XNA's FrameworkDispatcher.Update).
            const double seconds = gameTime.getTotalGameTimeProperty().getTotalSecondsProperty();
            const double position = MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty();
            if (seconds > 1.0 && seconds < 1.5 && !midwayChecked_)
            {
                Context().Print("after {:.2f} s: PlayPosition {:.2f} s", seconds, position);
                Context().Check(position > 0.5, "PlayPosition advances while the song plays");
                midwayChecked_ = true;
            }
            if (seconds > 3.0 && !endChecked_)
            {
                // The tune lasts 2 s; MediaPlayer stops by itself unless IsRepeating is set.
                Context().Check(MediaPlayer::getStateProperty() == MediaState::Stopped,
                                "after the song's end MediaPlayer is Stopped");
                endChecked_ = true;
                if (Context().Automated())
                    Exit();  // everything is checked; an interactive run keeps the window open
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            const double position = MediaPlayer::getPlayPositionProperty().getTotalSecondsProperty();
            spriteBatch_->DrawString(*font_, "Song \"" + song_->getNameProperty() + "\": " + StateName(MediaPlayer::getStateProperty()),
                                     Vector2(40.0f, 40.0f), Color::White);
            spriteBatch_->DrawString(*font_, "PlayPosition " + std::to_string(position).substr(0, 4) + " s",
                                     Vector2(40.0f, 70.0f), Color::LightGray);
            spriteBatch_->Draw(*pixel_, Rectangle(40, 120, 720, 20), Color::DimGray);
            spriteBatch_->Draw(*pixel_, Rectangle(40, 120, static_cast<int>(720.0 * position / 2.0), 20), Color::Orange);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<Song> song_;
        bool midwayChecked_ = false;
        bool endChecked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(PlaySongDemo, {
    .id = "media.media_player.play_song",
    .title = "Play a song",
    .description = "Plays a 2-second tune with Song::FromUri and MediaPlayer::Play, shows MediaPlayer::State and "
                   "PlayPosition, and checks that playback advances and stops by itself at the end.",
    .apis = {"Song::FromUri", "MediaPlayer::Play", "MediaPlayer::State", "MediaPlayer::PlayPosition",
             "MediaPlayer::Stop", "MediaPlayer::GameHasControl", "MediaState", "TitleLocation::Path"},
    .tags = {"media", "music", "song", "background-music", "audio"},
    .needs = CnaDemos::Need::Audio,
    .frames = 200,
    .assets = {"fonts/pixel_font.cnb", "music/rising.wav"},
})
