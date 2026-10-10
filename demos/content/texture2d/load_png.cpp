// SPDX-License-Identifier: MS-PL
// Loading a PNG through the ContentManager.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class LoadPngDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());

            // The asset name has no extension. ContentManager looks for "textures/checker.xnb",
            // then ".cnb", and finally image files such as "textures/checker.png", all relative
            // to Content.RootDirectory ("assets" in cna-demos).
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");

            Context().Print("loaded textures/checker: {}x{}", checker_.getWidthProperty(),
                            checker_.getHeightProperty());
            Context().Check(checker_.getWidthProperty() == 64 && checker_.getHeightProperty() == 64,
                            "the loaded texture is 64x64");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);

            spriteBatch_->Begin();
            spriteBatch_->Draw(checker_, Rectangle(40, 40, 256, 256), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
    };
}

CNA_DEMOS_GAME(LoadPngDemo, {
    .id = "content.texture2d.load_png",
    .title = "Load a PNG texture",
    .description = "Loads assets/textures/checker.png with ContentManager::Load<Texture2D> - no content "
                   "build step - and draws it enlarged.",
    .apis = {"ContentManager::Load", "ContentManager::RootDirectory", "Texture2D::Width", "Texture2D::Height"},
    .tags = {"png", "image", "loading"},
    .frames = 10,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
