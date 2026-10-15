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
            const std::array<ColorWriteChannels, 7> masks = {
                ColorWriteChannels::Red, ColorWriteChannels::Green, ColorWriteChannels::Blue,
                ColorWriteChannels::Alpha, ColorWriteChannels::Red | ColorWriteChannels::Green,
                ColorWriteChannels::All, ColorWriteChannels::None};
            // A new BlendState starts as opaque writing (One, Zero). Built-in states such as
            // BlendState::Opaque cannot be modified -- copies of them share the same, already used state.
            for (std::size_t i = 0; i < masks.size(); ++i)
                states_[i].setColorWriteChannelsProperty(masks[i]);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const Color background(0, 0, 64, 32);
            const Color source(255, 255, 255, 192);
            device.Clear(background);

            // A white box in each state: what survives is exactly the enabled channels.
            for (std::size_t i = 0; i < states_.size(); ++i)
            {
                spriteBatch_->Begin(SpriteSortMode::Deferred, &states_[i], nullptr, nullptr, nullptr);
                spriteBatch_->Draw(*pixel_, Rectangle(16 + static_cast<int>(i) * 112, 160, 96, 160), source);
                spriteBatch_->End();
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const std::array<Color, 7> expected = {
                    Color(255, 0, 64, 32), Color(0, 255, 64, 32), Color(0, 0, 255, 32),
                    Color(0, 0, 64, 192), Color(255, 255, 64, 32), source, background};
                const std::array<const char*, 7> names = {
                    "Red", "Green", "Blue", "Alpha", "Red | Green", "All", "None"};
                for (std::size_t i = 0; i < expected.size(); ++i)
                {
                    const Color got = pixels.At(64 + static_cast<int>(i) * 112, 240);
                    Context().Print("{:<12} {}", names[i], got.ToString());
                    Context().Check(got == expected[i],
                                    std::string("ColorWriteChannels ") + names[i] + " keeps only those channels");
                }
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> pixel_;
        std::array<BlendState, 7> states_;
    };
}

CNA_DEMOS_GAME(ColorWriteChannelsDemo, {
    .id = "graphics.blend_state.color_write_channels",
    .title = "Write only some colour channels",
    .description = "Draws white boxes over translucent dark blue with all individual ColorWriteChannels, a combined "
                   "mask, All and None; only the enabled RGBA channels change.",
    .apis = {"BlendState::ColorWriteChannels", "ColorWriteChannels::Red", "ColorWriteChannels::Green",
             "ColorWriteChannels::Blue", "ColorWriteChannels::Alpha", "ColorWriteChannels::All",
             "ColorWriteChannels::None"},
    .tags = {"blend", "channels", "mask", "state"},
    .frames = 3,
    .deterministic = true,
})
