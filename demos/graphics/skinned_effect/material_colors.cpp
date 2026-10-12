// SPDX-License-Identifier: MS-PL
// Emissive and ambient material terms are textured, then global alpha supplies premultiplied blending.
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
    class MaterialColorsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            std::vector<VertexPositionNormalTextureSkinned> vertices;
            for (const Vector2& position : {Vector2(-.18f, -.36f), Vector2(.18f, -.36f),
                                           Vector2(.18f, .36f), Vector2(-.18f, .36f)})
                vertices.emplace_back(Vector3(position.X, position.Y, 0),
                    Vector3(0, 0, 1),
                    Vector2::Zero, Vector4(1, 0, 0, 0), std::array<std::uint8_t, 4>{0, 0, 0, 0});
            vertices_ = std::make_unique<VertexBuffer>(device,
                VertexPositionNormalTextureSkinned::getVertexDeclarationStatic(), 4, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), 4);
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
            const std::array<std::uint16_t, 6> triangles{0, 1, 2, 0, 2, 3};
            indices_->SetData(triangles.data(), 6);
            texture_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color texel(128, 255, 64);
            texture_->SetData(&texel, 1);
            effect_ = std::make_unique<SkinnedEffect>(device);
            effect_->setTextureProperty(texture_.get());
            effect_->setWeightsPerVertexProperty(1);
            effect_->setViewProperty(Matrix::getIdentityProperty());
            effect_->setProjectionProperty(Matrix::CreateOrthographic(2, 1.2f, .1f, 10));
            effect_->setSpecularColorProperty(Vector3::Zero);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
            effect_->setDiffuseColorProperty(Vector3(.8f, .4f, .2f));
            effect_->SetBoneTransforms({Matrix::CreateTranslation(0, 0, -3)});
            Context().Print("Left to right: emissive only; ambient times diffuse only; their sum; the same sum with Alpha=.5 over MidnightBlue. All use texel (128,255,64), no directional/specular light, and a bone translating the bind mesh into view. AlphaBlend expects premultiplied shader output.");
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
            for (int panel = 0; panel < 4; ++panel)
            {
                effect_->setWorldProperty(Matrix::CreateTranslation(-.75f + .5f * panel, 0, 0));
                effect_->setAmbientLightColorProperty(panel == 0 ? Vector3::Zero : Vector3(.5f, .25f, .75f));
                effect_->setEmissiveColorProperty(panel == 1 ? Vector3::Zero : Vector3(.25f, .5f, .75f));
                effect_->setAlphaProperty(panel == 3 ? .5f : 1.0f);
                device.setBlendStateProperty(panel == 3 ? BlendState::AlphaBlend : BlendState::Opaque);
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                }
            }
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                // RGB = texture * (emissive + ambient * diffuse), before global Alpha.
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 8, pixels.Height() / 2), Color(32, 128, 48), 1),
                                "emissive color remains visible without ambient or directional light and is textured");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 3 / 8, pixels.Height() / 2), Color(51, 26, 10), 1),
                                "ambient light multiplies diffuse material and the non-white texture");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 5 / 8, pixels.Height() / 2), Color(83, 153, 58), 1),
                                "emissive and ambient-diffuse terms add before texture modulation");
                // Alpha=.5: .5*(83.2,153,57.6) + .5*(25,25,112) = (54.1,89,84.8).
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 7 / 8, pixels.Height() / 2), Color(54, 89, 85), 1),
                                "material Alpha supplies premultiplied color for AlphaBlend over the background");
            }
            Game::Draw(time);
        }
    private:
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<Texture2D> texture_;
        std::unique_ptr<SkinnedEffect> effect_;
    };
}
CNA_DEMOS_GAME(MaterialColorsDemo, {
    .id = "graphics.skinned_effect.material_colors",
    .title = "Emissive, ambient and translucent skinned materials",
    .description = "Renders four skinned material controls: textured emission, ambient-diffuse response, their sum "
                   "and half-alpha premultiplied composition over a known background.",
    .apis = {"SkinnedEffect::EmissiveColor", "SkinnedEffect::AmbientLightColor", "SkinnedEffect::DiffuseColor",
             "SkinnedEffect::Alpha", "SkinnedEffect::Texture", "SkinnedEffect::SetBoneTransforms", "BlendState::AlphaBlend"},
    .tags = {"skinning", "bones", "effect", "material", "alpha", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "Opaque texture texels and no vertex colors or specular lights. This scene isolates material "
                    "Alpha; texture/vertex-alpha interactions and specular power remain separate behaviors.",
})
