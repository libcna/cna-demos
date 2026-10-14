// SPDX-License-Identifier: MS-PL
// BasicEffect::EnableDefaultLighting: XNA's three-light rig makes a flat disc read as a sphere.
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
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DefaultLightingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const DirectionalLight standaloneLight;
            Context().Check(!standaloneLight.getEnabledProperty() &&
                                standaloneLight.getDirectionProperty() == Vector3::Down &&
                                standaloneLight.getDiffuseColorProperty() == Vector3::One &&
                                standaloneLight.getSpecularColorProperty() == Vector3::Zero,
                            "a standalone directional light starts disabled, white, downward and nonspecular");
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            sphere_ = CnaDemos::MakeSphere(1.0f);

            const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up);
            const Matrix projection = Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
            for (auto* effect : {&unlit_, &lit_})
            {
                *effect = std::make_unique<BasicEffect>(device);
                (*effect)->setViewProperty(view);
                (*effect)->setProjectionProperty(projection);
                (*effect)->setDiffuseColorProperty(Color::Orange.ToVector3());
            }
            unlit_->setWorldProperty(Matrix::CreateTranslation(-1.5f, 0.0f, 0.0f));
            lit_->setWorldProperty(Matrix::CreateTranslation(1.5f, 0.0f, 0.0f));

            // Without lighting every pixel is DiffuseColor. EnableDefaultLighting switches lighting
            // on and sets up a key, fill and back light plus ambient light, as XNA does.
            lit_->EnableDefaultLighting();
            Context().Check(lit_->getLightingEnabledProperty(), "EnableDefaultLighting turns LightingEnabled on");
            Context().Check(lit_->getDirectionalLight0Property().getEnabledProperty() &&
                                lit_->getDirectionalLight1Property().getEnabledProperty() &&
                                lit_->getDirectionalLight2Property().getEnabledProperty(),
                            "and enables all three directional lights");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            for (BasicEffect* effect : {unlit_.get(), lit_.get()})
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
                // Each sphere's centre and a point near its lower-left rim.
                const Color unlitCentre = p.At(263, 240), unlitRim = p.At(222, 280);
                const Color litCentre = p.At(537, 240), litRim = p.At(496, 280);
                Context().Print("unlit: centre {} rim {}; lit: centre {} rim {}", unlitCentre.ToString(),
                                unlitRim.ToString(), litCentre.ToString(), litRim.ToString());
                Context().Check(unlitCentre == unlitRim, "unlit: the same colour everywhere");
                Context().Check(litCentre != litRim, "lit: brightness changes with the surface direction");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "LightingEnabled = false", Vector2(90.0f, 400.0f), Color::White);
            spriteBatch_->DrawString(*font_, "EnableDefaultLighting()", Vector2(440.0f, 400.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        CnaDemos::Mesh sphere_;
        std::unique_ptr<BasicEffect> unlit_;
        std::unique_ptr<BasicEffect> lit_;
    };
}

CNA_DEMOS_GAME(DefaultLightingDemo, {
    .id = "graphics.basic_effect.default_lighting",
    .title = "Default lighting",
    .description = "Draws the same sphere unlit (a flat disc of DiffuseColor) and with BasicEffect::"
                   "EnableDefaultLighting, which turns on XNA's standard three-light rig.",
    .apis = {"DirectionalLight::DirectionalLight", "BasicEffect::EnableDefaultLighting",
             "BasicEffect::LightingEnabled", "BasicEffect::DiffuseColor",
             "BasicEffect::DirectionalLight0", "DirectionalLight::Enabled", "BasicEffect::DirectionalLight1",
             "BasicEffect::DirectionalLight2", "BasicEffect::World", "BasicEffect::View", "BasicEffect::Projection"},
    .tags = {"3d", "lighting", "effect", "sphere"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
