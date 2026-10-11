// SPDX-License-Identifier: MS-PL
// Compiled content wins over editable sources; cached assets keep their original value.
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>
#include <nlohmann/json.hpp>

#include "CnaDemos/Demo.hpp"
#include "System/IO/EndOfStreamException.hpp"
#include "CNA/Platform/CurrentPlatform.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReaderManager.hpp"
#include "Microsoft/Xna/Framework/Content/LooseFileContentTypeReader.hpp"

using namespace Microsoft::Xna::Framework::Content;
using namespace CNA::Content::Cnb;

namespace
{
    struct Rules { int health = 0; };
    constexpr const char* TypeName = "CnaDemos.ResolutionRules";
    constexpr const char* ReaderName = "CnaDemos.ResolutionRulesReader";
    constexpr auto HealthChunk = MakeChunkId('H', 'P', ' ', ' ');
    int xnbReads = 0, cnbReads = 0, looseReads = 0;

    class XnbRulesReader final : public ContentTypeReader<Rules>
    {
    public:
        XnbRulesReader() : ContentTypeReader<Rules>(TypeName) {}
    protected:
        Rules Read(ContentReader& input, std::optional<Rules>) override
        {
            ++xnbReads;
            return {input.ReadInt32()};
        }
    };
    class LooseRulesReader final : public LooseFileContentTypeReader<Rules>
    {
    public:
        std::vector<std::string> GetExtensions() const override { return {".rules"}; }
        Rules Read(const std::string& path, ContentManager&) override
        {
            std::ifstream stream(path);
            if (!stream) throw ContentLoadException("Cannot read rules: " + path);
            ++looseReads;
            // The manager selects a path; this game's reader owns its JSON/text schemas.
            if (std::filesystem::path(path).extension() == ".cnj")
            {
                const auto json = nlohmann::json::parse(stream);
                if (json.at("cnjVersion") != 1 || json.at("type") != TypeName)
                    throw ContentLoadException("Unsupported rules CNJ envelope");
                return {json.at("data").at("health").get<int>()};
            }
            int health = 0;
            if (!(stream >> health)) throw ContentLoadException("Missing rules health");
            return {health};
        }
    };
    struct TemporaryContent
    {
        std::filesystem::path path;
        ~TemporaryContent()
        {
            ContentTypeReaderManager::RemoveTypeCreatorEXT(ReaderName);
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    };
    void WriteText(const std::filesystem::path& path, const std::string& text)
    {
        std::ofstream file(path, std::ios::binary);
        file << text;
        file.close();
        if (!file) throw std::runtime_error("Cannot write private rules fixture");
    }

    void AssetPrecedence(CnaDemos::DemoContext& ctx)
    {
        if (ContentTypeReaderManager::IsRegistered(ReaderName))
            throw std::logic_error("The demo's unique reader key is already registered");
        const auto path = std::filesystem::temp_directory_path() /
            ("cna-demos-resolution-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(path)) throw std::runtime_error("Cannot create private content directory");
        TemporaryContent cleanup{path};
        const auto authored = std::filesystem::path(CNA::Platform::GetCurrentPlatform().GetFileSystem()->GetBasePath()) /
            "assets/resolution/rules.xnb";
        const auto restoreXnb = [&] { std::filesystem::copy_file(authored, path / "rules.xnb", std::filesystem::copy_options::overwrite_existing); };
        restoreXnb();
        WriteText(path / "rules.cnj", R"({"cnjVersion":1,"type":"CnaDemos.ResolutionRules","data":{"health":200}})");
        WriteText(path / "rules.rules", "100\n");
        CnbByteWriter payload;
        payload.WriteI32(300);
        CnbWriter writer(CnbAssetTypeIdFromName(TypeName), 1);
        writer.SetMetadata(TypeName, "rules");
        writer.AddChunk(HealthChunk, payload.Take(), CnbChunkFlags::Mandatory);
        writer.WriteToFile((path / "rules.cnb").string());

        ContentTypeReaderManager::AddTypeCreator(ReaderName, [] { return std::make_unique<XnbRulesReader>(); });
        // The process-wide CNB factory captures nothing from this invocation.
        ContentManager::RegisterCnbLoaderEXT<Rules>(CnbAssetTypeIdFromName(TypeName), TypeName,
            [](const CnbDocument& document, ContentManager&) {
                document.RequireAsset(CnbAssetTypeIdFromName(TypeName), 1);
                const std::array known{HealthChunk};
                document.RequireMandatoryChunksUnderstood(known);
                auto input = document.OpenChunk(document.RequireSingle(HealthChunk));
                Rules value{input.ReadI32()};
                input.RequireExhausted();
                ++cnbReads;
                return value;
            });
        xnbReads = cnbReads = looseReads = 0;
        ContentManager cached(nullptr, path.string());
        cached.RegisterTypeReader<Rules>(std::make_unique<LooseRulesReader>());
        ctx.Check(cached.Load<Rules>("rules").health == 400 && xnbReads == 1 && cnbReads == 0 && looseReads == 0,
                  "XNB wins when all four formats exist for the same logical name");
        std::filesystem::remove(path / "rules.xnb");
        ctx.Check(cached.Load<Rules>("rules").health == 400 && xnbReads == 1 && cnbReads == 0,
                  "a cache hit keeps the loaded XNB value after its file disappears");
        ContentManager fresh(nullptr, path.string()); // compiled formats need no loose reader
        ctx.Check(fresh.Load<Rules>("rules").health == 300 && cnbReads == 1,
                  "a fresh manager selects CNB ahead of both editable source files");
        cached.Unload();
        ctx.Check(cached.Load<Rules>("rules").health == 300 && cnbReads == 2,
                  "Unload permits the original manager to resolve the remaining CNB again");

        // A present but invalid compiled file must not silently fall through to editable data.
        WriteText(path / "rules.xnb", "XNB");
        ContentManager retry(nullptr, path.string());
        bool refused = false, wrappedXnbFailure = false;
        try { (void)retry.Load<Rules>("rules"); }
        catch (const ContentLoadException& error)
        {
            refused = wrappedXnbFailure = true;
            ctx.Print("Invalid XNB (ContentLoadException): {}", error.what());
        }
        catch (const System::IO::EndOfStreamException& error)
        {
            refused = true;
            ctx.Print("Invalid XNB (unwrapped EndOfStreamException): {}", error.what());
        }
        ctx.Check(refused && xnbReads == 1 && cnbReads == 2 && looseReads == 0,
                  "an invalid higher-priority XNB is refused without trying the valid CNB or sources");
        ctx.Check(wrappedXnbFailure,
                  "a truncated compiled XNB reaches the caller as ContentLoadException (CNA-DEMOS-ISSUE-045)");
        restoreXnb();
        ctx.Check(retry.Load<Rules>("rules").health == 400 && xnbReads == 2,
                  "repairing XNB lets the same manager retry because failure was not cached");
        std::filesystem::remove(path / "rules.xnb");
        WriteText(path / "rules.cnb", "CNB");
        retry.Unload();
        retry.RegisterTypeReader<Rules>(std::make_unique<LooseRulesReader>());
        refused = false;
        try { (void)retry.Load<Rules>("rules"); }
        catch (const ContentLoadException& error) { refused = true; ctx.Print("Invalid CNB: {}", error.what()); }
        ctx.Check(refused && cnbReads == 2 && looseReads == 0,
                  "an invalid CNB is refused instead of silently loading valid JSON or native text");
        writer.WriteToFile((path / "rules.cnb").string());
        ctx.Check(retry.Load<Rules>("rules").health == 300 && cnbReads == 3,
                  "repairing CNB also permits retry without discarding the manager");

        std::filesystem::remove(path / "rules.cnb");
        cached.Unload();
        ctx.Check(cached.Load<Rules>("rules").health == 200 && looseReads == 1,
                  "the CNJ sidecar wins over the reader's native extension after compiled files are removed");
        ctx.Check(cached.Load<Rules>("rules.rules").health == 100 && looseReads == 2,
                  "an explicit native filename selects that literal file despite the sibling CNJ");
        std::filesystem::remove(path / "rules.cnj");
        ctx.Check(cached.Load<Rules>("rules").health == 200 && looseReads == 2,
                  "the logical-name cache still retains JSON data after removing its sidecar");
        cached.Unload();
        ctx.Check(cached.Load<Rules>("rules").health == 100 && looseReads == 3,
                  "without compiled files or CNJ, the reader's native extension is selected");
        ctx.Check(fresh.Load<Rules>("rules").health == 300 && cnbReads == 3,
                  "another manager's cached compiled asset is unaffected by disk changes or Unload elsewhere");
        ctx.Print("Health tiers: XNB 400 > CNB 300 > CNJ 200 > native 100; all private files removed on exit");
    }
}

CNA_DEMOS_CONSOLE(AssetPrecedence, {
    .id = "content.content_manager.asset_precedence",
    .title = "Compiled content versus editable sources",
    .description = "Loads four competing same-name formats, distinguishes fresh resolution from cached values, "
                   "and refuses corrupt compiled content without silently falling back to editable sources.",
    .apis = {"ContentManager::Load", "ContentManager::Unload", "Microsoft::Xna::Framework::Content::ContentManager::RegisterTypeReader",
             "Microsoft::Xna::Framework::Content::ContentManager::RegisterCnbLoaderEXT", "ContentTypeReaderManager::AddTypeCreator",
             "LooseFileContentTypeReader::GetExtensions", "LooseFileContentTypeReader::Read"},
    .tags = {"content", "cache", "xnb", "cnb", "cnj", "resolution"},
    .assets = {"resolution/rules.xnb"},
    .restrictions = "CNA filesystem format priority; original authored XNB, private temporary files, no packaged-asset or genuine-XNA comparison.",
    .knownIssue = "CNA-DEMOS-ISSUE-045",
})
