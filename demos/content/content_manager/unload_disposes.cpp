// SPDX-License-Identifier: MS-PL
// XNA's ContentManager::Unload disposes every asset it loaded (CNA-DEMOS-ISSUE-011 in CNA).
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using Microsoft::Xna::Framework::Content::ContentManager;

namespace
{
    class UnloadDisposesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            // Unload is how an XNA game frees a level's textures and sounds: they are disposed,
            // whether or not the game still holds them, and using one afterwards is an error.
            ContentManager level(&getServicesProperty(), "assets");
            const Texture2D checker = level.Load<Texture2D>("textures/checker");
            level.Unload();
            Context().Print("after Unload: IsDisposed = {}", checker.getIsDisposedProperty());
            Context().Check(checker.getIsDisposedProperty(), "Unload disposed the loaded texture");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            Game::Draw(gameTime);
        }
    };
}

CNA_DEMOS_GAME(UnloadDisposesDemo, {
    .id = "content.content_manager.unload_disposes",
    .title = "Unload disposes loaded assets",
    .description = "Loads a texture with its own ContentManager, calls Unload and checks XNA's rule that the texture "
                   "is now disposed.",
    .apis = {"ContentManager::Unload", "GraphicsResource::IsDisposed"},
    .tags = {"content", "unload", "dispose", "memory"},
    .frames = 1,
    .assets = {"textures/checker.png"},
    .knownIssue = "CNA-DEMOS-ISSUE-011",
})
