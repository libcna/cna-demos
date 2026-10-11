// SPDX-License-Identifier: MS-PL
// Guide::NotificationPosition: where the Guide's notifications (sign-ins, achievements) appear.
#include <chrono>
#include <memory>
#include <optional>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Guide.hpp"
#include "Microsoft/Xna/Framework/GamerServices/NotificationPosition.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Color Background(20, 22, 30);

    // Counts the pixels in a region that are not the background: the notification's footprint.
    int Covered(const CnaDemos::BackBufferPixels& pixels, int x0, int y0, int x1, int y1)
    {
        int covered = 0;
        for (int y = y0; y < y1; ++y)
            for (int x = x0; x < x1; ++x)
                if (!CnaDemos::Near(pixels.At(x, y), Background, 4))
                    ++covered;
        return covered;
    }

    class NotificationPositionDemo final : public CnaDemos::GamerServicesDemo
    {
    public:
        NotificationPositionDemo()
        {
            // Set before the profile signs in, so its "signed in" notification already uses it.
            // (XNA's default is BottomCenter.)
            defaultPosition_ = Guide::getNotificationPositionProperty();
            Guide::setNotificationPositionProperty(NotificationPosition::TopRight);
        }

    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            ctx.Check(defaultPosition_ == NotificationPosition::BottomCenter, "notifications start at BottomCenter");
            ctx.Check(Guide::getNotificationPositionProperty() == NotificationPosition::TopRight,
                      "NotificationPosition is now TopRight");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Background);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Guide notifications: TopRight", Vector2(40.0f, 220.0f), Color::White);
            spriteBatch_->End();
            if (!checked_ && FramesDrawn() == 10)
            {
                // The Guide slides its notification in over a quarter of a second of real time;
                // an unattended run draws frames faster than that, so it waits before looking.
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                look_ = true;
            }
            DemoGame::Draw(gameTime);
        }

        // The Guide -- and its notification -- is drawn over the game after Draw, before the
        // frame is presented: the place to look at the finished frame.
        void EndDraw() override
        {
            if (look_)
            {
                look_ = false;
                checked_ = true;
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                const int topRight = Covered(pixels, 500, 0, 800, 150);
                const int bottomCenter = Covered(pixels, 250, 330, 550, 480);
                Context().Print("notification pixels: {} at the top right, {} at the bottom centre", topRight,
                                bottomCenter);
                Context().Check(topRight > 500 && bottomCenter == 0,
                                "the sign-in notification appears at the top right, not at the bottom centre");
            }
            DemoGame::EndDraw();
        }

        void EndRun() override
        {
            if (Context().Automated())
            {
                Context().Check(checked_, "the notification was looked at during the run");
            }
            DemoGame::EndRun();
        }

    private:
        NotificationPosition defaultPosition_ = NotificationPosition::BottomCenter;
        bool look_ = false;
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(NotificationPositionDemo, {
    .id = "gamer_services.guide.notification_position",
    .title = "Where notifications appear",
    .description = "Moves the Guide's notifications from the default BottomCenter to TopRight with "
                   "Guide::NotificationPosition before the profile signs in, and finds the sign-in notification "
                   "the GamerServicesComponent draws in the top-right corner of the screen.",
    .apis = {"Guide::NotificationPosition", "NotificationPosition", "GamerServicesComponent"},
    .tags = {"gamer-services", "guide", "notification", "overlay"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
