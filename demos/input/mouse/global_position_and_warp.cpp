// SPDX-License-Identifier: MS-PL
// Desktop-global mouse coordinates and a reversible, explicitly triggered warp.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class GlobalPositionAndWarpDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Initialize() override
        {
            setIsMouseVisibleProperty(true);
            DemoGame::Initialize();
        }

        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            wWasDown_ = Keyboard::GetState().IsKeyDown(Keys::W);
        }

        void UnloadContent() override { RestoreCursor(); }

        void Update(GameTime& gameTime) override
        {
            Mouse::GetGlobalPositionEXT(globalX_, globalY_);
            window_ = Mouse::GetState();

            const bool wDown = Keyboard::GetState().IsKeyDown(Keys::W);
            if (!Context().Automated() && wDown && !wWasDown_ && !restorePending_)
            {
                originalX_ = globalX_;
                originalY_ = globalY_;
                const bool moved = Mouse::WarpGlobalEXT(globalX_ + 100, globalY_);
                status_ = moved ? "Warp accepted; original position will be restored after one second."
                                : "The platform refused the desktop-global warp.";
                restorePending_ = moved;
                restoreAfter_ = 1.0;
            }
            wWasDown_ = wDown;

            if (restorePending_)
            {
                restoreAfter_ -= gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty();
                if (restoreAfter_ <= 0.0)
                    RestoreCursor();
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Press W to move the pointer +100 desktop pixels for one second.",
                                     Vector2(24.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, "Window GetState: (" + std::to_string(window_.getXProperty()) + ", " +
                                         std::to_string(window_.getYProperty()) + ")",
                                     Vector2(24.0f, 85.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, "Desktop GetGlobalPositionEXT: (" + std::to_string(globalX_) + ", " +
                                         std::to_string(globalY_) + ")",
                                     Vector2(24.0f, 120.0f), Color::Gold);
            spriteBatch_->DrawString(*font_, status_, Vector2(24.0f, 175.0f), Color::LightGreen);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void RestoreCursor()
        {
            if (!restorePending_)
                return;
            const bool restored = Mouse::WarpGlobalEXT(originalX_, originalY_);
            status_ = restored ? "Original desktop position restored."
                               : "The platform refused restoration; move the pointer manually.";
            restorePending_ = false;
        }

        bool wWasDown_ = false;
        bool restorePending_ = false;
        int globalX_ = 0;
        int globalY_ = 0;
        int originalX_ = 0;
        int originalY_ = 0;
        double restoreAfter_ = 0.0;
        MouseState window_;
        std::string status_ = "No global warp has been requested.";
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(GlobalPositionAndWarpDemo, {
    .id = "input.mouse.global_position_and_warp",
    .title = "Desktop-global mouse position and warp",
    .description = "Shows window-relative and desktop-global pointer coordinates; pressing W requests a 100-pixel "
                   "global warp and automatically restores the original position after one second or on exit.",
    .apis = {"Mouse::GetState", "Mouse::GetGlobalPositionEXT", "Mouse::WarpGlobalEXT"},
    .tags = {"mouse", "pointer", "desktop", "global-position", "warp", "manual", "cnaext"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Mouse | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Manual desktop workflow; W is required to move the real pointer and cleanup attempts restoration.",
})
