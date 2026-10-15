// SPDX-License-Identifier: MS-PL
// The Reach and HiDef graphics profiles.
#include <vector>

#include "CNA/ProjectGraphicsProfile.hpp"
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
            // cna-demos declares HiDef before constructing any Game. GraphicsDeviceManager takes
            // that project-wide value as its initial profile, just as XNA reads the project file.
            const GraphicsProfile projectProfile = CNA::GetProjectGraphicsProfileEXT();
            Context().Check(projectProfile == GraphicsProfile::HiDef &&
                                Graphics().getGraphicsProfileProperty() == projectProfile,
                            "a new manager inherits the project's declared HiDef profile");

            // The declaration helper and direct setter both target future managers. An existing
            // manager keeps its own value until game code changes GraphicsProfile explicitly.
            const CNA::ProjectGraphicsProfileEXT reachProject{GraphicsProfile::Reach};
            Context().Check(CNA::GetProjectGraphicsProfileEXT() == GraphicsProfile::Reach &&
                                Graphics().getGraphicsProfileProperty() == projectProfile,
                            "a later project declaration does not rewrite an existing manager");
            CNA::SetProjectGraphicsProfileEXT(projectProfile);
            Context().Check(CNA::GetProjectGraphicsProfileEXT() == projectProfile,
                            "the process-wide project profile is restored before device creation");

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
    .description = "Shows how CNA's project-wide profile seeds a new GraphicsDeviceManager, restores that global "
                   "declaration, then requests Reach on the manager and observes a HiDef-only call being refused.",
    .apis = {"GraphicsDeviceManager::GraphicsProfile", "GraphicsDevice::GraphicsProfile", "GraphicsProfile::Reach",
             "GraphicsProfile::HiDef", "GraphicsDevice::GetBackBufferData", "CNA::ProjectGraphicsProfileEXT",
             "CNA::GetProjectGraphicsProfileEXT", "CNA::SetProjectGraphicsProfileEXT"},
    .tags = {"profile", "reach", "hidef", "device", "project", "cnaext"},
    .frames = 3,
})
