// SPDX-License-Identifier: MS-PL
// TouchPanel capabilities and its coordinate space (DisplayWidth, DisplayHeight, orientation).
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanelCapabilities.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input::Touch;

namespace
{
    class TouchCapabilitiesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // Whether there is a touch screen, and how many fingers XNA promises to track.
            const TouchPanelCapabilities caps = TouchPanel::GetCapabilities();
            lines_.push_back(std::string("IsConnected: ") + (caps.getIsConnectedProperty() ? "true" : "false"));
            const bool deviceExists = TouchPanel::getTouchDeviceExistsProperty();
            lines_.push_back(std::string("TouchDeviceExistsEXT: ") + (deviceExists ? "true" : "false"));
            lines_.push_back("MaximumTouchCount: " + std::to_string(caps.getMaximumTouchCountProperty()));

            // Touch positions are reported in a coordinate space of DisplayWidth x DisplayHeight.
            // The graphics device sets it to the back-buffer size, so touches line up with what
            // is drawn; a game may set another size (say, a fixed virtual resolution).
            const PresentationParameters& pp = getGraphicsDeviceProperty().getPresentationParametersProperty();
            lines_.push_back("DisplayWidth x DisplayHeight: " + std::to_string(TouchPanel::getDisplayWidthProperty()) +
                             " x " + std::to_string(TouchPanel::getDisplayHeightProperty()));
            lines_.push_back("Back buffer: " + std::to_string(pp.getBackBufferWidthProperty()) + " x " +
                             std::to_string(pp.getBackBufferHeightProperty()));
            lines_.push_back("DisplayOrientation: " +
                             std::string(CnaDemos::Name(TouchPanel::getDisplayOrientationProperty())));
            for (const std::string& line : lines_)
                Context().Print("{}", line);

            Context().Check(TouchPanel::getDisplayWidthProperty() == pp.getBackBufferWidthProperty() &&
                                TouchPanel::getDisplayHeightProperty() == pp.getBackBufferHeightProperty(),
                            "the touch coordinate space is the back-buffer size");
            Context().Check(deviceExists == caps.getIsConnectedProperty(),
                            "TouchDeviceExistsEXT agrees with TouchPanelCapabilities::IsConnected");
            if (!caps.getIsConnectedProperty())
                Context().Check(caps.getMaximumTouchCountProperty() == 0, "without a touch screen no touches are promised");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "TouchPanel", Vector2(40.0f, 30.0f), Color::White);
            for (std::size_t i = 0; i < lines_.size(); ++i)
                spriteBatch_->DrawString(*font_, lines_[i], Vector2(40.0f, 80.0f + 32.0f * static_cast<float>(i)),
                                         Color::LightGray);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(TouchCapabilitiesDemo, {
    .id = "input.touch.capabilities",
    .title = "Touch panel capabilities",
    .description = "Shows TouchPanel::GetCapabilities (IsConnected, MaximumTouchCount), the CNAEXT physical-device "
                   "presence query, and the touch coordinate "
                   "space -- DisplayWidth and DisplayHeight, set to the back-buffer size by the device -- and "
                   "DisplayOrientation.",
    .apis = {"TouchPanel::GetCapabilities", "TouchPanel::TouchDeviceExistsEXT",
             "TouchPanelCapabilities::IsConnected",
             "TouchPanelCapabilities::MaximumTouchCount", "TouchPanel::DisplayWidth", "TouchPanel::DisplayHeight",
             "TouchPanel::DisplayOrientation",
             "DisplayOrientation"},
    .tags = {"touch", "capabilities", "coordinates", "phone", "tablet"},
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb"},
})
