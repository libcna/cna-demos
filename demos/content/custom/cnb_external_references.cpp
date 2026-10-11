// SPDX-License-Identifier: MS-PL
// Two encounters share one independently compiled enemy profile through CNB's XREF table.
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"

using namespace Microsoft::Xna::Framework::Content;
using namespace CNA::Content::Cnb;

namespace
{
    struct EnemyProfile { std::string name; std::uint32_t health; };
    using SharedProfile = std::shared_ptr<const EnemyProfile>;
    struct Encounter { std::uint32_t count; SharedProfile enemy; };
    constexpr const char* ProfileName = "CnaDemos.EncounterEnemyProfile";
    constexpr const char* EncounterName = "CnaDemos.Encounter";
    constexpr auto ProfileChunk = MakeChunkId('E', 'N', 'M', 'Y');
    constexpr auto EncounterChunk = MakeChunkId('S', 'P', 'W', 'N');
    int profileReads = 0, encounterReads = 0;

    struct TemporaryContent
    {
        std::filesystem::path path;
        ~TemporaryContent()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    };

    void WriteProfile(const std::filesystem::path& path, const std::string& logicalName)
    {
        CnbByteWriter data;
        data.WriteString("Guard");
        data.WriteU32(40);
        CnbWriter writer(CnbAssetTypeIdFromName(ProfileName), 1);
        writer.SetMetadata(ProfileName, logicalName);
        writer.AddChunk(ProfileChunk, data.Take(), CnbChunkFlags::Mandatory);
        writer.WriteToFile((path / (logicalName + ".cnb")).string());
    }

    void WriteEncounter(const std::filesystem::path& path, const std::string& logicalName,
                        const std::string& profile, std::uint32_t count,
                        std::uint32_t index = 0, std::uint32_t expectedType = CnbAssetTypeIdFromName(ProfileName))
    {
        CnbByteWriter data;
        data.WriteU32(count);
        data.WriteU32(index); // an index into XREF, not a filesystem path embedded in this schema
        CnbWriter writer(CnbAssetTypeIdFromName(EncounterName), 1);
        writer.SetMetadata(EncounterName, logicalName);
        writer.SetExternalReferences({CnbExternalReference{0u, expectedType, profile}});
        writer.AddChunk(EncounterChunk, data.Take(), CnbChunkFlags::Mandatory);
        writer.WriteToFile((path / (logicalName + ".cnb")).string());
    }

    void ExternalReferences(CnaDemos::DemoContext& ctx)
    {
        // Registrations persist beyond this invocation: neither factory captures stack state.
        ContentManager::RegisterCnbLoaderEXT<SharedProfile>(CnbAssetTypeIdFromName(ProfileName), ProfileName,
            [](const CnbDocument& document, ContentManager&) -> SharedProfile {
                document.RequireAsset(CnbAssetTypeIdFromName(ProfileName), 1);
                const std::array known{ProfileChunk};
                document.RequireMandatoryChunksUnderstood(known);
                auto input = document.OpenChunk(document.RequireSingle(ProfileChunk));
                EnemyProfile profile{input.ReadString(), input.ReadU32()};
                input.RequireExhausted();
                ++profileReads;
                return std::make_shared<const EnemyProfile>(std::move(profile));
            });
        ContentManager::RegisterCnbLoaderEXT<Encounter>(CnbAssetTypeIdFromName(EncounterName), EncounterName,
            [](const CnbDocument& document, ContentManager& owner) {
                document.RequireAsset(CnbAssetTypeIdFromName(EncounterName), 1);
                const std::array known{EncounterChunk};
                document.RequireMandatoryChunksUnderstood(known);
                auto input = document.OpenChunk(document.RequireSingle(EncounterChunk));
                const auto count = input.ReadU32();
                const auto index = input.ReadU32();
                input.RequireExhausted();
                const auto& reference = document.ExternalReferenceAt(index, "encounter enemy");
                // The game schema checks its expected type. The container validates the table
                // and names, but does not automatically traverse custom schemas or load children.
                if (reference.expectedAssetTypeId != CnbAssetTypeIdFromName(ProfileName))
                    throw ContentLoadException("The encounter needs an enemy-profile reference.");
                auto enemy = owner.Load<SharedProfile>(reference.logicalName);
                ++encounterReads;
                return Encounter{count, std::move(enemy)};
            });
        profileReads = encounterReads = 0;
        const auto path = std::filesystem::temp_directory_path() /
            ("cna-demos-encounter-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(path)) throw std::runtime_error("Could not create content directory");
        TemporaryContent cleanup{path};
        std::filesystem::create_directory(path / "rooms");
        std::filesystem::create_directory(path / "profiles");
        WriteProfile(path, "profiles/guard");
        WriteEncounter(path, "rooms/east", "profiles/guard", 2);
        WriteEncounter(path, "rooms/west", "profiles/guard", 5);

        const auto document = CnbDocument::ParseFile((path / "rooms/east.cnb").string());
        const auto& references = document.ExternalReferences();
        ctx.Check(references.size() == 1 && references.front().logicalName == "profiles/guard" &&
                  references.front().expectedAssetTypeId == CnbAssetTypeIdFromName(ProfileName),
                  "the compiled encounter carries a typed logical reference in its XREF table");
        ContentManager content(nullptr, path.string());
        const auto east = content.Load<Encounter>("rooms/east");
        const auto west = content.Load<Encounter>("rooms/west");
        ctx.Check(east.count == 2 && west.count == 5 && east.enemy->name == "Guard" && east.enemy->health == 40 &&
                  east.enemy == west.enemy && profileReads == 1 && encounterReads == 2,
                  "two encounters resolve one cached profile through their owning manager");
        ctx.Check(content.Load<SharedProfile>("profiles/guard") == east.enemy && profileReads == 1,
                  "external names resolve from the content root, and direct Load sees the same profile");
        ContentManager another(nullptr, path.string());
        const auto independent = another.Load<Encounter>("rooms/east");
        ctx.Check(independent.enemy != east.enemy && independent.enemy->health == east.enemy->health && profileReads == 2,
                  "another manager constructs its own external profile graph");

        WriteEncounter(path, "rooms/bad_index", "profiles/guard", 1, 1);
        WriteEncounter(path, "rooms/bad_type", "profiles/guard", 1, 0, CnbAssetTypeId::Texture2D);
        const int profilesBefore = profileReads, encountersBefore = encounterReads;
        for (const char* name : {"rooms/bad_index", "rooms/bad_type"})
        {
            bool rejected = false;
            try { (void)content.Load<Encounter>(name); }
            catch (const ContentLoadException& error)
            {
                rejected = true;
                ctx.Print("{} rejected: {}", name, error.what());
            }
            ctx.Check(rejected && profileReads == profilesBefore && encounterReads == encountersBefore,
                      std::string(name) + " is rejected before the encounter or a new profile is returned");
        }

        WriteEncounter(path, "rooms/late", "profiles/late", 3);
        bool missing = false;
        try { (void)content.Load<Encounter>("rooms/late"); }
        catch (const ContentLoadException& error)
        {
            missing = true;
            ctx.Print("Missing child rejected: {}", error.what());
        }
        ctx.Check(missing && encounterReads == encountersBefore,
                  "a missing external asset prevents the parent from being returned");
        WriteProfile(path, "profiles/late");
        const auto repaired = content.Load<Encounter>("rooms/late");
        ctx.Check(repaired.count == 3 && repaired.enemy->health == 40 && profileReads == profilesBefore + 1 &&
                  encounterReads == encountersBefore + 1,
                  "the same manager retries the parent after its missing external asset is supplied");

        bool unsafe = false;
        try { WriteEncounter(path, "rooms/unsafe", "../outside", 1); }
        catch (const ContentLoadException& error)
        {
            unsafe = true;
            ctx.Print("Unsafe external name rejected by writer: {}", error.what());
        }
        ctx.Check(unsafe, "the writer refuses an external name containing a parent-directory segment");
    }
}

CNA_DEMOS_CONSOLE(ExternalReferences, {
    .id = "content.custom.cnb_external_references",
    .title = "Composite CNB assets and their owning manager (CNAEXT)",
    .description = "Two compiled encounters reference one enemy profile through XREF; their loaders use the owning "
                   "manager's cache, reject invalid reference indices and schema type declarations, and retry after a missing child is supplied.",
    .apis = {"ContentManager::RegisterCnbLoaderEXT", "ContentManager::Load",
             "CNA::Content::Cnb::CnbWriter::SetExternalReferences", "CNA::Content::Cnb::CnbWriter::WriteToFile",
             "CNA::Content::Cnb::CnbDocument::ParseFile", "CNA::Content::Cnb::CnbDocument::ExternalReferences",
             "CNA::Content::Cnb::CnbDocument::ExternalReferenceAt"},
    .tags = {"content", "custom", "cnb", "references", "cache", "cnaext"},
    .restrictions = "An acyclic custom asset graph; not XNB fixups, automatic child compilation or cycle detection.",
})
