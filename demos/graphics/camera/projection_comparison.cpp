// SPDX-License-Identifier: MS-PL
// Perspective and orthographic projection applied to the same depth-staggered scene.
#include <array>
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
    const Color Background(30, 35, 48);

    class ProjectionComparisonDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            cube_ = CnaDemos::MakeCube(0.65f);
            worlds_ = {
                Matrix::CreateTranslation(-1.9f, 0.0f, 0.0f),
                Matrix::CreateTranslation(0.0f, 0.0f, -2.0f),
                Matrix::CreateTranslation(1.9f, 0.0f, -4.0f),
            };
            view_ = Matrix::CreateLookAt(Vector3(0.0f, 2.0f, 9.0f), Vector3(0.0f, 0.0f, -2.0f), Vector3::Up);
            effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
            effect_->setLightingEnabledProperty(false);
            effect_->setViewProperty(view_);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const Viewport full = device.getViewportProperty();
            const int half = full.getWidthProperty() / 2;
            const std::array<Viewport, 2> panels = {
                Viewport(0, 0, half, full.getHeightProperty()),
                Viewport(half, 0, full.getWidthProperty() - half, full.getHeightProperty()),
            };
            const float aspect = static_cast<float>(panels[0].getWidthProperty()) /
                static_cast<float>(panels[0].getHeightProperty());
            const std::array<Matrix, 2> projections = {
                Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, aspect, 0.1f, 100.0f),
                Matrix::CreateOrthographic(8.0f, 8.0f / aspect, 0.1f, 100.0f),
            };

            device.Clear(Background);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            for (std::size_t panel = 0; panel < panels.size(); ++panel)
            {
                device.setViewportProperty(panels[panel]);
                effect_->setProjectionProperty(projections[panel]);
                DrawScene(device);
            }
            device.setViewportProperty(full);

            if (FramesDrawn() == 0)
            {
                // Measure the projected height of an identical one-unit segment at the near and
                // far scene depths. Perspective shrinks it; orthographic projection does not.
                const auto projectedHeight = [&](const Viewport& viewport, const Matrix& projection, float z) {
                    const Vector3 low = viewport.Project(Vector3(0.0f, -0.5f, z), projection, view_,
                                                         Matrix::getIdentityProperty());
                    const Vector3 high = viewport.Project(Vector3(0.0f, 0.5f, z), projection, view_,
                                                          Matrix::getIdentityProperty());
                    return low.Y - high.Y;
                };
                const float perspectiveNear = projectedHeight(panels[0], projections[0], 0.0f);
                const float perspectiveFar = projectedHeight(panels[0], projections[0], -4.0f);
                const float orthographicNear = projectedHeight(panels[1], projections[1], 0.0f);
                const float orthographicFar = projectedHeight(panels[1], projections[1], -4.0f);
                Context().Check(perspectiveNear > perspectiveFar * 1.25f,
                                "perspective makes the farther copy visibly smaller");
                Context().CheckNear(orthographicNear, orthographicFar, 0.001,
                                    "orthographic projection preserves size across depth");

                const CnaDemos::BackBufferPixels pixels(device);
                int leftCovered = 0;
                int rightCovered = 0;
                for (int y = 0; y < pixels.Height(); y += 3)
                    for (int x = 0; x < half; x += 3)
                    {
                        leftCovered += CnaDemos::Near(pixels.At(x, y), Background, 2) ? 0 : 1;
                        rightCovered += CnaDemos::Near(pixels.At(x + half, y), Background, 2) ? 0 : 1;
                    }
                Context().Check(leftCovered > 350 && rightCovered > 350,
                                "the same three-cube scene is visible in both projection panels");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        void DrawScene(GraphicsDevice& device)
        {
            const std::array<Vector3, 3> colours = {
                Vector3(0.95f, 0.30f, 0.20f),
                Vector3(0.25f, 0.90f, 0.40f),
                Vector3(0.30f, 0.50f, 1.00f),
            };
            for (std::size_t i = 0; i < worlds_.size(); ++i)
            {
                effect_->setWorldProperty(worlds_[i]);
                effect_->setDiffuseColorProperty(colours[i]);
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, cube_.vertices.data(), 0,
                                                     cube_.VertexCount(), cube_.indices.data(), 0,
                                                     cube_.PrimitiveCount());
                }
            }
        }

        CnaDemos::Mesh cube_;
        std::array<Matrix, 3> worlds_{};
        std::unique_ptr<BasicEffect> effect_;
        Matrix view_ = Matrix::getIdentityProperty();
    };
}

CNA_DEMOS_GAME(ProjectionComparisonDemo, {
    .id = "graphics.camera.projection_comparison",
    .title = "Perspective versus orthographic",
    .description = "Draws the same depth-staggered three-cube scene in two viewports: perspective makes the far "
                   "cube smaller, while orthographic projection preserves an object's screen size across depth.",
    .apis = {"Matrix::CreatePerspectiveFieldOfView", "Matrix::CreateOrthographic", "BasicEffect::Projection",
             "GraphicsDevice::Viewport", "Viewport::Project", "GraphicsDevice::DrawUserIndexedPrimitives"},
    .tags = {"3d", "camera", "projection", "perspective", "orthographic", "viewport"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
