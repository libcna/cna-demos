// SPDX-License-Identifier: MS-PL
// CNAEXT content manifest: what is under the content root, and in which formats.
#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManifestEntry.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Content;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ManifestDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // One entry per asset name, with every form found for it: an .xnb, a .cnj, or native
            // files (.cnb, .png, .wav, ...). Useful for tools, mod loaders and content checks.
            // The content pipeline's own bookkeeping files are listed too (CNA-DEMOS-ISSUE-013).
            std::vector<ContentManifestEntry> entries = getContentProperty().GetContentManifest();
            std::sort(entries.begin(), entries.end(), [](const ContentManifestEntry& a, const ContentManifestEntry& b) {
                return a.relativePath < b.relativePath;
            });
            const auto find = [&entries](const std::string& name) {
                return std::find_if(entries.begin(), entries.end(),
                                    [&name](const ContentManifestEntry& e) { return e.relativePath == name; });
            };
            for (const ContentManifestEntry& entry : entries)
            {
                std::string forms = entry.hasXnb ? ".xnb" : "";
                if (entry.hasCnj)
                    forms += " .cnj";
                for (const std::string& extension : entry.nativeExtensions)
                    forms += (forms.empty() ? "" : " ") + extension;
                lines_.push_back(entry.relativePath + " " + forms);
                Context().Print("{:<36} {}", entry.relativePath, forms);
            }

            const auto font = find("fonts/pixel_font");
            Context().Check(font != entries.end() &&
                                std::ranges::find(font->nativeExtensions, ".cnb") != font->nativeExtensions.end(),
                            "the compiled font is listed as a .cnb asset");
            const auto wheel = find("xnb/textures/colour_wheel");
            Context().Check(wheel != entries.end() && wheel->hasXnb, "an XNB asset is listed with hasXnb");
            Context().Check(find("textures/checker") != entries.end(),
                            "loose files are listed by asset name, without extension");

            // For .xnb files: which XNA type readers they need, and whether this process has them.
            for (const ContentManifestReaderUsage& usage : getContentProperty().GetXnbReaderUsageSummary())
            {
                Context().Print("XNB reader {}: {} file(s), {}", usage.readerName, usage.fileCount,
                                usage.isRegistered ? "available" : "NOT available");
                // XNA's own readers must all be there; a game's own reader (levels/castle.xnb names
                // CnaDemos.LevelReader) is available once the game registers it.
                if (usage.readerName.starts_with("Microsoft.Xna.Framework."))
                {
                    Context().Check(usage.isRegistered, "CNA has the reader " + usage.readerName);
                }
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // Two columns of entries.
            const std::size_t perColumn = 22;
            for (std::size_t i = 0; i < lines_.size() && i < perColumn * 2; ++i)
                spriteBatch_->DrawString(*font_, lines_[i],
                                         Vector2(i < perColumn ? 10.0f : 405.0f,
                                                 10.0f + 21.0f * static_cast<float>(i % perColumn)),
                                         Color::White, 0.0f, Vector2::Zero, 0.8f, SpriteEffects::None, 0.0f);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(ManifestDemo, {
    .id = "content.content_manager.manifest",
    .title = "Content manifest (CNAEXT)",
    .description = "Lists every asset below Game::Content's root with ContentManager::GetContentManifest -- XNB, CNJ "
                   "and native forms per asset name -- and the XNA type readers the .xnb files need "
                   "(GetXnbReaderUsageSummary).",
    .apis = {"ContentManager::GetContentManifest", "ContentManager::GetXnbReaderUsageSummary",
             "ContentManifestEntry", "ContentManifestReaderUsage"},
    .tags = {"content", "manifest", "tools", "modding", "xnb", "cnaext"},
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb", "xnb/textures/colour_wheel.xnb", "textures/checker.png"},
})
