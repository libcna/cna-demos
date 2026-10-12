// SPDX-License-Identifier: MS-PL
// CNAEXT content manifest: what is under the content root, and in which formats.
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <system_error>
#include <memory>
#include <map>
#include <set>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
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
    void ShowSnapshotRefresh(CnaDemos::DemoContext& ctx)
    {
        // Use a private directory to model a mod arriving after the initial scan.
        const auto path = std::filesystem::temp_directory_path() /
            ("cna-demos-manifest-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(path))
            throw std::runtime_error("Could not create a private content directory");
        struct Cleanup
        {
            std::filesystem::path path;
            ~Cleanup() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
        } cleanup{path};
        ContentManager mods(nullptr, path.string());
        ctx.Check(mods.GetContentManifest().empty(), "the first scan captures an empty content root");

        std::ofstream file;
        file.exceptions(std::ios::failbit | std::ios::badbit);
        file.open(path / "brightness.cnj");
        file << R"({"cnjVersion":1,"type":"Curve","keys":[{"position":0,"value":0.75}]})";
        file.close();
        ctx.Check(mods.GetContentManifest().empty(), "adding a file does not change the cached manifest");
        ctx.CheckNear(mods.Load<Curve>("brightness").Evaluate(0.0f), 0.75, 1e-6,
                      "Load finds the new file independently of the manifest snapshot");
        mods.RefreshContentManifest();
        const auto entries = mods.GetContentManifest();
        ctx.Check(entries.size() == 1 && entries.front().relativePath == "brightness" && entries.front().hasCnj,
                  "RefreshContentManifest discovers the new CNJ asset");
        std::filesystem::remove(path / "brightness.cnj");
        ctx.Check(mods.GetContentManifest().size() == 1, "removing a file also leaves the snapshot unchanged");
        mods.RefreshContentManifest();
        ctx.Check(mods.GetContentManifest().empty(), "refresh removes the deleted file from the manifest");
        ctx.CheckNear(mods.Load<Curve>("brightness").Evaluate(0.0f), 0.75, 1e-6,
                      "refreshing the manifest does not unload already cached assets");
        ctx.Print("Manifest snapshots: empty -> add file -> refresh -> one entry -> remove -> refresh -> empty");
    }

    class ManifestDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            ShowSnapshotRefresh(Context());
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            // The inventory is a file snapshot, but availability reflects the current process.
            // Loading and registry queries can make declared readers available; earlier demos
            // may already have registered them in a shared smoke run.
            const auto beforeLoad = getContentProperty().GetXnbReaderUsageSummary();
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
                // Keep long paths inside their column; the console below retains the full entry.
                std::string& label = lines_.back();
                if (font_->MeasureString(label).X * 0.8f > 380.0f)
                {
                    while (!label.empty() && font_->MeasureString(label + "...").X * 0.8f > 380.0f)
                        label.pop_back();
                    label += "...";
                }
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

            // Aggregate distinct reader names per file independently of the summary API.
            std::map<std::string, int> expectedCounts, beforeCounts, afterCounts;
            for (const auto& entry : entries)
                for (const auto& reader : std::set<std::string>(entry.xnbReaderNames.begin(), entry.xnbReaderNames.end()))
                    ++expectedCounts[reader];
            for (const auto& usage : beforeLoad)
                beforeCounts.emplace(usage.readerName, usage.fileCount);
            bool fontReader = false, textureReader = false;
            const auto afterLoad = getContentProperty().GetXnbReaderUsageSummary();
            for (const ContentManifestReaderUsage& usage : afterLoad)
            {
                afterCounts.emplace(usage.readerName, usage.fileCount);
                Context().Print("XNB reader {}: {} file(s), {}", usage.readerName, usage.fileCount,
                                usage.isRegistered ? "available" : "not registered yet");
                if (usage.readerName == "Microsoft.Xna.Framework.Content.SpriteFontReader")
                    fontReader = usage.isRegistered;
                if (usage.readerName == "Microsoft.Xna.Framework.Content.Texture2DReader")
                    textureReader = usage.isRegistered;
            }
            Context().Check(afterCounts == expectedCounts && afterLoad.size() == expectedCounts.size(),
                            "reader summary counts each referencing file once with no duplicate rows");
            Context().Check(beforeCounts == afterCounts,
                            "loading an asset does not change the manifest's reader inventory");
            Context().Check(fontReader && textureReader,
                            "the loaded SpriteFont has its font and texture reader family available");
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
                   "(GetXnbReaderUsageSummary). Demonstrates explicit refresh after files arrive or disappear and reader availability after typed loading.",
    .apis = {"ContentManager::GetContentManifest", "ContentManager::GetXnbReaderUsageSummary",
             "ContentManager::RefreshContentManifest", "ContentManager::Load",
             "ContentManifestEntry", "ContentManifestReaderUsage"},
    .tags = {"content", "manifest", "tools", "modding", "xnb", "cnaext"},
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb", "xnb/textures/colour_wheel.xnb", "textures/checker.png"},
})
