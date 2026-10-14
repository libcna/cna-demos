// SPDX-License-Identifier: MS-PL
// IEffectMatrices, IEffectLights, IEffectFog: configuring any effect without knowing its type.
#include <array>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/AlphaTestEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DualTextureEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/EnvironmentMapEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/IEffectFog.hpp"
#include "Microsoft/Xna/Framework/Graphics/IEffectLights.hpp"
#include "Microsoft/Xna/Framework/Graphics/IEffectMatrices.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Color Background(70, 80, 100);

    class EffectInterfacesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            GraphicsDevice& device = getGraphicsDeviceProperty();
            sphere_ = CnaDemos::MakeSphere(0.8f);

            white_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);
            sky_ = std::make_unique<TextureCube>(device, 4, false, SurfaceFormat::Color);
            for (int face = 0; face < 6; ++face)
            {
                const std::vector<Color> pixels(16, face == 2 ? Color(220, 220, 255) : Color(90, 110, 160));
                sky_->SetData(static_cast<CubeMapFace>(face), pixels.data(), 16);
            }

            // Five stock effects with their own type-specific settings...
            auto basic = std::make_unique<BasicEffect>(device);
            basic->setDiffuseColorProperty(Vector3(1.0f, 0.5f, 0.2f));
            auto alphaTest = std::make_unique<AlphaTestEffect>(device);
            alphaTest->setTextureProperty(white_.get());
            alphaTest->setDiffuseColorProperty(Vector3(0.3f, 0.9f, 0.4f));
            auto environment = std::make_unique<EnvironmentMapEffect>(device);
            environment->setTextureProperty(white_.get());
            environment->setEnvironmentMapProperty(sky_.get());
            effects_.push_back(std::move(basic));
            effects_.push_back(std::move(alphaTest));
            effects_.push_back(std::move(environment));
            dual_ = std::make_unique<DualTextureEffect>(device);
            skinned_ = std::make_unique<SkinnedEffect>(device);

            // ...and which of the three interfaces each one implements, as in XNA.
            struct Expected
            {
                const char* name;
                Effect* effect;
                bool lights;
            };
            const std::array<Expected, 5> expected = {{{"BasicEffect", effects_[0].get(), true},
                                                       {"AlphaTestEffect", effects_[1].get(), false},
                                                       {"EnvironmentMapEffect", effects_[2].get(), true},
                                                       {"DualTextureEffect", dual_.get(), false},
                                                       {"SkinnedEffect", skinned_.get(), true}}};
            bool allMatch = true;
            for (const Expected& e : expected)
            {
                const bool matrices = dynamic_cast<IEffectMatrices*>(e.effect) != nullptr;
                const bool lights = dynamic_cast<IEffectLights*>(e.effect) != nullptr;
                const bool fog = dynamic_cast<IEffectFog*>(e.effect) != nullptr;
                ctx.Print("{:<22} IEffectMatrices {}  IEffectLights {}  IEffectFog {}", e.name, matrices, lights, fog);
                allMatch = allMatch && matrices && fog && lights == e.lights;
            }
            ctx.Check(allMatch, "every stock effect has matrices and fog; AlphaTest and DualTexture have no lights");

            // The scene-wide settings, applied to whatever effects there are through the interfaces --
            // what a model-drawing loop does with ModelMesh::Effects.
            const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 0.4f, 7.0f), Vector3::Zero, Vector3::Up);
            const Matrix projection = Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
            for (std::size_t i = 0; i < effects_.size(); ++i)
            {
                if (auto* matrices = dynamic_cast<IEffectMatrices*>(effects_[i].get()))
                {
                    // Each sphere a step further away, so the fog grows from left to right.
                    matrices->setWorldProperty(Matrix::CreateTranslation(-2.2f + 2.2f * static_cast<float>(i), 0.0f,
                                                                         -3.0f * static_cast<float>(i)));
                    matrices->setViewProperty(view);
                    matrices->setProjectionProperty(projection);
                }
                if (auto* lights = dynamic_cast<IEffectLights*>(effects_[i].get()))
                {
                    lights->EnableDefaultLighting();
                }
                if (auto* fog = dynamic_cast<IEffectFog*>(effects_[i].get()))
                {
                    fog->setFogEnabledProperty(true);
                    fog->setFogColorProperty(Background.ToVector3());
                    fog->setFogStartProperty(6.0f);
                    fog->setFogEndProperty(16.0f);
                }
            }
            auto* basicEffect = dynamic_cast<BasicEffect*>(effects_[0].get());
            ctx.Check(basicEffect->getViewProperty() == view && basicEffect->getLightingEnabledProperty() &&
                          basicEffect->getFogEnabledProperty(),
                      "the concrete BasicEffect sees what was set through its interfaces");
            auto* alphaEffect = dynamic_cast<AlphaTestEffect*>(effects_[1].get());
            ctx.Check(alphaEffect->getWorldProperty() == Matrix::CreateTranslation(0.0f, 0.0f, -3.0f) &&
                          alphaEffect->getViewProperty() == view && alphaEffect->getFogEnabledProperty() &&
                          alphaEffect->getFogColorProperty() == Background.ToVector3() &&
                          alphaEffect->getFogStartProperty() == 6.0f &&
                          alphaEffect->getFogEndProperty() == 16.0f,
                      "AlphaTestEffect exposes the exact matrix and fog state set through interfaces");
            auto* environmentEffect = dynamic_cast<EnvironmentMapEffect*>(effects_[2].get());
            ctx.Check(environmentEffect->getWorldProperty() == Matrix::CreateTranslation(2.2f, 0.0f, -6.0f) &&
                          environmentEffect->getViewProperty() == view &&
                          environmentEffect->getFogEnabledProperty() &&
                          environmentEffect->getFogColorProperty() == Background.ToVector3() &&
                          environmentEffect->getFogStartProperty() == 6.0f &&
                          environmentEffect->getFogEndProperty() == 16.0f &&
                          environmentEffect->getLightingEnabledProperty() &&
                          environmentEffect->getAmbientLightColorProperty() != Vector3::Zero,
                      "EnvironmentMapEffect exposes exact interface state and enabled default lighting");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Background);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            for (const auto& effect : effects_)
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
                // Fog pulls the farther spheres towards the background colour.
                const CnaDemos::BackBufferPixels pixels(device);
                const Viewport viewport = device.getViewportProperty();
                std::array<int, 3> difference{};
                for (std::size_t i = 0; i < effects_.size(); ++i)
                {
                    const auto* matrices = dynamic_cast<IEffectMatrices*>(effects_[i].get());
                    const Vector3 centre = viewport.Project(Vector3::Zero, matrices->getProjectionProperty(),
                                                            matrices->getViewProperty(), matrices->getWorldProperty());
                    const Color c = pixels.At(static_cast<int>(centre.X), static_cast<int>(centre.Y));
                    difference[i] = std::abs(c.getRProperty() - Background.getRProperty()) +
                                    std::abs(c.getGProperty() - Background.getGProperty()) +
                                    std::abs(c.getBProperty() - Background.getBProperty());
                    Context().Print("sphere {} centre {} (distance from the fog colour {})", i, c.ToString(), difference[i]);
                }
                Context().Check(difference[0] > difference[1] && difference[1] > difference[2],
                                "the farther the sphere, the more it fades into the fog");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        CnaDemos::Mesh sphere_;
        std::unique_ptr<Texture2D> white_;
        std::unique_ptr<TextureCube> sky_;
        std::vector<std::unique_ptr<Effect>> effects_;
        std::unique_ptr<DualTextureEffect> dual_;
        std::unique_ptr<SkinnedEffect> skinned_;
    };
}

CNA_DEMOS_GAME(EffectInterfacesDemo, {
    .id = "graphics.effect.effect_interfaces",
    .title = "Effect interfaces",
    .description = "Asks five stock effects which of IEffectMatrices, IEffectLights and IEffectFog they implement, "
                   "then sets camera, lighting and fog on a list of different effects through those interfaces "
                   "alone -- as model-drawing code does -- and draws one sphere with each, fading with distance.",
    .apis = {"IEffectMatrices", "IEffectLights", "IEffectFog", "IEffectMatrices::World", "IEffectMatrices::View",
             "IEffectMatrices::Projection", "IEffectLights::EnableDefaultLighting", "IEffectFog::FogEnabled",
             "IEffectFog::FogColor", "IEffectFog::FogStart", "IEffectFog::FogEnd",
             "IEffectLights::AmbientLightColor", "IEffectLights::LightingEnabled",
             "AlphaTestEffect::World", "AlphaTestEffect::View", "AlphaTestEffect::FogEnabled",
             "AlphaTestEffect::FogColor", "AlphaTestEffect::FogStart", "AlphaTestEffect::FogEnd",
             "EnvironmentMapEffect::World", "EnvironmentMapEffect::View",
             "EnvironmentMapEffect::FogEnabled", "EnvironmentMapEffect::FogColor",
             "EnvironmentMapEffect::FogStart", "EnvironmentMapEffect::FogEnd",
             "EnvironmentMapEffect::Microsoft#Xna#Framework#Graphics#IEffectLights#LightingEnabled"},
    .tags = {"3d", "effect", "interfaces", "fog", "lighting"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
