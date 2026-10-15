// SPDX-License-Identifier: MS-PL
// Exercise CNA's public external-Video importer, processor, and writer workflow.
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbMediaCodec.hpp"
#include "CNA/Content/Pipeline/VideoContentPipeline.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
using Microsoft::Xna::Framework::TitleLocation;

namespace
{
    struct TemporarySource
    {
        std::filesystem::path path;
        ~TemporarySource()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    };

    void VideoPipelineComponents(CnaDemos::DemoContext& ctx)
    {
        ctx.Check(std::string(Pipeline::ImportedVideoSourceType) ==
                          "CNA.Content.Pipeline.ImportedVideoSource" &&
                      std::string(Pipeline::ProcessedVideoType) == "CNA.Content.Cnb.VideoData" &&
                      std::string(Pipeline::VideoStreamReferenceParameter) == "streamReference" &&
                      std::string(Pipeline::VideoDurationMsParameter) == "durationMs" &&
                      std::string(Pipeline::VideoWidthParameter) == "width" &&
                      std::string(Pipeline::VideoHeightParameter) == "height" &&
                      std::string(Pipeline::VideoFramesPerSecondParameter) == "framesPerSecond" &&
                      std::string(Pipeline::VideoSoundtrackTypeParameter) == "soundtrackType",
                  "the Video route publishes stable value and parameter identities");

        const Pipeline::ProbedVideoMetadata probedDefaults;
        const Pipeline::ImportedVideoSource importedDefaults;
        ctx.Check(probedDefaults.width == 0u && probedDefaults.height == 0u &&
                      probedDefaults.framesPerSecond == 0.0f && probedDefaults.durationMs == 0u &&
                      probedDefaults.soundtrackType == 0u && importedDefaults.mediaSource.empty() &&
                      importedDefaults.streamReference.empty() && importedDefaults.byteSize == 0u &&
                      !importedDefaults.probed.has_value(),
                  "the probed and imported source carriers start in explicit unknown states");

        const Pipeline::VideoImporter importer;
        ctx.Check(importer.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.VideoImporter", "2"} &&
                      importer.SourceExtensions() == std::vector<std::string>{
                          ".wmv", ".mp4", ".ogv", ".webm", ".mkv", ".avi", ".mov"} &&
                      importer.OutputTypes() ==
                          std::vector<std::string>{Pipeline::ImportedVideoSourceType} &&
                      !importer.SelectedByNameOnly(),
                  "the Video importer declares every ordinary external-stream extension");

        const Pipeline::VideoProcessor parameterProcessor;
        const Pipeline::VideoProcessor probeProcessor(true);
        ctx.Check(parameterProcessor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.VideoProcessor", "2"} &&
                      parameterProcessor.InputType() == Pipeline::ImportedVideoSourceType &&
                      parameterProcessor.OutputType() == Pipeline::ProcessedVideoType &&
                      probeProcessor.Identity() == parameterProcessor.Identity(),
                  "both metadata modes share one stable processor route");

        Pipeline::ContentProcessorParameters frameParameters;
        frameParameters.Set(Pipeline::VideoWidthParameter, std::uint64_t{320u});
        frameParameters.Set(Pipeline::VideoHeightParameter, std::uint64_t{180u});
        frameParameters.Set(Pipeline::VideoFramesPerSecondParameter, 24.0);
        parameterProcessor.ValidateParameters(frameParameters);
        const Pipeline::ContentProcessorParameters noParameters;
        probeProcessor.ValidateParameters(noParameters);
        ctx.Check(true,
                  "explicit metadata satisfies the parameter mode while probe mode accepts overrides as optional");

        Pipeline::ContentProcessorParameters invalid;
        invalid.Set("quality", std::uint64_t{1u});
        bool unknownParameter = false;
        try { parameterProcessor.ValidateParameters(invalid); }
        catch (const Pipeline::ContentParameterError& error)
        {
            unknownParameter = error.Fault() == Pipeline::ContentParameterFault::UnknownName &&
                error.Parameter() == "quality";
        }
        invalid = {};
        invalid.Set(Pipeline::VideoHeightParameter, std::uint64_t{180u});
        invalid.Set(Pipeline::VideoFramesPerSecondParameter, 24.0);
        bool missingWidth = false;
        try { parameterProcessor.ValidateParameters(invalid); }
        catch (const std::invalid_argument&) { missingWidth = true; }
        invalid = frameParameters;
        invalid.Set(Pipeline::VideoWidthParameter, std::uint64_t{0u});
        bool zeroWidth = false;
        try { parameterProcessor.ValidateParameters(invalid); }
        catch (const std::invalid_argument&) { zeroWidth = true; }
        invalid = frameParameters;
        invalid.Set(Pipeline::VideoFramesPerSecondParameter, 0.0);
        bool zeroRate = false;
        try { parameterProcessor.ValidateParameters(invalid); }
        catch (const std::invalid_argument&) { zeroRate = true; }
        invalid = frameParameters;
        invalid.Set(Pipeline::VideoDurationMsParameter, std::uint64_t{2147483648ull});
        bool durationOverflow = false;
        try { parameterProcessor.ValidateParameters(invalid); }
        catch (const std::invalid_argument&) { durationOverflow = true; }
        invalid = frameParameters;
        invalid.Set(Pipeline::VideoSoundtrackTypeParameter, std::uint64_t{3u});
        bool soundtrackRange = false;
        try { parameterProcessor.ValidateParameters(invalid); }
        catch (const std::invalid_argument&) { soundtrackRange = true; }
        invalid = frameParameters;
        invalid.Set(Pipeline::VideoStreamReferenceParameter, std::string("../escape.mkv"));
        bool unsafeReference = false;
        try { parameterProcessor.ValidateParameters(invalid); }
        catch (const std::invalid_argument&) { unsafeReference = true; }
        ctx.Check(unknownParameter && missingWidth && zeroWidth && zeroRate && durationOverflow &&
                      soundtrackRange && unsafeReference,
                  "validation distinguishes unknown, missing, zero, overflowing, enum, and unsafe values");

        const Pipeline::VideoContentWriter writer;
        ctx.Check(writer.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.VideoContentWriter", "1"} &&
                      writer.OutputFormat() == Pipeline::ContentOutputFormat::Cnb &&
                      writer.InputType() == Pipeline::ProcessedVideoType &&
                      writer.OutputSchemaIdentities() ==
                          std::vector<Pipeline::ContentWriterSchemaIdentity>{{
                              Cnb::CnbAssetTypeId::Video,
                              Cnb::CnbMediaSchemaVersion,
                              "Microsoft.Xna.Framework.Media.Video",
                              {"CNA.Cnb.EncodeVideoToCnb", "1"}}},
                  "the writer declares the exact Video asset, schema, and codec identity");

        const auto assetRoot =
            std::filesystem::canonical(std::filesystem::path(TitleLocation::Path()) / "assets");
        const Pipeline::ProbedVideoMetadata sourceMetadata{160u, 90u, 10.0f, 1000u, 2u};
        std::size_t probeCalls = 0u;
        std::filesystem::path probedPath;
        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterVideoContentPipeline(
            *registry,
            [&](const std::filesystem::path& source)
                -> std::optional<Pipeline::ProbedVideoMetadata>
            {
                ++probeCalls;
                probedPath = source;
                return sourceMetadata;
            });
        ctx.Check(registry->ResolveImporter("movie.mkv")->Identity() == importer.Identity() &&
                      registry->ResolveProcessor(Pipeline::ImportedVideoSourceType)->Identity() ==
                          parameterProcessor.Identity() &&
                      registry->ResolveWriter(Pipeline::ProcessedVideoType)->Identity() ==
                          writer.Identity(),
                  "registration wires the importer, processor, and writer through their declared types");

        const Pipeline::ContentPipeline pipeline(registry);
        Pipeline::ContentBuildRequest request;
        request.sourceRoot = assetRoot;
        request.source = "media/videos/track_matrix.mkv";
        request.logicalName = "videos/probed";
        const Pipeline::ContentBuildResult built = pipeline.Build(request);
        const Cnb::CnbVideoData decoded = Cnb::DecodeVideoFromCnb(
            Cnb::CnbDocument::Parse(built.output.bytes, "pipeline video"));
        ctx.Check(probeCalls == 1u &&
                      probedPath == std::filesystem::canonical(assetRoot / request.source) &&
                      built.importer == importer.Identity() &&
                      built.processor == parameterProcessor.Identity() &&
                      built.writer == writer.Identity() &&
                      decoded.streamReference == "media/videos/track_matrix.mkv" &&
                      decoded.durationMs == sourceMetadata.durationMs &&
                      decoded.width == sourceMetadata.width && decoded.height == sourceMetadata.height &&
                      decoded.framesPerSecond == sourceMetadata.framesPerSecond &&
                      decoded.soundtrackType == sourceMetadata.soundtrackType,
                  "a real MKV source keeps its root-relative stream and all caller-probed metadata");

        ctx.Check(built.messages.size() == 2u &&
                      built.messages[0].stage == Pipeline::ContentPipelineStage::Import &&
                      built.messages[0].component == importer.Identity().name &&
                      built.messages[1].stage == Pipeline::ContentPipelineStage::Process &&
                      built.messages[1].component == parameterProcessor.Identity().name &&
                      built.dependencies.size() == 1u &&
                      built.dependencies[0].kind == Pipeline::ContentDependencyKind::PrimarySource &&
                      built.runtimeReferences == std::vector<Pipeline::RuntimeContentReference>{{
                          "media/videos/track_matrix.mkv", 0u}} &&
                      built.deploymentFiles == std::vector<Pipeline::ContentDeploymentFile>{{
                          std::filesystem::canonical(assetRoot / request.source),
                          "media/videos/track_matrix.mkv", ""}},
                  "the build separates its source dependency, runtime XREF, and deployment file");

        ctx.Check(pipeline.Build(request).output.bytes == built.output.bytes && probeCalls == 2u,
                  "an identical Video build probes again but produces byte-identical metadata");

        Pipeline::ContentProcessorParameters overrides;
        overrides.Set(Pipeline::VideoStreamReferenceParameter,
                      std::string("streams/override.mkv"));
        overrides.Set(Pipeline::VideoDurationMsParameter, std::uint64_t{2500u});
        overrides.Set(Pipeline::VideoWidthParameter, std::uint64_t{320u});
        overrides.Set(Pipeline::VideoHeightParameter, std::uint64_t{180u});
        overrides.Set(Pipeline::VideoFramesPerSecondParameter, 24.0);
        overrides.Set(Pipeline::VideoSoundtrackTypeParameter, std::uint64_t{1u});
        request.parameters = overrides;
        const auto overridden = pipeline.Build(request);
        const auto overriddenData = Cnb::DecodeVideoFromCnb(
            Cnb::CnbDocument::Parse(overridden.output.bytes, "overridden video"));
        ctx.Check(overriddenData.streamReference == "streams/override.mkv" &&
                      overriddenData.durationMs == 2500u && overriddenData.width == 320u &&
                      overriddenData.height == 180u && overriddenData.framesPerSecond == 24.0f &&
                      overriddenData.soundtrackType == 1u &&
                      overridden.runtimeReferences ==
                          std::vector<Pipeline::RuntimeContentReference>{{"streams/override.mkv", 0u}} &&
                      overridden.deploymentFiles.size() == 1u &&
                      overridden.deploymentFiles[0].outputPath == "streams/override.mkv",
                  "processor parameters override the stream and every probed metadata field");

        auto unreadableRegistry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterVideoContentPipeline(
            *unreadableRegistry,
            [](const std::filesystem::path&) -> std::optional<Pipeline::ProbedVideoMetadata>
            {
                return std::nullopt;
            });
        const Pipeline::ContentPipeline unreadablePipeline(unreadableRegistry);
        request.parameters = {};
        bool missingProbeMetadata = false;
        try
        {
            (void)unreadablePipeline.Build(request);
        }
        catch (const Pipeline::ContentPipelineError& error)
        {
            missingProbeMetadata = error.Stage() == Pipeline::ContentPipelineStage::Process &&
                error.Component() == parameterProcessor.Identity().name &&
                std::string(error.what()).find("could not read the frame metadata") !=
                    std::string::npos;
        }
        request.parameters = frameParameters;
        const auto recovered = unreadablePipeline.Build(request);
        const auto recoveredData = Cnb::DecodeVideoFromCnb(
            Cnb::CnbDocument::Parse(recovered.output.bytes, "parameter video"));
        ctx.Check(missingProbeMetadata && recoveredData.width == 320u &&
                      recoveredData.height == 180u && recoveredData.framesPerSecond == 24.0f &&
                      recoveredData.durationMs == 0u && recoveredData.soundtrackType == 0u,
                  "an unreadable probe result needs frame parameters and then uses optional defaults");

        auto parameterRegistry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterVideoContentPipeline(*parameterRegistry);
        const auto parameterBuilt = Pipeline::ContentPipeline(parameterRegistry).Build(request);
        const auto parameterData = Cnb::DecodeVideoFromCnb(
            Cnb::CnbDocument::Parse(parameterBuilt.output.bytes, "no-probe video"));
        ctx.Check(parameterData.width == 320u && parameterData.height == 180u &&
                      parameterData.framesPerSecond == 24.0f && parameterData.durationMs == 0u,
                  "a build registered without a probe succeeds from the three required parameters");

        const auto temporaryRoot = std::filesystem::temp_directory_path() /
            ("cna-demos-empty-video-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(temporaryRoot))
        {
            throw std::runtime_error("Could not create the private empty-video directory");
        }
        TemporarySource cleanup{temporaryRoot};
        std::ofstream(temporaryRoot / "empty.mkv", std::ios::binary);
        Pipeline::ContentBuildRequest emptyRequest;
        emptyRequest.sourceRoot = temporaryRoot;
        emptyRequest.source = "empty.mkv";
        emptyRequest.logicalName = "videos/empty";
        emptyRequest.parameters = frameParameters;
        bool emptyRefused = false;
        try
        {
            (void)Pipeline::ContentPipeline(parameterRegistry).Build(emptyRequest);
        }
        catch (const Pipeline::ContentPipelineError& error)
        {
            emptyRefused = error.Stage() == Pipeline::ContentPipelineStage::Import &&
                error.Component() == importer.Identity().name &&
                std::string(error.what()).find("must not be empty") != std::string::npos;
        }
        ctx.Check(emptyRefused,
                  "an empty media source is refused at the Video importer boundary");
    }
}

CNA_DEMOS_CONSOLE(VideoPipelineComponents, {
    .id = "content.video.pipeline_components",
    .title = "Video pipeline components",
    .description = "Builds a real external video through CNA's public probe-aware pipeline, including metadata "
                   "overrides, XREF/deployment output, parameter fallback, and import refusal.",
    .apis = {"CNA::Content::Pipeline::ImportedVideoSourceType",
             "CNA::Content::Pipeline::ProcessedVideoType",
             "CNA::Content::Pipeline::VideoStreamReferenceParameter",
             "CNA::Content::Pipeline::VideoDurationMsParameter",
             "CNA::Content::Pipeline::VideoWidthParameter",
             "CNA::Content::Pipeline::VideoHeightParameter",
             "CNA::Content::Pipeline::VideoFramesPerSecondParameter",
             "CNA::Content::Pipeline::VideoSoundtrackTypeParameter",
             "CNA::Content::Pipeline::ProbedVideoMetadata",
             "CNA::Content::Pipeline::VideoMetadataProbe",
             "CNA::Content::Pipeline::ImportedVideoSource",
             "CNA::Content::Pipeline::VideoImporter::VideoImporter",
             "CNA::Content::Pipeline::VideoImporter::Identity",
             "CNA::Content::Pipeline::VideoImporter::SourceExtensions",
             "CNA::Content::Pipeline::VideoImporter::OutputTypes",
             "CNA::Content::Pipeline::VideoImporter::Import",
             "CNA::Content::Pipeline::VideoProcessor::VideoProcessor",
             "CNA::Content::Pipeline::VideoProcessor::Identity",
             "CNA::Content::Pipeline::VideoProcessor::InputType",
             "CNA::Content::Pipeline::VideoProcessor::OutputType",
             "CNA::Content::Pipeline::VideoProcessor::ValidateParameters",
             "CNA::Content::Pipeline::VideoProcessor::Process",
             "CNA::Content::Pipeline::VideoContentWriter::Identity",
             "CNA::Content::Pipeline::VideoContentWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::VideoContentWriter::InputType",
             "CNA::Content::Pipeline::VideoContentWriter::Write",
             "CNA::Content::Pipeline::RegisterVideoContentPipeline"},
    .tags = {"content", "pipeline", "media", "video", "mkv", "cnb"},
    .needs = CnaDemos::Need::Video,
    .assets = {"media/videos/track_matrix.mkv"},
})
