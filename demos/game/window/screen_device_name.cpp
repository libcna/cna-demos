// SPDX-License-Identifier: MS-PL
// GameWindow::ScreenDeviceName: the display the window is on, and the event raised when the
// window moves to another one.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ScreenDeviceNameDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            GameWindow& window = getWindowProperty();
            const std::string& screen = window.getScreenDeviceNameProperty();
            const GraphicsAdapter& adapter = getGraphicsDeviceProperty().getAdapterProperty();
            Log("ScreenDeviceName: \"" + screen + "\"");
            Log("the device's adapter: \"" + adapter.getDeviceNameProperty() + "\" (" +
                adapter.getDescriptionProperty() + ")");
            Context().Check(!screen.empty(), "a window on a display reports the display's name");

            // Raised when the window is dragged to another monitor (a full-screen game picks the
            // adapter of that monitor again).
            changedToken_ = window.ScreenDeviceNameChanged.Add([this](System::Object*, const System::EventArgs&) {
                Log("ScreenDeviceNameChanged: now \"" + getWindowProperty().getScreenDeviceNameProperty() + "\"");
            });
            if (!Context().Automated())
            {
                Log("drag the window to another monitor");
            }
        }

        void UnloadContent() override { getWindowProperty().ScreenDeviceNameChanged.Remove(changedToken_); }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(28, 36, 48));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            float y = 40.0f;
            for (const std::string& line : log_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(30.0f, y), Color::White);
                y += 30.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        void Log(const std::string& line)
        {
            Context().Print("{}", line);
            log_.push_back(line);
        }

        System::EventHandler<System::EventArgs>::Token changedToken_{};
        std::vector<std::string> log_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(ScreenDeviceNameDemo, {
    .id = "game.window.screen_device_name",
    .title = "The window's display",
    .description = "Shows GameWindow::ScreenDeviceName, the display the window is on, beside the graphics "
                   "adapter's DeviceName, and logs ScreenDeviceNameChanged when the window is dragged to another "
                   "monitor.",
    .apis = {"GameWindow::ScreenDeviceName", "GameWindow::ScreenDeviceNameChanged", "GraphicsDevice::Adapter",
             "GraphicsAdapter::DeviceName", "GraphicsAdapter::Description"},
    .tags = {"window", "display", "monitor", "adapter"},
    .needs = CnaDemos::Need::Window,
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb"},
})
