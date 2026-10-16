// SPDX-License-Identifier: MS-PL
// CNA extends SkinnedEffect with an optional COLOR channel, multiplied by lit material and texture.
#include <array>
#include <cstdint>
#include <cstddef>
#include <type_traits>
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
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
namespace
{
    // Custom vertex streams contain plain bytes. CNA Color is polymorphic (ISSUE-005), so
    // store its packed RGBA value, not a Color object or an IVertexType-derived structure.
    struct TintedVertex
    {
        std::array<float, 3> position, normal;
        std::array<float, 2> uv;
        std::array<float, 4> weights;
        std::array<std::uint8_t, 4> joints;
        std::uint32_t tint;
    };
    static_assert(std::is_standard_layout_v<TintedVertex> &&
                  std::is_trivially_copyable_v<TintedVertex> && sizeof(TintedVertex) == 56);
    const VertexDeclaration& Declaration()
    {
        static const VertexDeclaration declaration{
            VertexElement(static_cast<int>(offsetof(TintedVertex, position)), VertexElementFormat::Vector3, VertexElementUsage::Position, 0),
            VertexElement(static_cast<int>(offsetof(TintedVertex, normal)), VertexElementFormat::Vector3, VertexElementUsage::Normal, 0),
            VertexElement(static_cast<int>(offsetof(TintedVertex, uv)), VertexElementFormat::Vector2, VertexElementUsage::TextureCoordinate, 0),
            VertexElement(static_cast<int>(offsetof(TintedVertex, weights)), VertexElementFormat::Vector4, VertexElementUsage::BlendWeight, 0),
            VertexElement(static_cast<int>(offsetof(TintedVertex, joints)), VertexElementFormat::Byte4, VertexElementUsage::BlendIndices, 0),
            VertexElement(static_cast<int>(offsetof(TintedVertex, tint)), VertexElementFormat::Color, VertexElementUsage::Color, 0),
        };
        return declaration;
    }
    class VertexColorsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            std::vector<TintedVertex> vertices;
            for (const Vector2& position : {Vector2(-.22f, -.18f), Vector2(.22f, -.18f),
                                           Vector2(.22f, .18f), Vector2(-.22f, .18f)})
            {
                const Color tint = position.X < 0 ? Color(255, 64, 32) : Color(32, 128, 255);
                vertices.push_back({{position.X, position.Y, 0}, {0, 0, 1}, {0, 0},
                                    {1, 0, 0, 0}, {0, 0, 0, 0}, tint.getPackedValueProperty()});
            }
            for (const Vector2& position : {Vector2(-.22f, -.18f), Vector2(.22f, -.18f),
                                           Vector2(.22f, .18f), Vector2(-.22f, .18f)})
            {
                const Color tint(0, 0, 0, 128);
                vertices.push_back({{position.X, position.Y, 0}, {0, 0, 1}, {0, 0},
                                    {1, 0, 0, 0}, {0, 0, 0, 0}, tint.getPackedValueProperty()});
            }
            vertices_ = std::make_unique<VertexBuffer>(device, Declaration(), 8, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), 8);
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 12, BufferUsage::WriteOnly);
            const std::array<std::uint16_t, 12> triangles{0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
            indices_->SetData(triangles.data(), 12);
            texture_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color texel(128, 255, 128);
            texture_->SetData(&texel, 1);
            halfAlphaTexture_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color halfAlphaTexel(0, 0, 0, 128);
            halfAlphaTexture_->SetData(&halfAlphaTexel, 1);
            effect_ = std::make_unique<SkinnedEffect>(device);
            effect_->setTextureProperty(texture_.get());
            effect_->setWeightsPerVertexProperty(1);
            effect_->setViewProperty(Matrix::getIdentityProperty());
            effect_->setProjectionProperty(Matrix::CreateOrthographic(2, 1.2f, .1f, 10));
            effect_->setAmbientLightColorProperty(Vector3::One);
            effect_->setSpecularColorProperty(Vector3::Zero);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
            effect_->setDiffuseColorProperty(Vector3(.5f, 1, .75f));
            effect_->SetBoneTransforms({Matrix::CreateTranslation(0, 0, -3)});
            Context().Print("The top pair contrasts disabled and enabled red-to-blue vertex RGB. The lower row isolates half vertex alpha, then multiplies it by half material alpha and half texture alpha. One bone moves every bind mesh into view.");
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
            effect_->setAlphaProperty(1.0f);
            effect_->setTextureProperty(texture_.get());
            for (int panel = 0; panel < 2; ++panel)
            {
                effect_->setWorldProperty(Matrix::CreateTranslation(panel == 0 ? -.5f : .5f, .25f, 0));
                effect_->VertexColorEnabled = panel == 1; // Actual public CNAEXT field, not an XNA property.
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                }
            }
            device.setBlendStateProperty(BlendState::AlphaBlend);
            effect_->VertexColorEnabled = true;
            for (int panel = 0; panel < 3; ++panel)
            {
                effect_->setAlphaProperty(panel == 0 ? 1.0f : .5f);
                effect_->setTextureProperty(panel == 2 ? halfAlphaTexture_.get() : texture_.get());
                effect_->setWorldProperty(Matrix::CreateTranslation((panel - 1) * .6f, -.25f, 0));
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 4, 4, 6, 2);
                }
            }
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const Color base(64, 255, 96); // texture (128,255,128) times material (.5,1,.75)
                const int topY = pixels.Height() * 7 / 24;
                const int bottomY = pixels.Height() * 17 / 24;
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 4, topY), base, 1) &&
                                CnaDemos::Near(pixels.At(pixels.Width() * 39 / 200, topY), base, 1) &&
                                CnaDemos::Near(pixels.At(pixels.Width() * 61 / 200, topY), base, 1),
                                "disabled vertex color preserves the uniform textured ambient material");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 3 / 4, topY), Color(36, 96, 54), 1),
                                "enabled skinned vertex color multiplies both material and texture at the center");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 139 / 200, topY), Color(50, 80, 33), 1) &&
                                CnaDemos::Near(pixels.At(pixels.Width() * 161 / 200, topY), Color(22, 112, 75), 1),
                                "the enabled COLOR channel interpolates distinct quarter-point tints");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 5, bottomY), Color(12, 12, 56), 1),
                                "half vertex alpha alone attenuates the opaque background by one half");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 2, bottomY), Color(19, 19, 84), 1),
                                "half material alpha multiplies half vertex alpha before blending");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 4 / 5, bottomY), Color(22, 22, 98), 1),
                                "half texture alpha also multiplies the material and vertex alpha");
            }
            Game::Draw(time);
        }
    private:
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<Texture2D> texture_;
        std::unique_ptr<Texture2D> halfAlphaTexture_;
        std::unique_ptr<SkinnedEffect> effect_;
    };
}
CNA_DEMOS_GAME(VertexColorsDemo, {
    .id = "graphics.skinned_effect.vertex_colors",
    .title = "Vertex tint on a skinned mesh",
    .description = "Uses CNA's SkinnedEffect vertex-color extension with a plain custom vertex declaration, "
                   "contrasting disabled tint with interpolated RGB and composed vertex/material/texture alpha.",
    .apis = {"SkinnedEffect::VertexColorEnabled", "SkinnedEffect::SetBoneTransforms", "SkinnedEffect::DiffuseColor",
             "SkinnedEffect::Texture", "VertexDeclaration::VertexDeclaration", "VertexElementUsage::Color",
             "VertexElementFormat::Byte4", "VertexElementFormat::Vector2", "VertexElementUsage::BlendIndices",
             "VertexElementUsage::BlendWeight"},
    .tags = {"skinning", "bones", "effect", "vertex", "color", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "CNAEXT VertexColorEnabled is not an XNA SkinnedEffect member. Custom vertices use packed color "
                    "bytes because CNA Color is polymorphic (ISSUE-005). Alpha checks use premultiplied black so only "
                    "the exact destination attenuation is observed.",
})
