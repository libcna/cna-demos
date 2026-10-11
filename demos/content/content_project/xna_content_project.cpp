// SPDX-License-Identifier: MS-PL
// An XNA Game Studio content project (.contentproj) built by CNA's pipeline as it is: its items'
// Name, Importer, Processor and processor parameters, and the files it only copies.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IO/Stream.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using Microsoft::Xna::Framework::Audio::SoundEffect;
using Microsoft::Xna::Framework::Content::ContentLoadException;

namespace
{
    class XnaContentProjectDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());

            // content_project/Game.contentproj, built by cna_add_content(... CONTENT_PROJECT ...)
            // into assets/contentproj: one .xnb per Compile item, at its folder plus its Name.
            ship_.emplace(getContentProperty().Load<Texture2D>("contentproj/Textures/ship"));
            Color corner, middle;
            const Rectangle cornerTexel(0, 0, 1, 1), hullTexel(32, 40, 1, 1);
            ship_->GetData(0, &cornerTexel, &corner, 0, 1);
            ship_->GetData(0, &hullTexel, &middle, 0, 1);
            ctx.Print("ship: corner {}, hull {}", corner.ToString(), middle.ToString());
            ctx.Check(corner.getAProperty() == 0 && middle.getAProperty() == 255,
                      "TextureProcessor keyed out the magenta background (its default colour key)");

            bool renamed = false;
            try
            {
                (void)getContentProperty().Load<Texture2D>("contentproj/Textures/ship_source");
            }
            catch (const ContentLoadException&)
            {
                renamed = true;
            }
            ctx.Check(renamed, "the item's <Name> names the asset: there is no \"ship_source\"");

            // <ProcessorParameters_GenerateMipmaps>True</...> on that item only.
            grid_.emplace(getContentProperty().Load<Texture2D>("contentproj/Textures/grid"));
            ctx.Print("grid: {}x{}, LevelCount {}; ship LevelCount {}", grid_->getWidthProperty(),
                      grid_->getHeightProperty(), grid_->getLevelCountProperty(), ship_->getLevelCountProperty());
            ctx.Check(grid_->getLevelCountProperty() == 7 && ship_->getLevelCountProperty() == 1,
                      "the per-item processor parameter gave the grid a full mipmap chain, and only the grid");

            blip_.emplace(getContentProperty().Load<SoundEffect>("contentproj/Sounds/blip"));
            const double seconds = blip_->getDurationProperty().getTotalSecondsProperty();
            ctx.Print("blip: {:.3f} s", seconds);
            ctx.CheckNear(seconds, 0.15, 0.005, "the WAV item went through WavImporter and SoundEffectProcessor");

            // A <None> item with CopyToOutputDirectory is copied, not compiled.
            const std::unique_ptr<System::IO::Stream> credits =
                TitleContainer::OpenStream("assets/contentproj/credits.txt");
            std::string text(static_cast<std::size_t>(credits->getLengthProperty()), '\0');
            credits->Read(reinterpret_cast<std::uint8_t*>(text.data()), 0, static_cast<int>(text.size()));
            ctx.Print("credits.txt: {}", text.substr(0, text.find('\n')));
            ctx.Check(text.starts_with("cna-demos content project"), "the None item was copied beside the content");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(40, 44, 56));
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::AlphaBlend, &SamplerState::PointClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(*ship_, Rectangle(200, 140, 192, 192), Color::White);
            spriteBatch_->Draw(*grid_, Rectangle(440, 140, 192, 192), Color::White);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<Texture2D> ship_;
        std::optional<Texture2D> grid_;
        std::optional<SoundEffect> blip_;
    };
}

CNA_DEMOS_GAME(XnaContentProjectDemo, {
    .id = "content.content_project.xna_content_project",
    .title = "An XNA content project",
    .description = "Loads what CNA's pipeline built from an XNA Game Studio .contentproj: assets named by each "
                   "item's <Name>, the default colour key, a per-item processor parameter (mipmaps), a WAV through "
                   "SoundEffectProcessor, and a None item copied beside the XNB files.",
    .apis = {"ContentManager::Load", "ContentLoadException", "Texture::LevelCount", "Texture2D::GetData",
             "SoundEffect::Duration", "TitleContainer::OpenStream"},
    .tags = {"content", "contentproj", "xnb", "msbuild", "porting"},
    .needs = CnaDemos::Need::Audio,
    .frames = 3,
    .deterministic = true,
    .assets = {"contentproj/Textures/ship.xnb", "contentproj/Textures/grid.xnb", "contentproj/Sounds/blip.xnb",
               "contentproj/credits.txt"},
})
