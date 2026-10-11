// SPDX-License-Identifier: MS-PL
// Import text waypoints, process a patrol's length, and write the game's CNB schema.
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "CNA/Content/Pipeline/ContentPipeline.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace Pipeline = CNA::Content::Pipeline;
namespace Cnb = CNA::Content::Cnb;
using Microsoft::Xna::Framework::Content::ContentManager;
using Microsoft::Xna::Framework::TitleLocation;
using Microsoft::Xna::Framework::Vector2;

namespace
{
    constexpr const char* ImportedType = "CnaDemos.PatrolSource";
    constexpr const char* RuntimeType = "CnaDemos.Patrol";
    constexpr auto PathChunk = Cnb::MakeChunkId('P', 'A', 'T', 'H');
    struct Patrol { float length; std::vector<Vector2> points; };

    class PatrolImporter final : public Pipeline::ContentImporter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.PatrolImporter", "1"}; }
        std::vector<std::string> SourceExtensions() const override { return {".route"}; }
        std::vector<std::string> OutputTypes() const override { return {ImportedType}; }
        Pipeline::ContentValue Import(Pipeline::ContentImporterContext& context) const override
        {
            std::ifstream source(context.SourcePath());
            if (!source) throw std::runtime_error("Could not open the patrol route");
            std::vector<Vector2> points;
            float x, y;
            while (source >> x)
            {
                if (!(source >> y) || !std::isfinite(x) || !std::isfinite(y))
                    throw std::runtime_error("Each waypoint must contain two finite coordinates");
                if (points.size() == 128) throw std::runtime_error("The demo route supports at most 128 waypoints");
                points.emplace_back(x, y);
            }
            if (!source.eof() || points.size() < 2)
                throw std::runtime_error("A patrol needs at least two valid waypoints");
            return Pipeline::ContentValue::Create(ImportedType, std::move(points));
        }
    };

    class PatrolProcessor final : public Pipeline::ContentProcessor
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.PatrolProcessor", "1"}; }
        std::string InputType() const override { return ImportedType; }
        std::string OutputType() const override { return RuntimeType; }
        void ValidateParameters(const Pipeline::ContentProcessorParameters& parameters) const override
        {
            for (const auto& [name, value] : parameters.Values())
                if (name != "closed" || !std::holds_alternative<bool>(value))
                    throw std::invalid_argument("PatrolProcessor accepts only the boolean parameter 'closed'");
        }
        Pipeline::ContentValue Process(const Pipeline::ContentValue& input,
                                       Pipeline::ContentProcessorContext& context) const override
        {
            Patrol patrol{0.0f, input.Get<std::vector<Vector2>>()};
            for (std::size_t i = 1; i < patrol.points.size(); ++i)
                patrol.length += Vector2::Distance(patrol.points[i - 1], patrol.points[i]);
            const auto* closed = context.Parameters().Find("closed");
            if (closed && std::get<bool>(*closed))
                patrol.length += Vector2::Distance(patrol.points.back(), patrol.points.front());
            // Do this work once at build time; the running game loads the precomputed length.
            return Pipeline::ContentValue::Create(RuntimeType, std::move(patrol));
        }
    };

    class PatrolWriter final : public Pipeline::ContentTypeWriter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.PatrolWriter", "1"}; }
        std::string InputType() const override { return RuntimeType; }
        std::vector<Pipeline::ContentWriterSchemaIdentity> OutputSchemaIdentities() const override
        {
            return {{Cnb::CnbAssetTypeIdFromName(RuntimeType), 1u, RuntimeType, {"CnaDemos.PatrolCodec", "1"}}};
        }
        Pipeline::ContentWriteResult Write(const Pipeline::ContentValue& input,
                                           const std::string& logicalName) const override
        {
            const Patrol& patrol = input.Get<Patrol>();
            Cnb::CnbByteWriter payload;
            payload.WriteF32(patrol.length);
            payload.WriteU32(static_cast<std::uint32_t>(patrol.points.size()));
            for (const Vector2& point : patrol.points)
            {
                payload.WriteF32(point.X);
                payload.WriteF32(point.Y);
            }
            Cnb::CnbWriter writer(Cnb::CnbAssetTypeIdFromName(RuntimeType), 1);
            writer.SetMetadata(RuntimeType, logicalName);
            writer.AddChunk(PathChunk, payload.Take(), Cnb::CnbChunkFlags::Mandatory);
            Pipeline::ContentWriteResult result;
            result.bytes = writer.Build();
            result.assetTypeId = Cnb::CnbAssetTypeIdFromName(RuntimeType);
            result.assetTypeName = RuntimeType;
            result.assetSchemaVersion = 1;
            return result;
        }
    };

    struct TemporaryOutput
    {
        std::filesystem::path path;
        ~TemporaryOutput() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };

    void PipelineComponents(CnaDemos::DemoContext& ctx)
    {
        // This registry is explicit and local, the way an embedded build tool configures its
        // game components. It does not modify the separately built cna-content executable.
        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        registry->RegisterImporter(std::make_shared<PatrolImporter>());
        registry->RegisterProcessor(std::make_shared<PatrolProcessor>());
        registry->RegisterWriter(std::make_shared<PatrolWriter>());
        const Pipeline::ContentPipeline pipeline(registry);
        Pipeline::ContentBuildRequest request;
        request.sourceRoot = TitleLocation::Path() + "/assets";
        request.source = "custom/patrol.route";
        request.logicalName = "patrol";
        request.parameters.Set("closed", true);
        const auto built = pipeline.Build(request);
        ctx.Check(built.importer.name == "CnaDemos.PatrolImporter" &&
                  built.processor.name == "CnaDemos.PatrolProcessor" && built.writer.name == "CnaDemos.PatrolWriter",
                  "the registry selects the game's importer, processor and writer by their declared types");
        ctx.Check(pipeline.Build(request).output.bytes == built.output.bytes,
                  "identical source and parameters produce identical CNB bytes");

        request.parameters.Set("closed", false);
        const auto open = pipeline.Build(request);
        const auto document = Cnb::CnbDocument::Parse(open.output.bytes, "open patrol");
        auto payload = document.OpenChunk(document.RequireSingle(PathChunk));
        ctx.CheckNear(payload.ReadF32(), 7.0, 1e-6,
                      "disabling the closed parameter omits the return segment at build time");

        // The runtime half knows only its schema, not the build-tool classes above.
        ContentManager::RegisterCnbLoaderEXT<Patrol>(Cnb::CnbAssetTypeIdFromName(RuntimeType), RuntimeType,
            [](const Cnb::CnbDocument& document, ContentManager&) {
                document.RequireAsset(Cnb::CnbAssetTypeIdFromName(RuntimeType), 1);
                document.RequireMandatoryChunksUnderstood(std::array{PathChunk});
                auto bytes = document.OpenChunk(document.RequireSingle(PathChunk));
                Patrol patrol{bytes.ReadF32(), {}};
                const auto count = bytes.ReadCount(8, "waypoints");
                patrol.points.reserve(count);
                for (std::uint32_t i = 0; i < count; ++i)
                {
                    // Read in explicit order: function-argument evaluation order is not a wire format.
                    const float x = bytes.ReadF32(), y = bytes.ReadF32();
                    patrol.points.emplace_back(x, y);
                }
                bytes.RequireExhausted();
                return patrol;
            });
        const auto directory = std::filesystem::temp_directory_path() /
            ("cna-demos-patrol-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(directory)) throw std::runtime_error("Could not create private output");
        TemporaryOutput cleanup{directory};
        {
            std::ofstream output(directory / "patrol.cnb", std::ios::binary);
            output.write(reinterpret_cast<const char*>(built.output.bytes.data()),
                         static_cast<std::streamsize>(built.output.bytes.size()));
            if (!output) throw std::runtime_error("Could not write the compiled patrol");
        }
        ContentManager content(nullptr, directory.string());
        const Patrol patrol = content.Load<Patrol>("patrol");
        ctx.CheckNear(patrol.length, 12.0, 1e-6, "the closed 3-4-5 patrol stores its precomputed length of 12");
        ctx.Check(patrol.points.size() == 3 && patrol.points.back() == Vector2(3.0f, 4.0f),
                  "the runtime loader retains the authored waypoints");

        request.parameters.Set("closed", std::string("yes"));
        bool rejected = false;
        try { (void)pipeline.Build(request); }
        catch (const Pipeline::ContentPipelineError& error)
        {
            rejected = error.Stage() == Pipeline::ContentPipelineStage::Process &&
                       error.Component() == "CnaDemos.PatrolProcessor";
            ctx.Print("Invalid processor parameter: {}", error.what());
        }
        ctx.Check(rejected, "an invalid parameter reports the processor and the failing pipeline stage");
    }
}

CNA_DEMOS_CONSOLE(PipelineComponents, {
    .id = "content.custom.pipeline_components",
    .title = "Build a custom patrol asset (CNAEXT)",
    .description = "Registers a text importer, a length-precomputing processor and a CNB writer, builds a patrol "
                   "through ContentPipeline, loads its runtime schema and checks contextual parameter errors.",
    .apis = {"CNA::Content::Pipeline::ContentPipelineRegistry::RegisterImporter",
             "CNA::Content::Pipeline::ContentPipelineRegistry::RegisterProcessor",
             "CNA::Content::Pipeline::ContentPipelineRegistry::RegisterWriter",
             "CNA::Content::Pipeline::ContentPipeline::Build", "CNA::Content::Pipeline::ContentValue::Create",
             "CNA::Content::Pipeline::ContentValue::Get", "CNA::Content::Pipeline::ContentProcessorParameters::Set",
             "CNA::Content::Pipeline::ContentPipelineError::Stage", "ContentManager::RegisterCnbLoaderEXT",
             "CNA::Content::Cnb::CnbByteReader::ReadCount"},
    .tags = {"content", "custom", "pipeline", "cnb", "cnaext"},
    .assets = {"custom/patrol.route"},
})
