// SPDX-License-Identifier: MS-PL
// Model parts carry sampler metadata; the application chooses device state before drawing.
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
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureAddressMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionTexture.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class PartSamplerDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            texture_ = std::make_unique<Texture2D>(device, 2, 1);
            const std::array<Color, 2> texels{Color::Red, Color::Lime};
            texture_->SetData(texels.data(), static_cast<int>(texels.size()));
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setViewProperty(Matrix::getIdentityProperty());
            effect_->setProjectionProperty(Matrix::getIdentityProperty());
            effect_->setLightingEnabledProperty(false);
            effect_->setTextureEnabledProperty(true);
            effect_->setTextureProperty(texture_.get());
            // Constant out-of-range U: Wrap reaches the first (red) texel; Clamp the last (green).
            const Vector2 uv(1.125f, .5f);
            const std::array<VertexPositionTexture, 4> vertices = {
                VertexPositionTexture(Vector3(-.3f, -.5f, .5f), uv),
                VertexPositionTexture(Vector3( .3f, -.5f, .5f), uv),
                VertexPositionTexture(Vector3( .3f,  .5f, .5f), uv),
                VertexPositionTexture(Vector3(-.3f,  .5f, .5f), uv),
            };
            const std::array<std::uint16_t, 6> indices{0, 1, 2, 0, 2, 3};
            vertices_ = std::make_unique<VertexBuffer>(device, VertexPositionTexture::getVertexDeclarationStatic(),
                                                       4, BufferUsage::WriteOnly);
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), static_cast<int>(vertices.size()));
            indices_->SetData(indices.data(), static_cast<int>(indices.size()));
            part_ = std::make_unique<ModelMeshPart>(vertices_.get(), indices_.get(), 4, 2, 0, 0);
            mesh_ = std::make_unique<ModelMesh>(&device, "Sampler metadata", std::vector<ModelMeshPart*>{part_.get()});
            part_->setEffectProperty(effect_.get());
            part_->setSamplerStateEXTProperty(0, SamplerState::PointClamp);
            const auto& sampler = part_->getSamplerStatesEXTProperty()[0];
            Context().Check(sampler.getAddressUProperty() == TextureAddressMode::Clamp &&
                            sampler.getFilterProperty() == TextureFilter::Point,
                            "the part carries point-clamp metadata for its base-color texture");
            Context().Print("Left: application PointWrap. Right: explicitly selected part PointClamp.");
        }

        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.getSamplerStatesProperty()(0, SamplerState::PointWrap);
            effect_->setWorldProperty(Matrix::CreateTranslation(-.5f, 0, 0));
            mesh_->Draw(); // intentionally preserves the application's sampler, despite part metadata
            if (!checked_)
                Context().Check(device.getSamplerStatesProperty()[0].getAddressUProperty() == TextureAddressMode::Wrap,
                                "drawing a part preserves the application's device sampler");

            // Explicit selection is the public CNAEXT boundary used for imported part samplers too.
            // This procedural fixture demonstrates selection, not glTF importer correctness.
            device.getSamplerStatesProperty()(0, part_->getSamplerStatesEXTProperty()[0]);
            effect_->setWorldProperty(Matrix::CreateTranslation(.5f, 0, 0));
            mesh_->Draw();
            if (!checked_)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() / 4, pixels.Height() / 2), Color::Red, 2),
                                "application PointWrap samples the red texel on the left");
                Context().Check(CnaDemos::Near(pixels.At(3 * pixels.Width() / 4, pixels.Height() / 2), Color::Lime, 2),
                                "explicitly selecting the part's PointClamp samples green on the right");
                checked_ = true;
            }
            Game::Draw(time);
        }

    private:
        // The mesh/part/effect must be destroyed before the buffers and texture they reference.
        std::unique_ptr<Texture2D> texture_;
        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<ModelMeshPart> part_;
        std::unique_ptr<ModelMesh> mesh_;
        bool checked_ = false;
    };
}

CNA_DEMOS_GAME(PartSamplerDemo, {
    .id = "graphics.model.part_sampler_state",
    .title = "Select a model part's sampler explicitly",
    .description = "Stores clamp metadata on a model part, preserves the application's wrap sampler during Draw, "
                   "then explicitly selects the part sampler and checks both texture samples with pixels.",
    .apis = {"Microsoft::Xna::Framework::Graphics::ModelMeshPart::SamplerStatesEXT",
             "Microsoft::Xna::Framework::Graphics::ModelMeshPart::setSamplerStateEXTProperty",
             "ModelMesh::Draw", "GraphicsDevice::SamplerStates"},
    .tags = {"model", "sampler", "texture", "cnaext", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
