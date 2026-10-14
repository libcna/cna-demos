// SPDX-License-Identifier: MS-PL
// State objects become read-only once bound to the device: create one per configuration up front.
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/Blend.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/CullMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureAddressMode.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class BoundStateDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();

            // Before it is used, a state object is freely configurable...
            BlendState additive;
            additive.setColorSourceBlendProperty(Blend::One);
            additive.setColorDestinationBlendProperty(Blend::One);

            // ...once set on the device it is frozen, for as long as it exists.
            device.setBlendStateProperty(additive);
            Expect("BlendState", [&] { additive.setColorDestinationBlendProperty(Blend::Zero); });

            RasterizerState rasterizer;
            device.setRasterizerStateProperty(rasterizer);
            Expect("RasterizerState", [&] { rasterizer.setCullModeProperty(CullMode::None); });

            DepthStencilState depth;
            device.setDepthStencilStateProperty(depth);
            Expect("DepthStencilState", [&] { depth.setDepthBufferWriteEnableProperty(false); });

            SamplerState sampler;
            device.getSamplerStatesProperty()(0, sampler);
            Expect("SamplerState", [&] { sampler.setAddressUProperty(TextureAddressMode::Clamp); });

            // A different configuration is a different object.
            BlendState replaced;
            replaced.setColorSourceBlendProperty(Blend::One);
            replaced.setColorDestinationBlendProperty(Blend::Zero);
            device.setBlendStateProperty(replaced);
            Context().Check(device.getBlendStateProperty().getColorDestinationBlendProperty() == Blend::Zero,
                            "setting another state object changes the device state");

            // Every state type is a GraphicsResource. Dispose marks it immediately and a later
            // attempt to bind that state is rejected before the live device state changes.
            BlendState disposedBlend;
            DepthStencilState disposedDepth;
            RasterizerState disposedRasterizer;
            SamplerState disposedSampler;
            disposedBlend.Dispose();
            disposedDepth.Dispose();
            disposedRasterizer.Dispose();
            disposedSampler.Dispose();
            Context().Check(disposedBlend.getIsDisposedProperty() && disposedDepth.getIsDisposedProperty() &&
                                disposedRasterizer.getIsDisposedProperty() && disposedSampler.getIsDisposedProperty(),
                            "Dispose marks all four graphics-state resource types disposed");
            ExpectDisposed("BlendState", [&] { device.setBlendStateProperty(disposedBlend); });
            ExpectDisposed("DepthStencilState", [&] { device.setDepthStencilStateProperty(disposedDepth); });
            ExpectDisposed("RasterizerState", [&] { device.setRasterizerStateProperty(disposedRasterizer); });
            ExpectDisposed("SamplerState", [&] { device.getSamplerStatesProperty()(1, disposedSampler); });
        }

    private:
        template <typename Action>
        void Expect(const std::string& what, Action action)
        {
            try
            {
                action();
                Context().Check(false, "changing a bound " + what + " throws InvalidOperationException");
            }
            catch (const System::InvalidOperationException& e)
            {
                Context().Print("{:<18} {}", what + ":", e.what());
                Context().Check(true, "changing a bound " + what + " throws InvalidOperationException");
            }
        }

        template <typename Action>
        void ExpectDisposed(const std::string& what, Action action)
        {
            try
            {
                action();
                Context().Check(false, "binding a disposed " + what + " throws ObjectDisposedException");
            }
            catch (const System::ObjectDisposedException&)
            {
                Context().Check(true, "binding a disposed " + what + " throws ObjectDisposedException");
            }
        }
    };
}

CNA_DEMOS_GAME(BoundStateDemo, {
    .id = "graphics.blend_state.bound_state_is_immutable",
    .title = "Bound state objects are read-only",
    .description = "Binds all four graphics-state resource types and shows that changing one afterwards throws; "
                   "disposing independent states marks them disposed and prevents later binding.",
    .apis = {"GraphicsDevice::BlendState", "GraphicsDevice::RasterizerState", "GraphicsDevice::DepthStencilState",
             "GraphicsDevice::SamplerStates", "BlendState::BlendState", "RasterizerState::RasterizerState",
             "DepthStencilState::DepthStencilState", "BlendState::Dispose", "DepthStencilState::Dispose",
             "RasterizerState::Dispose", "SamplerState::Dispose"},
    .tags = {"state", "immutable", "errors"},
    .frames = 1,
})
