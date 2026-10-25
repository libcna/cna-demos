// SPDX-License-Identifier: MS-PL
// Real asynchronous host file/folder dialogs with callback lifetime isolation.
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "CNA/Devices/FileDialog.hpp"
#include "CNA/Devices/FileDialogFilter.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    struct DialogState
    {
        std::mutex mutex;
        bool waiting = false;
        int resultCount = 0;
        std::vector<std::string> paths;
        std::string status = "No dialog has been requested.";
    };

    CNA::Devices::FileDialog::ResultCallback ResultHandler(const std::weak_ptr<DialogState>& weak)
    {
        return [weak](const std::vector<std::string>& paths) {
            if (const auto state = weak.lock())
            {
                std::lock_guard lock(state->mutex);
                state->waiting = false;
                ++state->resultCount;
                state->paths = paths;
                state->status = paths.empty() ? "Dialog closed with no selected path."
                                              : "Dialog returned " + std::to_string(paths.size()) + " path(s).";
            }
        };
    }

    class FileDialogDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            supported_ = CNA::Devices::FileDialog::getIsSupportedProperty();
            state_ = std::make_shared<DialogState>();
            const KeyboardState keyboard = Keyboard::GetState();
            oWasDown_ = keyboard.IsKeyDown(Keys::O);
            fWasDown_ = keyboard.IsKeyDown(Keys::F);
        }

        void UnloadContent() override
        {
            // A native dialog has no cancellation API. Its callback holds only a weak reference,
            // so closing a still-open dialog after this demo exits cannot touch the destroyed game.
            state_.reset();
        }

        void Update(GameTime& gameTime) override
        {
            const KeyboardState keyboard = Keyboard::GetState();
            const bool oDown = keyboard.IsKeyDown(Keys::O);
            const bool fDown = keyboard.IsKeyDown(Keys::F);
            if (!Context().Automated() && ((oDown && !oWasDown_) || (fDown && !fWasDown_)))
            {
                bool canStart = supported_;
                {
                    std::lock_guard lock(state_->mutex);
                    if (!supported_)
                        state_->status = "Native file dialogs are unsupported on this host.";
                    else if (state_->waiting)
                    {
                        state_->status = "A dialog is already open.";
                        canStart = false;
                    }
                    else
                    {
                        state_->waiting = true;
                        state_->status = "Waiting for the native dialog callback...";
                    }
                }
                if (canStart)
                {
                    try
                    {
                        const auto callback = ResultHandler(state_);
                        if (oDown)
                        {
                            CNA::Devices::FileDialog::ShowOpenFile(
                                callback, {{"Text files", "txt"}, {"All files", "*"}}, "", false);
                        }
                        else
                            CNA::Devices::FileDialog::ShowOpenFolder(callback, "", false);
                    }
                    catch (const std::exception& exception)
                    {
                        std::lock_guard lock(state_->mutex);
                        state_->waiting = false;
                        state_->status = std::string("Dialog launch failed: ") + exception.what();
                    }
                }
            }
            oWasDown_ = oDown;
            fWasDown_ = fDown;

            {
                std::lock_guard lock(state_->mutex);
                waiting_ = state_->waiting;
                resultCount_ = state_->resultCount;
                paths_ = state_->paths;
                status_ = state_->status;
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "O: native open-file dialog   F: native folder dialog",
                                     Vector2(24.0f, 24.0f), Color::White);
            spriteBatch_->DrawString(*font_, "The call returns immediately; the callback may use another thread.",
                                     Vector2(24.0f, 58.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, std::string("Supported: ") + (supported_ ? "yes" : "no") +
                                         "  waiting: " + (waiting_ ? "yes" : "no") +
                                         "  callbacks: " + std::to_string(resultCount_),
                                     Vector2(24.0f, 110.0f), Color::Gold);
            spriteBatch_->DrawString(*font_, status_, Vector2(24.0f, 144.0f), Color::LightGreen);
            float y = 190.0f;
            for (const std::string& path : paths_)
            {
                spriteBatch_->DrawString(*font_, path, Vector2(32.0f, y), Color::White);
                y += 28.0f;
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        bool supported_ = false;
        bool oWasDown_ = false;
        bool fWasDown_ = false;
        bool waiting_ = false;
        int resultCount_ = 0;
        std::vector<std::string> paths_;
        std::string status_;
        std::shared_ptr<DialogState> state_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(FileDialogDemo, {
    .id = "devices.desktop.file_dialog",
    .title = "Native file and folder dialogs",
    .description = "Press O or F to open real asynchronous host file/folder dialogs; results are transferred safely "
                   "from a possibly foreign callback thread without retaining the destroyed game.",
    .apis = {"CNA::Devices::FileDialog::IsSupported", "CNA::Devices::FileDialog::ShowOpenFile",
             "CNA::Devices::FileDialog::ShowOpenFolder", "CNA::Devices::FileDialogFilter"},
    .tags = {"devices", "desktop", "dialog", "file-picker", "folder-picker", "callback", "manual", "cnaext"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Manual: opens a native picker only after O/F; close it before leaving because CNA has no cancel API.",
})
