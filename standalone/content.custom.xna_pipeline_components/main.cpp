// SPDX-License-Identifier: MS-PL
// Port XNA-shaped components into CNA's real coordinator, including a nested conversion.
#include <algorithm>
#include <cmath>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>
#include <system_error>
#include <vector>

#include "CnaDemos/Process.hpp"
#include "CNA/Content/Cnb/CnbByteReader.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "CNA/Content/Pipeline/XnaPipelineBridge.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace Pipeline = CNA::Content::Pipeline;
namespace Xna = Microsoft::Xna::Framework::Content::Pipeline;
namespace Cnb = CNA::Content::Cnb;
using Microsoft::Xna::Framework::Vector2;

namespace
{
    constexpr const char* ImporterName = "CnaDemos.UnitRouteImporter";
    constexpr const char* ScaleName = "CnaDemos.PointScaleProcessor";
    constexpr const char* BoundsName = "CnaDemos.RouteBoundsProcessor";
    constexpr const char* BoundsType = "CnaDemos.RouteBounds";
    constexpr auto BoundsChunk = Cnb::MakeChunkId('B', 'N', 'D', 'S');

    struct RoutePoints
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.RoutePoints";
        std::vector<Vector2> points;
        float unitScale = 1.0f;
        std::filesystem::path sourceDirectory;
    };
    struct RouteBounds
    {
        static constexpr std::string_view XnaTypeName = BoundsType;
        float minX, minY, maxX, maxY;
    };

    class UnitRouteImporter final : public Xna::ContentImporter<RoutePoints>
    {
    public:
        RoutePoints Import(const std::string& filename, Xna::ContentImporterContext& context) override
        {
            // CNA's string boundary is UTF-8; let filesystem convert to the native path encoding.
            const std::filesystem::path sourcePath(std::u8string(filename.begin(), filename.end()));
            std::ifstream source(sourcePath);
            if (!source) throw Xna::InvalidContentException("Could not read the route source.");
            RoutePoints route;
            route.sourceDirectory = sourcePath.parent_path();
            float x, y;
            while (source >> x)
            {
                if (!(source >> y) || !std::isfinite(x) || !std::isfinite(y) || route.points.size() == 128)
                    throw Xna::InvalidContentException("A route needs finite coordinate pairs (at most 128).");
                route.points.emplace_back(x, y);
            }
            if (!source.eof() || route.points.size() < 2)
                throw Xna::InvalidContentException("A route needs at least two valid waypoints.");
            context.AddDependency("patrol.scale"); // relative to this source's directory
            std::ifstream units(sourcePath.parent_path() / "patrol.scale");
            if (!(units >> route.unitScale) || !std::isfinite(route.unitScale) || route.unitScale <= 0.0f)
                throw Xna::InvalidContentException("The route's unit scale must be finite and positive.");
            units >> std::ws;
            if (!units.eof()) throw Xna::InvalidContentException("The scale sidecar must contain one number.");
            return route;
        }
    };

    class PointScaleProcessor final : public Xna::ContentProcessor<RoutePoints, RoutePoints>
    {
    public:
        float getScaleProperty() const { return scale_; }
        void setScaleProperty(float value)
        {
            if (!std::isfinite(value) || value <= 0.0f) throw System::ArgumentOutOfRangeException("value");
            scale_ = value;
        }
        static void DescribeParameters(Xna::ProcessorParameterBindings<PointScaleProcessor>& parameters)
        {
            parameters.Add<float>("Scale", &PointScaleProcessor::getScaleProperty, &PointScaleProcessor::setScaleProperty);
        }
        RoutePoints Process(const RoutePoints& input, Xna::ContentProcessorContext&) override
        {
            RoutePoints converted = input;
            for (auto& point : converted.points) point *= scale_;
            converted.unitScale = 1.0f;
            return converted;
        }
    private:
        float scale_ = 1.0f;
    };

    class RouteBoundsProcessor final : public Xna::ContentProcessor<RoutePoints, RouteBounds>
    {
    public:
        bool getExportSummaryProperty() const { return exportSummary_; }
        void setExportSummaryProperty(bool value) { exportSummary_ = value; }
        float getPaddingProperty() const { return padding_; }
        void setPaddingProperty(float value)
        {
            if (!std::isfinite(value) || value < 0.0f) throw System::ArgumentOutOfRangeException("value");
            padding_ = value;
        }
        static void DescribeParameters(Xna::ProcessorParameterBindings<RouteBoundsProcessor>& parameters)
        {
            parameters.Add<bool>("ExportSummary", &RouteBoundsProcessor::getExportSummaryProperty,
                                 &RouteBoundsProcessor::setExportSummaryProperty);
            parameters.Add<float>("Padding", &RouteBoundsProcessor::getPaddingProperty,
                                  &RouteBoundsProcessor::setPaddingProperty);
        }
        RouteBounds Process(const RoutePoints& input, Xna::ContentProcessorContext& context) override
        {
            Xna::OpaqueDataDictionary parameters;
            parameters.SetValue<float>("Scale", input.unitScale);
            const auto converted = context.Convert<RoutePoints, RoutePoints>(input, ScaleName, parameters);
            RouteBounds bounds{converted.points.front().X, converted.points.front().Y,
                               converted.points.front().X, converted.points.front().Y};
            for (const auto& point : converted.points)
            {
                bounds.minX = std::min(bounds.minX, point.X); bounds.minY = std::min(bounds.minY, point.Y);
                bounds.maxX = std::max(bounds.maxX, point.X); bounds.maxY = std::max(bounds.maxY, point.Y);
            }
            bounds.minX -= padding_; bounds.minY -= padding_;
            bounds.maxX += padding_; bounds.maxY += padding_;
            if (exportSummary_)
            {
                context.AddDependency("patrol.label"); // a processor input, separate from the importer's unit scale
                std::ifstream labelFile(input.sourceDirectory / "patrol.label");
                std::string label;
                if (!std::getline(labelFile, label) || label.empty())
                    throw Xna::InvalidContentException("The route summary needs a nonempty patrol.label input.");
                const auto outputUtf8 = context.getOutputDirectoryProperty();
                if (outputUtf8.empty()) throw Xna::InvalidContentException("ExportSummary needs an output directory.");
                const std::filesystem::path directory(std::u8string(outputUtf8.begin(), outputUtf8.end()));
                std::filesystem::create_directories(directory);
                const auto report = directory / "patrol.bounds.txt";
                std::ofstream file(report);
                file << label << '\n' << bounds.minX << ' ' << bounds.minY << ' '
                     << bounds.maxX << ' ' << bounds.maxY << '\n';
                file.close();
                if (!file) throw Xna::InvalidContentException("Could not write the route bounds summary.");
                const auto reportUtf8 = report.generic_u8string();
                context.AddOutputFile(std::string(reportUtf8.begin(), reportUtf8.end()));
            }
            return bounds;
        }
    private:
        bool exportSummary_ = false;
        float padding_ = 0.0f;
    };

    // The last stage uses CNA's native CNB writer interface; no XNB compiler is claimed here.
    class BoundsWriter final : public Pipeline::ContentTypeWriter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override { return {"CnaDemos.BoundsWriter", "1"}; }
        std::string InputType() const override { return BoundsType; }
        std::vector<Pipeline::ContentWriterSchemaIdentity> OutputSchemaIdentities() const override
        {
            return {{Cnb::CnbAssetTypeIdFromName(BoundsType), 1u, BoundsType, {"CnaDemos.BoundsCodec", "1"}}};
        }
        Pipeline::ContentWriteResult Write(const Pipeline::ContentValue& input, const std::string& logicalName) const override
        {
            const auto& bounds = input.Get<RouteBounds>();
            Cnb::CnbByteWriter payload;
            payload.WriteF32(bounds.minX); payload.WriteF32(bounds.minY);
            payload.WriteF32(bounds.maxX); payload.WriteF32(bounds.maxY);
            Cnb::CnbWriter writer(Cnb::CnbAssetTypeIdFromName(BoundsType), 1);
            writer.SetMetadata(BoundsType, logicalName);
            writer.AddChunk(BoundsChunk, payload.Take(), Cnb::CnbChunkFlags::Mandatory);
            Pipeline::ContentWriteResult result;
            result.bytes = writer.Build();
            result.assetTypeId = Cnb::CnbAssetTypeIdFromName(BoundsType);
            result.assetTypeName = BoundsType;
            result.assetSchemaVersion = 1;
            return result;
        }
    };

    struct TemporaryDirectory
    {
        std::filesystem::path path;
        ~TemporaryDirectory() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };
    void WriteLabel(const std::filesystem::path& path, std::string_view label)
    {
        std::ofstream file(path); file << label << '\n'; file.close();
        if (!file) throw std::runtime_error("Could not write the original summary label.");
    }
    std::string ReadText(const std::filesystem::path& path)
    {
        std::ifstream file(path); std::ostringstream text; text << file.rdbuf();
        if (!file || file.bad()) throw std::runtime_error("Could not read the generated summary.");
        return text.str();
    }
    RouteBounds ReadBounds(const Pipeline::ContentBuildResult& built)
    {
        const auto document = Cnb::CnbDocument::Parse(built.output.bytes, "XNA bridge bounds");
        auto payload = document.OpenChunk(document.RequireSingle(BoundsChunk));
        RouteBounds bounds;
        bounds.minX = payload.ReadF32(); bounds.minY = payload.ReadF32();
        bounds.maxX = payload.ReadF32(); bounds.maxY = payload.ReadF32();
        payload.RequireExhausted();
        return bounds;
    }
}

int main()
{
    try
    {
        bool passed = true;
        const auto check = [&](bool condition, std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = condition && passed;
        };
        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Xna::ContentImporterAttribute importer(".route");
        importer.setDefaultProcessorProperty(BoundsName);
        Pipeline::RegisterXnaImporter<UnitRouteImporter>(*registry, ImporterName, importer);
        Pipeline::RegisterXnaProcessor<PointScaleProcessor>(*registry, ScaleName, Xna::ContentProcessorAttribute{});
        Pipeline::RegisterXnaProcessor<RouteBoundsProcessor>(*registry, BoundsName, Xna::ContentProcessorAttribute{});
        registry->RegisterWriter(std::make_shared<BoundsWriter>());
        const Pipeline::ContentPipeline pipeline(registry);
        Pipeline::ContentBuildRequest request;
        request.sourceRoot = CnaDemos::ExecutableDirectory() / "assets";
        request.source = "custom/patrol.route";
        request.logicalName = "route_bounds";
        const auto built = pipeline.Build(request);
        check(built.importer.name == ImporterName && built.processor.name == BoundsName &&
              built.writer.name == "CnaDemos.BoundsWriter",
              "the importer's default selects Bounds from two processors accepting RoutePoints");
        const auto bounds = ReadBounds(built);
        check(bounds.minX == 0.0f && bounds.minY == 0.0f && bounds.maxX == 6.0f && bounds.maxY == 8.0f,
              "the nested Scale conversion applies the sidecar's factor before bounds are compiled");
        bool primary = false, scale = false;
        for (const auto& dependency : built.dependencies)
        {
            primary |= dependency.kind == Pipeline::ContentDependencyKind::PrimarySource &&
                       dependency.identity.ends_with("/custom/patrol.route");
            scale |= dependency.kind == Pipeline::ContentDependencyKind::SourceFile &&
                     dependency.identity.ends_with("/custom/patrol.scale");
        }
        check(primary && scale && built.runtimeReferences.empty() && built.deploymentFiles.empty(),
              "the build records the route and scale source dependencies with no runtime references");
        request.parameters.Set("Padding", 1.5); // canonical Double is converted to the processor's Single
        const auto padded = ReadBounds(pipeline.Build(request));
        check(padded.minX == -1.5f && padded.minY == -1.5f && padded.maxX == 7.5f && padded.maxY == 9.5f,
              "registered parameter bindings assign the typed Padding property");
        request.parameters = Pipeline::ContentProcessorParameters{};
        check(pipeline.Build(request).output.bytes == built.output.bytes,
              "a following default build is independent of the preceding parameter override");
        for (bool unknown : {false, true})
        {
            auto invalid = request;
            if (unknown) invalid.parameters.Set("UnknownPadding", 1.5);
            else invalid.parameters.Set("Padding", std::string("wide"));
            bool rejected = false;
            try { (void)pipeline.Build(invalid); }
            catch (const Pipeline::ContentPipelineError& error)
            {
                rejected = error.Stage() == Pipeline::ContentPipelineStage::Process && error.Component() == BoundsName;
                std::cout << "Rejected parameter: " << error.what() << '\n';
            }
            check(rejected, unknown ? "an undeclared parameter reports the responsible processor"
                                    : "an unconvertible parameter reports the process stage and component");
        }
        const auto directory = std::filesystem::temp_directory_path() /
            ("cna-demos-route-summary-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(directory)) throw std::runtime_error("Could not reserve a private source/output directory.");
        {
            TemporaryDirectory cleanup{directory};
            const auto sources = directory / "source";
            std::filesystem::create_directories(sources / "custom");
            for (const char* name : {"patrol.route", "patrol.scale"})
                std::filesystem::copy_file(request.sourceRoot / "custom" / name, sources / "custom" / name);
            const auto label = sources / "custom/patrol.label";
            WriteLabel(label, "Courtyard patrol");
            auto summary = request; summary.sourceRoot = sources;
            summary.parameters.Set("ExportSummary", true);
            summary.environment.outputDirectory = directory / "output";
            bool separateOutput = false;
            try
            {
                const auto exported = pipeline.Build(summary);
                separateOutput = exported.deploymentFiles.size() == 1 &&
                    exported.deploymentFiles.front().outputPath == "patrol.bounds.txt";
            }
            catch (const Pipeline::ContentPipelineError& error)
            {
                // Preserve unrelated failures; this specific refusal reproduces CNA-DEMOS-ISSUE-047.
                if (error.Stage() != Pipeline::ContentPipelineStage::Process || error.Component() != BoundsName ||
                    std::string(error.what()).find("outside the primary source root") == std::string::npos) throw;
                std::cout << "Separate output refused: " << error.what() << '\n';
            }
            check(separateOutput, "AddOutputFile records a processor-written summary in the separate output directory (CNA-DEMOS-ISSUE-047)");
            check(ReadText(summary.environment.outputDirectory / "patrol.bounds.txt") == "Courtyard patrol\n0 0 6 8\n",
                  "the processor actually writes the label and computed bounds before registering the related output");
            // An explicit independent control for the currently accepted source-contained output layout.
            summary.environment.outputDirectory = sources / "reports";
            const auto contained = pipeline.Build(summary);
            const auto report = summary.environment.outputDirectory / "patrol.bounds.txt";
            bool labelDependency = false, reportDependency = false;
            for (const auto& dependency : contained.dependencies)
            {
                labelDependency |= dependency.kind == Pipeline::ContentDependencyKind::SourceFile &&
                    dependency.identity == label.generic_string();
                reportDependency |= dependency.kind == Pipeline::ContentDependencyKind::SourceFile &&
                    dependency.identity == report.generic_string();
            }
            check(labelDependency && reportDependency && contained.deploymentFiles.size() == 1 &&
                  contained.deploymentFiles.front().source == report &&
                  contained.deploymentFiles.front().outputPath == "patrol.bounds.txt" && contained.runtimeReferences.empty(),
                  "the source-contained control records the processor label dependency and the actual deployment file separately from runtime references");
            WriteLabel(label, "Revised courtyard patrol");
            const auto revised = pipeline.Build(summary);
            check(revised.output.bytes == contained.output.bytes &&
                  ReadText(report) == "Revised courtyard patrol\n0 0 6 8\n",
                  "an explicit new build reads the changed processor input and rewrites the summary without changing compiled bounds");
            std::filesystem::remove(label);
            bool missing = false;
            try { (void)pipeline.Build(summary); }
            catch (const Pipeline::ContentPipelineError& error)
            {
                missing = error.Stage() == Pipeline::ContentPipelineStage::Process && error.Component() == BoundsName &&
                    error.Source() == sources / summary.source && error.LogicalName() == summary.logicalName &&
                    std::string(error.what()).find("patrol.label") != std::string::npos;
                std::cout << "Missing label refused: " << error.what() << '\n';
            }
            check(missing, "a missing processor dependency is refused with the responsible process-stage context");
        }
        check(!std::filesystem::exists(directory), "private route copies, labels and generated summaries are removed");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "XNA pipeline components failed: " << error.what() << '\n';
        return 1;
    }
}
