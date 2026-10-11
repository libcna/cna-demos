// SPDX-License-Identifier: MS-PL
// BasicEffect::Alpha: a see-through object, drawn after the opaque scene with alpha blending.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class AlphaDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
            wall_ = CnaDemos::MakeCube(1.0f);
            sphere_ = CnaDemos::MakeSphere(1.0f);
            const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up);
            const Matrix projection = Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);

            background_ = std::make_unique<BasicEffect>(device);
            background_->setTextureEnabledProperty(true);
            background_->setTextureProperty(&checker_);
            background_->setWorldProperty(Matrix::CreateScale(3.0f, 2.0f, 0.1f) * Matrix::CreateTranslation(0.0f, 0.0f, -1.5f));
            background_->setViewProperty(view);
            background_->setProjectionProperty(projection);

            glass_ = std::make_unique<BasicEffect>(device);
            glass_->setViewProperty(view);
            glass_->setProjectionProperty(projection);
            glass_->setDiffuseColorProperty(Color::Orange.ToVector3());
            glass_->EnableDefaultLighting();
            // 0 = invisible, 1 = opaque. BasicEffect outputs premultiplied colour, matching AlphaBlend.
            glass_->setAlphaProperty(0.5f);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            // Opaque things first, normally.
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            DrawMesh(*background_, wall_);
            const CnaDemos::BackBufferPixels before(device);

            // Then transparent ones: blended over what is there, depth-tested but not depth-written.
            device.setBlendStateProperty(BlendState::AlphaBlend);
            device.setDepthStencilStateProperty(DepthStencilState::DepthRead);
            DrawMesh(*glass_, sphere_);

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels after(device);
                // A row through the middle of a row of checker cells, behind the sphere.
                constexpr int y = 262;
                int differentCells = 0;
                for (int x = 330; x < 470; ++x)
                    differentCells += before.At(x, y) != before.At(330, y) ? 1 : 0;
                Context().Check(differentCells > 0, "the checker shows two colours behind the sphere");
                Context().Check(after.At(400, y) != before.At(400, y), "the sphere tints the background");
                bool seeThrough = false;
                for (int x = 330; x < 470; ++x)
                    seeThrough = seeThrough || (before.At(x, y) != before.At(330, y) && after.At(x, y) != after.At(330, y));
                Context().Check(seeThrough, "the checker pattern is still visible through it");
            }

            Game::Draw(gameTime);
        }

    private:
        void DrawMesh(BasicEffect& effect, const CnaDemos::Mesh& mesh)
        {
            for (EffectPass& pass : effect.getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                getGraphicsDeviceProperty().DrawUserIndexedPrimitives(PrimitiveType::TriangleList, mesh.vertices.data(), 0,
                                                                      mesh.VertexCount(), mesh.indices.data(), 0,
                                                                      mesh.PrimitiveCount());
            }
        }

        Texture2D checker_;
        CnaDemos::Mesh wall_;
        CnaDemos::Mesh sphere_;
        std::unique_ptr<BasicEffect> background_;
        std::unique_ptr<BasicEffect> glass_;
    };
}

CNA_DEMOS_GAME(AlphaDemo, {
    .id = "graphics.basic_effect.alpha",
    .title = "Transparent object",
    .description = "Draws a lit sphere with BasicEffect::Alpha 0.5 over a textured wall, with BlendState::AlphaBlend "
                   "and DepthStencilState::DepthRead after the opaque geometry.",
    .apis = {"BasicEffect::Alpha", "BlendState::AlphaBlend", "DepthStencilState::DepthRead", "BasicEffect::World",
             "BasicEffect::View", "BasicEffect::Projection"},
    .tags = {"3d", "transparency", "blend", "effect"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
