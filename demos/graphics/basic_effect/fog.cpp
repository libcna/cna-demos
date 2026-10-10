// SPDX-License-Identifier: MS-PL
// BasicEffect fog: objects fade into FogColor between FogStart and FogEnd.
#include <cstdlib>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Color FogColor = Color::LightSteelBlue;
    constexpr int Pillars = 8;

    class FogDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            cube_ = CnaDemos::MakeCube(0.5f);
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 1.5f, 3.0f), Vector3(0.0f, 0.5f, -10.0f),
                                                          Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            effect_->setDiffuseColorProperty(Color::DarkRed.ToVector3());
            effect_->EnableDefaultLighting();

            // Linear fog by distance from the camera: none before FogStart, all FogColor after FogEnd.
            effect_->setFogEnabledProperty(true);
            effect_->setFogColorProperty(FogColor.ToVector3());
            effect_->setFogStartProperty(4.0f);
            effect_->setFogEndProperty(22.0f);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            // Clearing to the fog colour lets distant objects melt into the background.
            device.Clear(FogColor);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            // Two rows of pillars receding from the camera, 3 units apart.
            for (int i = 0; i < Pillars; ++i)
            {
                for (const float x : {-1.5f, 1.5f})
                {
                    effect_->setWorldProperty(Matrix::CreateScale(1.0f, 3.0f, 1.0f) *
                                              Matrix::CreateTranslation(x, 1.0f, -3.0f * static_cast<float>(i)));
                    for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                    {
                        pass.Apply();
                        device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, cube_.vertices.data(), 0,
                                                         cube_.VertexCount(), cube_.indices.data(), 0,
                                                         cube_.PrimitiveCount());
                    }
                }
            }

            if (FramesDrawn() == 0)
            {
                // The inner (street-facing) side of right-hand pillars 0, 3 and 7: nearer pillars do
                // not hide it. The farther the pillar, the closer its colour is to the fog colour.
                const CnaDemos::BackBufferPixels p(device);
                const auto distanceToFog = [&](int i) {
                    const Vector3 s = device.getViewportProperty().Project(Vector3(1.0f, 1.0f, -3.0f * static_cast<float>(i)),
                                                                           effect_->getProjectionProperty(),
                                                                           effect_->getViewProperty(),
                                                                           Matrix::getIdentityProperty());
                    const Color c = p.At(static_cast<int>(s.X) + 1, static_cast<int>(s.Y));
                    return std::abs(c.getRProperty() - FogColor.getRProperty()) +
                           std::abs(c.getGProperty() - FogColor.getGProperty()) +
                           std::abs(c.getBProperty() - FogColor.getBProperty());
                };
                const int nearest = distanceToFog(0), middle = distanceToFog(3), farthest = distanceToFog(Pillars - 1);
                Context().Print("difference from the fog colour: pillar 0 {}, pillar 3 {}, pillar {} {}", nearest, middle,
                                Pillars - 1, farthest);
                Context().Check(nearest > middle && middle > farthest, "the farther the pillar, the foggier");
                Context().Check(farthest <= 24, "the last pillar, beyond FogEnd, is all fog");
            }

            Game::Draw(gameTime);
        }

    private:
        CnaDemos::Mesh cube_;
        std::unique_ptr<BasicEffect> effect_;
    };
}

CNA_DEMOS_GAME(FogDemo, {
    .id = "graphics.basic_effect.fog",
    .title = "Linear fog",
    .description = "Draws two rows of pillars receding into BasicEffect fog (FogEnabled, FogColor, FogStart 4, "
                   "FogEnd 22): the farther a pillar, the more it takes the fog colour.",
    .apis = {"BasicEffect::FogEnabled", "BasicEffect::FogColor", "BasicEffect::FogStart", "BasicEffect::FogEnd"},
    .tags = {"3d", "fog", "atmosphere", "effect"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
