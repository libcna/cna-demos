// SPDX-License-Identifier: MS-PL
// CNAEXT DepthEffect: drawing with fewer colours (16-bit, 8-bit, grey levels, palettes), dithered.
#include <array>
#include <memory>
#include <optional>
#include <string>

#include "CNA/Graphics/DepthEffect.hpp"
#include "CNA/Graphics/DepthEffectMode.hpp"
#include "CNA/Graphics/DitherMode.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
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
using CNA::Graphics::DepthEffectMode;
using CNA::Graphics::DitherMode;

namespace
{
    struct Panel
    {
        DepthEffectMode mode;
        DitherMode dither;
        const char* label;
    };

    constexpr std::array<Panel, 8> Panels = {{
        {DepthEffectMode::Color16Bit, DitherMode::None, "Color16Bit"},
        {DepthEffectMode::Color8Bit, DitherMode::None, "Color8Bit"},
        {DepthEffectMode::Color8Bit, DitherMode::Bayer4x4, "8Bit+Bayer4x4"},
        {DepthEffectMode::Grayscale4Bit, DitherMode::None, "Grayscale4Bit"},
        {DepthEffectMode::Grayscale2Bit, DitherMode::None, "Grayscale2Bit"},
        {DepthEffectMode::Grayscale1Bit, DitherMode::Bayer4x4, "1Bit+Bayer4x4"},
        {DepthEffectMode::Palette256, DitherMode::None, "Palette256"},
        {DepthEffectMode::Palette16, DitherMode::Bayer8x8, "Palette16+8x8"},
    }};

    class ColorReductionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            gradient_ = getContentProperty().Load<Texture2D>("textures/gradient");
            effect_ = std::make_unique<CNA::Graphics::DepthEffect>(getGraphicsDeviceProperty());
            Context().Check(effect_->getMode() == DepthEffectMode::Color16Bit &&
                                effect_->getDitherMode() == DitherMode::None,
                            "colour depth and dithering start at their documented defaults");

            bool everyModeReadsBack = true;
            for (const auto mode : {DepthEffectMode::Color16Bit, DepthEffectMode::Color8Bit,
                                    DepthEffectMode::Grayscale4Bit, DepthEffectMode::Grayscale2Bit,
                                    DepthEffectMode::Grayscale1Bit, DepthEffectMode::Palette256,
                                    DepthEffectMode::Palette16})
            {
                effect_->setMode(mode);
                everyModeReadsBack = everyModeReadsBack && effect_->getMode() == mode;
            }
            Context().Check(everyModeReadsBack, "all seven colour-depth modes round-trip");

            bool everyDitherReadsBack = true;
            for (const auto dither : {DitherMode::None, DitherMode::Bayer4x4, DitherMode::Bayer8x8})
            {
                effect_->setDitherMode(dither);
                everyDitherReadsBack = everyDitherReadsBack && effect_->getDitherMode() == dither;
            }
            Context().Check(everyDitherReadsBack, "all three ordered-dither choices round-trip");

            effect_->setMode(DepthEffectMode::Palette256);
            effect_->setDitherMode(DitherMode::Bayer8x8);
            std::unique_ptr<Effect> cloneOwner(effect_->Clone());
            auto* clone = dynamic_cast<CNA::Graphics::DepthEffect*>(cloneOwner.get());
            const bool cloneRetained = clone && clone != effect_.get() &&
                clone->GetTypeName() == "CNA.Graphics.DepthEffect" &&
                clone->getMode() == DepthEffectMode::Palette256 && clone->getDitherMode() == DitherMode::Bayer8x8;
            if (clone)
            {
                clone->setMode(DepthEffectMode::Color8Bit);
                clone->setDitherMode(DitherMode::None);
            }
            Context().Check(cloneRetained && effect_->getMode() == DepthEffectMode::Palette256 &&
                                effect_->getDitherMode() == DitherMode::Bayer8x8,
                            "Clone returns an independent DepthEffect with the current mode and dither");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(24, 24, 24));
            // Unlike CRTEffect, DepthEffect works per draw call: each sprite can be reduced on
            // its own, here the same gradient six ways.
            for (std::size_t i = 0; i < Panels.size(); ++i)
            {
                effect_->setMode(Panels[i].mode);
                effect_->setDitherMode(Panels[i].dither);
                const int x = 10 + static_cast<int>(i % 4) * 198;
                const int y = 30 + static_cast<int>(i / 4) * 220;
                spriteBatch_->Begin(SpriteSortMode::Immediate, &BlendState::Opaque, &SamplerState::PointClamp, nullptr,
                                    nullptr, effect_.get());
                spriteBatch_->Draw(gradient_, Rectangle(x, y + 24, 180, 180), Color::White);
                spriteBatch_->End();
                spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
                spriteBatch_->DrawString(*font_, Panels[i].label, Vector2(static_cast<float>(x), static_cast<float>(y)),
                                         Color::White);
                spriteBatch_->End();
            }
            if (!checked_)
            {
                checked_ = true;
                Context().Check(effect_->getMode() == Panels.back().mode && effect_->getDitherMode() == Panels.back().dither,
                                "the mode and dither settings read back");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D gradient_;
        std::unique_ptr<CNA::Graphics::DepthEffect> effect_;
    };
}

CNA_DEMOS_GAME(ColorReductionDemo, {
    .id = "cnaext.depth_effect.color_reduction",
    .title = "Colour depth reduction (CNAEXT)",
    .description = "Draws one gradient eight ways through CNA's DepthEffect, covering every DepthEffectMode and all "
                   "three DitherMode choices: colour, greyscale and both fixed palettes with and without Bayer dithering.",
    .apis = {"CNA::Graphics::DepthEffect::DepthEffect", "CNA::Graphics::DepthEffect::setMode",
             "CNA::Graphics::DepthEffect::getMode", "CNA::Graphics::DepthEffect::setDitherMode",
             "CNA::Graphics::DepthEffect::getDitherMode", "CNA::Graphics::DepthEffect::Clone",
             "CNA::Graphics::DepthEffect::GetTypeName", "CNA::Graphics::DepthEffectMode", "CNA::Graphics::DitherMode",
             "SpriteBatch::Begin"},
    .tags = {"cnaext", "post-process", "palette", "dithering", "retro", "shader"},
    .needs = CnaDemos::Need::CustomEffects,
    .frames = 10,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "textures/gradient.png"},
    .restrictions = "Shader renderers only (GLSL or SPIR-V package); SOFTWARE and other 2D renderers skip it.",
})
