// SPDX-License-Identifier: MS-PL
// ContentManager caches what it loads: loading again is free, Unload empties the cache.
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

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    class CacheAndUnloadDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // The device announces every texture created on it: that shows when Load really loads.
            created_ = device.ResourceCreated.Add([this](System::Object*, const ResourceCreatedEventArgs&) { ++textures_; });

            // A manager of its own, as a game would use for one level's content.
            ContentManager level(&getServicesProperty(), "assets");
            const int before = textures_;
            checker_ = level.Load<Texture2D>("textures/checker");
            const int firstLoad = textures_ - before;
            // Loading the same name again -- for example from Draw every frame, as some XNA code
            // does -- returns the cached asset without reading the file or creating a texture.
            for (int i = 0; i < 100; ++i)
                checker_ = level.Load<Texture2D>("textures/checker");
            const int repeatedLoads = textures_ - before - firstLoad;
            // Unload forgets everything this manager loaded; the next Load reads the file again.
            level.Unload();
            checker_ = level.Load<Texture2D>("textures/checker");
            const int afterUnload = textures_ - before - firstLoad - repeatedLoads;

            Context().Print("textures created: first Load {}, 100 more Loads {}, Load after Unload {}", firstLoad,
                            repeatedLoads, afterUnload);
            Context().Check(firstLoad == 1, "the first Load creates the texture");
            Context().Check(repeatedLoads == 0, "loading the same asset again is a cache hit");
            Context().Check(afterUnload == 1, "after Unload the asset is loaded anew");
            summary_ = "first Load: " + std::to_string(firstLoad) + ", 100 more: " + std::to_string(repeatedLoads) +
                       ", after Unload: " + std::to_string(afterUnload);
        }

        void UnloadContent() override { getGraphicsDeviceProperty().ResourceCreated.Remove(created_); }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Textures created by ContentManager::Load", Vector2(40.0f, 30.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, summary_, Vector2(40.0f, 60.0f), Color::LightGreen);
            spriteBatch_->Draw(checker_, Rectangle(40, 120, 192, 192), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        int textures_ = 0;
        System::EventHandler<ResourceCreatedEventArgs>::Token created_{};
        std::string summary_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D checker_;
    };
}

CNA_DEMOS_GAME(CacheAndUnloadDemo, {
    .id = "content.content_manager.cache_and_unload",
    .title = "Asset cache and Unload",
    .description = "Counts the textures the device creates while a ContentManager loads one asset once, 100 more "
                   "times (cache hits, no new texture) and again after Unload (loaded anew).",
    .apis = {"ContentManager::ContentManager", "ContentManager::Load", "ContentManager::Unload",
             "GraphicsDevice::ResourceCreated"},
    .tags = {"content", "cache", "unload", "level", "memory"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "textures/checker.png"},
})
