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

    constexpr std::array<Panel, 6> Panels = {{
        {DepthEffectMode::Color16Bit, DitherMode::None, "Color16Bit"},
        {DepthEffectMode::Color8Bit, DitherMode::None, "Color8Bit"},
        {DepthEffectMode::Color8Bit, DitherMode::Bayer4x4, "8Bit+Bayer4x4"},
        {DepthEffectMode::Palette16, DitherMode::Bayer8x8, "Palette16+8x8"},
        {DepthEffectMode::Grayscale2Bit, DitherMode::None, "Grayscale2Bit"},
        {DepthEffectMode::Grayscale1Bit, DitherMode::Bayer4x4, "1Bit+Bayer4x4"},
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
                const int x = 20 + static_cast<int>(i % 3) * 260;
                const int y = 30 + static_cast<int>(i / 3) * 220;
                spriteBatch_->Begin(SpriteSortMode::Immediate, &BlendState::Opaque, &SamplerState::PointClamp, nullptr,
                                    nullptr, effect_.get());
                spriteBatch_->Draw(gradient_, Rectangle(x, y + 24, 240, 180), Color::White);
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
    .description = "Draws one gradient six times through CNA's DepthEffect with different DepthEffectMode and "
                   "DitherMode settings -- 16- and 8-bit colour, a 16-colour palette, 2- and 1-bit grey -- with and "
                   "without Bayer dithering.",
    .apis = {"CNA::Graphics::DepthEffect::DepthEffect", "CNA::Graphics::DepthEffect::setMode",
             "CNA::Graphics::DepthEffect::setDitherMode", "CNA::Graphics::DepthEffectMode", "CNA::Graphics::DitherMode",
             "SpriteBatch::Begin"},
    .tags = {"cnaext", "post-process", "palette", "dithering", "retro", "shader"},
    .needs = CnaDemos::Need::CustomEffects,
    .frames = 10,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "textures/gradient.png"},
    .restrictions = "Shader renderers only (GLSL or SPIR-V package); SOFTWARE and other 2D renderers skip it.",
})
