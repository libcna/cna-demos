// SPDX-License-Identifier: MS-PL
// BasicEffect material colours: diffuse, ambient, emissive and specular -- one sphere each.
#include <array>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DirectionalLight.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr std::array<const char*, 4> Names = {"Diffuse", "+ Ambient", "+ Emissive", "+ Specular"};

    class MaterialColorsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            sphere_ = CnaDemos::MakeSphere(0.8f);

            for (std::size_t i = 0; i < effects_.size(); ++i)
            {
                auto& effect = effects_[i];
                effect = std::make_unique<BasicEffect>(device);
                effect->setWorldProperty(Matrix::CreateTranslation(-3.0f + 2.0f * static_cast<float>(i), 0.0f, 0.0f));
                effect->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 7.0f), Vector3::Zero, Vector3::Up));
                effect->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                    MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
                effect->setLightingEnabledProperty(true);
                effect->setPreferPerPixelLightingProperty(true);
                DirectionalLight& light = effect->getDirectionalLight0Property();
                light.setEnabledProperty(true);
                light.setDirectionProperty(Vector3::Normalize(Vector3(-1.0f, -0.6f, -0.6f)));
                light.setDiffuseColorProperty(Vector3::One);
                light.setSpecularColorProperty(Vector3::One);

                // Diffuse: the colour the surface reflects from lights; it fades to black away from them.
                effect->setDiffuseColorProperty(Vector3(0.8f, 0.2f, 0.2f));
                effect->setAmbientLightColorProperty(Vector3::Zero);
                effect->setEmissiveColorProperty(Vector3::Zero);
                effect->setSpecularColorProperty(Vector3::Zero);
                // Ambient: light from everywhere, so the shadow side is not black.
                if (i >= 1)
                    effect->setAmbientLightColorProperty(Vector3(0.25f, 0.25f, 0.25f));
                // Emissive: light the surface gives off itself, added regardless of the lights.
                if (i >= 2)
                    effect->setEmissiveColorProperty(Vector3(0.0f, 0.0f, 0.35f));
                // Specular: the shiny highlight; SpecularPower sets its size.
                if (i >= 3)
                {
                    effect->setSpecularColorProperty(Vector3::One);
                    effect->setSpecularPowerProperty(24.0f);
                }
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            for (auto& effect : effects_)
            {
                for (EffectPass& pass : effect->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, sphere_.vertices.data(), 0,
                                                     sphere_.VertexCount(), sphere_.indices.data(), 0,
                                                     sphere_.PrimitiveCount());
                }
            }

            if (FramesDrawn() == 0)
            {
                // A point on each sphere facing the camera but away from the light (lower left).
                const CnaDemos::BackBufferPixels p(device);
                const Vector3 shadowSide = Vector3::Normalize(Vector3(-1.0f, -0.6f, 0.5f)) * 0.8f;
                const auto shade = [&](std::size_t i) {
                    const Vector3 s = device.getViewportProperty().Project(shadowSide, effects_[i]->getProjectionProperty(),
                                                                           effects_[i]->getViewProperty(),
                                                                           effects_[i]->getWorldProperty());
                    return p.At(static_cast<int>(s.X), static_cast<int>(s.Y));
                };
                const Color plain = shade(0), ambient = shade(1), emissive = shade(2);
                Context().Print("shadow side: diffuse only {}, + ambient {}, + emissive {}", plain.ToString(),
                                ambient.ToString(), emissive.ToString());
                Context().Check(plain.getRProperty() < 20, "diffuse only: the side away from the light is black");
                Context().Check(ambient.getRProperty() > plain.getRProperty(), "ambient light lifts the shadow side");
                Context().Check(emissive.getBProperty() > ambient.getBProperty(), "emissive colour adds its own light");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            for (std::size_t i = 0; i < Names.size(); ++i)
                spriteBatch_->DrawString(*font_, Names[i], Vector2(100.0f + 160.0f * static_cast<float>(i), 340.0f),
                                         Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        CnaDemos::Mesh sphere_;
        std::array<std::unique_ptr<BasicEffect>, 4> effects_;
    };
}

CNA_DEMOS_GAME(MaterialColorsDemo, {
    .id = "graphics.basic_effect.material_colors",
    .title = "Material colours",
    .description = "Builds a material up step by step on four spheres: DiffuseColor, then AmbientLightColor, "
                   "EmissiveColor and finally SpecularColor with SpecularPower.",
    .apis = {"BasicEffect::DiffuseColor", "BasicEffect::AmbientLightColor", "BasicEffect::EmissiveColor",
             "BasicEffect::SpecularColor", "BasicEffect::SpecularPower", "BasicEffect::LightingEnabled",
             "BasicEffect::DirectionalLight0", "BasicEffect::World", "BasicEffect::View", "BasicEffect::Projection"},
    .tags = {"3d", "lighting", "material", "effect"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
