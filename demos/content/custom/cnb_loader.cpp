// SPDX-License-Identifier: MS-PL
// A game's binary asset schema and its process-wide CNA loader.
#include <array>
#include <chrono>
#include <filesystem>
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
    constexpr auto RulesChunk = MakeChunkId('R', 'U', 'L', 'E');
    constexpr const char* RulesName = "CnaDemos.GameRules";
    int reads = 0;

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
        // Registration outlives every manager: the factory must not capture local references.
        ContentManager::RegisterCnbLoaderEXT<Rules>(typeId, RulesName,
            [](const CnbDocument& document, ContentManager&) {
                document.RequireAsset(CnbAssetTypeIdFromName(RulesName), 1);
                const std::array known{RulesChunk};
                document.RequireMandatoryChunksUnderstood(known);
                auto input = document.OpenChunk(document.RequireSingle(RulesChunk));
                Rules result{input.ReadString(), input.ReadF32(), input.ReadU32()};
                input.RequireExhausted();
                ++reads;
                return result;
            });
        reads = 0;

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
        CnbWriter writer(typeId, 1);
        writer.SetMetadata(RulesName, "rules");
        writer.AddChunk(RulesChunk, bytes, CnbChunkFlags::Mandatory);
        writer.WriteToFile((path / "rules.cnb").string());

        ContentManager content(nullptr, path.string());
        const Rules rules = content.Load<Rules>("rules");
        ctx.Print("{}: gravity {}, lives {}", rules.name, rules.gravity, rules.lives);
        ctx.Check(rules.name == "Moon" && rules.lives == 3, "custom string and integer fields round trip");
        ctx.CheckNear(rules.gravity, 1.62, 1e-6, "custom float field round trips");
        (void)content.Load<Rules>("rules");
        ctx.Check(reads == 1, "a second Load uses the manager's cache");
        ContentManager another(nullptr, path.string());
        (void)another.Load<Rules>("rules");
        ctx.Check(reads == 2, "registration is shared but caches belong to each manager");

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
    .description = "Writes a small game-rules CNB asset, registers its typed loader, checks per-manager caching "
                   "and process-wide registration, and rejects a newer unsupported schema.",
    .apis = {"ContentManager::RegisterCnbLoaderEXT", "ContentManager::Load",
             "CNA::Content::Cnb::CnbAssetTypeIdFromName", "CNA::Content::Cnb::CnbWriter::SetMetadata",
             "CNA::Content::Cnb::CnbWriter::AddChunk", "CNA::Content::Cnb::CnbWriter::WriteToFile",
             "CNA::Content::Cnb::CnbDocument::RequireAsset", "CNA::Content::Cnb::CnbDocument::OpenChunk",
             "CNA::Content::Cnb::CnbDocument::RequireMandatoryChunksUnderstood"},
    .tags = {"content", "custom", "cnb", "cnaext"},
})
