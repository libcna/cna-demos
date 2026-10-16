// SPDX-License-Identifier: MS-PL
// Guide::NotificationPosition: where the Guide's notifications (sign-ins, achievements) appear.
#include <algorithm>
#include <array>
#include <chrono>
#include <limits>
#include <memory>
#include <optional>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Guide.hpp"
#include "Microsoft/Xna/Framework/GamerServices/NotificationPosition.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
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

    constexpr std::array<NotificationPosition, 9> Positions = {
        NotificationPosition::TopLeft, NotificationPosition::TopCenter, NotificationPosition::TopRight,
        NotificationPosition::CenterLeft, NotificationPosition::Center, NotificationPosition::CenterRight,
        NotificationPosition::BottomLeft, NotificationPosition::BottomCenter, NotificationPosition::BottomRight};

    struct Footprint
    {
        int covered = 0;
        int minX = std::numeric_limits<int>::max(), minY = std::numeric_limits<int>::max();
        int maxX = -1, maxY = -1;
    };

    Footprint FindFootprint(const CnaDemos::BackBufferPixels& pixels)
    {
        Footprint result;
        for (int y = 0; y < pixels.Height(); ++y)
        {
            for (int x = 0; x < pixels.Width(); ++x)
            {
                if (!CnaDemos::Near(pixels.At(x, y), Background, 4))
                {
                    ++result.covered;
                    result.minX = std::min(result.minX, x);
                    result.minY = std::min(result.minY, y);
                    result.maxX = std::max(result.maxX, x);
                    result.maxY = std::max(result.maxY, y);
                }
            }
        }
        return result;
    }

    class NotificationPositionDemo final : public CnaDemos::GamerServicesDemo
    {
    public:
        NotificationPositionDemo()
        {
            // Set before the profile signs in, so its "signed in" notification already uses it.
            // (XNA's default is BottomCenter.)
            defaultPosition_ = Guide::getNotificationPositionProperty();
            Guide::setNotificationPositionProperty(Positions.front());
        }

    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            ctx.Check(defaultPosition_ == NotificationPosition::BottomCenter, "notifications start at BottomCenter");
            ctx.Check(Guide::getNotificationPositionProperty() == NotificationPosition::TopLeft,
                      "NotificationPosition is now TopLeft");

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
            if (checked_)
            {
                spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
                spriteBatch_->DrawString(*font_, "Guide notifications: all nine positions checked",
                                         Vector2(40.0f, 220.0f), Color::White);
                spriteBatch_->End();
            }
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
                // The offline sign-in is delivered by GamerServicesComponent::Update. Under a
                // loaded test run that asynchronous precondition can take longer than the toast's
                // own rendering check; start the bounded observation only once the public gamer
                // collection proves that the notification has actually been queued.
                if (Gamer::getSignedInGamersProperty()->getCountProperty() > 0)
                {
                    if (!observationStarted_) observationStarted_ = now;
                    const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                    const Footprint footprint = FindFootprint(pixels);
                    ++observations_;
                    if (footprint.covered > 500)
                    {
                        CheckPosition(pixels, footprint);
                    }
                    else if (now - *observationStarted_ >= std::chrono::seconds(2))
                    {
                        Context().Check(false, "the queued sign-in notification became visible within two seconds");
                        checked_ = true;
                    }
                }
            }
            DemoGame::EndDraw();
        }

        void EndRun() override
        {
            if (Context().Automated())
            {
                Context().Check(positionIndex_ == Positions.size(),
                                "the live notification was observed at all nine positions");
                Context().Check(observations_ > 0, "the notification was looked at during the run");
            }
            DemoGame::EndRun();
        }

    private:
        void CheckPosition(const CnaDemos::BackBufferPixels& pixels, const Footprint& footprint)
        {
            const auto expected = Positions[positionIndex_];
            const int ordinal = static_cast<int>(expected);
            const int centreX = (footprint.minX + footprint.maxX) / 2;
            const int centreY = (footprint.minY + footprint.maxY) / 2;
            const int observedColumn = std::min(2, centreX * 3 / pixels.Width());
            const int observedRow = std::min(2, centreY * 3 / pixels.Height());
            Context().Print("position {}: {} pixels, bounds ({}, {})-({}, {})", ordinal, footprint.covered,
                            footprint.minX, footprint.minY, footprint.maxX, footprint.maxY);
            Context().Check(Guide::getNotificationPositionProperty() == expected &&
                                observedColumn == ordinal % 3 && observedRow == ordinal / 3,
                            "notification position " + std::to_string(ordinal) + " selects its rendered grid cell");
            ++positionIndex_;
            if (positionIndex_ == Positions.size())
            {
                checked_ = true;
            }
            else
            {
                Guide::setNotificationPositionProperty(Positions[positionIndex_]);
            }
        }

        NotificationPosition defaultPosition_ = NotificationPosition::BottomCenter;
        bool checked_ = false;
        std::size_t positionIndex_ = 0;
        int observations_ = 0;
        std::optional<std::chrono::steady_clock::time_point> observationStarted_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(NotificationPositionDemo, {
    .id = "gamer_services.guide.notification_position",
    .title = "Where notifications appear",
    .description = "Moves one live sign-in notification through all nine NotificationPosition grid cells, verifies "
                   "each rendered placement, and toggles then restores the platform screen-saver state.",
    .apis = {"Guide::NotificationPosition", "Guide::IsScreenSaverEnabled", "NotificationPosition",
             "NotificationPosition::TopLeft", "NotificationPosition::TopCenter", "NotificationPosition::TopRight",
             "NotificationPosition::CenterLeft", "NotificationPosition::Center", "NotificationPosition::CenterRight",
             "NotificationPosition::BottomLeft", "NotificationPosition::BottomCenter",
             "NotificationPosition::BottomRight",
             "GamerServicesComponent"},
    .tags = {"gamer-services", "guide", "notification", "overlay"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
