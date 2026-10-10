// SPDX-License-Identifier: MS-PL
// BlendState::BlendFactor: a constant mixing weight, independent of the sprite's alpha.
#include <algorithm>
#include <array>
#include <cstdlib>
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/Blend.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr std::array<int, 4> Factors = {32, 96, 160, 224};

    class BlendFactorDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");

            // result = source * factor + destination * (1 - factor), per channel.
            for (std::size_t i = 0; i < Factors.size(); ++i)
            {
                states_[i].setColorSourceBlendProperty(Blend::BlendFactor);
                states_[i].setColorDestinationBlendProperty(Blend::InverseBlendFactor);
                states_[i].setAlphaSourceBlendProperty(Blend::One);
                states_[i].setAlphaDestinationBlendProperty(Blend::Zero);
                states_[i].setBlendFactorProperty(Color(Factors[i], Factors[i], Factors[i], 255));
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            for (std::size_t i = 0; i < states_.size(); ++i)
            {
                spriteBatch_->Begin(SpriteSortMode::Deferred, &states_[i], nullptr, nullptr, nullptr);
                spriteBatch_->Draw(checker_, Rectangle(40 + static_cast<int>(i) * 190, 120, 160, 240), Color::White);
                spriteBatch_->End();
            }

            if (FramesDrawn() == 0)
            {
                // Applying the state also sets GraphicsDevice::BlendFactor.
                Context().Print("GraphicsDevice::BlendFactor after the last batch: {}",
                                device.getBlendFactorProperty().ToString());
                Context().Check(device.getBlendFactorProperty() == states_.back().getBlendFactorProperty(),
                                "the device's BlendFactor is the last applied state's");

                // A white texel over black comes out as the factor itself.
                const CnaDemos::BackBufferPixels pixels(device);
                for (std::size_t i = 0; i < Factors.size(); ++i)
                {
                    int brightest = 0;
                    for (int x = 0; x < 160; ++x)
                        brightest = std::max(brightest, static_cast<int>(pixels.At(40 + static_cast<int>(i) * 190 + x, 200).getRProperty()));
                    Context().Check(std::abs(brightest - Factors[i]) <= 2,
                                    "factor " + std::to_string(Factors[i]) + " scales white to " +
                                        std::to_string(Factors[i]));
                }
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
        std::array<BlendState, 4> states_;
    };
}

CNA_DEMOS_GAME(BlendFactorDemo, {
    .id = "graphics.blend_state.blend_factor",
    .title = "Constant blend factor",
    .description = "Blends a texture over black with Blend::BlendFactor / InverseBlendFactor at four BlendFactor "
                   "values: a fixed mix that ignores alpha.",
    .apis = {"BlendState::BlendFactor", "Blend::BlendFactor", "Blend::InverseBlendFactor", "GraphicsDevice::BlendFactor"},
    .tags = {"blend", "fade", "state"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
