// SPDX-License-Identifier: MS-PL
// Adjusting the device before it is created: the PreparingDeviceSettings event.
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceInformation.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/PreparingDeviceSettingsEventArgs.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DeviceSelectionProbe final : public GraphicsDeviceManager
    {
    public:
        [[nodiscard]] bool CanReset(const GraphicsDeviceInformation& information)
        {
            return CanResetDevice(information);
        }

        [[nodiscard]] GraphicsDeviceInformation FindBest(bool anySuitable)
        {
            return FindBestDevice(anySuitable);
        }

        void Rank(std::vector<GraphicsDeviceInformation>& devices)
        {
            RankDevices(devices);
        }
    };

    class PreparingSettingsDemo final : public CnaDemos::DemoGame
    {
    public:
        PreparingSettingsDemo()
        {
            // Raised with the settings the manager is about to use; the handler may change them.
            // CNA event handlers receive the arguments as const; getGraphicsDeviceInformationEXT()
            // is CNA's mutable access to the GraphicsDeviceInformation (C# handlers simply write
            // through e.GraphicsDeviceInformation).
            Graphics().PreparingDeviceSettings += [this](System::Object*, const PreparingDeviceSettingsEventArgs& e) {
                ++events_;
                PresentationParameters& parameters = e.getGraphicsDeviceInformationEXT().getPresentationParametersProperty();
                parameters.setBackBufferWidthProperty(512);
                parameters.setBackBufferHeightProperty(256);
                parameters.setDepthStencilFormatProperty(DepthFormat::Depth24Stencil8);
            };
        }

    protected:
        void LoadContent() override
        {
            const PresentationParameters& actual = getGraphicsDeviceProperty().getPresentationParametersProperty();
            Context().Print("PreparingDeviceSettings raised {} time(s); device back buffer {}x{}", events_,
                            actual.getBackBufferWidthProperty(), actual.getBackBufferHeightProperty());
            Context().Check(events_ >= 1, "the event was raised before the device was created");
            Context().Check(actual.getBackBufferWidthProperty() == 512 && actual.getBackBufferHeightProperty() == 256,
                            "the device uses the size the handler chose");
            Context().Check(actual.getDepthStencilFormatProperty() == DepthFormat::Depth24Stencil8,
                            "and the depth/stencil format it chose");

            GraphicsDeviceInformation information;
            Context().Check(information.getAdapterProperty() == nullptr &&
                                information.getGraphicsProfileProperty() == GraphicsProfile::Reach,
                            "new device information starts with no adapter and the Reach profile");
            information.setAdapterProperty(&GraphicsAdapter::getDefaultAdapterProperty());
            information.setGraphicsProfileProperty(GraphicsProfile::HiDef);
            information.getPresentationParametersProperty().setBackBufferWidthProperty(320);
            information.getPresentationParametersProperty().setBackBufferHeightProperty(180);

            GraphicsDeviceInformation clone = information.Clone();
            Context().Check(information.Equals(&clone) && clone.Equals(&information) &&
                                information.GetHashCode() == clone.GetHashCode() && !information.Equals(nullptr),
                            "a deep clone has symmetric value equality and the same hash");
            clone.getPresentationParametersProperty().setBackBufferWidthProperty(321);
            Context().Check(!information.Equals(&clone) &&
                                information.getPresentationParametersProperty().getBackBufferWidthProperty() == 320,
                            "changing cloned presentation parameters leaves the source independent and unequal");

            PreparingDeviceSettingsEventArgs explicitArgs(information);
            Context().Check(&explicitArgs.getGraphicsDeviceInformationProperty() == &information,
                            "explicit preparing-device args retain the mutable information object");

            DeviceSelectionProbe probe;
            GraphicsDeviceInformation best = probe.FindBest(true);
            std::vector<GraphicsDeviceInformation> candidates{best, best.Clone()};
            probe.Rank(candidates);
            Context().Check(!probe.CanReset(best) && best.getAdapterProperty() != nullptr &&
                                best.getPresentationParametersProperty().getBackBufferWidthProperty() ==
                                    GraphicsDeviceManager::DefaultBackBufferWidth &&
                                candidates.size() == 2 && candidates[0].Equals(&candidates[1]),
                            "a manager without a device cannot reset; selection supplies defaults and ranking is stable");
        }

    private:
        int events_ = 0;
    };
}

CNA_DEMOS_GAME(PreparingSettingsDemo, {
    .id = "game.graphics_device_manager.preparing_device_settings",
    .title = "Customise device creation",
    .description = "Handles GraphicsDeviceManager.PreparingDeviceSettings to change the back-buffer size and "
                   "depth format in the PresentationParameters before the device is created.",
    .apis = {"GraphicsDeviceManager::PreparingDeviceSettings", "GraphicsDeviceManager::OnPreparingDeviceSettings",
             "GraphicsDeviceManager::CanResetDevice", "GraphicsDeviceManager::FindBestDevice",
             "GraphicsDeviceManager::RankDevices", "PreparingDeviceSettingsEventArgs::PreparingDeviceSettingsEventArgs",
             "PreparingDeviceSettingsEventArgs::GraphicsDeviceInformation",
             "PreparingDeviceSettingsEventArgs::getGraphicsDeviceInformationEXT",
             "GraphicsDeviceInformation::GraphicsDeviceInformation", "GraphicsDeviceInformation::Adapter",
             "GraphicsDeviceInformation::GraphicsProfile", "GraphicsDeviceInformation::PresentationParameters",
             "GraphicsDeviceInformation::Clone", "GraphicsDeviceInformation::Equals",
             "GraphicsDeviceInformation::GetHashCode", "PresentationParameters::DepthStencilFormat",
             "PresentationParameters::BackBufferWidth", "PresentationParameters::BackBufferHeight"},
    .tags = {"device", "settings", "event"},
    .frames = 3,
})
