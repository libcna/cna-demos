// SPDX-License-Identifier: MS-PL
// The Reach and HiDef graphics profiles.
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class GraphicsProfileDemo final : public CnaDemos::DemoGame
    {
    public:
        GraphicsProfileDemo()
        {
            // Reach is the portable subset (phone-class hardware); HiDef unlocks the full API.
            // cna-demos runs everything else under HiDef; this demo opts into Reach.
            Graphics().setGraphicsProfileProperty(GraphicsProfile::Reach);
        }

    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            Context().Print("GraphicsDeviceManager profile: {}, device profile: {}",
                            Graphics().getGraphicsProfileProperty() == GraphicsProfile::Reach ? "Reach" : "HiDef",
                            device.getGraphicsProfileProperty() == GraphicsProfile::Reach ? "Reach" : "HiDef");
            Context().Check(device.getGraphicsProfileProperty() == GraphicsProfile::Reach, "the device runs under Reach");

            // A HiDef-only operation is refused under Reach: reading the back buffer back.
            std::vector<Color> pixels(16);
            bool refused = false;
            try
            {
                device.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
            }
            catch (const std::exception& e)
            {
                refused = true;
                Context().Print("GetBackBufferData under Reach: {}", e.what());
            }
            Context().Check(refused, "GetBackBufferData is not available in the Reach profile");
        }
    };
}

CNA_DEMOS_GAME(GraphicsProfileDemo, {
    .id = "game.graphics_device_manager.graphics_profile",
    .title = "Reach and HiDef profiles",
    .description = "Requests the Reach profile through GraphicsDeviceManager::GraphicsProfile and shows a HiDef-only "
                   "call (GetBackBufferData) being refused.",
    .apis = {"GraphicsDeviceManager::GraphicsProfile", "GraphicsDevice::GraphicsProfile", "GraphicsProfile::Reach",
             "GraphicsProfile::HiDef", "GraphicsDevice::GetBackBufferData"},
    .tags = {"profile", "reach", "hidef", "device"},
    .frames = 3,
})
