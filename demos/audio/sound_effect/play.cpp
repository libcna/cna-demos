// SPDX-License-Identifier: MS-PL
// Loading a WAV file as a SoundEffect and playing it fire-and-forget.
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    class PlaySoundDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            // ContentManager reads assets/sounds/tone_440.wav (a 0.5 s, 440 Hz sine tone).
            tone_.emplace(getContentProperty().Load<SoundEffect>("sounds/tone_440"));

            const double seconds = tone_->getDurationProperty().getTotalSecondsProperty();
            Context().Print("tone_440 lasts {:.3f} s", seconds);
            Context().Check(seconds > 0.49 && seconds < 0.51, "the tone lasts half a second");
        }

        void Update(GameTime& gameTime) override
        {
            // Play once, on the first update. SoundEffect::Play starts a new voice and returns at
            // once; the sound keeps playing while the game continues.
            if (!played_)
            {
                played_ = true;
                Context().Check(tone_->Play(), "SoundEffect::Play() returns true");
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkOliveGreen);
            Game::Draw(gameTime);
        }

    private:
        std::optional<SoundEffect> tone_;
        bool played_ = false;
    };
}

CNA_DEMOS_GAME(PlaySoundDemo, {
    .id = "audio.sound_effect.play",
    .title = "Play a sound effect",
    .description = "Loads a WAV file with ContentManager::Load<SoundEffect> and plays it once with "
                   "SoundEffect::Play.",
    .apis = {"ContentManager::Load", "SoundEffect::Play", "SoundEffect::Duration"},
    .tags = {"sound", "wav"},
    .needs = CnaDemos::Need::Audio,
    .frames = 40,
    .assets = {"sounds/tone_440.wav"},
})
