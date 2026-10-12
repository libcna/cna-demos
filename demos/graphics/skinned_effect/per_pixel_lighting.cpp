// SPDX-License-Identifier: MS-PL
// A coarse skinned mesh contrasts interpolated vertex light with normalized per-pixel normals.
#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTextureSkinned.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
namespace
{
    class LightingModesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            std::vector<VertexPositionNormalTextureSkinned> vertices;
            for (const Vector2& position : {Vector2(-.3f, -.36f), Vector2(.3f, -.36f),
                                           Vector2(.3f, .36f), Vector2(-.3f, .36f)})
                vertices.emplace_back(Vector3(position.X, position.Y, 0),
                    Vector3(position.X < 0 ? -.8f : .8f, 0, .6f),
                    Vector2::Zero, Vector4(1, 0, 0, 0), std::array<std::uint8_t, 4>{0, 0, 0, 0});
            vertices_ = std::make_unique<VertexBuffer>(device,
                VertexPositionNormalTextureSkinned::getVertexDeclarationStatic(), 4, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), 4);
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
            const std::array<std::uint16_t, 6> triangles{0, 1, 2, 0, 2, 3};
            indices_->SetData(triangles.data(), 6);
            white_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);
            effect_ = std::make_unique<SkinnedEffect>(device);
            effect_->setTextureProperty(white_.get());
            effect_->setWeightsPerVertexProperty(1);
            effect_->setViewProperty(Matrix::getIdentityProperty());
            effect_->setProjectionProperty(Matrix::CreateOrthographic(2, 1.2f, .1f, 10));
            effect_->setAmbientLightColorProperty(Vector3::Zero);
            effect_->setSpecularColorProperty(Vector3::Zero);
            effect_->getDirectionalLight0Property().setEnabledProperty(true);
            effect_->getDirectionalLight0Property().setDirectionProperty(Vector3(0, 0, -1));
            effect_->getDirectionalLight0Property().setDiffuseColorProperty(Vector3::One);
            effect_->getDirectionalLight0Property().setSpecularColorProperty(Vector3::Zero);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
            effect_->setDiffuseColorProperty(Vector3::One);
            effect_->SetBoneTransforms({Matrix::CreateTranslation(0, 0, -3)});
            Context().Print("The same coarse quad has smooth authored normals (-.8,0,.6) and (.8,0,.6). Left: light evaluated at vertices stays .6. Right: per-pixel normalized interpolation reaches +Z and full light in the center. One bone moves both meshes into view.");
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::MidnightBlue); // different from every expected lit sample
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            for (int panel = 0; panel < 2; ++panel)
            {
                effect_->setWorldProperty(Matrix::CreateTranslation(panel == 0 ? -.5f : .5f, 0, 0));
                effect_->setPreferPerPixelLightingProperty(panel == 1);
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                }
            }
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const Color vertexLight(153, 153, 153); // .6 * 255 at every vertex
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 4, pixels.Height() / 2), vertexLight, 1) &&
                                CnaDemos::Near(pixels.At(pixels.Width() * 7 / 40, pixels.Height() / 2), vertexLight, 1),
                                "per-vertex lighting interpolates equal endpoint intensities across the coarse mesh");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 3 / 4, pixels.Height() / 2), Color::White, 1),
                                "per-pixel lighting normalizes the center normal before evaluating full diffuse light");
                // Halfway from center to the right edge: normalize(.4,0,.6).Z * 255 ~= 212.
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 33 / 40, pixels.Height() / 2), Color(212, 212, 212), 1),
                                "per-pixel lighting preserves the intermediate directional-light gradient");
            }
            Game::Draw(time);
        }
    private:
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<Texture2D> white_;
        std::unique_ptr<SkinnedEffect> effect_;
    };
}
CNA_DEMOS_GAME(LightingModesDemo, {
    .id = "graphics.skinned_effect.per_pixel_lighting",
    .title = "Per-pixel light on a coarse skinned mesh",
    .description = "Renders the same skinned quad with authored smooth normals in both lighting modes, "
                   "contrasting interpolated vertex intensities with normalized per-pixel normals and diffuse colors.",
    .apis = {"SkinnedEffect::PreferPerPixelLighting", "SkinnedEffect::DirectionalLight0", "SkinnedEffect::SetBoneTransforms"},
    .tags = {"skinning", "bones", "effect", "lighting", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
