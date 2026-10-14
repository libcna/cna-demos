// SPDX-License-Identifier: MS-PL
// Vertex-stage texture and sampler slots are separate from the pixel-stage collections.
#include <array>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerStateCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureAddressMode.hpp"
#include "System/NotSupportedException.hpp"

using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class VertexStageBindingsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            auto floating = std::make_unique<Texture2D>(device, 2, 2, false, SurfaceFormat::Single);
            const std::array<float, 4> values = {0.0f, 0.25f, 0.5f, 1.0f};
            floating->SetData(values.data(), static_cast<int>(values.size()));

            device.getVertexTexturesProperty()(1, floating.get());
            Context().Check(device.getVertexTexturesProperty()[1] == floating.get() &&
                                device.getTexturesProperty()[1] == nullptr,
                            "a supported floating texture binds only to the vertex-stage slot");

            device.getVertexSamplerStatesProperty()(1, SamplerState::PointClamp);
            const SamplerState& vertexSampler = device.getVertexSamplerStatesProperty()[1];
            const SamplerState& pixelSampler = device.getSamplerStatesProperty()[1];
            Context().Check(vertexSampler.getFilterProperty() == TextureFilter::Point &&
                                vertexSampler.getAddressUProperty() == TextureAddressMode::Clamp &&
                                vertexSampler.getAddressVProperty() == TextureAddressMode::Clamp &&
                                pixelSampler.getFilterProperty() == TextureFilter::Linear &&
                                pixelSampler.getAddressUProperty() == TextureAddressMode::Wrap,
                            "the vertex sampler is PointClamp without changing the pixel sampler");

            Texture2D color(device, 2, 2);
            bool colorRefused = false;
            try
            {
                device.getVertexTexturesProperty()(1, &color);
            }
            catch (const System::NotSupportedException&)
            {
                colorRefused = true;
            }
            Context().Check(colorRefused && device.getVertexTexturesProperty()[1] == floating.get(),
                            "a Color vertex texture is refused without replacing the floating binding");

            floating->Dispose();
            Context().Check(device.getVertexTexturesProperty()[1] == nullptr,
                            "disposing a bound vertex texture clears its device slot");
        }
    };
}

CNA_DEMOS_GAME(VertexStageBindingsDemo, {
    .id = "graphics.device.vertex_stage_bindings",
    .title = "Vertex-stage texture and sampler bindings",
    .description = "Binds a floating texture and PointClamp sampler to a vertex-stage slot, keeps pixel-stage state separate, refuses Color format and clears the slot on disposal.",
    .apis = {"GraphicsDevice::VertexTextures", "GraphicsDevice::VertexSamplerStates",
             "GraphicsDevice::Textures", "GraphicsDevice::SamplerStates",
             "TextureCollection::Item", "SamplerStateCollection::Item", "SurfaceFormat::Single",
             "GraphicsResource::Dispose"},
    .tags = {"device", "texture", "sampler", "vertex-stage", "state"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "Validates public binding state and format rules; no compiled vertex shader samples the texture.",
})
