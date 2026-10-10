// SPDX-License-Identifier: MS-PL
// Custom BlendState objects: multiply, screen, subtract, min and max from blend factors and functions.
#include <array>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/Blend.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendFunction.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // result = source * SourceBlend  (function)  destination * DestinationBlend
    BlendState MakeBlend(Blend source, Blend destination, BlendFunction function)
    {
        BlendState state;
        state.setColorSourceBlendProperty(source);
        state.setColorDestinationBlendProperty(destination);
        state.setColorBlendFunctionProperty(function);
        state.setAlphaSourceBlendProperty(Blend::One);
        state.setAlphaDestinationBlendProperty(Blend::Zero);
        return state;
    }

    struct Mode
    {
        const char* name;
        BlendState state;
        Color expected;  // for background (200, 100, 50) and source (100, 150, 200)
    };

    const Color Background(200, 100, 50);
    const Color Source(100, 150, 200);

    class BlendFunctionsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            modes_ = {{
                // source * destination: darkens, like a colour filter.
                {"Multiply", MakeBlend(Blend::DestinationColor, Blend::Zero, BlendFunction::Add), Color(78, 59, 39)},
                // source + destination * (1 - source): lightens.
                {"Screen", MakeBlend(Blend::One, Blend::InverseSourceColor, BlendFunction::Add), Color(222, 191, 211)},
                // destination - source.
                {"Subtract", MakeBlend(Blend::One, Blend::One, BlendFunction::ReverseSubtract), Color(100, 0, 0)},
                // per channel minimum / maximum (the blend factors do not take part).
                {"Min", MakeBlend(Blend::One, Blend::One, BlendFunction::Min), Color(100, 100, 50)},
                {"Max", MakeBlend(Blend::One, Blend::One, BlendFunction::Max), Color(200, 150, 200)},
            }};
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Background);

            for (std::size_t i = 0; i < modes_.size(); ++i)
            {
                spriteBatch_->Begin(SpriteSortMode::Deferred, &modes_[i].state, nullptr, nullptr, nullptr);
                spriteBatch_->Draw(*pixel_, Box(i), Source);
                spriteBatch_->End();
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                for (std::size_t i = 0; i < modes_.size(); ++i)
                {
                    const Color got = pixels.At(Box(i).X + 60, Box(i).Y + 60);
                    Context().Print("{:<9} {}", modes_[i].name, got.ToString());
                    Context().Check(CnaDemos::Near(got, modes_[i].expected, 2),
                                    std::string(modes_[i].name) + " gives " + modes_[i].expected.ToString());
                }
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Source (100, 150, 200) over background (200, 100, 50)",
                                     Vector2(40.0f, 40.0f), Color::Black);
            for (std::size_t i = 0; i < modes_.size(); ++i)
                spriteBatch_->DrawString(*font_, modes_[i].name,
                                         Vector2(static_cast<float>(Box(i).X), static_cast<float>(Box(i).getBottomProperty() + 8)),
                                         Color::Black);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        static Rectangle Box(std::size_t i) { return Rectangle(40 + static_cast<int>(i) * 150, 150, 120, 120); }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
        std::array<Mode, 5> modes_{};
    };
}

CNA_DEMOS_GAME(BlendFunctionsDemo, {
    .id = "graphics.blend_state.blend_functions",
    .title = "Custom blend modes",
    .description = "Builds BlendState objects for multiply, screen, subtract, min and max from Blend factors and "
                   "BlendFunction, and checks each result against the formula.",
    .apis = {"BlendState::BlendState", "BlendState::ColorSourceBlend", "BlendState::ColorDestinationBlend",
             "BlendState::ColorBlendFunction", "BlendState::AlphaSourceBlend", "BlendState::AlphaDestinationBlend",
             "Blend::DestinationColor", "Blend::InverseSourceColor", "BlendFunction::ReverseSubtract",
             "BlendFunction::Min", "BlendFunction::Max"},
    .tags = {"blend", "multiply", "screen", "state"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
