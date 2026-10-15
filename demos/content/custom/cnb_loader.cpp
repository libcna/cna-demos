// SPDX-License-Identifier: MS-PL
// A game's binary asset schema and its process-wide CNA loader.
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <vector>
#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbLoaderRegistry.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"

using namespace Microsoft::Xna::Framework::Content;
using CNA::Content::CnbLoaderRegistry;
using namespace CNA::Content::Cnb;

namespace
{
    struct Rules { std::string name; float gravity; std::uint32_t lives; };
    struct CollisionAsset { std::uint8_t marker; };
    struct LoaderState { int reads = 0; };
    struct ConcurrentAsset { std::uint32_t registration; std::uint32_t payload; };
    struct ConcurrentLoaderState
    {
        std::uint32_t registration;
        std::atomic<int> reads{0};
    };
    constexpr auto RulesChunk = MakeChunkId('R', 'U', 'L', 'E');
    constexpr auto CollisionChunk = MakeChunkId('M', 'A', 'R', 'K');
    constexpr auto ConcurrentChunk = MakeChunkId('R', 'A', 'C', 'E');
    constexpr const char* RulesName = "CnaDemos.GameRules";
    constexpr const char* ConcurrentName = "CnaDemos.ConcurrentRules";
    // These distinct names deliberately share the same 31-bit FNV-1a custom identifier. Keeping
    // the pair fixed makes the collision defense deterministic instead of doing a search at run time.
    constexpr const char* CollisionAlpha = "CnaDemos.CollisionAsset.AlphavbnQ8GeK";
    constexpr const char* CollisionBeta = "CnaDemos.CollisionAsset.Beta9nGwK9Ry";

    template <typename Exception, typename Action>
    bool RefusesAs(Action&& action)
    {
        try { action(); }
        catch (const Exception&) { return true; }
        return false;
    }

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
        ctx.Check(Format::Magic == std::array<std::uint8_t, 4>{0x43u, 0x4Eu, 0x42u, 0x1Au} &&
                      Format::HeaderSize == 64u && Format::TocEntrySize == 48u &&
                      Format::HeaderChecksumCoverage == 44u &&
                      Format::HeaderChecksumOffset == 44u &&
                      Format::HeaderReservedSize == 16u && Format::ContainerMajor == 1u &&
                      Format::ContainerMinor == 0u && Format::DefaultTocOffset == 64u,
                  "the container magic, layout sizes, checksum range, version and default table offset are frozen");

        constexpr CnbChunkId exactRule = MakeChunkId('R', 'U', 'L', 'E');
        constexpr CnbChunkId malformedId{0xFF7E0A41u};
        ctx.Check(exactRule == RulesChunk && exactRule.value == 0x454C5552u &&
                      ChunkIdToString(exactRule) == "RULE" && IsWellFormedChunkId(exactRule) &&
                      ChunkIdToString(malformedId) == "A?~?" &&
                      !IsWellFormedChunkId(malformedId),
                  "chunk IDs pack little-endian, compare by value and sanitize malformed bytes in diagnostics");

        ctx.Check(CnbChunkFlags::None == 0u && CnbChunkFlags::Mandatory == 1u &&
                      CnbChunkFlags::KnownMask == CnbChunkFlags::Mandatory &&
                      static_cast<std::uint32_t>(CnbCompression::None) == 0u &&
                      static_cast<std::uint32_t>(CnbCompression::Lz4) == 1u &&
                      static_cast<std::uint32_t>(CnbCompression::Zstd) == 2u &&
                      static_cast<std::uint32_t>(CnbCompression::Deflate) == 3u &&
                      CnbCompression::ReservedLz4 == CnbCompression::Lz4 &&
                      CnbCompression::ReservedZstd == CnbCompression::Zstd &&
                      CnbCompression::ReservedDeflate == CnbCompression::Deflate,
                  "chunk flags and every assigned or compatibility compression identifier keep their wire values");

        struct AssetTypeExpectation
        {
            std::uint32_t id;
            const char* name;
        };
        constexpr std::array assetTypes{
            AssetTypeExpectation{CnbAssetTypeId::Invalid, "Invalid"},
            AssetTypeExpectation{CnbAssetTypeId::Texture2D, "Texture2D"},
            AssetTypeExpectation{CnbAssetTypeId::Texture3D, "Texture3D"},
            AssetTypeExpectation{CnbAssetTypeId::TextureCube, "TextureCube"},
            AssetTypeExpectation{CnbAssetTypeId::SpriteFont, "SpriteFont"},
            AssetTypeExpectation{CnbAssetTypeId::Model, "Model"},
            AssetTypeExpectation{CnbAssetTypeId::AnimationClip, "AnimationClip"},
            AssetTypeExpectation{CnbAssetTypeId::Curve, "Curve"},
            AssetTypeExpectation{CnbAssetTypeId::SoundEffect, "SoundEffect"},
            AssetTypeExpectation{CnbAssetTypeId::Song, "Song"},
            AssetTypeExpectation{CnbAssetTypeId::Video, "Video"},
            AssetTypeExpectation{CnbAssetTypeId::Effect, "Effect"},
        };
        bool exactAssetTypes = true;
        for (std::size_t index = 0u; index < assetTypes.size(); ++index)
        {
            exactAssetTypes = exactAssetTypes && assetTypes[index].id == index &&
                AssetTypeIdToString(assetTypes[index].id) == assetTypes[index].name;
        }
        ctx.Check(exactAssetTypes && CnbAssetTypeId::ReservedRangeFirst == 0x40000000u &&
                      CnbAssetTypeId::CustomRangeFirst == 0x80000000u &&
                      !IsCustomAssetTypeId(CnbAssetTypeId::ReservedRangeFirst) &&
                      IsCustomAssetTypeId(CnbAssetTypeId::CustomRangeFirst) &&
                      AssetTypeIdToString(0x40000001u) == "unknown type 0x40000001" &&
                      AssetTypeIdToString(0x80000001u) == "custom type 0x80000001",
                  "every built-in asset ID/name and both reserved/custom range boundaries are exact");

        bool emptyTypeNameRefused = false;
        try
        {
            (void)CnbAssetTypeIdFromName("");
        }
        catch (const std::invalid_argument&)
        {
            emptyTypeNameRefused = true;
        }
        ctx.Check(CnbAssetTypeIdFromName(RulesName) == 0xE7632F12u &&
                      IsCustomAssetTypeId(CnbAssetTypeIdFromName(RulesName)) &&
                      emptyTypeNameRefused,
                  "custom type hashing is deterministic, sets the custom bit and refuses an empty canonical name");

        const std::string invalidUtf8{"\xC0\xAF", 2u};
        ctx.Check(CnbLogicalNameProblem("textures/hero") == "" &&
                      CnbLogicalNameProblem("textures/a..b") == "" &&
                      CnbLogicalNameProblem("") == "is empty" &&
                      CnbLogicalNameProblem(invalidUtf8) == "is not well-formed UTF-8" &&
                      !CnbLogicalNameProblem("textures\\hero").empty() &&
                      CnbLogicalNameProblem("/textures/hero") == "is an absolute path" &&
                      CnbLogicalNameProblem("C:textures/hero") ==
                          "is a drive-qualified absolute path" &&
                      CnbLogicalNameProblem("textures/../hero") == "contains a '..' segment",
                  "the shared logical-name rule accepts contained names and classifies every unsafe form");

        ctx.Check(CnbContainerChunk::Metadata == MakeChunkId('C', 'M', 'E', 'T') &&
                      CnbContainerChunk::ExternalReferences == MakeChunkId('X', 'R', 'E', 'F'),
                  "the two container-owned chunk identifiers are exact and distinct from schema chunks");

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

        // The registry promises that every accessor is safe across threads and that equivalent
        // repeat registrations retain the first loader. Release several public registration
        // calls from one gate so that this covers the contended path rather than serial setup.
        constexpr std::size_t ConcurrentRegistrations = 8;
        const auto concurrentTypeId = CnbAssetTypeIdFromName(ConcurrentName);
        std::vector<std::shared_ptr<ConcurrentLoaderState>> concurrentStates;
        std::vector<std::weak_ptr<ConcurrentLoaderState>> concurrentWeak;
        concurrentStates.reserve(ConcurrentRegistrations);
        concurrentWeak.reserve(ConcurrentRegistrations);
        for (std::size_t index = 0; index < ConcurrentRegistrations; ++index)
        {
            auto state = std::make_shared<ConcurrentLoaderState>();
            state->registration = static_cast<std::uint32_t>(index + 1);
            concurrentWeak.push_back(state);
            concurrentStates.push_back(std::move(state));
        }
        std::mutex registrationGateMutex;
        std::condition_variable registrationGateChanged;
        std::size_t registrationsReady = 0;
        bool releaseRegistrations = false;
        std::atomic<int> registrationFailures{0};
        std::vector<std::thread> registrationThreads;
        registrationThreads.reserve(ConcurrentRegistrations);
        for (const auto& state : concurrentStates)
        {
            registrationThreads.emplace_back([&, state] {
                {
                    std::unique_lock gateLock(registrationGateMutex);
                    ++registrationsReady;
                    registrationGateChanged.notify_all();
                    registrationGateChanged.wait(gateLock, [&] { return releaseRegistrations; });
                }
                try
                {
                    ContentManager::RegisterCnbLoaderEXT<ConcurrentAsset>(
                        concurrentTypeId, ConcurrentName,
                        [state, concurrentTypeId](const CnbDocument& document, ContentManager&) {
                            document.RequireAsset(concurrentTypeId, 1);
                            auto input = document.OpenChunk(document.RequireSingle(ConcurrentChunk));
                            const std::uint32_t payload = input.ReadU32();
                            input.RequireExhausted();
                            ++state->reads;
                            return ConcurrentAsset{state->registration, payload};
                        });
                }
                catch (...) { ++registrationFailures; }
            });
        }
        bool allRegistrationsReady = false;
        {
            std::unique_lock gateLock(registrationGateMutex);
            allRegistrationsReady = registrationGateChanged.wait_for(
                gateLock, std::chrono::seconds(5),
                [&] { return registrationsReady == ConcurrentRegistrations; });
            releaseRegistrations = true;
        }
        registrationGateChanged.notify_all();
        for (std::thread& registration : registrationThreads) registration.join();
        registrationThreads.clear();
        concurrentStates.clear();
        std::shared_ptr<ConcurrentLoaderState> retainedConcurrent;
        std::size_t retainedConcurrentCount = 0;
        for (const auto& weak : concurrentWeak)
        {
            if (auto state = weak.lock())
            {
                ++retainedConcurrentCount;
                retainedConcurrent = std::move(state);
            }
        }
        ctx.Check(allRegistrationsReady && registrationFailures == 0,
                  "all eight synchronized registrations of one custom type complete safely");
        ctx.Check(retainedConcurrentCount == 1 && retainedConcurrent,
                  "concurrent equivalent registrations retain exactly one factory capture");

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
        CnbByteWriter concurrentPayload;
        concurrentPayload.WriteU32(77);
        CnbWriter concurrentWriter(concurrentTypeId, 1);
        concurrentWriter.SetMetadata(ConcurrentName, "concurrent");
        concurrentWriter.AddChunk(ConcurrentChunk, concurrentPayload.Take(), CnbChunkFlags::Mandatory);
        concurrentWriter.WriteToFile((path / "concurrent.cnb").string());

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
        const ConcurrentAsset concurrent = content.Load<ConcurrentAsset>("concurrent");
        ctx.Check(retainedConcurrent && concurrent.registration == retainedConcurrent->registration &&
                      concurrent.payload == 77 && retainedConcurrent->reads == 1,
                  "the retained concurrent-registration winner decodes the real asset exactly once");
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

        constexpr std::array<std::uint32_t, 10> BuiltInIds{
            CnbAssetTypeId::Curve, CnbAssetTypeId::AnimationClip, CnbAssetTypeId::Model,
            CnbAssetTypeId::Texture2D, CnbAssetTypeId::TextureCube, CnbAssetTypeId::Texture3D,
            CnbAssetTypeId::SpriteFont, CnbAssetTypeId::SoundEffect,
            CnbAssetTypeId::Song, CnbAssetTypeId::Video};
        constexpr std::array<const char*, 10> BuiltInNames{
            "Microsoft.Xna.Framework.Curve",
            "Microsoft.Xna.Framework.Graphics.AnimationClipEXT",
            "Microsoft.Xna.Framework.Graphics.Model",
            "Microsoft.Xna.Framework.Graphics.Texture2D",
            "Microsoft.Xna.Framework.Graphics.TextureCube",
            "Microsoft.Xna.Framework.Graphics.Texture3D",
            "Microsoft.Xna.Framework.Graphics.SpriteFont",
            "Microsoft.Xna.Framework.Audio.SoundEffect",
            "Microsoft.Xna.Framework.Media.Song",
            "Microsoft.Xna.Framework.Media.Video"};
        bool declaredBuiltIns = true;
        for (std::size_t index = 0; index < BuiltInIds.size(); ++index)
        {
            declaredBuiltIns = declaredBuiltIns &&
                CnbLoaderRegistry::IsRegistered(BuiltInIds[index]) &&
                CnbLoaderRegistry::RegisteredTypeName(BuiltInIds[index]) == BuiltInNames[index] &&
                CnbLoaderRegistry::Find(BuiltInIds[index]).has_value();
        }
        ctx.Check(declaredBuiltIns,
                  "a ContentManager declares all ten built-in loader IDs, exact names and lookup copies");

        const auto resolvedWrapper = CnbLoaderRegistry::ResolveForDocument(validDocument);
        const Rules directlyResolved = std::any_cast<Rules>(
            resolvedWrapper(validDocument, content, "direct wrapper resolution"));
        ctx.Check(CnbLoaderRegistry::IsRegistered(typeId) &&
                      CnbLoaderRegistry::RegisteredTypeName(typeId) == RulesName &&
                      directlyResolved.name == "Moon" && ReadCount() == readsBeforeFailure + 2,
                  "direct registry resolution returns the process-wide custom loader by value");

        const auto retainedLoader = CnbLoaderRegistry::Find(typeId);
        const bool firstRemoval = CnbLoaderRegistry::Remove(typeId);
        const bool secondRemoval = CnbLoaderRegistry::Remove(typeId);
        const Rules afterRemoval = std::any_cast<Rules>(
            (*retainedLoader)(validDocument, content, "retained lookup copy"));
        ctx.Check(firstRemoval && !secondRemoval && !CnbLoaderRegistry::IsRegistered(typeId) &&
                      !CnbLoaderRegistry::Find(typeId).has_value() &&
                      CnbLoaderRegistry::RegisteredTypeName(typeId).empty() &&
                      afterRemoval.name == "Moon",
                  "Remove reports exact state while a previously returned loader copy stays callable");
        ctx.Check(RefusesAs<ContentLoadException>([&] {
                      (void)CnbLoaderRegistry::ResolveForDocument(validDocument);
                  }),
                  "document resolution refuses a custom asset after its loader is removed");

        CnbLoaderRegistry::Clear();
        bool clearRemovedEverything = !CnbLoaderRegistry::IsRegistered(collisionId);
        for (const auto id : BuiltInIds)
            clearRemovedEverything = clearRemovedEverything && !CnbLoaderRegistry::IsRegistered(id);
        ctx.Check(clearRemovedEverything && !CnbLoaderRegistry::Remove(typeId),
                  "Clear withdraws custom and declared built-in registrations for process isolation");

        CnbLoaderRegistry::LoaderFn emptyLoader;
        const CnbLoaderRegistry::LoaderFn placeholderLoader =
            [](const CnbDocument&, ContentManager&, const std::string&) -> std::any {
                return Rules{};
            };
        ctx.Check(RefusesAs<std::invalid_argument>([&] {
                      CnbLoaderRegistry::Register(CnbAssetTypeId::Invalid, RulesName, placeholderLoader);
                  }) &&
                      RefusesAs<std::invalid_argument>([&] {
                          CnbLoaderRegistry::Register(CnbAssetTypeId::Curve, RulesName, placeholderLoader);
                      }) &&
                      RefusesAs<std::invalid_argument>([&] {
                          CnbLoaderRegistry::Register(CnbAssetTypeId::ReservedRangeFirst,
                                                      RulesName, placeholderLoader);
                      }) &&
                      RefusesAs<std::invalid_argument>([&] {
                          CnbLoaderRegistry::Register(typeId, "", placeholderLoader);
                      }) &&
                      RefusesAs<std::invalid_argument>([&] {
                          CnbLoaderRegistry::Register(typeId, "CnaDemos.OtherRules", placeholderLoader);
                      }) &&
                      RefusesAs<std::invalid_argument>([&] {
                          CnbLoaderRegistry::Register(typeId, RulesName, emptyLoader);
                      }),
                  "direct registration refuses invalid, CNA-owned, unnamed, mismatched and empty-loader inputs");

        auto directCalls = std::make_shared<int>(0);
        auto directAssetName = std::make_shared<std::string>();
        CnbLoaderRegistry::Register(
            typeId, RulesName,
            [directCalls, directAssetName](const CnbDocument& document, ContentManager&,
                                            const std::string& assetName) -> std::any {
                ++*directCalls;
                *directAssetName = assetName;
                return Rules{document.Metadata().contentName, 2.5f,
                             document.AssetSchemaVersion()};
            });
        auto ignoredDirectCalls = std::make_shared<int>(0);
        CnbLoaderRegistry::Register(
            typeId, RulesName,
            [ignoredDirectCalls](const CnbDocument&, ContentManager&,
                                 const std::string&) -> std::any {
                ++*ignoredDirectCalls;
                return Rules{"ignored", 0.0f, 0u};
            });
        const auto directFound = CnbLoaderRegistry::Find(typeId);
        const Rules directFoundValue = std::any_cast<Rules>(
            (*directFound)(validDocument, content, "tools/rules"));
        const auto directResolved = CnbLoaderRegistry::ResolveForDocument(validDocument);
        const Rules directResolvedValue = std::any_cast<Rules>(
            directResolved(validDocument, content, "runtime/rules"));
        ctx.Check(directFound && directFoundValue.name == "rules" &&
                      directResolvedValue.name == "rules" && directResolvedValue.gravity == 2.5f &&
                      directResolvedValue.lives == 1u && *directCalls == 2 &&
                      *ignoredDirectCalls == 0 && *directAssetName == "runtime/rules",
                  "Register, Find and Resolve retain the first loader and pass document, manager and asset name");

        CnbLoaderRegistry::Register(collisionId, CollisionAlpha,
            [](const CnbDocument&, ContentManager&, const std::string&) -> std::any {
                return CollisionAsset{1u};
            });
        const auto collidingDocument = CnbDocument::Parse(colliding.Build(), "direct collision probe");
        ctx.Check(RefusesAs<std::logic_error>([&] {
                      CnbLoaderRegistry::Register(collisionId, CollisionBeta,
                          [](const CnbDocument&, ContentManager&, const std::string&) -> std::any {
                              return CollisionAsset{2u};
                          });
                  }) &&
                      RefusesAs<ContentLoadException>([&] {
                          (void)CnbLoaderRegistry::ResolveForDocument(collidingDocument);
                      }),
                  "a genuine valid-name hash collision is refused at registration and document resolution");

        CnbLoaderRegistry::Clear();
        CnbLoaderRegistry::RegisterBuiltIns();
        CnbLoaderRegistry::RegisterBuiltIns();
        CnbWriter builtInProbe(CnbAssetTypeId::Curve, 1u);
        builtInProbe.SetMetadata("Diagnostic.WrongCurveName", "built_in_probe");
        const auto builtInDocument = CnbDocument::Parse(builtInProbe.Build(), "built-in identity probe");
        ctx.Check(CnbLoaderRegistry::IsRegistered(CnbAssetTypeId::Curve) &&
                      CnbLoaderRegistry::IsRegistered(CnbAssetTypeId::AnimationClip) &&
                      !CnbLoaderRegistry::IsRegistered(CnbAssetTypeId::Texture2D) &&
                      CnbLoaderRegistry::RegisteredTypeName(CnbAssetTypeId::Curve) == BuiltInNames[0] &&
                      CnbLoaderRegistry::RegisteredTypeName(CnbAssetTypeId::AnimationClip) == BuiltInNames[1] &&
                      CnbLoaderRegistry::Find(CnbAssetTypeId::Curve).has_value() &&
                      CnbLoaderRegistry::Find(CnbAssetTypeId::AnimationClip).has_value() &&
                      static_cast<bool>(CnbLoaderRegistry::ResolveForDocument(builtInDocument)),
                  "RegisterBuiltIns idempotently installs only Curve and AnimationClip and built-in identity is numeric");
        ctx.Check(CnbLoaderRegistry::Remove(CnbAssetTypeId::Curve) &&
                      !CnbLoaderRegistry::Remove(CnbAssetTypeId::Curve) &&
                      !CnbLoaderRegistry::IsRegistered(CnbAssetTypeId::Curve),
                  "a directly installed built-in follows the same exact removal lifecycle");
        CnbLoaderRegistry::Clear();
    }
}

CNA_DEMOS_CONSOLE(CnbLoader, {
    .id = "content.custom.cnb_loader",
    .title = "A custom CNB asset schema (CNAEXT)",
    .description = "Writes a game-rules CNB asset, checks canonical custom-type identity and collision refusal, "
                   "first-wins serial and concurrent registration, caching, extensions, retry after repair and invalid schema layouts.",
    .apis = {"ContentManager::RegisterCnbLoaderEXT", "ContentManager::Load",
             "CNA::Content::Cnb::Format", "CNA::Content::Cnb::CnbChunkId",
             "CNA::Content::Cnb::MakeChunkId", "CNA::Content::Cnb::ChunkIdToString",
             "CNA::Content::Cnb::IsWellFormedChunkId", "CNA::Content::Cnb::CnbChunkFlags",
             "CNA::Content::Cnb::CnbCompression", "CNA::Content::Cnb::CnbAssetTypeId",
             "CNA::Content::Cnb::CnbAssetTypeIdFromName", "CNA::Content::Cnb::CnbWriter::SetMetadata",
             "CNA::Content::Cnb::IsCustomAssetTypeId", "CNA::Content::Cnb::AssetTypeIdToString",
             "CNA::Content::Cnb::CnbLogicalNameProblem", "CNA::Content::Cnb::CnbContainerChunk",
             "CNA::Content::Cnb::CnbWriter::AddChunk", "CNA::Content::Cnb::CnbWriter::WriteToFile",
             "CNA::Content::Cnb::CnbDocument::RequireAsset", "CNA::Content::Cnb::CnbDocument::OpenChunk",
             "CNA::Content::Cnb::CnbDocument::ParseFile", "CNA::Content::Cnb::CnbDocument::FindAll",
             "CNA::Content::Cnb::CnbDocument::RequireSingle", "CNA::Content::Cnb::CnbByteReader::RequireExhausted",
             "CNA::Content::Cnb::CnbDocument::RequireMandatoryChunksUnderstood",
             "CNA::Content::CnbLoaderRegistry", "CNA::Content::CnbLoaderRegistry::LoaderFn",
             "CNA::Content::CnbLoaderRegistry::Register", "CNA::Content::CnbLoaderRegistry::Remove",
             "CNA::Content::CnbLoaderRegistry::Clear", "CNA::Content::CnbLoaderRegistry::IsRegistered",
             "CNA::Content::CnbLoaderRegistry::Find", "CNA::Content::CnbLoaderRegistry::RegisteredTypeName",
             "CNA::Content::CnbLoaderRegistry::ResolveForDocument",
             "CNA::Content::CnbLoaderRegistry::RegisterBuiltIns"},
    .tags = {"content", "custom", "cnb", "cnaext"},
    .restrictions = "Process-global custom registrations, including the concurrent first-wins probe, are isolated in a child process.",
    .ownProcess = true,
})
