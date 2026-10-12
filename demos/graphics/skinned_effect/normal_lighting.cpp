// SPDX-License-Identifier: MS-PL
// Bone rotations transform the lighting normal; a weighted normal must be normalized.
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "System/NotSupportedException.hpp"
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
    class NormalLightingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            std::vector<VertexPositionNormalTextureSkinned> vertices;
            for (int card = 0; card < 4; ++card)
                for (const Vector2& position : {Vector2(-.18f, -.36f), Vector2(.18f, -.36f),
                                               Vector2(.18f, .36f), Vector2(-.18f, .36f)})
                    vertices.emplace_back(Vector3(position.X, position.Y, 0), Vector3::UnitZ,
                        Vector2::Zero, card >= 2 ? Vector4(.5f, .5f, 0, 0) : Vector4(1, 0, 0, 0),
                        std::array<std::uint8_t, 4>{0, 1, 0, 0});
            vertices_ = std::make_unique<VertexBuffer>(device,
                VertexPositionNormalTextureSkinned::getVertexDeclarationStatic(), 16, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), 16);
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
            const std::array<std::uint16_t, 6> triangles{0, 1, 2, 0, 2, 3};
            indices_->SetData(triangles.data(), 6);
            white_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);
            effect_ = std::make_unique<SkinnedEffect>(device);
            // This effect always lights its mesh; ambient-only shading still uses that pipeline.
            bool refusedUnlit = false;
            try { effect_->setLightingEnabledProperty(false); }
            catch (const System::NotSupportedException&) { refusedUnlit = true; }
            Context().Check(refusedUnlit && effect_->getLightingEnabledProperty(),
                            "disabling SkinnedEffect lighting is refused and leaves lighting enabled");
            effect_->setTextureProperty(white_.get());
            effect_->setWeightsPerVertexProperty(1);
            effect_->setViewProperty(Matrix::getIdentityProperty());
            effect_->setProjectionProperty(Matrix::CreateOrthographic(2, 1.2f, .1f, 10));
            effect_->setAmbientLightColorProperty(Vector3::Zero);
            effect_->setSpecularColorProperty(Vector3::Zero);
            effect_->getDirectionalLight0Property().setEnabledProperty(true);
            effect_->getDirectionalLight0Property().setDirectionProperty(Vector3(-.6f, 0, -.8f));
            effect_->getDirectionalLight0Property().setDiffuseColorProperty(Vector3::One);
            effect_->getDirectionalLight0Property().setSpecularColorProperty(Vector3::Zero);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
            effect_->setDiffuseColorProperty(Color::Gold.ToVector3());
            Context().Print("Left to right: bind normal +Z; one 60-degree Y rotation; equal identity/120-degree influences; the same blend with nonuniform World scale. Normalization and the World inverse transpose determine the diffuse colors.");
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::MidnightBlue); // different from every expected lit surface
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            for (int card = 0; card < 4; ++card)
            {
                // The fourth card adds nonuniform World scale; lighting needs its inverse transpose.
                const Matrix scale = Matrix::CreateScale(card == 3 ? 2.0f : 1.0f, 1, 1);
                effect_->setWorldProperty(scale * Matrix::CreateTranslation(.5f * card - .75f, 0, -3));
                effect_->setWeightsPerVertexProperty(card >= 2 ? 2 : 1);
                const float pi = std::acos(-1.0f);
                if (card == 0) effect_->SetBoneTransforms({Matrix::getIdentityProperty()});
                else if (card == 1) effect_->SetBoneTransforms({Matrix::CreateRotationY(pi / 3)});
                else effect_->SetBoneTransforms({Matrix::getIdentityProperty(), Matrix::CreateRotationY(2 * pi / 3)});
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 4 * card, 0, 4, 0, 2);
                }
            }
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                // Lambert factors: .8 for +Z; .6*sqrt(3)/2 + .8/2 for the two 60-degree normals.
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 8, pixels.Height() / 2), Color(204, 172, 0), 1),
                                "the bind normal receives the expected directional diffuse light");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 3 / 8, pixels.Height() / 2), Color(235, 198, 0), 1),
                                "rotating the bone rotates the surface lighting normal");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 5 / 8, pixels.Height() / 2), Color(235, 198, 0), 1),
                                "two weighted rotations produce a normalized blended lighting normal");
                // Inverse-transpose World scale changes (.866,0,.5) to normalize(.433,0,.5).
                // Its Lambert factor is .6*sqrt(3/7) + .8*2/sqrt(7), approximately .99754.
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 7 / 8, pixels.Height() / 2), Color(254, 214, 0), 1),
                                "nonuniform World scale uses an inverse-transpose lighting normal after skinning");
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
CNA_DEMOS_GAME(NormalLightingDemo, {
    .id = "graphics.skinned_effect.normal_lighting",
    .title = "Lighting follows the skinned normal",
    .description = "Compares an unchanged normal, a bone-rotated normal and a shorter two-bone blend, "
                   "checking directional diffuse colors after skinning, normalization and nonuniform World scale.",
    .apis = {"SkinnedEffect::DirectionalLight0", "SkinnedEffect::DiffuseColor", "SkinnedEffect::LightingEnabled",
             "SkinnedEffect::World", "SkinnedEffect::SetBoneTransforms", "SkinnedEffect::WeightsPerVertex"},
    .tags = {"skinning", "bones", "effect", "lighting", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
