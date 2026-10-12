// SPDX-License-Identifier: MS-PL
// Forward contextual build messages while quantizing an authored opacity strip.
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbByteReader.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "CNA/Content/Pipeline/ContentPipeline.hpp"

namespace Pipeline = CNA::Content::Pipeline;
namespace Cnb = CNA::Content::Cnb;

namespace
{
    constexpr const char* SourceType = "CnaDemos.OpacityPercentages";
    constexpr const char* RuntimeType = "CnaDemos.OpacityStrip";
    constexpr const char* Reading = "Reading authored opacity percentages.";
    constexpr const char* Converting = "Converting percentages to normalized opacity samples.";
    constexpr const char* Loss = "Opacity values were quantized to 8-bit samples.";
    constexpr auto Data = Cnb::MakeChunkId('D', 'A', 'T', 'A');

    class OpacityImporter final : public Pipeline::ContentImporter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.OpacityImporter", "1"}; }
        std::vector<std::string> SourceExtensions() const override { return {".opacity"}; }
        std::vector<std::string> OutputTypes() const override { return {SourceType}; }
        Pipeline::ContentValue Import(Pipeline::ContentImporterContext& context) const override
        {
            context.LogInfo(Reading);
            std::ifstream source(context.SourcePath());
            if (!source) throw std::runtime_error("Could not open the opacity strip");
            std::vector<float> values;
            std::string token;
            while (source >> token)
            {
                std::istringstream number(token);
                float value;
                if (!(number >> value) || number.peek() != std::char_traits<char>::eof())
                    throw std::runtime_error("Opacity requires a nonempty list of numeric percentages");
                if (!std::isfinite(value) || value < 0 || value > 100 || values.size() == 256)
                    throw std::runtime_error("Opacity requires at most 256 finite percentages in [0, 100]");
                values.push_back(value);
            }
            if (!source.eof() || values.empty())
                throw std::runtime_error("Opacity requires a nonempty list of numeric percentages");
            return Pipeline::ContentValue::Create(SourceType, std::move(values));
        }
    };

    class OpacityProcessor final : public Pipeline::ContentProcessor
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.OpacityProcessor", "1"}; }
        std::string InputType() const override { return SourceType; }
        std::string OutputType() const override { return RuntimeType; }
        void ValidateParameters(const Pipeline::ContentProcessorParameters& parameters) const override
        {
            if (!parameters.Values().empty()) throw std::invalid_argument("Opacity has no processor parameters");
        }
        Pipeline::ContentValue Process(const Pipeline::ContentValue& input,
                                       Pipeline::ContentProcessorContext& context) const override
        {
            context.LogImportant(Converting);
            auto samples = input.Get<std::vector<float>>();
            for (float& value : samples) value /= 100.0f;
            return Pipeline::ContentValue::Create(RuntimeType, std::move(samples));
        }
    };

    class OpacityWriter final : public Pipeline::ContentTypeWriter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.OpacityWriter", "1"}; }
        std::string InputType() const override { return RuntimeType; }
        std::vector<Pipeline::ContentWriterSchemaIdentity> OutputSchemaIdentities() const override
        {
            return {{Cnb::CnbAssetTypeIdFromName(RuntimeType), 1u, RuntimeType, {"CnaDemos.OpacityCodec", "1"}}};
        }
        Pipeline::ContentWriteResult Write(const Pipeline::ContentValue& input,
                                           const std::string& logicalName) const override
        {
            const auto& samples = input.Get<std::vector<float>>();
            Cnb::CnbByteWriter payload;
            payload.WriteU32(static_cast<std::uint32_t>(samples.size()));
            bool lossy = false;
            for (const float value : samples)
            {
                const auto encoded = static_cast<std::uint8_t>(std::lround(value * 255.0f));
                payload.WriteU8(encoded);
                lossy |= static_cast<float>(encoded) / 255.0f != value;
            }
            Cnb::CnbWriter writer(Cnb::CnbAssetTypeIdFromName(RuntimeType), 1);
            writer.SetMetadata(RuntimeType, logicalName);
            writer.AddChunk(Data, payload.Take(), Cnb::CnbChunkFlags::Mandatory);
            Pipeline::ContentWriteResult result;
            result.bytes = writer.Build();
            result.assetTypeId = Cnb::CnbAssetTypeIdFromName(RuntimeType);
            result.assetTypeName = RuntimeType;
            result.assetSchemaVersion = 1;
            if (lossy) result.warnings.emplace_back(Loss);
            return result;
        }
    };

    // This example makes synchronous Build calls. A concurrently shared sink needs locking.
    class CaptureLogger final : public Pipeline::ContentBuildLogger
    {
    public:
        std::vector<Pipeline::ContentLogMessage> messages;
        void Log(const Pipeline::ContentLogMessage& message) override { messages.push_back(message); }
    };

    struct TemporarySource
    {
        std::filesystem::path path;
        ~TemporarySource() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };

    void PipelineDiagnostics(CnaDemos::DemoContext& ctx)
    {
        const auto root = std::filesystem::temp_directory_path() / ("cna-demos-opacity-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(root)) throw std::runtime_error("Private opacity directory already exists");
        {
            TemporarySource cleanup{root};
            const auto source = std::filesystem::canonical(root) / "fade.opacity";
            const auto writeSource = [&](const char* text)
            {
                std::ofstream file(source, std::ios::trunc);
                file << text;
                file.close();
                if (!file) throw std::runtime_error("Could not write the original opacity fixture");
            };
            writeSource("0 50 100\n");
            auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
            registry->RegisterImporter(std::make_shared<OpacityImporter>());
            registry->RegisterProcessor(std::make_shared<OpacityProcessor>());
            registry->RegisterWriter(std::make_shared<OpacityWriter>());
            const Pipeline::ContentPipeline pipeline(registry);
            CaptureLogger logger;
            Pipeline::ContentBuildRequest request;
            request.sourceRoot = root;
            request.source = "fade.opacity";
            request.logicalName = "ui/fade";
            request.logger = &logger;
            const auto built = pipeline.Build(request);

            const auto document = Cnb::CnbDocument::Parse(built.output.bytes, "opacity result");
            Cnb::CnbByteReader samples(document.ChunkData(document.FindSingle(Data).value()), "opacity samples");
            ctx.Check(samples.ReadU32() == 3 && samples.ReadU8() == 0 && samples.ReadU8() == 128 &&
                      samples.ReadU8() == 255 && samples.Remaining() == 0 && 128.0f / 255.0f != 0.5f,
                      "the real CNB stores 0, 128 and 255: the authored midpoint loses precision");
            const std::vector<Pipeline::ContentLogMessage> expected{
                {Pipeline::ContentLogLevel::Info, source, "ui/fade", Pipeline::ContentPipelineStage::Import,
                 "CnaDemos.OpacityImporter", Reading},
                {Pipeline::ContentLogLevel::Important, source, "ui/fade", Pipeline::ContentPipelineStage::Process,
                 "CnaDemos.OpacityProcessor", Converting},
                {Pipeline::ContentLogLevel::Warning, source, "ui/fade", Pipeline::ContentPipelineStage::Write,
                 "CnaDemos.OpacityWriter", Loss}};
            ctx.Check(built.messages == expected && built.output.warnings == std::vector<std::string>{Loss},
                      "import information, important processing and actual writer loss retain complete ordered context");
            ctx.Check(logger.messages == built.messages,
                      "the supplied ContentBuildLogger receives the same messages retained in the build result");

            request.logger = nullptr;
            const auto silent = pipeline.Build(request);
            ctx.Check(silent.messages == expected && silent.output.bytes == built.output.bytes && logger.messages == expected,
                      "a null downstream sink still records result messages without calling the previous logger");

            request.logger = &logger;
            logger.messages.clear();
            writeSource("0 100\n");
            const auto exact = pipeline.Build(request);
            const auto exactDocument = Cnb::CnbDocument::Parse(exact.output.bytes, "exact opacity result");
            Cnb::CnbByteReader endpoints(exactDocument.ChunkData(exactDocument.FindSingle(Data).value()), "endpoints");
            const std::vector<Pipeline::ContentLogMessage> noLoss(expected.begin(), expected.begin() + 2);
            ctx.Check(endpoints.ReadU32() == 2 && endpoints.ReadU8() == 0 && endpoints.ReadU8() == 255 &&
                      endpoints.Remaining() == 0 && exact.output.warnings.empty() &&
                      exact.messages == noLoss && logger.messages == noLoss,
                      "exactly representable endpoints produce the same component messages without a loss warning");

            logger.messages.clear();
            writeSource("0 invalid 100\n");
            bool refused = false;
            try { (void)pipeline.Build(request); }
            catch (const Pipeline::ContentPipelineError& error)
            {
                refused = error.Stage() == Pipeline::ContentPipelineStage::Import &&
                    error.Component() == "CnaDemos.OpacityImporter" && error.Source() == source &&
                    error.LogicalName() == "ui/fade" &&
                    std::string(error.what()).find("numeric percentages") != std::string::npos;
            }
            ctx.Check(refused, "malformed authored input refuses the build with importer, source and asset context");
            ctx.Check(logger.messages == std::vector<Pipeline::ContentLogMessage>{expected.front()},
                      "a failed build leaves its earlier import message in the caller's sink without later-stage messages");

            logger.messages.clear();
            writeSource("0 50 100\n");
            const auto repaired = pipeline.Build(request);
            ctx.Check(repaired.output.bytes == built.output.bytes && repaired.messages == expected && logger.messages == expected,
                      "repair and retry restore the original bytes and messages without carrying earlier build messages");
        }
        ctx.Check(!std::filesystem::exists(root), "the private authored opacity source is removed after all controls");
    }
}

CNA_DEMOS_CONSOLE(PipelineDiagnostics, {
    .id = "content.custom.pipeline_diagnostics",
    .title = "Content build messages and quantization loss (CNAEXT)",
    .description = "Quantizes an authored opacity strip and forwards contextual component messages and actual precision-loss warnings; checks silent sinks, failed builds and retry.",
    .apis = {"CNA::Content::Pipeline::ContentPipeline::Build",
             "CNA::Content::Pipeline::ContentBuildLogger::Log",
             "CNA::Content::Pipeline::ContentImporterContext::LogInfo",
             "CNA::Content::Pipeline::ContentProcessorContext::LogImportant",
             "CNA::Content::Pipeline::ContentWriteResult::warnings",
             "CNA::Content::Pipeline::ContentBuildResult::messages",
             "CNA::Content::Pipeline::ContentPipelineError::Stage"},
    .tags = {"content", "custom", "pipeline", "logging", "cnb", "cnaext"},
    .restrictions = "Synchronous in-memory builds with private authored inputs. Concurrent sink access, CLI verbosity and logger-thrown exceptions are not tested.",
})
