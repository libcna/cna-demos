// SPDX-License-Identifier: MS-PL
// Effect::Clone: a copy with all settings, then changed independently -- one effect per material.
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
    class CloneDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            sphere_ = CnaDemos::MakeSphere(1.0f);

            // Configure one effect fully...
            original_ = std::make_unique<BasicEffect>(device);
            original_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up));
            original_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            original_->EnableDefaultLighting();
            original_->setPreferPerPixelLightingProperty(true);
            original_->setDiffuseColorProperty(Color::SteelBlue.ToVector3());
            original_->setWorldProperty(Matrix::CreateTranslation(-1.5f, 0.0f, 0.0f));

            // ...then clone it and change only what differs. In CNA Clone returns a new object the
            // caller owns.
            clone_.reset(static_cast<BasicEffect*>(original_->Clone()));
            clone_->setDiffuseColorProperty(Color::IndianRed.ToVector3());
            clone_->setWorldProperty(Matrix::CreateTranslation(1.5f, 0.0f, 0.0f));

            Context().Check(clone_->getLightingEnabledProperty() && clone_->getPreferPerPixelLightingProperty(),
                            "the clone has the original's settings");
            Context().Check(original_->getDiffuseColorProperty() == Color::SteelBlue.ToVector3(),
                            "changing the clone leaves the original unchanged");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            for (BasicEffect* effect : {original_.get(), clone_.get()})
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
                const CnaDemos::BackBufferPixels p(device);
                const Color left = p.At(263, 240), right = p.At(537, 240);
                Context().Check(left.getBProperty() > left.getRProperty(), "the original draws steel blue");
                Context().Check(right.getRProperty() > right.getBProperty(), "the clone draws red");
            }

            Game::Draw(gameTime);
        }

    private:
        CnaDemos::Mesh sphere_;
        std::unique_ptr<BasicEffect> original_;
        std::unique_ptr<BasicEffect> clone_;
    };
}

CNA_DEMOS_GAME(CloneDemo, {
    .id = "graphics.effect.clone",
    .title = "Clone an effect",
    .description = "Configures a BasicEffect, clones it with Effect::Clone and changes only the clone's colour and "
                   "position; both draw with their own settings.",
    .apis = {"Effect::Clone", "BasicEffect::Clone", "BasicEffect::DiffuseColor", "BasicEffect::World",
             "BasicEffect::View", "BasicEffect::Projection"},
    .tags = {"effect", "material", "copy"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
