// SPDX-License-Identifier: MS-PL
// BasicEffect's three DirectionalLights: direction and colour of each, set by hand.
#include <memory>

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
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DirectionalLightsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            sphere_ = CnaDemos::MakeSphere(1.6f);

            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));

            // A white surface, no ambient light: what you see is only the three lights.
            effect_->setLightingEnabledProperty(true);
            effect_->setDiffuseColorProperty(Vector3::One);
            effect_->setAmbientLightColorProperty(Vector3::Zero);
            effect_->setPreferPerPixelLightingProperty(true);

            // Direction is where the light travels: (1, 0, 0) shines from the left towards +X.
            DirectionalLight& red = effect_->getDirectionalLight0Property();
            red.setEnabledProperty(true);
            red.setDirectionProperty(Vector3::Normalize(Vector3(1.0f, 0.0f, -0.4f)));
            red.setDiffuseColorProperty(Vector3(1.0f, 0.0f, 0.0f));
            red.setSpecularColorProperty(Vector3::Zero);

            DirectionalLight& green = effect_->getDirectionalLight1Property();
            green.setEnabledProperty(true);
            green.setDirectionProperty(Vector3::Normalize(Vector3(0.0f, -1.0f, -0.4f)));
            green.setDiffuseColorProperty(Vector3(0.0f, 1.0f, 0.0f));
            green.setSpecularColorProperty(Vector3::Zero);

            DirectionalLight& blue = effect_->getDirectionalLight2Property();
            blue.setEnabledProperty(true);
            blue.setDirectionProperty(Vector3::Normalize(Vector3(-1.0f, 0.0f, -0.4f)));
            blue.setDiffuseColorProperty(Vector3(0.0f, 0.0f, 1.0f));
            blue.setSpecularColorProperty(Vector3::Zero);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, sphere_.vertices.data(), 0,
                                                 sphere_.VertexCount(), sphere_.indices.data(), 0,
                                                 sphere_.PrimitiveCount());
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                const Color left = p.At(320, 240), top = p.At(400, 160), right = p.At(480, 240);
                Context().Print("left {}, top {}, right {}", left.ToString(), top.ToString(), right.ToString());
                Context().Check(left.getRProperty() > left.getBProperty(), "the left side faces the red light");
                Context().Check(top.getGProperty() > top.getRProperty() && top.getGProperty() > top.getBProperty(),
                                "the top faces the green light");
                Context().Check(right.getBProperty() > right.getRProperty(), "the right side faces the blue light");
            }

            Game::Draw(gameTime);
        }

    private:
        CnaDemos::Mesh sphere_;
        std::unique_ptr<BasicEffect> effect_;
    };
}

CNA_DEMOS_GAME(DirectionalLightsDemo, {
    .id = "graphics.basic_effect.directional_lights",
    .title = "Three coloured lights",
    .description = "Lights a white sphere with BasicEffect's DirectionalLight0..2 set by hand: red from the left, "
                   "green from above, blue from the right, and no ambient light.",
    .apis = {"BasicEffect::DirectionalLight0", "BasicEffect::DirectionalLight1", "BasicEffect::DirectionalLight2",
             "DirectionalLight::Direction", "DirectionalLight::DiffuseColor", "DirectionalLight::SpecularColor",
             "DirectionalLight::Enabled", "BasicEffect::AmbientLightColor", "BasicEffect::LightingEnabled"},
    .tags = {"3d", "lighting", "effect", "light"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
