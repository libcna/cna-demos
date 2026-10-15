// SPDX-License-Identifier: MS-PL
// Custom BlendState objects: visible blend functions plus the complete per-pixel factor family.
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/Blend.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendFunction.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
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
        state.setAlphaDestinationBlendProperty(
            function == BlendFunction::Min || function == BlendFunction::Max ? Blend::One : Blend::Zero);
        state.setAlphaBlendFunctionProperty(function);
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
    const Color FactorSource(153, 51, 204, 102);
    const Color FactorDestination(51, 128, 26, 178);
    const Color ConstantFactor(64, 192, 128, 32);

    constexpr std::array<Blend, 13> Factors = {
        Blend::One, Blend::Zero, Blend::SourceColor, Blend::InverseSourceColor,
        Blend::SourceAlpha, Blend::InverseSourceAlpha, Blend::DestinationColor,
        Blend::InverseDestinationColor, Blend::DestinationAlpha, Blend::InverseDestinationAlpha,
        Blend::BlendFactor, Blend::InverseBlendFactor, Blend::SourceAlphaSaturation,
    };

    constexpr std::array<const char*, 13> FactorNames = {
        "One", "Zero", "SourceColor", "InverseSourceColor", "SourceAlpha", "InverseSourceAlpha",
        "DestinationColor", "InverseDestinationColor", "DestinationAlpha", "InverseDestinationAlpha",
        "BlendFactor", "InverseBlendFactor", "SourceAlphaSaturation",
    };

    std::array<float, 4> Components(const Color& value)
    {
        return {value.getRProperty() / 255.0f, value.getGProperty() / 255.0f,
                value.getBProperty() / 255.0f, value.getAProperty() / 255.0f};
    }

    float FactorComponent(Blend factor, int channel)
    {
        const auto source = Components(FactorSource);
        const auto destination = Components(FactorDestination);
        const auto constant = Components(ConstantFactor);
        switch (factor)
        {
            case Blend::One: return 1.0f;
            case Blend::Zero: return 0.0f;
            case Blend::SourceColor: return source[static_cast<std::size_t>(channel)];
            case Blend::InverseSourceColor: return 1.0f - source[static_cast<std::size_t>(channel)];
            case Blend::SourceAlpha: return source[3];
            case Blend::InverseSourceAlpha: return 1.0f - source[3];
            case Blend::DestinationColor: return destination[static_cast<std::size_t>(channel)];
            case Blend::InverseDestinationColor: return 1.0f - destination[static_cast<std::size_t>(channel)];
            case Blend::DestinationAlpha: return destination[3];
            case Blend::InverseDestinationAlpha: return 1.0f - destination[3];
            case Blend::BlendFactor: return constant[static_cast<std::size_t>(channel)];
            case Blend::InverseBlendFactor: return 1.0f - constant[static_cast<std::size_t>(channel)];
            case Blend::SourceAlphaSaturation:
                return channel == 3 ? 1.0f : std::min(source[3], 1.0f - destination[3]);
        }
        return 0.0f;
    }

    Color ExpectedFactor(Blend factor)
    {
        const auto source = Components(FactorSource);
        const auto byte = [](float value) {
            return std::clamp(static_cast<int>(std::lround(value * 255.0f)), 0, 255);
        };
        return Color(byte(source[0] * FactorComponent(factor, 0)),
                     byte(source[1] * FactorComponent(factor, 1)),
                     byte(source[2] * FactorComponent(factor, 2)),
                     static_cast<int>(FactorSource.getAProperty()));
    }

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
            factorTarget_ = std::make_unique<RenderTarget2D>(
                getGraphicsDeviceProperty(), 16, 16, false, SurfaceFormat::Color, DepthFormat::None, 0,
                RenderTargetUsage::PreserveContents);

            modes_ = {{
                // source * destination: darkens, like a colour filter.
                {"Multiply", MakeBlend(Blend::DestinationColor, Blend::Zero, BlendFunction::Add), Color(78, 59, 39)},
                // source + destination * (1 - source): lightens.
                {"Screen", MakeBlend(Blend::One, Blend::InverseSourceColor, BlendFunction::Add), Color(222, 191, 211)},
                // destination - source.
                {"Subtract", MakeBlend(Blend::One, Blend::One, BlendFunction::ReverseSubtract), Color(100, 0, 0, 0)},
                // source - destination.
                {"Src - dst", MakeBlend(Blend::One, Blend::One, BlendFunction::Subtract), Color(0, 50, 150, 255)},
                // per channel minimum / maximum (the blend factors do not take part).
                {"Min", MakeBlend(Blend::One, Blend::One, BlendFunction::Min), Color(100, 100, 50)},
                {"Max", MakeBlend(Blend::One, Blend::One, BlendFunction::Max), Color(200, 150, 200)},
            }};
            Context().Check(modes_[0].state.getAlphaBlendFunctionProperty() == BlendFunction::Add &&
                                modes_[2].state.getAlphaBlendFunctionProperty() == BlendFunction::ReverseSubtract &&
                                modes_[3].state.getAlphaBlendFunctionProperty() == BlendFunction::Subtract &&
                                modes_[4].state.getAlphaBlendFunctionProperty() == BlendFunction::Min &&
                                modes_[5].state.getAlphaBlendFunctionProperty() == BlendFunction::Max,
                            "each custom state retains its separate alpha blend function");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            if (FramesDrawn() == 0)
                CheckFactors(device);
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
        void CheckFactors(GraphicsDevice& device)
        {
            for (std::size_t i = 0; i < Factors.size(); ++i)
            {
                BlendState state;
                state.setColorSourceBlendProperty(Factors[i]);
                state.setColorDestinationBlendProperty(Blend::Zero);
                state.setAlphaSourceBlendProperty(Blend::One);
                state.setAlphaDestinationBlendProperty(Blend::Zero);
                state.setBlendFactorProperty(ConstantFactor);

                device.SetRenderTarget(factorTarget_.get());
                device.Clear(FactorDestination);
                spriteBatch_->Begin(SpriteSortMode::Deferred, &state, nullptr, nullptr, nullptr);
                spriteBatch_->Draw(*pixel_, Rectangle(0, 0, 16, 16), FactorSource);
                spriteBatch_->End();
                device.SetRenderTarget(static_cast<RenderTarget2D*>(nullptr));

                Color got;
                const Rectangle centre(8, 8, 1, 1);
                factorTarget_->GetData(0, &centre, &got, 0, 1);
                const Color expected = ExpectedFactor(Factors[i]);
                Context().Print("factor {:<25} {}", FactorNames[i], got.ToString());
                Context().Check(CnaDemos::Near(got, expected, 2),
                                std::string(FactorNames[i]) + " scales the source by its documented components");
            }
        }

        static Rectangle Box(std::size_t i) { return Rectangle(20 + static_cast<int>(i) * 130, 150, 110, 120); }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
        std::unique_ptr<RenderTarget2D> factorTarget_;
        std::array<Mode, 6> modes_{};
    };
}

CNA_DEMOS_GAME(BlendFunctionsDemo, {
    .id = "graphics.blend_state.blend_functions",
    .title = "Custom blend modes",
    .description = "Builds BlendState objects for six blend functions and checks every public per-pixel Blend factor "
                   "against its component formula on a real render target.",
    .apis = {"BlendState::BlendState", "BlendState::ColorSourceBlend", "BlendState::ColorDestinationBlend",
             "BlendState::ColorBlendFunction", "BlendState::AlphaSourceBlend", "BlendState::AlphaDestinationBlend",
             "BlendState::AlphaBlendFunction", "Blend::One", "Blend::Zero", "Blend::SourceColor",
             "Blend::InverseSourceColor", "Blend::SourceAlpha", "Blend::InverseSourceAlpha",
             "Blend::DestinationColor", "Blend::InverseDestinationColor", "Blend::DestinationAlpha",
             "Blend::InverseDestinationAlpha", "Blend::BlendFactor", "Blend::InverseBlendFactor",
             "Blend::SourceAlphaSaturation", "BlendFunction::Add", "BlendFunction::Subtract",
             "BlendFunction::ReverseSubtract", "BlendFunction::Min", "BlendFunction::Max"},
    .tags = {"blend", "multiply", "screen", "state"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
