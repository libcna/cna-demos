// SPDX-License-Identifier: MS-PL
// Magnifying a small texture: point, linear and the four mixed min/mag/mip filters.
#include <array>
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureAddressMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class SamplerStatesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
            probe_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 4, 4);
            std::array<Color, 16> texels{};
            for (int y = 0; y < 4; ++y)
                for (int x = 0; x < 4; ++x)
                    texels[static_cast<std::size_t>(y * 4 + x)] = (x + y) % 2 == 0 ? Dark : Light;
            probe_->SetData(texels.data(), static_cast<int>(texels.size()));

            for (std::size_t i = 0; i < Cases.size(); ++i)
            {
                states_[i].setFilterProperty(Cases[i].filter);
                states_[i].setAddressUProperty(TextureAddressMode::Clamp);
                states_[i].setAddressVProperty(TextureAddressMode::Clamp);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            if (FramesDrawn() == 0)
                CheckFilterComponents(device);
            device.Clear(Color::Black);

            // Point sampling picks the nearest texel: crisp, blocky pixels (pixel art).
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::AlphaBlend, &SamplerState::PointClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(checker_, Rectangle(20, 40, 370, 370), Color::White);
            spriteBatch_->End();

            // Linear sampling blends neighbouring texels: smooth, blurred edges when magnified.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::AlphaBlend, &SamplerState::LinearClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(checker_, Rectangle(410, 40, 370, 370), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        struct FilterCase
        {
            const char* name;
            TextureFilter filter;
            bool linearMagnification;
        };

        static constexpr std::array<FilterCase, 6> Cases = {{
            {"Point", TextureFilter::Point, false},
            {"Linear", TextureFilter::Linear, true},
            {"MinLinearMagPointMipLinear", TextureFilter::MinLinearMagPointMipLinear, false},
            {"MinLinearMagPointMipPoint", TextureFilter::MinLinearMagPointMipPoint, false},
            {"MinPointMagLinearMipLinear", TextureFilter::MinPointMagLinearMipLinear, true},
            {"MinPointMagLinearMipPoint", TextureFilter::MinPointMagLinearMipPoint, true},
        }};
        static const Color Dark;
        static const Color Light;

        void CheckFilterComponents(GraphicsDevice& device)
        {
            device.Clear(Color::Black);
            for (std::size_t i = 0; i < Cases.size(); ++i)
            {
                spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &states_[i], nullptr, nullptr);
                spriteBatch_->Draw(*probe_, Rectangle(28 + static_cast<int>(i) * 126, 190, 96, 96), Color::White);
                spriteBatch_->End();
            }

            const CnaDemos::BackBufferPixels pixels(device);
            for (std::size_t i = 0; i < Cases.size(); ++i)
            {
                int intermediate = 0;
                const int x0 = 32 + static_cast<int>(i) * 126;
                for (int y = 194; y < 282; ++y)
                    for (int x = x0; x < x0 + 88; ++x)
                    {
                        const Color pixel = pixels.At(x, y);
                        const auto near = [&pixel](const Color& value) { return CnaDemos::Near(pixel, value, 3); };
                        if (!near(Dark) && !near(Light))
                            ++intermediate;
                    }
                Context().Print("{:<31} {} intermediate pixels", Cases[i].name, intermediate);
                Context().Check(states_[i].getFilterProperty() == Cases[i].filter &&
                                    (Cases[i].linearMagnification ? intermediate > 500 : intermediate == 0),
                                std::string(Cases[i].name) +
                                    (Cases[i].linearMagnification ? " magnifies linearly" : " magnifies by point"));
            }
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
        std::unique_ptr<Texture2D> probe_;
        std::array<SamplerState, Cases.size()> states_;
    };

    const Color SamplerStatesDemo::Dark(20, 30, 40);
    const Color SamplerStatesDemo::Light(230, 220, 200);
}

CNA_DEMOS_GAME(SamplerStatesDemo, {
    .id = "graphics.sprite_batch.sampler_states",
    .title = "Point and linear sampling",
    .description = "Magnifies a checker through point, linear and all four mixed min/mag/mip filters, then shows "
                   "PointClamp and LinearClamp side by side.",
    .apis = {"SpriteBatch::Begin", "SamplerState::PointClamp", "SamplerState::LinearClamp",
             "TextureFilter::Linear", "TextureFilter::MinLinearMagPointMipLinear",
             "TextureFilter::MinLinearMagPointMipPoint", "TextureFilter::MinPointMagLinearMipLinear",
             "TextureFilter::MinPointMagLinearMipPoint"},
    .tags = {"sprite", "filtering", "pixel-art", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
