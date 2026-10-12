// SPDX-License-Identifier: MS-PL
// Fog follows the post-skin view depth, even when every bind-pose vertex has the same Z.
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
    class FogDepthDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            std::vector<VertexPositionNormalTextureSkinned> vertices;
            for (const Vector2& position : {Vector2(-.17f, -.36f), Vector2(.17f, -.36f),
                                           Vector2(.17f, .36f), Vector2(-.17f, .36f)})
                vertices.emplace_back(Vector3(position.X, position.Y, 0), Vector3::UnitZ,
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
            effect_->setAmbientLightColorProperty(Vector3::One);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
            effect_->setDiffuseColorProperty(Color::Gold.ToVector3());
            effect_->setFogColorProperty(Color::CornflowerBlue.ToVector3());
            effect_->setFogStartProperty(2);
            effect_->setFogEndProperty(6);
            Context().Print("The same Z=0 bind mesh is moved by its bone to view depths 2,4,6. World only separates the cards sideways; fog should follow the skinned depth.");
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::MidnightBlue); // deliberately different from the fully fogged surface
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            if (FramesDrawn() == 0)
            {
                effect_->setFogEnabledProperty(false);
                DrawCards();
                const CnaDemos::BackBufferPixels plain(device);
                Context().Check(CnaDemos::Near(plain.At(plain.Width() / 5, plain.Height() / 2), Color::Gold, 1) &&
                                CnaDemos::Near(plain.At(plain.Width() / 2, plain.Height() / 2), Color::Gold, 1) &&
                                CnaDemos::Near(plain.At(plain.Width() * 4 / 5, plain.Height() / 2), Color::Gold, 1),
                                "with fog disabled the three skinned depths retain the same material color");
            }
            effect_->setFogEnabledProperty(true);
            DrawCards();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels fogged(device);
                Context().Check(CnaDemos::Near(fogged.At(fogged.Width() / 5, fogged.Height() / 2), Color::Gold, 1),
                                "the bone placing the mesh at FogStart leaves its material unfogged");
                Context().Check(CnaDemos::Near(fogged.At(fogged.Width() / 2, fogged.Height() / 2), Color(178, 182, 119), 1),
                                "the middle bone depth blends material and fog color equally");
                Context().Check(CnaDemos::Near(fogged.At(fogged.Width() * 4 / 5, fogged.Height() / 2), Color::CornflowerBlue, 1),
                                "the bone placing the mesh at FogEnd produces the fog color rather than bind-pose fog");
            }
            Game::Draw(time);
        }
    private:
        void DrawCards()
        {
            auto& device = getGraphicsDeviceProperty();
            for (int card = 0; card < 3; ++card)
            {
                effect_->setWorldProperty(Matrix::CreateTranslation(.6f * (card - 1), 0, 0));
                // All depth comes from the palette, not World or the shared bind vertices.
                effect_->SetBoneTransforms({Matrix::CreateTranslation(0, 0, -2.0f * (card + 1))});
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                }
            }
        }
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<Texture2D> white_;
        std::unique_ptr<SkinnedEffect> effect_;
    };
}
CNA_DEMOS_GAME(FogDepthDemo, {
    .id = "graphics.skinned_effect.fog_depth",
    .title = "Fog follows the skinned depth",
    .description = "Moves one bind mesh to three depths using bone transforms, checks the fog-disabled material "
                   "control, then verifies FogStart, the half-fog depth and FogEnd from actual rendered colors.",
    .apis = {"SkinnedEffect::FogEnabled", "SkinnedEffect::FogColor", "SkinnedEffect::FogStart", "SkinnedEffect::FogEnd",
             "SkinnedEffect::SetBoneTransforms", "SkinnedEffect::World", "SkinnedEffect::View", "SkinnedEffect::Projection"},
    .tags = {"skinning", "bones", "effect", "fog", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
