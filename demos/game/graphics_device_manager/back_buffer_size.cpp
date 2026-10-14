// SPDX-License-Identifier: MS-PL
// Choosing the back-buffer size: PreferredBackBufferWidth/Height and ApplyChanges.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class BackBufferSizeDemo final : public CnaDemos::DemoGame
    {
    public:
        BackBufferSizeDemo()
        {
            defaultSizeMatchesConstants_ =
                Graphics().getPreferredBackBufferWidthProperty() == GraphicsDeviceManager::DefaultBackBufferWidth &&
                Graphics().getPreferredBackBufferHeightProperty() == GraphicsDeviceManager::DefaultBackBufferHeight;

            // Set before the device exists (in the constructor), these choose the initial size.
            Graphics().setPreferredBackBufferWidthProperty(640);
            Graphics().setPreferredBackBufferHeightProperty(360);
            Graphics().setPreferredBackBufferFormatProperty(SurfaceFormat::Bgr565);
            Graphics().setPreferMultiSamplingProperty(true);
        }

    protected:
        void LoadContent() override
        {
            const Viewport viewport = getGraphicsDeviceProperty().getViewportProperty();
            Context().Print("initial back buffer: {}x{}", viewport.getWidthProperty(), viewport.getHeightProperty());
            Context().Check(viewport.getWidthProperty() == 640 && viewport.getHeightProperty() == 360,
                            "the device was created at the preferred size");

            Context().Check(defaultSizeMatchesConstants_ && GraphicsDeviceManager::DefaultBackBufferWidth == 800 &&
                                GraphicsDeviceManager::DefaultBackBufferHeight == 480,
                            "the initial preferred size comes from the public 800x480 constants");
            Context().Check(Graphics().getPreferredBackBufferFormatProperty() == SurfaceFormat::Bgr565 &&
                                Graphics().getPreferMultiSamplingProperty(),
                            "the manager retains the requested packed format and multisampling preference");

            SurfaceFormat selectedFormat = SurfaceFormat::Color;
            DepthFormat selectedDepth = DepthFormat::None;
            int selectedSamples = 0;
            static_cast<void>(getGraphicsDeviceProperty().getAdapterProperty().QueryBackBufferFormat(
                Graphics().getGraphicsProfileProperty(), SurfaceFormat::Bgr565,
                Graphics().getPreferredDepthStencilFormatProperty(), 8,
                selectedFormat, selectedDepth, selectedSamples));
            const auto& parameters = getGraphicsDeviceProperty().getPresentationParametersProperty();
            Context().Check(parameters.getBackBufferFormatProperty() == selectedFormat &&
                                parameters.getDepthStencilFormatProperty() == selectedDepth,
                            "device creation applies the adapter-selected back-buffer and depth formats");
            Context().Check(parameters.getMultiSampleCountProperty() > 1,
                            "the multisampling preference creates a multisampled back buffer");
            Context().Check(parameters.getMultiSampleCountProperty() == selectedSamples,
                            "the adapter query predicts the multisample count that device creation applies");

            getWindowProperty().ClientSizeChanged +=
                [this](System::Object*, const System::EventArgs&) { ++clientSizeChanges_; };
        }

        void Update(GameTime& gameTime) override
        {
            // Later changes take effect when ApplyChanges is called.
            if (++updates_ == 3)
            {
                Graphics().setPreferredBackBufferWidthProperty(800);
                Graphics().setPreferredBackBufferHeightProperty(600);
                Graphics().setPreferredBackBufferFormatProperty(SurfaceFormat::Color);
                Graphics().setPreferMultiSamplingProperty(false);
                Graphics().ApplyChanges();

                const auto& parameters = getGraphicsDeviceProperty().getPresentationParametersProperty();
                Context().Print("after ApplyChanges: {}x{}", parameters.getBackBufferWidthProperty(),
                                parameters.getBackBufferHeightProperty());
                Context().Check(parameters.getBackBufferWidthProperty() == 800
                                    && parameters.getBackBufferHeightProperty() == 600,
                                "ApplyChanges resized the back buffer");
                Context().Check(Graphics().getPreferredBackBufferFormatProperty() == SurfaceFormat::Color &&
                                    !Graphics().getPreferMultiSamplingProperty() &&
                                    parameters.getBackBufferFormatProperty() == SurfaceFormat::Color &&
                                    parameters.getMultiSampleCountProperty() == 0,
                                "ApplyChanges applies the revised format and disables multisampling");
                Context().Check(getWindowProperty().getHandleProperty() == 0 || clientSizeChanges_ > 0,
                                "an attached window raises ClientSizeChanged for the applied size change");
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(updates_ < 3 ? Color::Navy : Color::DarkGreen);
            Game::Draw(gameTime);
        }

    private:
        bool defaultSizeMatchesConstants_ = false;
        int clientSizeChanges_ = 0;
        int updates_ = 0;
    };
}

CNA_DEMOS_GAME(BackBufferSizeDemo, {
    .id = "game.graphics_device_manager.back_buffer_size",
    .title = "Back-buffer size",
    .description = "Starts from GraphicsDeviceManager's default-size constants, creates a 640x360 multisampled "
                   "device with a requested packed format, then applies an 800x600 Color back buffer without "
                   "multisampling at run time.",
    .apis = {"GraphicsDeviceManager::DefaultBackBufferWidth", "GraphicsDeviceManager::DefaultBackBufferHeight",
             "GraphicsDeviceManager::PreferredBackBufferWidth", "GraphicsDeviceManager::PreferredBackBufferHeight",
             "GraphicsDeviceManager::PreferredBackBufferFormat", "GraphicsDeviceManager::PreferMultiSampling",
             "GraphicsDeviceManager::ApplyChanges", "PresentationParameters::BackBufferWidth",
             "PresentationParameters::BackBufferHeight", "PresentationParameters::BackBufferFormat",
             "PresentationParameters::MultiSampleCount", "GraphicsAdapter::QueryBackBufferFormat",
             "GameWindow::BeginScreenDeviceChange", "GameWindow::EndScreenDeviceChange",
             "GameWindow::ClientSizeChanged", "GameWindow::OnClientSizeChanged", "GraphicsDevice::Viewport",
             "GraphicsDevice::PresentationParameters"},
    .tags = {"resolution", "window", "device", "multisampling", "format"},
    .frames = 6,
    .restrictions = "CNA-DEMOS-ISSUE-083: QueryBackBufferFormat reports zero samples before the device applies "
                    "a supported multisample count.",
    .knownIssue = "CNA-DEMOS-ISSUE-083",
})
