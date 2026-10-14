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
#include <typeindex>
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
    constexpr const char* XnbAbsence = "This game publishes patrols only with its CNB PATH schema.";
    constexpr auto PathChunk = Cnb::MakeChunkId('P', 'A', 'T', 'H');
    struct Patrol { float length; std::vector<Vector2> points; };

    // Deliberately broken component outputs below distinguish coordinator type checks from
    // the receiving component's Get<T>() check. Normal builds keep the default output.
    enum class ReturnedValue { Valid, Empty, WrongStableType, WrongCppType };

    class PatrolImporter final : public Pipeline::ContentImporter
    {
        ReturnedValue returned_;
        std::string name_;
        bool selectedByNameOnly_;
    public:
        explicit PatrolImporter(ReturnedValue returned = ReturnedValue::Valid,
                                std::string name = "CnaDemos.PatrolImporter",
                                bool selectedByNameOnly = false)
            : returned_(returned), name_(std::move(name)), selectedByNameOnly_(selectedByNameOnly) {}
        Pipeline::ContentComponentIdentity Identity() const override { return {name_, "1"}; }
        std::vector<std::string> SourceExtensions() const override { return {".route"}; }
        std::vector<std::string> OutputTypes() const override { return {ImportedType}; }
        bool SelectedByNameOnly() const override { return selectedByNameOnly_; }
        Pipeline::ContentValue Import(Pipeline::ContentImporterContext& context) const override
        {
            if (returned_ == ReturnedValue::Empty) return {};
            if (returned_ == ReturnedValue::WrongStableType)
                return Pipeline::ContentValue::Create("CnaDemos.UndeclaredRoute", std::vector<Vector2>{});
            if (returned_ == ReturnedValue::WrongCppType)
                return Pipeline::ContentValue::Create(ImportedType, std::string("not waypoints"));
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
        ReturnedValue returned_;
        std::string name_;
        std::string inputType_;
        bool selectedByNameOnly_;
    public:
        explicit PatrolProcessor(ReturnedValue returned = ReturnedValue::Valid,
                                 std::string name = "CnaDemos.PatrolProcessor",
                                 std::string inputType = ImportedType,
                                 bool selectedByNameOnly = false)
            : returned_(returned), name_(std::move(name)), inputType_(std::move(inputType)),
              selectedByNameOnly_(selectedByNameOnly) {}
        Pipeline::ContentComponentIdentity Identity() const override { return {name_, "1"}; }
        std::string InputType() const override { return inputType_; }
        std::string OutputType() const override { return RuntimeType; }
        bool SelectedByNameOnly() const override { return selectedByNameOnly_; }
        void ValidateParameters(const Pipeline::ContentProcessorParameters& parameters) const override
        {
            for (const auto& [name, value] : parameters.Values())
                if (name != "closed" || !std::holds_alternative<bool>(value))
                    throw std::invalid_argument("PatrolProcessor accepts only the boolean parameter 'closed'");
        }
        Pipeline::ContentValue Process(const Pipeline::ContentValue& input,
                                       Pipeline::ContentProcessorContext& context) const override
        {
            if (returned_ == ReturnedValue::Empty) return {};
            if (returned_ == ReturnedValue::WrongStableType)
                return Pipeline::ContentValue::Create("CnaDemos.UndeclaredPatrol", Patrol{});
            if (returned_ == ReturnedValue::WrongCppType)
                return Pipeline::ContentValue::Create(RuntimeType, std::string("not a patrol"));
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
        std::string name_;
        std::string inputType_;
    public:
        explicit PatrolWriter(std::string name = "CnaDemos.PatrolWriter",
                              std::string inputType = RuntimeType)
            : name_(std::move(name)), inputType_(std::move(inputType)) {}
        Pipeline::ContentComponentIdentity Identity() const override { return {name_, "1"}; }
        std::string InputType() const override { return inputType_; }
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
        registry->DocumentAbsentWriter(Pipeline::ContentOutputFormat::Xnb, RuntimeType, XnbAbsence);
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

        // Put two ordinary candidates on every default route. The coordinator must stop at the
        // first unresolved stage, preserve source/asset context, and never use registration order.
        // Supplying one stable name at a time advances selection to the next ambiguity; naming all
        // three produces the same valid build. Registered but route-incompatible overrides remain
        // refusals rather than silently falling back to a compatible default.
        auto selectionRegistry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        selectionRegistry->RegisterImporter(std::make_shared<PatrolImporter>());
        selectionRegistry->RegisterImporter(std::make_shared<PatrolImporter>(
            ReturnedValue::Valid, "CnaDemos.AlternatePatrolImporter"));
        selectionRegistry->RegisterProcessor(std::make_shared<PatrolProcessor>());
        selectionRegistry->RegisterProcessor(std::make_shared<PatrolProcessor>(
            ReturnedValue::Valid, "CnaDemos.AlternatePatrolProcessor"));
        selectionRegistry->RegisterProcessor(std::make_shared<PatrolProcessor>(
            ReturnedValue::Valid, "CnaDemos.IncompatiblePatrolProcessor", "CnaDemos.OtherSource"));
        selectionRegistry->RegisterWriter(std::make_shared<PatrolWriter>());
        selectionRegistry->RegisterWriter(
            std::make_shared<PatrolWriter>("CnaDemos.AlternatePatrolWriter"));
        selectionRegistry->RegisterWriter(std::make_shared<PatrolWriter>(
            "CnaDemos.IncompatiblePatrolWriter", "CnaDemos.OtherRuntime"));
        const Pipeline::ContentPipeline selectionPipeline(selectionRegistry);
        auto selectionRequest = request;
        selectionRequest.parameters.Set("closed", true);
        const auto contextualSelection = [&selectionRequest](const Pipeline::ContentPipelineError& error,
                                                            const std::string& text) {
            return error.Stage() == Pipeline::ContentPipelineStage::Selection &&
                   error.Component().empty() && error.LogicalName() == selectionRequest.logicalName &&
                   error.Source() == std::filesystem::canonical(
                       selectionRequest.sourceRoot / selectionRequest.source) &&
                   std::string(error.what()).find(text) != std::string::npos;
        };

        bool importerAmbiguous = false;
        try { (void)selectionPipeline.Build(selectionRequest); }
        catch (const Pipeline::ContentPipelineError& error)
        {
            importerAmbiguous = contextualSelection(error, "ambiguous importer") &&
                std::string(error.what()).find("CnaDemos.AlternatePatrolImporter") != std::string::npos &&
                std::string(error.what()).find("CnaDemos.PatrolImporter") != std::string::npos;
            ctx.Print("Importer ambiguity: {}", error.what());
        }
        ctx.Check(importerAmbiguous,
                  "two default importers are refused deterministically with Selection context");

        selectionRequest.importer = "CnaDemos.PatrolImporter";
        bool processorAmbiguous = false;
        try { (void)selectionPipeline.Build(selectionRequest); }
        catch (const Pipeline::ContentPipelineError& error)
        {
            processorAmbiguous = contextualSelection(error, "ambiguous processor") &&
                std::string(error.what()).find("CnaDemos.AlternatePatrolProcessor") != std::string::npos &&
                std::string(error.what()).find("CnaDemos.PatrolProcessor") != std::string::npos;
            ctx.Print("Processor ambiguity: {}", error.what());
        }
        ctx.Check(processorAmbiguous,
                  "an explicit importer advances the same build to deterministic processor ambiguity");

        selectionRequest.processor = "CnaDemos.PatrolProcessor";
        bool writerAmbiguous = false;
        try { (void)selectionPipeline.Build(selectionRequest); }
        catch (const Pipeline::ContentPipelineError& error)
        {
            writerAmbiguous = contextualSelection(error, "ambiguous writer") &&
                std::string(error.what()).find("CnaDemos.AlternatePatrolWriter") != std::string::npos &&
                std::string(error.what()).find("CnaDemos.PatrolWriter") != std::string::npos;
            ctx.Print("Writer ambiguity: {}", error.what());
        }
        ctx.Check(writerAmbiguous,
                  "explicit importer and processor advance the build to deterministic writer ambiguity");

        selectionRequest.writer = "CnaDemos.PatrolWriter";
        const auto explicitlySelected = selectionPipeline.Build(selectionRequest);
        ctx.Check(explicitlySelected.importer.name == selectionRequest.importer &&
                      explicitlySelected.processor.name == selectionRequest.processor &&
                      explicitlySelected.writer.name == selectionRequest.writer &&
                      explicitlySelected.output.bytes == built.output.bytes,
                  "stable explicit names resolve all three ambiguities to the original deterministic output");

        selectionRequest.processor = "CnaDemos.IncompatiblePatrolProcessor";
        bool incompatibleProcessor = false;
        try { (void)selectionPipeline.Build(selectionRequest); }
        catch (const Pipeline::ContentPipelineError& error)
        {
            incompatibleProcessor = contextualSelection(error, "does not accept") &&
                std::string(error.what()).find(selectionRequest.processor) != std::string::npos &&
                std::string(error.what()).find(ImportedType) != std::string::npos;
            ctx.Print("Incompatible processor override: {}", error.what());
        }
        ctx.Check(incompatibleProcessor,
                  "an explicit processor that rejects the imported type fails at Selection without fallback");

        selectionRequest.processor = "CnaDemos.PatrolProcessor";
        selectionRequest.writer = "CnaDemos.IncompatiblePatrolWriter";
        bool incompatibleWriter = false;
        try { (void)selectionPipeline.Build(selectionRequest); }
        catch (const Pipeline::ContentPipelineError& error)
        {
            incompatibleWriter = contextualSelection(error, "does not accept") &&
                std::string(error.what()).find(selectionRequest.writer) != std::string::npos &&
                std::string(error.what()).find(RuntimeType) != std::string::npos;
            ctx.Print("Incompatible writer override: {}", error.what());
        }
        ctx.Check(incompatibleWriter,
                  "an explicit writer that rejects the processed type fails at Selection without fallback");

        auto namedRegistry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        namedRegistry->RegisterImporter(std::make_shared<PatrolImporter>());
        namedRegistry->RegisterImporter(std::make_shared<PatrolImporter>(
            ReturnedValue::Valid, "CnaDemos.NamedPatrolImporter", true));
        namedRegistry->RegisterProcessor(std::make_shared<PatrolProcessor>());
        namedRegistry->RegisterProcessor(std::make_shared<PatrolProcessor>(
            ReturnedValue::Valid, "CnaDemos.NamedPatrolProcessor", ImportedType, true));
        namedRegistry->RegisterWriter(std::make_shared<PatrolWriter>());
        const Pipeline::ContentPipeline namedPipeline(namedRegistry);
        auto namedRequest = request;
        namedRequest.parameters.Set("closed", true);
        const auto conventional = namedPipeline.Build(namedRequest);
        ctx.Check(conventional.importer.name == "CnaDemos.PatrolImporter" &&
                      conventional.processor.name == "CnaDemos.PatrolProcessor",
                  "name-only components do not make the conventional importer or processor route ambiguous");
        namedRequest.importer = "CnaDemos.NamedPatrolImporter";
        namedRequest.processor = "CnaDemos.NamedPatrolProcessor";
        const auto named = namedPipeline.Build(namedRequest);
        ctx.Check(named.importer.name == namedRequest.importer &&
                      named.processor.name == namedRequest.processor &&
                      named.output.bytes == conventional.output.bytes,
                  "explicit stable names select name-only importer and processor components");

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

        auto value = Pipeline::ContentValue::Create(RuntimeType, Patrol{12.0f, {Vector2(3, 4)}});
        const void* shared = value.RawData();
        const auto retained = value;
        value = {};
        ctx.Check(value.Empty() && value.StableType().empty() &&
                  value.CppType() == std::type_index(typeid(void)) && value.RawData() == nullptr,
                  "an empty carrier has no stable identity, concrete type or data pointer");
        // RawData is inspected only after CppType agrees. Get<T>() remains the checked API.
        ctx.Check(!retained.Empty() && retained.StableType() == RuntimeType &&
                  retained.CppType() == std::type_index(typeid(Patrol)) &&
                  retained.RawData() == shared && retained.RawData() == &retained.Get<Patrol>() &&
                  retained.Get<Patrol>().length == 12.0f && retained.Get<Patrol>().points.size() == 1 &&
                  retained.Get<Patrol>().points.front() == Vector2(3, 4),
                  "a copied carrier retains its typed immutable patrol after the original is reset");

        using Stage = Pipeline::ContentPipelineStage;
        using Output = ReturnedValue;
        struct Refusal
        {
            Output importer, processor;
            Stage stage;
            const char* component;
            const char* diagnostic;
            const char* check;
        };
        const std::array refusals{
            Refusal{Output::Empty, Output::Valid, Stage::Import, "CnaDemos.PatrolImporter",
                    "importer returned an empty value",
                    "an empty importer result fails at Import with source and component context"},
            Refusal{Output::WrongStableType, Output::Valid, Stage::Import, "CnaDemos.PatrolImporter",
                    "undeclared output type",
                    "an undeclared importer output identity fails at Import before processing"},
            Refusal{Output::WrongCppType, Output::Valid, Stage::Process, "CnaDemos.PatrolProcessor",
                    "does not contain the C++ type",
                    "a declared importer identity with the wrong C++ payload fails at the processor Get"},
            Refusal{Output::Valid, Output::Empty, Stage::Process, "CnaDemos.PatrolProcessor",
                    "processor returned an empty value",
                    "an empty processor result fails at Process with source and component context"},
            Refusal{Output::Valid, Output::WrongStableType, Stage::Process, "CnaDemos.PatrolProcessor",
                    "processor declared output type",
                    "an undeclared processor output identity fails at Process before writing"},
            Refusal{Output::Valid, Output::WrongCppType, Stage::Write, "CnaDemos.PatrolWriter",
                    "does not contain the C++ type",
                    "a declared processor identity with the wrong C++ payload fails at the writer Get"},
        };
        request.parameters.Set("closed", true); // Restore valid input before testing each output contract.
        for (const auto& refusal : refusals)
        {
            auto invalidRegistry = std::make_shared<Pipeline::ContentPipelineRegistry>();
            invalidRegistry->RegisterImporter(std::make_shared<PatrolImporter>(refusal.importer));
            invalidRegistry->RegisterProcessor(std::make_shared<PatrolProcessor>(refusal.processor));
            invalidRegistry->RegisterWriter(std::make_shared<PatrolWriter>());
            const Pipeline::ContentPipeline invalidPipeline(invalidRegistry);
            bool contextual = false;
            try { (void)invalidPipeline.Build(request); }
            catch (const Pipeline::ContentPipelineError& error)
            {
                contextual = error.Stage() == refusal.stage && error.Component() == refusal.component &&
                    error.LogicalName() == request.logicalName &&
                    error.Source() == std::filesystem::canonical(request.sourceRoot / request.source) &&
                    std::string(error.what()).find(refusal.diagnostic) != std::string::npos;
                ctx.Print("Expected component refusal: {}", error.what());
            }
            ctx.Check(contextual, refusal.check);
        }

        request.outputFormat = Pipeline::ContentOutputFormat::Xnb;
        bool intentionalAbsence = false;
        try { (void)pipeline.Build(request); }
        catch (const Pipeline::ContentPipelineError& error)
        {
            intentionalAbsence = error.Stage() == Stage::Selection && error.Component().empty() &&
                error.LogicalName() == request.logicalName &&
                error.Source() == std::filesystem::canonical(request.sourceRoot / request.source) &&
                std::string(error.what()).find(XnbAbsence) != std::string::npos;
            ctx.Print("Unsupported game format: {}", error.what());
        }
        ctx.Check(intentionalAbsence &&
                  registry->AbsentWriterReason(Pipeline::ContentOutputFormat::Xnb, RuntimeType) == XnbAbsence &&
                  registry->AbsentWriterReason(Pipeline::ContentOutputFormat::Cnb, RuntimeType).empty(),
                  "an intentionally absent XNB writer reports the game's reason at Selection without disabling CNB");
    }
}

CNA_DEMOS_CONSOLE(PipelineComponents, {
    .id = "content.custom.pipeline_components",
    .title = "Build a custom patrol asset (CNAEXT)",
    .description = "Builds and loads a CNB patrol through registered importer, processor and writer; checks deterministic "
                   "selection/overrides, typed result contracts, retained values and an intentionally absent XNB route.",
    .apis = {"CNA::Content::Pipeline::ContentPipelineRegistry::RegisterImporter",
             "CNA::Content::Pipeline::ContentPipelineRegistry::RegisterProcessor",
             "CNA::Content::Pipeline::ContentPipelineRegistry::RegisterWriter",
             "CNA::Content::Pipeline::ContentPipelineRegistry::DocumentAbsentWriter",
             "CNA::Content::Pipeline::ContentPipelineRegistry::AbsentWriterReason",
             "CNA::Content::Pipeline::ContentImporter::SelectedByNameOnly",
             "CNA::Content::Pipeline::ContentProcessor::SelectedByNameOnly",
             "CNA::Content::Pipeline::ContentPipeline::Build", "CNA::Content::Pipeline::ContentValue::Create",
             "CNA::Content::Pipeline::ContentValue::Get", "CNA::Content::Pipeline::ContentValue::StableType",
             "CNA::Content::Pipeline::ContentValue::CppType", "CNA::Content::Pipeline::ContentValue::RawData",
             "CNA::Content::Pipeline::ContentValue::Empty", "CNA::Content::Pipeline::ContentPipelineError::Component",
             "CNA::Content::Pipeline::ContentPipelineError::Source", "CNA::Content::Pipeline::ContentPipelineError::LogicalName",
             "CNA::Content::Pipeline::ContentProcessorParameters::Set",
             "CNA::Content::Pipeline::ContentPipelineError::Stage", "ContentManager::RegisterCnbLoaderEXT",
             "CNA::Content::Cnb::CnbByteReader::ReadCount"},
    .tags = {"content", "custom", "pipeline", "cnb", "cnaext"},
    .assets = {"custom/patrol.route"},
})
