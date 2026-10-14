// SPDX-License-Identifier: MS-PL
// Several ContentManagers: shared assets in Game::Content, each level's assets in its own manager.
#include <exception>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/ResourceCreatedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    class SeveralManagersDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            // Game::Content holds what every level uses, for the whole game.
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            created_ = device.ResourceCreated.Add([this](System::Object*, const ResourceCreatedEventArgs&) { ++textures_; });

            // A level gets a manager of its own, created with the game's services (they supply
            // the graphics device). Its RootDirectory may differ: here the XNB content.
            level_ = std::make_unique<ContentManager>(&getServicesProperty(), "assets/xnb");
            Context().Check(level_->getRootDirectoryProperty() == "assets/xnb", "the level manager has its own root");
            wheel_ = level_->Load<Texture2D>("textures/colour_wheel");

            // The managers cache separately: the same file loaded through both is two textures.
            const int before = textures_;
            const Texture2D shared = getContentProperty().Load<Texture2D>("xnb/textures/colour_wheel");
            Context().Check(textures_ - before == 1, "the same file loaded by another manager is a second texture");
            (void)shared;

            ContentManager disposed(&getServicesProperty(), "assets");
            disposed.Dispose();
            bool exactDisposedError = false;
            try { (void)disposed.Load<Texture2D>("textures/checker"); }
            catch (const System::ObjectDisposedException&) { exactDisposedError = true; }
            catch (const std::exception& error)
            {
                Context().Print("disposed Load<T> threw the wrong exception: {}", error.what());
            }
            Context().Check(exactDisposedError,
                            "Load<T> after Dispose throws ObjectDisposedException (CNA-DEMOS-ISSUE-010)");
        }

        void UnloadContent() override { getGraphicsDeviceProperty().ResourceCreated.Remove(created_); }

        void Update(GameTime& gameTime) override
        {
            // "Level change" after one second: everything of the level goes at once.
            if (level_ && gameTime.getTotalGameTimeProperty().getTotalSecondsProperty() > 1.0)
            {
                level_->Unload();
                level_->Dispose();
                level_.reset();
                wheel_ = Texture2D();
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, level_ ? "Level 1: its own ContentManager (root assets/xnb)"
                                                    : "Level 1 unloaded -- the shared font stays",
                                     Vector2(40.0f, 30.0f), Color::White);
            if (level_)
                spriteBatch_->Draw(wheel_, Rectangle(40, 100, 256, 256), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        int textures_ = 0;
        System::EventHandler<ResourceCreatedEventArgs>::Token created_{};
        std::unique_ptr<ContentManager> level_;
        Texture2D wheel_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(SeveralManagersDemo, {
    .id = "content.content_manager.several_managers",
    .title = "One ContentManager per level",
    .description = "Keeps shared assets in Game::Content and level assets in a second ContentManager, checks the "
                   "post-disposal Load contract, and unloads the level after a second.",
    .apis = {"ContentManager::ContentManager", "ContentManager::RootDirectory", "ContentManager::Unload",
             "ContentManager::Dispose", "Game::Content", "Game::Services"},
    .tags = {"content", "level", "unload", "memory", "loading"},
    .automation = CnaDemos::Automation::Automated,
    .assets = {"fonts/pixel_font.cnb", "xnb/textures/colour_wheel.xnb"},
    .restrictions = "CNA-DEMOS-ISSUE-010: Load<T> after Dispose throws std::runtime_error instead of ObjectDisposedException.",
    .knownIssue = "CNA-DEMOS-ISSUE-010",
})
