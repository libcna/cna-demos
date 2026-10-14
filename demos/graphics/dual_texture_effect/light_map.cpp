// SPDX-License-Identifier: MS-PL
// DualTextureEffect: a base texture modulated by a second one (a light map), each with its own UVs.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DualTextureEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerStateCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // Two texture coordinates per vertex: usage index 0 for the base texture, 1 for the light map.
    struct DualTextureVertex
    {
        Vector3 Position;
        std::uint32_t PackedTint;
        Vector2 BaseUv;
        Vector2 LightUv;
    };
    static_assert(std::is_trivially_copyable_v<DualTextureVertex> && sizeof(DualTextureVertex) == 32);

    const VertexDeclaration& DualTextureDeclaration()
    {
        static const VertexDeclaration declaration{
            VertexElement(0, VertexElementFormat::Vector3, VertexElementUsage::Position, 0),
            VertexElement(12, VertexElementFormat::Color, VertexElementUsage::Color, 0),
            VertexElement(16, VertexElementFormat::Vector2, VertexElementUsage::TextureCoordinate, 0),
            VertexElement(24, VertexElementFormat::Vector2, VertexElementUsage::TextureCoordinate, 1),
        };
        return declaration;
    }

    class LightMapDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            base_ = getContentProperty().Load<Texture2D>("textures/checker");

            // A light map: a soft bright spot in grey. DualTextureEffect computes base * light * 2,
            // so 128 (mid grey) leaves the base unchanged, darker darkens, brighter brightens.
            constexpr int size = 64;
            std::vector<Color> light(static_cast<std::size_t>(size) * size);
            for (int y = 0; y < size; ++y)
                for (int x = 0; x < size; ++x)
                {
                    const float dx = (x - 20.0f) / size, dy = (y - 24.0f) / size;
                    const float bright = std::max(0.0f, 1.0f - 4.0f * (dx * dx + dy * dy));
                    const auto v = static_cast<std::uint8_t>(40 + 215 * bright);
                    light[static_cast<std::size_t>(y) * size + x] = Color(v, v, v);
                }
            lightMap_ = std::make_unique<Texture2D>(device, size, size);
            lightMap_->SetData(light.data(), static_cast<int>(light.size()));

            effect_ = std::make_unique<DualTextureEffect>(device);
            effect_->setTextureProperty(&base_);
            effect_->setTexture2Property(lightMap_.get());
            const Matrix world = Matrix::CreateTranslation(5.0f, 0.0f, 0.0f);
            const Matrix view = Matrix::CreateTranslation(-5.0f, 0.0f, 0.0f);
            effect_->setWorldProperty(world);
            effect_->setViewProperty(view);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 10.0f));
            effect_->setDiffuseColorProperty(Vector3(1.0f, 0.5f, 1.0f));
            effect_->setAlphaProperty(0.8f);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setFogEnabledProperty(true);
            effect_->setFogColorProperty(Vector3::Zero);
            effect_->setFogStartProperty(0.0f);
            effect_->setFogEndProperty(2.0f);

            std::unique_ptr<Effect> cloneOwner(effect_->Clone());
            auto* clone = dynamic_cast<DualTextureEffect*>(cloneOwner.get());
            if (clone == nullptr)
                throw std::runtime_error("DualTextureEffect::Clone returned the wrong runtime type");
            Context().Check(clone->getTextureProperty() == &base_ && clone->getTexture2Property() == lightMap_.get() &&
                                clone->getWorldProperty() == world && clone->getViewProperty() == view &&
                                clone->getDiffuseColorProperty() == Vector3(1.0f, 0.5f, 1.0f) &&
                                clone->getAlphaProperty() == 0.8f && clone->getVertexColorEnabledProperty() &&
                                clone->getFogEnabledProperty() && clone->getFogColorProperty() == Vector3::Zero &&
                                clone->getFogStartProperty() == 0.0f && clone->getFogEndProperty() == 2.0f,
                            "Clone retains both textures, matrices, material, vertex color and fog state");
            renderedEffect_.reset(static_cast<DualTextureEffect*>(cloneOwner.release()));
            effect_->setAlphaProperty(0.2f);
            effect_->setVertexColorEnabledProperty(false);
            effect_->setFogEnabledProperty(false);
            Context().Check(renderedEffect_->getAlphaProperty() == 0.8f &&
                                renderedEffect_->getVertexColorEnabledProperty() &&
                                renderedEffect_->getFogEnabledProperty(),
                            "clone material and shader-variant state are independent from the source");

            // The base repeats 4 times across the quad, the light map is stretched once over it.
            const std::uint32_t cyanTint = Color(128, 255, 255, 255).getPackedValueProperty();
            quad_ = {DualTextureVertex{Vector3(160, 40, -1), cyanTint, Vector2(0, 0), Vector2(0, 0)},
                     DualTextureVertex{Vector3(640, 40, -1), cyanTint, Vector2(4, 0), Vector2(1, 0)},
                     DualTextureVertex{Vector3(160, 440, -1), cyanTint, Vector2(0, 4), Vector2(0, 1)},
                     DualTextureVertex{Vector3(640, 40, -1), cyanTint, Vector2(4, 0), Vector2(1, 0)},
                     DualTextureVertex{Vector3(640, 440, -1), cyanTint, Vector2(4, 4), Vector2(1, 1)},
                     DualTextureVertex{Vector3(160, 440, -1), cyanTint, Vector2(0, 4), Vector2(0, 1)}};
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            for (EffectPass& pass : renderedEffect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                // Sampler 0 for the base texture (repeating), sampler 1 for the light map.
                device.getSamplerStatesProperty()(0, SamplerState::PointWrap);
                device.getSamplerStatesProperty()(1, SamplerState::LinearClamp);
                device.DrawUserPrimitives(PrimitiveType::TriangleList, quad_, 0, 2, DualTextureDeclaration());
            }

            if (FramesDrawn() == 0)
            {
                // Two white checker cells: one under the bright spot, one in the dark corner.
                const CnaDemos::BackBufferPixels p(device);
                int lit = 0;
                Color brightest;
                for (int y = 40; y < 440; ++y)
                    for (int x = 160; x < 640; ++x)
                    {
                        const Color sample = p.At(x, y);
                        const int v = sample.getGProperty();
                        if (v > lit)
                        {
                            lit = v;
                            brightest = sample;
                        }
                    }
                const Color corner = p.At(620, 420);
                Context().Print("brightest pixel {}, far corner {}", brightest.ToString(), corner.ToString());
                Context().Check(lit >= 98 && lit <= 106 &&
                                    std::abs(brightest.getRProperty() - lit) <= 2 &&
                                    brightest.getBProperty() >= 200 && brightest.getBProperty() <= 208 &&
                                    brightest.getAProperty() >= 200 && brightest.getAProperty() <= 208,
                                "vertex tint, material diffuse/alpha and half fog produce the expected bright result");
                Context().Check(corner.getGProperty() < lit / 2,
                                "the dark light-map corner remains distinctly below the bright spot");
            }

            Game::Draw(gameTime);
        }

    private:
        Texture2D base_;
        std::unique_ptr<Texture2D> lightMap_;
        std::unique_ptr<DualTextureEffect> effect_;
        std::unique_ptr<DualTextureEffect> renderedEffect_;
        std::vector<DualTextureVertex> quad_;
    };
}

CNA_DEMOS_GAME(LightMapDemo, {
    .id = "graphics.dual_texture_effect.light_map",
    .title = "Light map with DualTextureEffect",
    .description = "Multiplies a repeating checker by a stretched light map with DualTextureEffect, using a vertex "
                   "with two sets of texture coordinates.",
    .apis = {"DualTextureEffect::DualTextureEffect", "DualTextureEffect::Texture", "DualTextureEffect::Texture2",
             "VertexElementUsage::TextureCoordinate", "GraphicsDevice::SamplerStates",
             "DualTextureEffect::Projection", "DualTextureEffect::World", "DualTextureEffect::View",
             "DualTextureEffect::DiffuseColor", "DualTextureEffect::Alpha",
             "DualTextureEffect::VertexColorEnabled", "DualTextureEffect::FogEnabled",
             "DualTextureEffect::FogColor", "DualTextureEffect::FogStart", "DualTextureEffect::FogEnd",
             "DualTextureEffect::Clone"},
    .tags = {"3d", "lightmap", "multitexture", "effect"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
