// SPDX-License-Identifier: MS-PL
// Manual real-camera workflow: enumerate, explicitly open, upload frames, close.
#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CNA/Devices/Camera.hpp"
#include "CNA/Devices/CameraPosition.hpp"
#include "CNA/Devices/CameraState.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    std::string StateName(CNA::Devices::CameraState state)
    {
        using enum CNA::Devices::CameraState;
        switch (state)
        {
            case NotSupported: return "NotSupported";
            case Closed: return "Closed";
            case Opening: return "Opening (permission pending)";
            case Denied: return "Denied";
            case Ready: return "Ready";
            case Lost: return "Lost";
        }
        return "unknown";
    }

    std::string PositionName(CNA::Devices::CameraPosition position)
    {
        using enum CNA::Devices::CameraPosition;
        return position == FrontFacing ? "front" : position == BackFacing ? "back" : "unknown position";
    }

    class LiveCaptureDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            supported_ = CNA::Devices::Camera::getIsSupportedProperty();
            cameras_ = CNA::Devices::Camera::getAvailableCamerasProperty();
            Context().Print("camera backend {}, {} physical device(s)", supported_ ? "available" : "unavailable",
                            cameras_.size());
        }

        void UnloadContent() override
        {
            // Destruction closes the real device and releases the permission/capture session.
            frame_.reset();
            camera_.reset();
        }

        void Update(GameTime& gameTime) override
        {
            const bool spaceDown = Keyboard::GetState().IsKeyDown(Keys::Space);
            if (spaceDown && !spaceWasDown_)
            {
                if (camera_)
                {
                    frame_.reset();
                    camera_.reset();
                    status_ = "Camera closed and resources released.";
                }
                else if (!supported_)
                    status_ = "This platform has no camera backend.";
                else if (cameras_.empty())
                    status_ = "No physical camera is available.";
                else
                {
                    camera_.emplace();
                    status_ = "Camera opened; waiting for permission/readiness.";
                }
            }
            spaceWasDown_ = spaceDown;

            if (camera_ && camera_->getStateProperty() == CNA::Devices::CameraState::Ready)
            {
                const int width = camera_->getFrameWidthProperty();
                const int height = camera_->getFrameHeightProperty();
                if (width > 0 && height > 0 &&
                    (!frame_ || frame_->getWidthProperty() != width || frame_->getHeightProperty() != height))
                {
                    frame_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), width, height);
                }
                if (frame_ && camera_->TryAcquireFrame(*frame_))
                    ++frames_;
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::LinearClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Space: open/close the first real camera", Vector2(24.0f, 20.0f),
                                     Color::White);
            std::string inventory = std::string("Supported: ") + (supported_ ? "yes" : "no") +
                "  cameras: " + std::to_string(cameras_.size());
            if (!cameras_.empty())
                inventory += "  first: " + cameras_[0].Name + " (" + PositionName(cameras_[0].Position) + ")";
            spriteBatch_->DrawString(*font_, inventory, Vector2(24.0f, 52.0f), Color::LightGray);
            const std::string state = camera_ ? StateName(camera_->getStateProperty()) : "Closed";
            spriteBatch_->DrawString(*font_, "State: " + state + "  frames: " + std::to_string(frames_),
                                     Vector2(24.0f, 84.0f), Color::Gold);
            spriteBatch_->DrawString(*font_, status_, Vector2(24.0f, 116.0f), Color::LightGray);
            if (frame_ && frames_ > 0)
            {
                const float scale = std::min(520.0f / frame_->getWidthProperty(),
                                             300.0f / frame_->getHeightProperty());
                const int width = static_cast<int>(frame_->getWidthProperty() * scale);
                const int height = static_cast<int>(frame_->getHeightProperty() * scale);
                spriteBatch_->Draw(*frame_, Rectangle((800 - width) / 2, 155, width, height), Color::White);
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        bool supported_ = false;
        bool spaceWasDown_ = false;
        int frames_ = 0;
        std::string status_ = "No device is opened until Space is pressed.";
        std::vector<CNA::Devices::CameraDeviceInfo> cameras_;
        std::optional<CNA::Devices::Camera> camera_;
        std::unique_ptr<Texture2D> frame_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(LiveCaptureDemo, {
    .id = "devices.camera.live_capture",
    .title = "Live camera capture",
    .description = "Enumerates physical cameras and, only after Space is pressed, opens the first device, waits for "
                   "permission, uploads real frames into a matching Texture2D, displays them and closes cleanly.",
    .apis = {"CNA::Devices::Camera::IsSupported", "CNA::Devices::Camera::AvailableCameras",
             "CNA::Devices::Camera::State", "CNA::Devices::Camera::FrameWidth",
             "CNA::Devices::Camera::FrameHeight", "CNA::Devices::Camera::TryAcquireFrame", "Texture2D::Texture2D"},
    .tags = {"devices", "camera", "webcam", "capture", "texture", "manual", "cnaext"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Manual: needs a real camera and may show an OS permission prompt; automation never opens a device.",
})
