// SPDX-License-Identifier: MS-PL
// CNAEXT GameWindow::FileDropEXT and TextDropEXT: files and text dragged onto the game window.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/FileDropEventArgsEXT.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/TextDropEventArgsEXT.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class FileDropDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            // A level editor opening a dropped level file, a viewer showing a dropped image...
            // Both events are raised on the game-loop thread, between Updates.
            fileToken_ = getWindowProperty().FileDropEXT.Add([this](System::Object*, const FileDropEventArgsEXT& args) {
                for (const std::string& file : args.getFilesProperty())
                {
                    Add("file: " + file);
                }
            });
            textToken_ = getWindowProperty().TextDropEXT.Add([this](System::Object*, const TextDropEventArgsEXT& args) {
                Add("text: " + args.getTextProperty()); // a link dragged from a browser arrives as its URL
            });
        }

        void UnloadContent() override
        {
            getWindowProperty().FileDropEXT.Remove(fileToken_);
            getWindowProperty().TextDropEXT.Remove(textToken_);
        }

        void Add(const std::string& line)
        {
            Context().Print("{}", line);
            drops_.push_back(line);
            if (drops_.size() > 12)
            {
                drops_.erase(drops_.begin());
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 34, 46));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Drop files or text onto this window", Vector2(20.0f, 20.0f), Color::White);
            float y = 60.0f;
            for (const std::string& line : drops_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(20.0f, y), Color::LightGreen);
                y += 26.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        System::EventHandler<FileDropEventArgsEXT>::Token fileToken_{};
        System::EventHandler<TextDropEventArgsEXT>::Token textToken_{};
        std::vector<std::string> drops_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(FileDropDemo, {
    .id = "game.window.file_drop",
    .title = "Drag and drop (CNAEXT)",
    .description = "Lists files dropped onto the window (GameWindow::FileDropEXT, all files of one drop at once) and "
                   "dropped text (TextDropEXT, e.g. a link dragged from a browser), raised on the game-loop thread.",
    .apis = {"GameWindow::FileDropEXT", "GameWindow::TextDropEXT", "FileDropEventArgsEXT::Files",
             "TextDropEventArgsEXT::Text"},
    .tags = {"window", "drag-and-drop", "files", "editor", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Mouse,
    .frames = 20,
    .assets = {"fonts/pixel_font.cnb"},
})
