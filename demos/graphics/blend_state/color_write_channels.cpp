// SPDX-License-Identifier: MS-PL
// BlendState::ColorWriteChannels: draw into some colour channels only, or into none.
#include <array>
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ColorWriteChannels.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ColorWriteChannelsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // Opaque writing, restricted to the listed channels; the others keep the background.
            const std::array<ColorWriteChannels, 5> masks = {
                ColorWriteChannels::Red, ColorWriteChannels::Green, ColorWriteChannels::Blue,
                ColorWriteChannels::Red | ColorWriteChannels::Green, ColorWriteChannels::None};
            // A new BlendState starts as opaque writing (One, Zero). Built-in states such as
            // BlendState::Opaque cannot be modified -- copies of them share the same, already used state.
            for (std::size_t i = 0; i < masks.size(); ++i)
                states_[i].setColorWriteChannelsProperty(masks[i]);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(0, 0, 64));

            // A white box in each state: what survives is exactly the enabled channels.
            for (std::size_t i = 0; i < states_.size(); ++i)
            {
                spriteBatch_->Begin(SpriteSortMode::Deferred, &states_[i], nullptr, nullptr, nullptr);
                spriteBatch_->Draw(*pixel_, Rectangle(40 + static_cast<int>(i) * 150, 160, 120, 160), Color::White);
                spriteBatch_->End();
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const std::array<Color, 5> expected = {Color(255, 0, 64), Color(0, 255, 64), Color(0, 0, 255),
                                                       Color(255, 255, 64), Color(0, 0, 64)};
                const std::array<const char*, 5> names = {"Red", "Green", "Blue", "Red | Green", "None"};
                for (std::size_t i = 0; i < expected.size(); ++i)
                {
                    const Color got = pixels.At(100 + static_cast<int>(i) * 150, 240);
                    Context().Print("{:<12} {}", names[i], got.ToString());
                    Context().Check(got.getRProperty() == expected[i].getRProperty() &&
                                        got.getGProperty() == expected[i].getGProperty() &&
                                        got.getBProperty() == expected[i].getBProperty(),
                                    std::string("ColorWriteChannels ") + names[i] + " keeps only those channels");
                }
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> pixel_;
        std::array<BlendState, 5> states_;
    };
}

CNA_DEMOS_GAME(ColorWriteChannelsDemo, {
    .id = "graphics.blend_state.color_write_channels",
    .title = "Write only some colour channels",
    .description = "Draws white boxes over dark blue with BlendState::ColorWriteChannels set to Red, Green, Blue, "
                   "Red | Green and None; only the enabled channels change.",
    .apis = {"BlendState::ColorWriteChannels", "ColorWriteChannels::Red", "ColorWriteChannels::Green",
             "ColorWriteChannels::Blue", "ColorWriteChannels::None"},
    .tags = {"blend", "channels", "mask", "state"},
    .frames = 3,
    .deterministic = true,
})
