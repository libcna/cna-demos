// SPDX-License-Identifier: MS-PL
// Adjusting the device before it is created: the PreparingDeviceSettings event.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/PreparingDeviceSettingsEventArgs.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
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
    .apis = {"GraphicsDeviceManager::PreparingDeviceSettings", "PreparingDeviceSettingsEventArgs::GraphicsDeviceInformation",
             "PreparingDeviceSettingsEventArgs::getGraphicsDeviceInformationEXT",
             "GraphicsDeviceInformation::PresentationParameters", "PresentationParameters::DepthStencilFormat",
             "PresentationParameters::BackBufferWidth", "PresentationParameters::BackBufferHeight"},
    .tags = {"device", "settings", "event"},
    .frames = 3,
})
