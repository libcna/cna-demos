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

            const bool screenSaverBefore = Guide::getIsScreenSaverEnabledProperty();
            Guide::setIsScreenSaverEnabledProperty(!screenSaverBefore);
            const bool screenSaverChanged = Guide::getIsScreenSaverEnabledProperty() != screenSaverBefore;
            Guide::setIsScreenSaverEnabledProperty(screenSaverBefore);
            ctx.Check(screenSaverChanged && Guide::getIsScreenSaverEnabledProperty() == screenSaverBefore,
                      "IsScreenSaverEnabled changes through the platform and restores its original state");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Background);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Guide notifications: TopRight", Vector2(40.0f, 220.0f), Color::White);
            spriteBatch_->End();
            // Inspect the overlay from its first frames instead of waiting a fixed frame count.
            // A small bounded observation cadence also lets an unthrottled run animate the toast.
            if (!checked_) std::this_thread::sleep_for(std::chrono::milliseconds(10));
            DemoGame::Draw(gameTime);
        }

        // The Guide -- and its notification -- is drawn over the game after Draw, before the
        // frame is presented: the place to look at the finished frame.
        void EndDraw() override
        {
            if (!checked_)
            {
                const auto now = std::chrono::steady_clock::now();
                if (!observationStarted_) observationStarted_ = now;
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                topRight_ = Covered(pixels, 500, 0, 800, 150);
                bottomCenter_ = Covered(pixels, 250, 330, 550, 480);
                ++observations_;
                // Observe either region: a misplaced toast must fail as soon as it is visible.
                // A missing toast remains a failure at the deadline or the end of a short run.
                if (topRight_ > 500 || bottomCenter_ > 500 ||
                    now - *observationStarted_ >= std::chrono::seconds(2))
                    CheckPosition();
            }
            DemoGame::EndDraw();
        }

        void EndRun() override
        {
            if (Context().Automated())
            {
                if (!checked_) CheckPosition();
                Context().Check(observations_ > 0, "the notification was looked at during the run");
            }
            DemoGame::EndRun();
        }

    private:
        void CheckPosition()
        {
            checked_ = true;
            Context().Print("notification pixels: {} at the top right, {} at the bottom centre; observed {} frame(s)",
                            topRight_, bottomCenter_, observations_);
            Context().Check(topRight_ > 500 && bottomCenter_ == 0,
                            "the sign-in notification appears at the top right, not at the bottom centre");
        }

        NotificationPosition defaultPosition_ = NotificationPosition::BottomCenter;
        bool checked_ = false;
        int observations_ = 0;
        int topRight_ = 0, bottomCenter_ = 0;
        std::optional<std::chrono::steady_clock::time_point> observationStarted_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(NotificationPositionDemo, {
    .id = "gamer_services.guide.notification_position",
    .title = "Where notifications appear",
    .description = "Moves the Guide's notifications from BottomCenter to TopRight, verifies the rendered sign-in "
                   "notification there, and toggles then restores the platform screen-saver state.",
    .apis = {"Guide::NotificationPosition", "Guide::IsScreenSaverEnabled", "NotificationPosition",
             "GamerServicesComponent"},
    .tags = {"gamer-services", "guide", "notification", "overlay"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
