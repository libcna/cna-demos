// SPDX-License-Identifier: MS-PL
// A game's binary asset schema and its process-wide CNA loader.
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <stdexcept>
#include <system_error>
#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"

using namespace Microsoft::Xna::Framework::Content;
using namespace CNA::Content::Cnb;

namespace
{
    struct Rules { std::string name; float gravity; std::uint32_t lives; };
    struct CollisionAsset { std::uint8_t marker; };
    struct LoaderState { int reads = 0; };
    constexpr auto RulesChunk = MakeChunkId('R', 'U', 'L', 'E');
    constexpr auto CollisionChunk = MakeChunkId('M', 'A', 'R', 'K');
    constexpr const char* RulesName = "CnaDemos.GameRules";
    // These distinct names deliberately share the same 31-bit FNV-1a custom identifier. Keeping
    // the pair fixed makes the collision defense deterministic instead of doing a search at run time.
    constexpr const char* CollisionAlpha = "CnaDemos.CollisionAsset.AlphavbnQ8GeK";
    constexpr const char* CollisionBeta = "CnaDemos.CollisionAsset.Beta9nGwK9Ry";

    struct TemporaryContent
    {
        std::filesystem::path path;
        ~TemporaryContent()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    };

    void CnbLoader(CnaDemos::DemoContext& ctx)
    {
        const auto typeId = CnbAssetTypeIdFromName(RulesName);
        // Registration outlives every manager, so capture owned state rather than local references.
        auto firstState = std::make_shared<LoaderState>();
        const std::weak_ptr<LoaderState> retainedFirst = firstState;
        ContentManager::RegisterCnbLoaderEXT<Rules>(typeId, RulesName,
            [firstState](const CnbDocument& document, ContentManager&) {
                document.RequireAsset(CnbAssetTypeIdFromName(RulesName), 1);
                const std::array known{RulesChunk};
                document.RequireMandatoryChunksUnderstood(known);
                auto input = document.OpenChunk(document.RequireSingle(RulesChunk));
                Rules result{input.ReadString(), input.ReadF32(), input.ReadU32()};
                input.RequireExhausted();
                ++firstState->reads;
                return result;
            });

        bool emptyFactoryRefused = false;
        try
        {
            ContentManager::RegisterCnbLoaderEXT<Rules>(
                typeId, RulesName, ContentManager::CnbLoaderFn<Rules>{});
        }
        catch (const std::invalid_argument&) { emptyFactoryRefused = true; }
        ctx.Check(emptyFactoryRefused && !retainedFirst.expired(),
                  "an empty factory is refused without releasing the installed registration");

        bool builtInIdRefused = false;
        bool reservedIdRefused = false;
        const auto placeholder = [](const CnbDocument&, ContentManager&) { return Rules{}; };
        try
        {
            ContentManager::RegisterCnbLoaderEXT<Rules>(
                CnbAssetTypeId::Curve, RulesName, placeholder);
        }
        catch (const std::invalid_argument&) { builtInIdRefused = true; }
        try
        {
            ContentManager::RegisterCnbLoaderEXT<Rules>(
                CnbAssetTypeId::ReservedRangeFirst, RulesName, placeholder);
        }
        catch (const std::invalid_argument&) { reservedIdRefused = true; }
        ctx.Check(builtInIdRefused && reservedIdRefused && !retainedFirst.expired(),
                  "game loaders cannot claim CNA built-in or reserved asset identifiers");

        bool mismatchedNameRefused = false;
        try
        {
            ContentManager::RegisterCnbLoaderEXT<Rules>(
                typeId, "CnaDemos.OtherRules", placeholder);
        }
        catch (const std::invalid_argument&) { mismatchedNameRefused = true; }
        ctx.Check(mismatchedNameRefused && !retainedFirst.expired(),
                  "a custom identifier refuses a canonical name that hashes to another value");

        auto replacementState = std::make_shared<LoaderState>();
        const std::weak_ptr<LoaderState> ignoredReplacement = replacementState;
        ContentManager::RegisterCnbLoaderEXT<Rules>(typeId, RulesName,
            [replacementState](const CnbDocument&, ContentManager&) {
                ++replacementState->reads;
                return Rules{"Replacement", -1.0f, 99};
            });
        firstState.reset();
        replacementState.reset();
        ctx.Check(!retainedFirst.expired() && ignoredReplacement.expired(),
                  "repeat registration retains the first factory capture and releases the ignored replacement");
        const auto ReadCount = [&retainedFirst] {
            const auto state = retainedFirst.lock();
            return state ? state->reads : -1;
        };

        // A real game writes this at build time. Generate a private temporary asset here so the
        // schema, writer and matching runtime reader are visible together in one focused demo.
        auto path = std::filesystem::temp_directory_path() /
            ("cna-demos-rules-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(path))
            throw std::runtime_error("Could not create a private content directory");
        TemporaryContent temporary{path};
        CnbByteWriter payload;
        payload.WriteString("Moon");
        payload.WriteF32(1.62f);
        payload.WriteU32(3);
        const auto bytes = payload.Take();

        // A custom file must identify the exact canonical type behind its 31-bit identifier. The
        // writer catches missing metadata and an ordinary wrong hash before publishing a file.
        bool missingNameRefused = false, wrongHashRefused = false;
        CnbWriter unnamed(typeId, 1);
        unnamed.AddChunk(RulesChunk, bytes, CnbChunkFlags::Mandatory);
        try { (void)unnamed.Build(); }
        catch (const ContentLoadException& error)
        {
            missingNameRefused = true;
            ctx.Print("Unnamed custom file refused: {}", error.what());
        }
        CnbWriter wrongHash(typeId, 1);
        wrongHash.SetMetadata("CnaDemos.OtherRules", "wrong_hash");
        wrongHash.AddChunk(RulesChunk, bytes, CnbChunkFlags::Mandatory);
        try { (void)wrongHash.Build(); }
        catch (const ContentLoadException& error)
        {
            wrongHashRefused = true;
            ctx.Print("Wrong custom name refused: {}", error.what());
        }
        ctx.Check(missingNameRefused && wrongHashRefused,
                  "the writer refuses a custom file with missing or differently hashed canonical metadata");

        CnbWriter writer(typeId, 1);
        writer.SetMetadata(RulesName, "rules");
        writer.AddChunk(RulesChunk, bytes, CnbChunkFlags::Mandatory);
        const auto validImage = writer.Build();
        const auto validDocument = CnbDocument::Parse(validImage, "rules identity probe");
        bool wrongTypeRefused = false;
        try { validDocument.RequireAsset(CnbAssetTypeIdFromName("CnaDemos.OtherRules"), 1); }
        catch (const ContentLoadException& error)
        {
            wrongTypeRefused = true;
            ctx.Print("Wrong requested asset type refused: {}", error.what());
        }
        ctx.Check(wrongTypeRefused,
                  "RequireAsset rejects a valid document when the decoder requests another asset type");
        writer.WriteToFile((path / "rules.cnb").string());

        // A real hash collision passes the writer's hash check, so dispatch must compare the full
        // canonical name before invoking a candidate factory. Repair the same path afterwards to
        // prove the rejected file did not enter the manager cache.
        const auto collisionId = CnbAssetTypeIdFromName(CollisionAlpha);
        ctx.Check(std::string(CollisionAlpha) != CollisionBeta &&
                      collisionId == CnbAssetTypeIdFromName(CollisionBeta),
                  "two distinct canonical names demonstrate a real custom identifier collision");
        auto collisionReads = std::make_shared<int>(0);
        ContentManager::RegisterCnbLoaderEXT<CollisionAsset>(
            collisionId, CollisionAlpha,
            [collisionReads, collisionId](const CnbDocument& document, ContentManager&) {
                document.RequireAsset(collisionId, 1);
                const auto payload = document.ChunkData(document.RequireSingle(CollisionChunk));
                ++*collisionReads;
                return CollisionAsset{payload.empty() ? std::uint8_t{0} : payload.front()};
            });
        CnbWriter colliding(collisionId, 1);
        colliding.SetMetadata(CollisionBeta, "collision");
        colliding.AddChunk(CollisionChunk, {42}, CnbChunkFlags::Mandatory);
        colliding.WriteToFile((path / "collision.cnb").string());

        ContentManager content(nullptr, path.string());
        bool collisionRefused = false;
        try { (void)content.Load<CollisionAsset>("collision"); }
        catch (const ContentLoadException& error)
        {
            const std::string diagnostic = error.what();
            collisionRefused = diagnostic.find(CollisionAlpha) != std::string::npos &&
                               diagnostic.find(CollisionBeta) != std::string::npos &&
                               diagnostic.find("collid") != std::string::npos;
            ctx.Print("Colliding custom type refused: {}", diagnostic);
        }
        ctx.Check(collisionRefused && *collisionReads == 0,
                  "dispatch rejects a colliding canonical name before the candidate factory runs");
        CnbWriter repairedCollision(collisionId, 1);
        repairedCollision.SetMetadata(CollisionAlpha, "collision");
        repairedCollision.AddChunk(CollisionChunk, {42}, CnbChunkFlags::Mandatory);
        repairedCollision.WriteToFile((path / "collision.cnb").string());
        const CollisionAsset collisionAsset = content.Load<CollisionAsset>("collision");
        ctx.Check(collisionAsset.marker == 42 && *collisionReads == 1,
                  "the same manager loads the repaired canonical name after collision refusal");

        const Rules rules = content.Load<Rules>("rules");
        ctx.Print("{}: gravity {}, lives {}", rules.name, rules.gravity, rules.lives);
        ctx.Check(rules.name == "Moon" && rules.lives == 3 && ReadCount() == 1,
                  "the first registered factory wins and its string and integer fields round trip");
        ctx.CheckNear(rules.gravity, 1.62, 1e-6, "custom float field round trips");
        (void)content.Load<Rules>("rules");
        ctx.Check(ReadCount() == 1, "a second Load uses the manager's cache");
        {
            ContentManager another(nullptr, path.string());
            (void)another.Load<Rules>("rules");
            ctx.Check(ReadCount() == 2, "registration is shared but caches belong to each manager");
        }
        ContentManager replacementManager(nullptr, path.string());
        const Rules afterManagerDestruction = replacementManager.Load<Rules>("rules");
        ctx.Check(afterManagerDestruction.name == "Moon" && ReadCount() == 3,
                  "the retained first factory remains live after a manager is destroyed and replaced");

        // A newer writer can add optional information without changing the known RULE schema.
        // The same extension marked mandatory must be refused: silently ignoring it would
        // change the meaning of the asset. No internal container code is needed for this policy.
        constexpr auto ExtensionChunk = MakeChunkId('N', 'E', 'W', '!');
        CnbWriter optional(typeId, 1);
        optional.SetMetadata(RulesName, "optional");
        optional.AddChunk(RulesChunk, bytes, CnbChunkFlags::Mandatory);
        optional.AddChunk(ExtensionChunk, {42}, CnbChunkFlags::None);
        optional.WriteToFile((path / "optional.cnb").string());
        const Rules extended = content.Load<Rules>("optional");
        ctx.Check(extended.name == rules.name && extended.gravity == rules.gravity && extended.lives == rules.lives,
                  "an unknown optional chunk leaves the understood rules intact");

        CnbWriter required(typeId, 1);
        required.SetMetadata(RulesName, "required");
        required.AddChunk(RulesChunk, bytes, CnbChunkFlags::Mandatory);
        required.AddChunk(ExtensionChunk, {42}, CnbChunkFlags::Mandatory);
        required.WriteToFile((path / "required.cnb").string());
        const int readsBeforeFailure = ReadCount();
        bool mandatoryRejected = false;
        try { (void)content.Load<Rules>("required"); }
        catch (const ContentLoadException& error)
        {
            mandatoryRejected = true;
            ctx.Print("Required extension refused: {}", error.what());
        }
        ctx.Check(mandatoryRejected && ReadCount() == readsBeforeFailure,
                  "an unknown mandatory chunk is rejected before decoding the rules");

        // Repair the same asset on disk and retry using the same manager, without Unload.
        // Failed loads must not leave a cached, partially initialized game object behind.
        writer.WriteToFile((path / "required.cnb").string());
        const Rules repaired = content.Load<Rules>("required");
        ctx.Check(repaired.name == rules.name && repaired.lives == rules.lives && ReadCount() == readsBeforeFailure + 1,
                  "the same manager retries a failed asset after its file is repaired");

        // The writer validates the CNB container, not our game-owned RULE schema. These
        // files have valid checksums/tables yet cannot describe one complete Rules value.
        // Keep the failure at the typed-loader boundary, before a result enters the cache.
        for (const std::string defect : {"missing", "duplicate", "truncated", "trailing"})
        {
            CnbWriter malformed(typeId, 1);
            malformed.SetMetadata(RulesName, defect);
            auto badBytes = bytes;
            if (defect == "truncated") badBytes.pop_back(); // only three bytes of lives
            if (defect == "trailing") badBytes.push_back(42); // undocumented schema field
            if (defect != "missing")
                malformed.AddChunk(RulesChunk, badBytes, CnbChunkFlags::Mandatory);
            if (defect == "duplicate")
                malformed.AddChunk(RulesChunk, bytes, CnbChunkFlags::Mandatory);
            const auto filename = (path / (defect + ".cnb")).string();
            malformed.WriteToFile(filename);
            // Successful ParseFile proves the later refusal is schema-level, not a bad CRC.
            const auto document = CnbDocument::ParseFile(filename);
            const auto count = document.FindAll(RulesChunk).size();
            const std::size_t expectedCount = defect == "missing" ? 0 : defect == "duplicate" ? 2 : 1;
            const int completedBefore = ReadCount();
            bool refused = false;
            try { (void)content.Load<Rules>(defect); }
            catch (const ContentLoadException& error)
            {
                const std::string diagnostic = error.what();
                refused = diagnostic.find(defect + ".cnb") != std::string::npos &&
                          diagnostic.find("RULE") != std::string::npos;
                ctx.Print("{} schema refused: {}", defect, diagnostic);
            }
            ctx.Check(count == expectedCount && refused && ReadCount() == completedBefore,
                      defect + " RULE data is rejected with file/chunk context before returning a value");
        }

        CnbWriter future(typeId, 2);
        future.SetMetadata(RulesName, "future");
        future.AddChunk(RulesChunk, bytes, CnbChunkFlags::Mandatory);
        future.WriteToFile((path / "future.cnb").string());
        bool rejected = false;
        try { (void)content.Load<Rules>("future"); }
        catch (const ContentLoadException& error)
        {
            rejected = true;
            ctx.Print("Newer schema refused: {}", error.what());
        }
        ctx.Check(rejected, "the loader refuses an unsupported schema instead of misreading it");
    }
}

CNA_DEMOS_CONSOLE(CnbLoader, {
    .id = "content.custom.cnb_loader",
    .title = "A custom CNB asset schema (CNAEXT)",
    .description = "Writes a game-rules CNB asset, checks canonical custom-type identity and collision refusal, "
                   "first-wins registration, caching, extensions, retry after repair and invalid schema layouts.",
    .apis = {"ContentManager::RegisterCnbLoaderEXT", "ContentManager::Load",
             "CNA::Content::Cnb::CnbAssetTypeIdFromName", "CNA::Content::Cnb::CnbWriter::SetMetadata",
             "CNA::Content::Cnb::CnbWriter::AddChunk", "CNA::Content::Cnb::CnbWriter::WriteToFile",
             "CNA::Content::Cnb::CnbDocument::RequireAsset", "CNA::Content::Cnb::CnbDocument::OpenChunk",
             "CNA::Content::Cnb::CnbDocument::ParseFile", "CNA::Content::Cnb::CnbDocument::FindAll",
             "CNA::Content::Cnb::CnbDocument::RequireSingle", "CNA::Content::Cnb::CnbByteReader::RequireExhausted",
             "CNA::Content::Cnb::CnbDocument::RequireMandatoryChunksUnderstood"},
    .tags = {"content", "custom", "cnb", "cnaext"},
    .restrictions = "Process-global custom registration is isolated in a child process; concurrent registration is not exercised.",
    .ownProcess = true,
})
