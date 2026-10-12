// SPDX-License-Identifier: MS-PL
// Evolve a small XNB schema and replace its reader policy without discarding cached assets.
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "CNA/Platform/CurrentPlatform.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReaderManager.hpp"

using namespace Microsoft::Xna::Framework::Content;

namespace
{
    constexpr const char* ReaderName = "CnaDemos.VersionedStatsReader";
    struct Stats { int health = 0; float armor = 0; };
    struct ReaderPolicy { bool acceptLegacy = false; };
    int reads = 0;

    class StatsReader final : public ContentTypeReader<Stats>
    {
    public:
        explicit StatsReader(bool acceptLegacy) : ContentTypeReader<Stats>("CnaDemos.VersionedStats"), acceptLegacy_(acceptLegacy) {}
        int getTypeVersionProperty() const override { return 1; }
        bool SupportsVersion(int version) const override
        {
            // CNA's default gate is strict equality. Compatibility is an explicit reader policy.
            return acceptLegacy_ ? version == 0 || version == 1 : ContentTypeReader<Stats>::SupportsVersion(version);
        }
    protected:
        Stats Read(ContentReader& input, std::optional<Stats>) override
        {
            ++reads;
            const auto fields = input.ReadByte();
            if (fields > 1) throw ContentLoadException("Unknown stat fields");
            Stats stats;
            stats.health = input.ReadInt32();
            if (fields & 1) stats.armor = input.ReadSingle(); // v0 has no Armor; preserve its default
            return stats;
        }
    private:
        bool acceptLegacy_;
    };
    struct ScopedRegistration
    {
        ~ScopedRegistration() { ContentTypeReaderManager::RemoveTypeCreatorEXT(ReaderName); }
    };

    void BuiltinReaderLifetime(CnaDemos::DemoContext& ctx, const std::filesystem::path& root)
    {
        constexpr const char* builtin = "Microsoft.Xna.Framework.Content.Int32Reader";
        constexpr const char* asset = "versions/builtin_score";
        const auto refusesPrimitive = [&](ContentManager& manager) {
            try { (void)manager.Load<std::int32_t>(asset); }
            catch (const ContentLoadException& error)
            { ctx.Print("Withdrawn built-in refused: {}", error.what()); return true; }
            return false;
        };
        {
            // This demo runs in its own process. Withdrawing a built-in must not replace an
            // unknown application's factory or change the registry of the catalog/browser.
            Microsoft::Xna::Framework::Game firstGame;
            ContentManager cached(nullptr, root.string());
            ctx.Check(cached.Load<std::int32_t>(asset) == -12345,
                      "a Game's declared built-in reader loads the authored signed primitive value");
            ctx.Check(ContentTypeReaderManager::RemoveTypeCreatorEXT(builtin) &&
                      !ContentTypeReaderManager::IsRegistered(builtin) &&
                      !ContentTypeReaderManager::CreateReader(builtin),
                      "withdrawing the declared Int32 reader removes its factory and direct construction");
            ctx.Check(cached.Load<std::int32_t>(asset) == -12345,
                      "the already-cached primitive remains usable after its built-in reader is withdrawn");
            ContentManager uncached(nullptr, root.string());
            ctx.Check(refusesPrimitive(uncached) && !ContentTypeReaderManager::IsRegistered(builtin) &&
                      !ContentTypeReaderManager::CreateReader(builtin),
                      "an uncached typed load refuses the withdrawn built-in instead of reinstalling it");
            cached.Unload();
            ctx.Check(refusesPrimitive(cached),
                      "unloading the old cache exposes the same withdrawn-reader boundary on its next load");
        }
        {
            Microsoft::Xna::Framework::Game nextGame;
            ContentManager fresh(nullptr, root.string());
            // Load before querying the registry: a query could itself materialize declarations.
            ctx.Check(fresh.Load<std::int32_t>(asset) == -12345,
                      "a subsequent Game declares the built-in readers again and a fresh typed load succeeds");
        }
    }

    void ReaderVersions(CnaDemos::DemoContext& ctx)
    {
        if (!ctx.Check(!ContentTypeReaderManager::IsRegistered(ReaderName), "this demo's reader key starts unregistered")) return;
        const auto root = std::filesystem::path(CNA::Platform::GetCurrentPlatform().GetFileSystem()->GetBasePath()) / "assets";
        const auto rejectBeforeRead = [&](ContentManager& manager, const char* name, const char* expectation) {
            const int before = reads;
            bool refused = false;
            try { (void)manager.Load<Stats>(name); }
            catch (const ContentLoadException& error) { refused = true; ctx.Print("{} refused: {}", name, error.what()); }
            ctx.Check(refused && reads == before, expectation);
        };
        std::weak_ptr<const ReaderPolicy> finalPolicyLifetime;
        reads = 0;
        {
            ScopedRegistration cleanup; // withdraw only our own key, even if a later load throws
            auto strictPolicy = std::make_shared<const ReaderPolicy>(ReaderPolicy{false});
            std::weak_ptr<const ReaderPolicy> strictLifetime = strictPolicy;
            ContentTypeReaderManager::AddTypeCreator(ReaderName, [strictPolicy] {
                return std::make_unique<StatsReader>(strictPolicy->acceptLegacy);
            });
            strictPolicy.reset();
            ctx.Check(ContentTypeReaderManager::IsRegistered(ReaderName) && !strictLifetime.expired(),
                      "the registered factory owns its immutable policy after the caller releases it");
            const auto strictReader = ContentTypeReaderManager::CreateReader(ReaderName);
            if (!ctx.Check(strictReader && strictReader->getTypeVersionProperty() == 1 && strictReader->SupportsVersion(1) &&
                           !strictReader->SupportsVersion(0) && !strictReader->SupportsVersion(2),
                           "default strict policy supports only its declared reader version")) return;
            ContentManager first(nullptr, root.string());
            auto current = first.Load<Stats>("versions/current_stats");
            ctx.Check(current.health == 120 && current.armor == 25 && reads == 1,
                      "version one reads both authored Health and Armor fields");
            rejectBeforeRead(first, "versions/legacy_stats", "strict policy rejects the older schema before invoking Read");

            auto ignoredPolicy = std::make_shared<const ReaderPolicy>(ReaderPolicy{true});
            std::weak_ptr<const ReaderPolicy> ignoredLifetime = ignoredPolicy;
            ContentTypeReaderManager::AddTypeCreator(ReaderName, [ignoredPolicy] {
                return std::make_unique<StatsReader>(ignoredPolicy->acceptLegacy);
            }); // a repeated key keeps its original factory
            ignoredPolicy.reset();
            ctx.Check(ignoredLifetime.expired(), "an ignored duplicate registration does not retain the proposed factory's policy");
            ContentManager uncached(nullptr, root.string());
            rejectBeforeRead(uncached, "versions/legacy_stats", "a fresh manager still sees strict policy after duplicate registration");

            const bool removed = ContentTypeReaderManager::RemoveTypeCreatorEXT(ReaderName);
            ctx.Check(removed && !ContentTypeReaderManager::IsRegistered(ReaderName) &&
                      !ContentTypeReaderManager::CreateReader(ReaderName) && strictLifetime.expired() &&
                      strictReader->SupportsVersion(1) && !strictReader->SupportsVersion(0),
                      "withdrawing the factory removes lookup and releases its policy while an already-created reader survives");
            current = first.Load<Stats>("versions/current_stats");
            ctx.Check(current.health == 120 && current.armor == 25 && reads == 1,
                      "withdrawing a reader leaves the owner's already-cached asset usable without another read");
            rejectBeforeRead(uncached, "versions/current_stats", "a fresh load cannot construct an unregistered reader");
            ctx.Check(!ContentTypeReaderManager::RemoveTypeCreatorEXT(ReaderName), "withdrawing an already-absent key reports false");

            auto compatiblePolicy = std::make_shared<const ReaderPolicy>(ReaderPolicy{true});
            finalPolicyLifetime = compatiblePolicy;
            ContentTypeReaderManager::AddTypeCreator(ReaderName, [compatiblePolicy] {
                return std::make_unique<StatsReader>(compatiblePolicy->acceptLegacy);
            });
            compatiblePolicy.reset();
            const auto compatibleReader = ContentTypeReaderManager::CreateReader(ReaderName);
            if (!ctx.Check(compatibleReader && compatibleReader->getTypeVersionProperty() == 1 && compatibleReader->SupportsVersion(0) &&
                           compatibleReader->SupportsVersion(1) && !compatibleReader->SupportsVersion(2) && !finalPolicyLifetime.expired(),
                           "the replacement factory explicitly accepts legacy and current versions but refuses the future version")) return;
            ContentManager compatible(nullptr, root.string());
            const auto old = compatible.Load<Stats>("versions/legacy_stats");
            ctx.Check(old.health == 80 && old.armor == 0, "legacy content retains Health and the default Armor omitted by its schema");
            const auto now = compatible.Load<Stats>("versions/current_stats");
            ctx.Check(now.health == 120 && now.armor == 25, "the same replacement reader still preserves current-schema values");
            ctx.Check(reads == 3, "only the three successful uncached loads invoke the payload reader");
            rejectBeforeRead(compatible, "versions/future_stats", "compatibility refuses the unsupported future schema before reading fields");
        }
        ctx.Check(!ContentTypeReaderManager::IsRegistered(ReaderName) && finalPolicyLifetime.expired(),
                  "scoped withdrawal releases the final policy and removes its registered factory");
        BuiltinReaderLifetime(ctx, root);
    }
}

CNA_DEMOS_CONSOLE(ReaderVersions, {
    .id = "content.xnb.reader_versions",
    .title = "Replace a versioned XNB reader policy",
    .description = "Loads real legacy/current schemas under strict and compatible reader policies, checks duplicate registration, "
                   "withdrawal and cache independence, and owns factory policy captures until scoped cleanup. Also contrasts "
                   "withdrawn built-in readers with cached values, uncached loads and a subsequent Game declaration.",
    .apis = {"ContentTypeReader::TypeVersion", "ContentTypeReader::Read", "ContentTypeReaderManager::AddTypeCreator",
             "Microsoft::Xna::Framework::Content::ContentTypeReaderBase::SupportsVersion",
             "Microsoft::Xna::Framework::Content::ContentTypeReaderManager::RemoveTypeCreatorEXT",
             "Microsoft::Xna::Framework::Content::ContentTypeReaderManager::CreateReader",
             "Microsoft::Xna::Framework::Content::ContentTypeReaderManager::IsRegistered", "ContentManager::Load",
             "ContentManager::Unload", "Game::Game"},
    .tags = {"content", "xnb", "version", "reader", "registry", "lifetime"},
    .assets = {"versions/legacy_stats.xnb", "versions/current_stats.xnb", "versions/future_stats.xnb", "versions/builtin_score.xnb"},
    .restrictions = "CNA reader-version enforcement and factory lifetime; container version remains five. "
                    "A private process withdraws a built-in factory and creates two sequential Games without running them; "
                    "no rendering or hardware validation is claimed.",
    .ownProcess = true,
})
