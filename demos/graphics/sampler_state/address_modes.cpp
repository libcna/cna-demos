// SPDX-License-Identifier: MS-PL
// SamplerState address modes: what a texture shows outside its 0..1 range -- Wrap, Mirror, Clamp.
#include <array>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureAddressMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    SamplerState MakeSampler(TextureAddressMode u, TextureAddressMode v)
    {
        SamplerState state;
        state.setFilterProperty(TextureFilter::Point);
        state.setAddressUProperty(u);
        state.setAddressVProperty(v);
        return state;
    }

    constexpr int Size = 128;  // gradient.png is 128x128; each panel shows a 2x2 range of it at 1:1

    class AddressModesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            gradient_ = getContentProperty().Load<Texture2D>("textures/gradient");
            samplers_ = {MakeSampler(TextureAddressMode::Wrap, TextureAddressMode::Wrap),
                         MakeSampler(TextureAddressMode::Mirror, TextureAddressMode::Mirror),
                         MakeSampler(TextureAddressMode::Clamp, TextureAddressMode::Clamp)};
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            // A source rectangle twice the texture's size asks for texture coordinates 0..2;
            // the sampler's address mode decides what 1..2 shows.
            const std::array<const char*, 3> names = {"Wrap", "Mirror", "Clamp"};
            for (std::size_t i = 0; i < samplers_.size(); ++i)
            {
                spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &samplers_[i], nullptr, nullptr);
                spriteBatch_->Draw(gradient_, Panel(i), Rectangle(0, 0, 2 * Size, 2 * Size), Color::White);
                spriteBatch_->End();
            }

            if (FramesDrawn() == 0)
                CheckPanels(device);

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            for (std::size_t i = 0; i < names.size(); ++i)
                spriteBatch_->DrawString(*font_, names[i], Vector2(static_cast<float>(Panel(i).X), 30.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        static Rectangle Panel(std::size_t i) { return Rectangle(8 + static_cast<int>(i) * 264, 70, 2 * Size, 2 * Size); }

        void CheckPanels(GraphicsDevice& device)
        {
            const CnaDemos::BackBufferPixels pixels(device);
            const auto at = [&](std::size_t panel, int x, int y) { return pixels.At(Panel(panel).X + x, Panel(panel).Y + y); };
            const int d = 20;  // a texel 20 pixels into the texture, and its counterparts past the edge
            Context().Check(at(0, Size + d, d) == at(0, d, d), "Wrap repeats the texture");
            Context().Check(at(1, Size + d, d) == at(1, Size - 1 - d, d), "Mirror reflects it at the edge");
            Context().Check(at(2, Size + d, d) == at(2, Size - 1, d) && at(2, 2 * Size - 1, d) == at(2, Size - 1, d),
                            "Clamp repeats the edge texel");
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D gradient_;
        std::array<SamplerState, 3> samplers_;
    };
}

CNA_DEMOS_GAME(AddressModesDemo, {
    .id = "graphics.sampler_state.address_modes",
    .title = "Texture address modes",
    .description = "Draws texture coordinates 0..2 of one texture with SamplerState address modes Wrap, Mirror and "
                   "Clamp and checks what each shows past the edge.",
    .apis = {"SamplerState::SamplerState", "SamplerState::AddressU", "SamplerState::AddressV", "SamplerState::Filter",
             "TextureAddressMode::Wrap", "TextureAddressMode::Mirror", "TextureAddressMode::Clamp"},
    .tags = {"sampler", "texture", "wrap", "mirror", "clamp", "state"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/gradient.png", "fonts/pixel_font.cnb"},
})
