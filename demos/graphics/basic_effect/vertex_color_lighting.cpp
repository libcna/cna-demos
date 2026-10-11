// SPDX-License-Identifier: MS-PL
// BasicEffect with VertexColorEnabled and LightingEnabled together: per-vertex colour, lit.
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

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
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // XNA has no stock VertexPositionNormalColor, so the vertex is a struct of our own. Its colour
    // is the packed RGBA value (CNA's Color is a class, CNA-DEMOS-ISSUE-005).
    struct VertexPositionNormalColor
    {
        Vector3 Position;
        Vector3 Normal;
        std::uint32_t PackedColor;
    };

    const VertexDeclaration& Declaration()
    {
        static const VertexDeclaration declaration{
            VertexElement(static_cast<int>(offsetof(VertexPositionNormalColor, Position)), VertexElementFormat::Vector3,
                          VertexElementUsage::Position, 0),
            VertexElement(static_cast<int>(offsetof(VertexPositionNormalColor, Normal)), VertexElementFormat::Vector3,
                          VertexElementUsage::Normal, 0),
            VertexElement(static_cast<int>(offsetof(VertexPositionNormalColor, PackedColor)), VertexElementFormat::Color,
                          VertexElementUsage::Color, 0),
        };
        return declaration;
    }

    class VertexColorLightingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
            // Both at once: the vertex colour is the material's diffuse colour, and the lights shade it.
            effect_->setVertexColorEnabledProperty(true);
            effect_->setLightingEnabledProperty(true);
            effect_->setAmbientLightColorProperty(Vector3(0.15f, 0.15f, 0.15f));
            effect_->getDirectionalLight0Property().setEnabledProperty(true);
            effect_->getDirectionalLight0Property().setDirectionProperty(Vector3::Normalize(Vector3(1.0f, -0.3f, -0.5f)));
            effect_->getDirectionalLight0Property().setDiffuseColorProperty(Vector3(1.0f, 1.0f, 1.0f));
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 4.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));

            // A sphere coloured by height: orange at the top, teal at the bottom.
            const CnaDemos::Mesh sphere = CnaDemos::MakeSphere(1.2f);
            for (const auto& v : sphere.vertices)
            {
                const float t = (v.Normal.Y + 1.0f) * 0.5f;
                const Color color(1.0f * t + 0.1f * (1.0f - t), 0.55f * t + 0.75f * (1.0f - t), 0.1f * t + 0.7f * (1.0f - t));
                vertices_.push_back({v.Position, v.Normal, color.getPackedValueProperty()});
            }
            indices_ = sphere.indices;
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(20, 20, 28));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, vertices_.data(), 0,
                                                 static_cast<int>(vertices_.size()), indices_.data(), 0,
                                                 static_cast<int>(indices_.size() / 3), Declaration());
            }

            if (FramesDrawn() == 0)
            {
                // The light comes from the upper left, towards the screen: surfaces facing left are
                // lit, those facing right get the ambient light only. The sphere (radius ~174 px
                // around the centre) is orange at the top and teal at the bottom.
                const CnaDemos::BackBufferPixels pixels(device);
                const Color litTop = pixels.At(296, 153);     // normal (-0.6, 0.5, 0.6)
                const Color litBottom = pixels.At(296, 327);  // normal (-0.6, -0.5, 0.6)
                const Color shadedTop = pixels.At(504, 153);  // normal (0.6, 0.5, 0.6)
                Context().Print("lit top {}, lit bottom {}, shaded top {}", litTop.ToString(), litBottom.ToString(),
                                shadedTop.ToString());
                Context().Check(litTop.getRProperty() > litTop.getBProperty() + 60 &&
                                    litBottom.getGProperty() > litBottom.getRProperty() + 40 &&
                                    litBottom.getBProperty() > litBottom.getRProperty(),
                                "the vertex colours survive lighting: orange top, teal bottom");
                Context().Check(litTop.getRProperty() > shadedTop.getRProperty() + 60,
                                "the lights shade them: the side facing the light is brighter");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::vector<VertexPositionNormalColor> vertices_;
        std::vector<std::uint16_t> indices_;
    };
}

CNA_DEMOS_GAME(VertexColorLightingDemo, {
    .id = "graphics.basic_effect.vertex_color_lighting",
    .title = "Vertex colours with lighting",
    .description = "Draws a sphere whose vertices carry a colour (a custom position-normal-colour vertex) with "
                   "BasicEffect's VertexColorEnabled and LightingEnabled both on: the vertex colour acts as the "
                   "diffuse material and one directional light plus ambient shade it.",
    .apis = {"BasicEffect::VertexColorEnabled", "BasicEffect::LightingEnabled", "BasicEffect::AmbientLightColor",
             "BasicEffect::DirectionalLight0", "DirectionalLight::Direction", "DirectionalLight::DiffuseColor",
             "GraphicsDevice::DrawUserIndexedPrimitives", "VertexElementUsage::Normal", "DirectionalLight::Enabled",
             "BasicEffect::View", "BasicEffect::Projection"},
    .tags = {"3d", "lighting", "vertex-color", "effect"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
