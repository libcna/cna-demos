// SPDX-License-Identifier: MS-PL
// A camera orbit: the scene stays still while Matrix::CreateLookAt changes every frame.
#include <array>
#include <cmath>
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
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Color Background(24, 31, 45);

    class OrbitingViewDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            cube_ = CnaDemos::MakeCube(0.65f);
            worlds_ = {
                Matrix::CreateTranslation(-1.8f, 0.0f, 0.0f),
                Matrix::CreateTranslation(0.0f, 0.8f, -1.3f),
                Matrix::CreateTranslation(1.8f, -0.2f, 0.5f),
            };
            effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4,
                getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            effect_->setLightingEnabledProperty(false);

            const Vector3 firstEye(7.0f, 3.0f, 0.0f);
            const Matrix firstView = Matrix::CreateLookAt(firstEye, Vector3::Zero, Vector3::Up);
            const Matrix laterView = Matrix::CreateLookAt(Vector3(0.0f, 3.0f, 7.0f), Vector3::Zero, Vector3::Up);
            Context().Check(firstView != laterView &&
                                worlds_[0] == Matrix::CreateTranslation(-1.8f, 0.0f, 0.0f) &&
                                worlds_[1] == Matrix::CreateTranslation(0.0f, 0.8f, -1.3f) &&
                                worlds_[2] == Matrix::CreateTranslation(1.8f, -0.2f, 0.5f),
                            "the view changes around three immutable world transforms");
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            angle_ = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty()) * 0.55f;
            const Vector3 eye(std::cos(angle_) * 7.0f, 3.0f, std::sin(angle_) * 7.0f);
            view_ = Matrix::CreateLookAt(eye, Vector3::Zero, Vector3::Up);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Background);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            effect_->setViewProperty(view_);

            const std::array<Vector3, 3> colours = {
                Vector3(0.95f, 0.25f, 0.18f),
                Vector3(0.20f, 0.85f, 0.35f),
                Vector3(0.25f, 0.45f, 1.00f),
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

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                int covered = 0;
                for (int y = 0; y < pixels.Height(); y += 3)
                    for (int x = 0; x < pixels.Width(); x += 3)
                        covered += CnaDemos::Near(pixels.At(x, y), Background, 2) ? 0 : 1;
                Context().Check(covered > 1200, "the orbiting view renders all three static cubes");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        CnaDemos::Mesh cube_;
        std::array<Matrix, 3> worlds_{};
        std::unique_ptr<BasicEffect> effect_;
        Matrix view_ = Matrix::getIdentityProperty();
        float angle_ = 0.0f;
    };
}

CNA_DEMOS_GAME(OrbitingViewDemo, {
    .id = "graphics.camera.orbiting_view",
    .title = "Orbiting view matrix",
    .description = "Keeps three cubes' World matrices fixed while the camera position travels around them and "
                   "Matrix::CreateLookAt is recomputed every frame, isolating camera motion from object motion.",
    .apis = {"Matrix::CreateLookAt", "BasicEffect::View", "BasicEffect::World",
             "Matrix::CreatePerspectiveFieldOfView", "GraphicsDevice::DrawUserIndexedPrimitives"},
    .tags = {"3d", "camera", "view", "orbit", "look-at"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 5,
    .deterministic = true,
})
