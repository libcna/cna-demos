// SPDX-License-Identifier: MS-PL
// Anisotropic filtering: a textured floor seen at a grazing angle stays sharp into the distance.
#include <array>
#include <cstddef>
#include <vector>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerStateCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionTexture.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class AnisotropicDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // A mipmapped copy of the checker texture (filtering at a distance needs mip levels).
            const Texture2D source = getContentProperty().Load<Texture2D>("textures/checker");
            std::array<Color, 64 * 64> pixels{};
            source.GetData(pixels.data(), static_cast<int>(pixels.size()));
            floorTexture_ = std::make_unique<Texture2D>(device, 64, 64, true, SurfaceFormat::Color);
            floorTexture_->SetData(pixels.data(), static_cast<int>(pixels.size()));
            GenerateMips(*floorTexture_);

            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setTextureEnabledProperty(true);
            effect_->setTextureProperty(floorTexture_.get());
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.6f, 4.0f), Vector3(0.0f, 0.0f, -10.0f),
                                                          Vector3::Up));
            effect_->setProjectionProperty(
                Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, 400.0f / 480.0f, 0.1f, 100.0f));

            // A long floor, the texture repeated 40 times along it.
            floor_ = {VertexPositionTexture(Vector3(-4.0f, 0.0f, 4.0f), Vector2(0.0f, 0.0f)),
                      VertexPositionTexture(Vector3(4.0f, 0.0f, 4.0f), Vector2(4.0f, 0.0f)),
                      VertexPositionTexture(Vector3(-4.0f, 0.0f, -76.0f), Vector2(0.0f, 40.0f)),
                      VertexPositionTexture(Vector3(4.0f, 0.0f, 4.0f), Vector2(4.0f, 0.0f)),
                      VertexPositionTexture(Vector3(4.0f, 0.0f, -76.0f), Vector2(4.0f, 40.0f)),
                      VertexPositionTexture(Vector3(-4.0f, 0.0f, -76.0f), Vector2(0.0f, 40.0f))};

            anisotropic_ = SamplerState::AnisotropicWrap;
            Context().Check(SamplerState::AnisotropicClamp.getFilterProperty() == TextureFilter::Anisotropic &&
                                SamplerState::AnisotropicClamp.getAddressUProperty() == TextureAddressMode::Clamp &&
                                SamplerState::AnisotropicClamp.getAddressVProperty() == TextureAddressMode::Clamp &&
                                SamplerState::AnisotropicClamp.getAddressWProperty() == TextureAddressMode::Clamp &&
                                SamplerState::AnisotropicClamp.getMaxAnisotropyProperty() == 4,
                            "AnisotropicClamp combines anisotropic filtering with clamping on every axis");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::SkyBlue);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            const Viewport full = device.getViewportProperty();

            // Left: trilinear (LinearWrap). Right: anisotropic (AnisotropicWrap, MaxAnisotropy 4).
            DrawFloor(Viewport(0, 0, 400, 480), SamplerState::LinearWrap);
            DrawFloor(Viewport(400, 0, 400, 480), anisotropic_);
            device.setViewportProperty(full);

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "LinearWrap", Vector2(20.0f, 20.0f), Color::Black);
            spriteBatch_->DrawString(*font_, "AnisotropicWrap", Vector2(420.0f, 20.0f), Color::Black);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        void DrawFloor(const Viewport& viewport, const SamplerState& sampler)
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.setViewportProperty(viewport);
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                // Set after Apply: the effect binds its texture, the sampler slot is ours.
                device.getSamplerStatesProperty()(0, sampler);
                device.DrawUserPrimitives(PrimitiveType::TriangleList, floor_.data(), 0, 2);
            }
        }

        // Box-filters each level from the one above (Texture2D has no automatic mip generation).
        static void GenerateMips(Texture2D& texture)
        {
            int size = texture.getWidthProperty();
            std::vector<Color> above(static_cast<std::size_t>(size) * size);
            texture.GetData(0, nullptr, above.data(), 0, static_cast<int>(above.size()));
            for (int level = 1; level < texture.getLevelCountProperty(); ++level)
            {
                const int half = size / 2;
                std::vector<Color> below(static_cast<std::size_t>(half) * half);
                for (int y = 0; y < half; ++y)
                    for (int x = 0; x < half; ++x)
                    {
                        const auto at = [&](int dx, int dy) {
                            return above[static_cast<std::size_t>(2 * y + dy) * size + 2 * x + dx].ToVector4();
                        };
                        below[static_cast<std::size_t>(y) * half + x] =
                            Color((at(0, 0) + at(1, 0) + at(0, 1) + at(1, 1)) * 0.25f);
                    }
                texture.SetData(level, nullptr, below.data(), 0, static_cast<int>(below.size()));
                above = std::move(below);
                size = half;
            }
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> floorTexture_;
        std::unique_ptr<BasicEffect> effect_;
        std::array<VertexPositionTexture, 6> floor_{};
        SamplerState anisotropic_;
    };
}

CNA_DEMOS_GAME(AnisotropicDemo, {
    .id = "graphics.sampler_state.anisotropic",
    .title = "Anisotropic filtering",
    .description = "Draws a textured floor receding into the distance with SamplerState::LinearWrap and "
                   "SamplerState::AnisotropicWrap side by side; anisotropic filtering keeps the far checkers sharper.",
    .apis = {"SamplerState::AnisotropicWrap", "SamplerState::AnisotropicClamp", "SamplerState::LinearWrap",
             "SamplerState::MaxAnisotropy",
             "GraphicsDevice::SamplerStates", "BasicEffect::TextureEnabled", "BasicEffect::Texture",
             "VertexPositionTexture::VertexPositionTexture"},
    .tags = {"3d", "sampler", "filtering", "texture", "state"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::AnisotropicFiltering,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png", "fonts/pixel_font.cnb"},
    .restrictions = "Visual only: how much sharper the far floor gets depends on the renderer's LOD selection "
                    "(clear on OPENGLES3; SOFTWARE's trilinear result is already sharp in this scene).",
})
