// SPDX-License-Identifier: MS-PL
// Exercise the public Song importer, processor, writer, and named WAV-as-song route.
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbMediaCodec.hpp"
#include "CNA/Content/Pipeline/SongContentPipeline.hpp"
#include "CNA/Content/Pipeline/SoundEffectContentPipeline.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
using Microsoft::Xna::Framework::TitleLocation;

namespace
{
    class DiscardLogger final : public Pipeline::ContentBuildLogger
    {
    public:
        void Log(const Pipeline::ContentLogMessage&) override {}
    };

    struct TemporarySource
    {
        std::filesystem::path path;
        ~TemporarySource()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    };

    void SongPipelineComponents(CnaDemos::DemoContext& ctx)
    {
        ctx.Check(std::string(Pipeline::ImportedSongSourceType) ==
                          "CNA.Content.Pipeline.ImportedSongSource" &&
                      std::string(Pipeline::ProcessedSongType) == "CNA.Content.Cnb.SongData" &&
                      std::string(Pipeline::SongStreamReferenceParameter) == "streamReference" &&
                      std::string(Pipeline::SongNameParameter) == "name" &&
                      std::string(Pipeline::SongDurationMsParameter) == "durationMs",
                  "the Song route publishes stable value and parameter identities");

        const Pipeline::ImportedSongSource importedDefaults;
        ctx.Check(importedDefaults.mediaSource.empty() && importedDefaults.streamReference.empty() &&
                      importedDefaults.byteSize == 0u && !importedDefaults.authoredName.has_value() &&
                      !importedDefaults.authoredDurationMs.has_value(),
                  "ImportedSongSource starts as an empty non-owning source description");

        const Pipeline::SongImporter importer;
        ctx.Check(importer.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.SongImporter", "3"} &&
                      importer.SourceExtensions() == std::vector<std::string>{
                          ".mp3", ".ogg", ".oga", ".qoa", ".flac", ".opus", ".aac", ".wma"} &&
                      importer.OutputTypes() ==
                          std::vector<std::string>{Pipeline::ImportedSongSourceType} &&
                      !importer.SelectedByNameOnly(),
                  "the ordinary Song importer declares every unambiguous streaming extension");

        const Pipeline::WavSongImporter wavSongImporter;
        ctx.Check(wavSongImporter.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.WavSongImporter", "1"} &&
                      wavSongImporter.SourceExtensions() == std::vector<std::string>{".wav"} &&
                      wavSongImporter.OutputTypes() ==
                          std::vector<std::string>{Pipeline::ImportedSongSourceType} &&
                      wavSongImporter.SelectedByNameOnly(),
                  "the WAV-as-Song importer is an explicit-name reading of the shared source type");

        const Pipeline::SongProcessor processor;
        ctx.Check(processor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.SongProcessor", "2"} &&
                      processor.InputType() == Pipeline::ImportedSongSourceType &&
                      processor.OutputType() == Pipeline::ProcessedSongType &&
                      !processor.SelectedByNameOnly(),
                  "the processor connects external media identity to canonical Song metadata");

        Pipeline::ContentProcessorParameters validParameters;
        validParameters.Set(Pipeline::SongStreamReferenceParameter,
                            std::string("streams/theme.ogg"));
        validParameters.Set(Pipeline::SongNameParameter, std::string("Pipeline Theme"));
        validParameters.Set(Pipeline::SongDurationMsParameter, std::uint64_t{42000u});
        processor.ValidateParameters(validParameters);

        Pipeline::ContentProcessorParameters invalidParameters;
        invalidParameters.Set("sampleRate", std::uint64_t{44100u});
        bool unknownParameter = false;
        try
        {
            processor.ValidateParameters(invalidParameters);
        }
        catch (const Pipeline::ContentParameterError& error)
        {
            unknownParameter = error.Fault() == Pipeline::ContentParameterFault::UnknownName &&
                error.Parameter() == "sampleRate";
        }
        invalidParameters = {};
        invalidParameters.Set(Pipeline::SongNameParameter, std::uint64_t{1u});
        bool wrongNameType = false;
        try { processor.ValidateParameters(invalidParameters); }
        catch (const std::invalid_argument&) { wrongNameType = true; }
        invalidParameters = {};
        invalidParameters.Set(Pipeline::SongDurationMsParameter, std::uint64_t{2147483648ull});
        bool durationOverflow = false;
        try { processor.ValidateParameters(invalidParameters); }
        catch (const std::invalid_argument&) { durationOverflow = true; }
        invalidParameters = {};
        invalidParameters.Set(Pipeline::SongStreamReferenceParameter, std::string("../escape.ogg"));
        bool unsafeReference = false;
        try { processor.ValidateParameters(invalidParameters); }
        catch (const std::invalid_argument&) { unsafeReference = true; }
        ctx.Check(unknownParameter && wrongNameType && durationOverflow && unsafeReference,
                  "parameter validation distinguishes unknown, mistyped, overflowing, and unsafe values");

        const Pipeline::SongContentWriter writer;
        ctx.Check(writer.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.SongContentWriter", "1"} &&
                      writer.OutputFormat() == Pipeline::ContentOutputFormat::Cnb &&
                      writer.InputType() == Pipeline::ProcessedSongType &&
                      writer.OutputSchemaIdentities() ==
                          std::vector<Pipeline::ContentWriterSchemaIdentity>{{
                              Cnb::CnbAssetTypeId::Song,
                              Cnb::CnbMediaSchemaVersion,
                              "Microsoft.Xna.Framework.Media.Song",
                              {"CNA.Cnb.EncodeSongToCnb", "1"}}},
                  "the writer declares the exact Song asset, schema, and codec identity");

        const auto assetRoot =
            std::filesystem::canonical(std::filesystem::path(TitleLocation::Path()) / "assets");
        std::size_t durationProbeCalls = 0u;
        std::filesystem::path probedSource;
        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterSoundEffectContentPipeline(*registry);
        Pipeline::RegisterSongContentPipeline(
            *registry,
            [&](const std::filesystem::path& source) -> std::optional<std::uint32_t>
            {
                ++durationProbeCalls;
                probedSource = source;
                return 1234u;
            });

        const auto selectedSong = registry->ResolveImporter("theme.ogg");
        const auto selectedWav = registry->ResolveImporter("tone.wav");
        const auto selectedWavSong =
            registry->ResolveImporter("tone.wav", "CNA.WavSongImporter");
        ctx.Check(selectedSong->Identity() == importer.Identity() &&
                      selectedWav->Identity().name == "CNA.WavImporter" &&
                      selectedWavSong->Identity() == wavSongImporter.Identity(),
                  "combined registration keeps WAV as a sound by convention and exposes WAV-as-Song by name");

        const Pipeline::ContentPipeline pipeline(registry);
        Pipeline::ContentBuildRequest request;
        request.sourceRoot = assetRoot;
        request.source = "media/library_formats/format_vorbis.ogg";
        request.logicalName = "songs/vorbis";
        const Pipeline::ContentBuildResult built = pipeline.Build(request);
        const Cnb::CnbSongData decoded = Cnb::DecodeSongFromCnb(
            Cnb::CnbDocument::Parse(built.output.bytes, "pipeline song"));
        ctx.Check(durationProbeCalls == 1u &&
                      probedSource == std::filesystem::canonical(assetRoot / request.source) &&
                      built.importer == importer.Identity() && built.processor == processor.Identity() &&
                      built.writer == writer.Identity() &&
                      decoded.streamReference == "media/library_formats/format_vorbis.ogg" &&
                      decoded.name.empty() && decoded.durationMs == 1234u,
                  "a real Ogg source keeps its root-relative stream and caller-probed duration");

        ctx.Check(built.messages.size() == 2u &&
                      built.messages[0].stage == Pipeline::ContentPipelineStage::Import &&
                      built.messages[0].component == importer.Identity().name &&
                      built.messages[1].stage == Pipeline::ContentPipelineStage::Process &&
                      built.messages[1].component == processor.Identity().name &&
                      built.dependencies.size() == 1u &&
                      built.dependencies[0].kind == Pipeline::ContentDependencyKind::PrimarySource &&
                      built.runtimeReferences == std::vector<Pipeline::RuntimeContentReference>{{
                          "media/library_formats/format_vorbis.ogg", 0u}} &&
                      built.deploymentFiles == std::vector<Pipeline::ContentDeploymentFile>{{
                          std::filesystem::canonical(assetRoot / request.source),
                          "media/library_formats/format_vorbis.ogg", ""}},
                  "the build separates its source dependency, runtime XREF, and deployment file");

        ctx.Check(pipeline.Build(request).output.bytes == built.output.bytes &&
                      durationProbeCalls == 2u,
                  "an identical Song build probes again but produces byte-identical metadata");

        request.parameters = validParameters;
        const auto overridden = pipeline.Build(request);
        const auto overriddenData = Cnb::DecodeSongFromCnb(
            Cnb::CnbDocument::Parse(overridden.output.bytes, "overridden song"));
        ctx.Check(overriddenData.streamReference == "streams/theme.ogg" &&
                      overriddenData.name == "Pipeline Theme" &&
                      overriddenData.durationMs == 42000u &&
                      overridden.runtimeReferences ==
                          std::vector<Pipeline::RuntimeContentReference>{{"streams/theme.ogg", 0u}} &&
                      overridden.deploymentFiles.size() == 1u &&
                      overridden.deploymentFiles[0].outputPath == "streams/theme.ogg",
                  "processor parameters override stream destination, display name, and probed duration together");

        Pipeline::ContentBuildRequest wavRequest;
        wavRequest.sourceRoot = assetRoot;
        wavRequest.source = "sounds/tone_660.wav";
        wavRequest.logicalName = "songs/wav";
        wavRequest.importer = "CNA.WavSongImporter";
        const auto wavBuilt = pipeline.Build(wavRequest);
        const auto wavData = Cnb::DecodeSongFromCnb(
            Cnb::CnbDocument::Parse(wavBuilt.output.bytes, "WAV song"));
        ctx.Check(wavBuilt.importer == wavSongImporter.Identity() &&
                      wavData.streamReference == "sounds/tone_660.wav" &&
                      wavData.durationMs == 1234u && wavBuilt.deploymentFiles.size() == 1u &&
                      wavBuilt.deploymentFiles[0].outputPath == "sounds/tone_660.wav",
                  "the explicitly named WAV route records the same external Song shape without decoding PCM");

        Pipeline::ImportedSongSource authored;
        authored.mediaSource = std::filesystem::canonical(assetRoot / request.source);
        authored.streamReference = "authored/theme.ogg";
        authored.byteSize = std::filesystem::file_size(authored.mediaSource);
        authored.authoredName = "Authored Theme";
        authored.authoredDurationMs = 9876u;
        Pipeline::ContentSourceRootCapabilities externalRoots;
        Pipeline::ContentDependencyCollector dependencies;
        DiscardLogger logger;
        const Pipeline::ContentProcessorParameters noParameters;
        Pipeline::ContentProcessorContext processContext(
            assetRoot, authored.mediaSource, "songs/authored", processor.Identity().name,
            noParameters, externalRoots, dependencies, logger);
        const auto authoredOutput = processor.Process(
            Pipeline::ContentValue::Create(Pipeline::ImportedSongSourceType, authored),
            processContext);
        const Cnb::CnbSongData& authoredData = authoredOutput.Get<Cnb::CnbSongData>();
        ctx.Check(authoredOutput.StableType() == Pipeline::ProcessedSongType &&
                      authoredData.streamReference == authored.streamReference &&
                      authoredData.name == *authored.authoredName &&
                      authoredData.durationMs == *authored.authoredDurationMs &&
                      dependencies.RuntimeReferences() ==
                          std::vector<Pipeline::RuntimeContentReference>{{authored.streamReference, 0u}} &&
                      dependencies.DeploymentFiles().size() == 1u,
                  "direct processing preserves optional compatibility-container name and duration");

        auto noProbeRegistry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterSongContentPipeline(*noProbeRegistry);
        request.parameters = {};
        const auto unknownDuration = Pipeline::ContentPipeline(noProbeRegistry).Build(request);
        const auto unknownDurationData = Cnb::DecodeSongFromCnb(
            Cnb::CnbDocument::Parse(unknownDuration.output.bytes, "unprobed song"));
        ctx.Check(unknownDurationData.durationMs == 0u,
                  "the default importer represents an unavailable duration explicitly as zero");

        const auto temporaryRoot = std::filesystem::temp_directory_path() /
            ("cna-demos-empty-song-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(temporaryRoot))
        {
            throw std::runtime_error("Could not create the private empty-song directory");
        }
        TemporarySource cleanup{temporaryRoot};
        std::ofstream(temporaryRoot / "empty.ogg", std::ios::binary);
        Pipeline::ContentBuildRequest emptyRequest;
        emptyRequest.sourceRoot = temporaryRoot;
        emptyRequest.source = "empty.ogg";
        emptyRequest.logicalName = "songs/empty";
        bool emptyRefused = false;
        try
        {
            (void)Pipeline::ContentPipeline(noProbeRegistry).Build(emptyRequest);
        }
        catch (const Pipeline::ContentPipelineError& error)
        {
            emptyRefused = error.Stage() == Pipeline::ContentPipelineStage::Import &&
                error.Component() == importer.Identity().name &&
                std::string(error.what()).find("must not be empty") != std::string::npos;
        }
        ctx.Check(emptyRefused,
                  "an empty media source is refused at the Song importer boundary");
    }
}

CNA_DEMOS_CONSOLE(SongPipelineComponents, {
    .id = "content.song.pipeline_components",
    .title = "Song pipeline components",
    .description = "Builds real Ogg and WAV files through CNA's public external-Song pipeline, including duration "
                   "probing, metadata overrides, XREF/deployment output, and parameter refusals.",
    .apis = {"CNA::Content::Pipeline::ImportedSongSourceType",
             "CNA::Content::Pipeline::ProcessedSongType",
             "CNA::Content::Pipeline::SongStreamReferenceParameter",
             "CNA::Content::Pipeline::SongNameParameter",
             "CNA::Content::Pipeline::SongDurationMsParameter",
             "CNA::Content::Pipeline::SongDurationProbe",
             "CNA::Content::Pipeline::ImportedSongSource",
             "CNA::Content::Pipeline::SongImporter::SongImporter",
             "CNA::Content::Pipeline::SongImporter::Identity",
             "CNA::Content::Pipeline::SongImporter::SourceExtensions",
             "CNA::Content::Pipeline::SongImporter::OutputTypes",
             "CNA::Content::Pipeline::SongImporter::Import",
             "CNA::Content::Pipeline::WavSongImporter::WavSongImporter",
             "CNA::Content::Pipeline::WavSongImporter::Identity",
             "CNA::Content::Pipeline::WavSongImporter::SourceExtensions",
             "CNA::Content::Pipeline::WavSongImporter::OutputTypes",
             "CNA::Content::Pipeline::WavSongImporter::SelectedByNameOnly",
             "CNA::Content::Pipeline::WavSongImporter::Import",
             "CNA::Content::Pipeline::SongProcessor::Identity",
             "CNA::Content::Pipeline::SongProcessor::InputType",
             "CNA::Content::Pipeline::SongProcessor::OutputType",
             "CNA::Content::Pipeline::SongProcessor::ValidateParameters",
             "CNA::Content::Pipeline::SongProcessor::Process",
             "CNA::Content::Pipeline::SongContentWriter::Identity",
             "CNA::Content::Pipeline::SongContentWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::SongContentWriter::InputType",
             "CNA::Content::Pipeline::SongContentWriter::Write",
             "CNA::Content::Pipeline::RegisterSongContentPipeline"},
    .tags = {"content", "pipeline", "media", "song", "ogg", "wav", "cnb"},
    .assets = {"media/library_formats/format_vorbis.ogg", "sounds/tone_660.wav"},
})
