// SPDX-License-Identifier: MS-PL
// Exercise CNA's built-in SoundEffect importer, processor, writer, and named compressed route.
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbSoundEffectCodec.hpp"
#include "CNA/Content/Cnb/CnbSourceImport.hpp"
#include "CNA/Content/Pipeline/SoundEffectContentPipeline.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Cnb = CNA::Content::Cnb;
namespace Import = CNA::Content::Import;
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

    void SoundEffectPipelineComponents(CnaDemos::DemoContext& ctx)
    {
        const Pipeline::WavImporter wavImporter;
        ctx.Check(wavImporter.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.WavImporter", "2"} &&
                      wavImporter.SourceExtensions() == std::vector<std::string>{".wav"} &&
                      wavImporter.OutputTypes() ==
                          std::vector<std::string>{Pipeline::ImportedSoundType} &&
                      !wavImporter.SelectedByNameOnly(),
                  "the WAV importer publishes one ordinary source route and a stable build identity");

        const Pipeline::SoundEffectProcessor processor;
        Pipeline::ContentProcessorParameters noParameters;
        processor.ValidateParameters(noParameters);
        ctx.Check(processor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.SoundEffectProcessor", "3"} &&
                      processor.InputType() == Pipeline::ImportedSoundType &&
                      processor.OutputType() == Pipeline::ProcessedSoundEffectType &&
                      !processor.SelectedByNameOnly(),
                  "the processor connects the imported and container-neutral sound types");

        Pipeline::ContentProcessorParameters unexpectedParameter;
        unexpectedParameter.Set("quality", std::string("Best"));
        bool directParameterRefusal = false;
        try
        {
            processor.ValidateParameters(unexpectedParameter);
        }
        catch (const std::invalid_argument& error)
        {
            directParameterRefusal = std::string(error.what()) ==
                "SoundEffectProcessor does not accept processor parameters.";
        }
        ctx.Check(directParameterRefusal,
                  "the fixed lossless processor rejects every parameter with a stable diagnostic");

        const Pipeline::SoundEffectContentWriter writer;
        const auto schemas = writer.OutputSchemaIdentities();
        ctx.Check(writer.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.SoundEffectContentWriter", "2"} &&
                      writer.OutputFormat() == Pipeline::ContentOutputFormat::Cnb &&
                      writer.InputType() == Pipeline::ProcessedSoundEffectType &&
                      schemas == std::vector<Pipeline::ContentWriterSchemaIdentity>{{
                          Cnb::CnbAssetTypeId::SoundEffect,
                          Cnb::CnbSoundEffectSchemaVersion,
                          "Microsoft.Xna.Framework.Audio.SoundEffect",
                          {"CNA.Cnb.EncodeSoundEffectToCnb", "1"}}},
                  "the writer declares its exact CNB asset, schema, and codec identity");

        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterSoundEffectContentPipeline(*registry);
        const auto selectedImporter = registry->ResolveImporter("tone.wav");
        const auto selectedProcessor = registry->ResolveProcessor(Pipeline::ImportedSoundType);
        const auto selectedWriter = registry->ResolveWriter(Pipeline::ProcessedSoundEffectType);
        ctx.Check(selectedImporter->Identity() == wavImporter.Identity() &&
                      selectedProcessor->Identity() == processor.Identity() &&
                      selectedWriter->Identity() == writer.Identity(),
                  "registration wires the three built-in components through their declared types");

        const auto assetRoot =
            std::filesystem::canonical(std::filesystem::path(TitleLocation::Path()) / "assets");
        Pipeline::ContentBuildRequest request;
        request.sourceRoot = assetRoot;
        request.source = "sounds/tone_660.wav";
        request.logicalName = "sounds/pipeline-tone";
        const Pipeline::ContentPipeline pipeline(registry);
        const Pipeline::ContentBuildResult built = pipeline.Build(request);
        ctx.Check(built.importer == wavImporter.Identity() &&
                      built.processor == processor.Identity() && built.writer == writer.Identity() &&
                      built.outputFormat == Pipeline::ContentOutputFormat::Cnb,
                  "a convention build selects the advertised importer, processor, and CNB writer");

        ctx.Check(built.messages.size() == 2u &&
                      built.messages[0].stage == Pipeline::ContentPipelineStage::Import &&
                      built.messages[0].component == wavImporter.Identity().name &&
                      built.messages[1].stage == Pipeline::ContentPipelineStage::Process &&
                      built.messages[1].component == processor.Identity().name &&
                      built.dependencies.size() == 1u && built.runtimeReferences.empty() &&
                      built.deploymentFiles.empty(),
                  "the successful route reports both transformations and only its primary dependency");

        const Cnb::CnbDocument document =
            Cnb::CnbDocument::Parse(built.output.bytes, "pipeline sound");
        const Cnb::CnbSoundEffectData decoded = Cnb::DecodeSoundEffectFromCnb(document);
        const Import::ImportedSound source =
            Cnb::ImportWavAsImportedSound(assetRoot / request.source);
        ctx.Check(document.Metadata().contentName == request.logicalName &&
                      decoded.format == Cnb::CnbAudioFormat::Pcm16 &&
                      decoded.sampleRate == source.sampleRate && decoded.channels == source.channels &&
                      decoded.frameCount == source.frameCount && decoded.loopStart == 0u &&
                      decoded.loopLength == source.frameCount && decoded.samples == source.samples,
                  "the emitted CNB keeps the real WAV PCM and applies the processor's whole-sound loop");

        ctx.Check(pipeline.Build(request).output.bytes == built.output.bytes,
                  "repeating the same build produces byte-identical SoundEffect content");

        request.parameters = unexpectedParameter;
        bool contextualParameterRefusal = false;
        try
        {
            (void)pipeline.Build(request);
        }
        catch (const Pipeline::ContentPipelineError& error)
        {
            contextualParameterRefusal =
                error.Stage() == Pipeline::ContentPipelineStage::Process &&
                error.Component() == processor.Identity().name &&
                error.Source() == std::filesystem::canonical(assetRoot / request.source) &&
                error.LogicalName() == request.logicalName;
        }
        ctx.Check(contextualParameterRefusal,
                  "the coordinator adds the source, asset, component, and Process stage to a refusal");

        const Pipeline::CompressedSoundImporter compressedDescriptor({});
        ctx.Check(compressedDescriptor.Identity() ==
                          Pipeline::ContentComponentIdentity{"CNA.CompressedSoundImporter", "1"} &&
                      compressedDescriptor.SourceExtensions() ==
                          std::vector<std::string>{".mp3", ".wma"} &&
                      compressedDescriptor.OutputTypes() ==
                          std::vector<std::string>{Pipeline::ImportedSoundType} &&
                      compressedDescriptor.SelectedByNameOnly(),
                  "the compressed importer advertises both XNA extensions as explicit-name routes");

        const auto temporaryRoot = std::filesystem::temp_directory_path() /
            ("cna-demos-compressed-sound-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(temporaryRoot))
        {
            throw std::runtime_error("Could not create the private compressed-sound directory");
        }
        TemporarySource cleanup{temporaryRoot};
        const auto disguisedWav = temporaryRoot / "tone.mp3";
        std::filesystem::copy_file(assetRoot / "sounds/tone_660.wav", disguisedWav);

        std::size_t decoderCalls = 0u;
        std::filesystem::path decodedPath;
        auto compressedRegistry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterSoundEffectContentPipeline(
            *compressedRegistry,
            [&](const std::filesystem::path& path)
                -> std::optional<Import::ImportedSound>
            {
                ++decoderCalls;
                decodedPath = path;
                return Cnb::ImportWavAsImportedSound(path);
            });
        bool defaultCompressedRefused = false;
        try
        {
            (void)compressedRegistry->ResolveImporter(disguisedWav);
        }
        catch (const std::logic_error&)
        {
            defaultCompressedRefused = true;
        }
        const auto namedCompressed = compressedRegistry->ResolveImporter(
            disguisedWav, "CNA.CompressedSoundImporter");
        ctx.Check(namedCompressed->Identity() == compressedDescriptor.Identity(),
                  "the compressed importer resolves by its stable name");

        Pipeline::ContentBuildRequest compressedRequest;
        compressedRequest.sourceRoot = temporaryRoot;
        compressedRequest.source = "tone.mp3";
        compressedRequest.logicalName = "sounds/compressed-tone";
        compressedRequest.importer = "CNA.CompressedSoundImporter";
        const Pipeline::ContentPipeline compressedPipeline(compressedRegistry);
        const auto compressedBuilt = compressedPipeline.Build(compressedRequest);
        const auto compressedData = Cnb::DecodeSoundEffectFromCnb(
            Cnb::CnbDocument::Parse(compressedBuilt.output.bytes, "compressed pipeline sound"));
        ctx.Check(decoderCalls == 1u && decodedPath == std::filesystem::canonical(disguisedWav) &&
                      compressedBuilt.importer == compressedDescriptor.Identity() &&
                      compressedData.samples == source.samples &&
                      compressedData.loopLength == source.frameCount,
                  "an explicitly named compressed route consumes the caller's decoder result end to end");

        auto missingDecoderRegistry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterSoundEffectContentPipeline(*missingDecoderRegistry, {});
        bool missingDecoderRefused = false;
        try
        {
            (void)Pipeline::ContentPipeline(missingDecoderRegistry).Build(compressedRequest);
        }
        catch (const Pipeline::ContentPipelineError& error)
        {
            missingDecoderRefused = error.Stage() == Pipeline::ContentPipelineStage::Import &&
                error.Component() == compressedDescriptor.Identity().name &&
                std::string(error.what()).find("no audio decoder") != std::string::npos;
        }

        auto unreadableRegistry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterSoundEffectContentPipeline(
            *unreadableRegistry,
            [](const std::filesystem::path&) -> std::optional<Import::ImportedSound>
            {
                return std::nullopt;
            });
        bool unreadableRefused = false;
        try
        {
            (void)Pipeline::ContentPipeline(unreadableRegistry).Build(compressedRequest);
        }
        catch (const Pipeline::ContentPipelineError& error)
        {
            unreadableRefused = error.Stage() == Pipeline::ContentPipelineStage::Import &&
                error.Component() == compressedDescriptor.Identity().name &&
                std::string(error.what()).find("could not read") != std::string::npos;
        }
        ctx.Check(missingDecoderRefused && unreadableRefused,
                  "missing and unsuccessful compressed decoders fail at the importer boundary");

        ctx.Check(defaultCompressedRefused,
                  "a named-only compressed importer is excluded from convention selection even when alone");
    }
}

CNA_DEMOS_CONSOLE(SoundEffectPipelineComponents, {
    .id = "content.sound_effect.pipeline_components",
    .title = "SoundEffect pipeline components",
    .description = "Builds a real WAV through CNA's public importer, processor, and CNB writer, then demonstrates the "
                   "explicit-name compressed-source adapter with a caller-supplied decoder.",
    .apis = {"CNA::Content::Pipeline::ImportedSoundType",
             "CNA::Content::Pipeline::ProcessedSoundEffectType",
             "CNA::Content::Pipeline::WavImporter::Identity",
             "CNA::Content::Pipeline::WavImporter::SourceExtensions",
             "CNA::Content::Pipeline::WavImporter::OutputTypes",
             "CNA::Content::Pipeline::WavImporter::Import",
             "CNA::Content::Pipeline::SoundEffectProcessor::Identity",
             "CNA::Content::Pipeline::SoundEffectProcessor::InputType",
             "CNA::Content::Pipeline::SoundEffectProcessor::OutputType",
             "CNA::Content::Pipeline::SoundEffectProcessor::ValidateParameters",
             "CNA::Content::Pipeline::SoundEffectProcessor::Process",
             "CNA::Content::Pipeline::SoundEffectContentWriter::Identity",
             "CNA::Content::Pipeline::SoundEffectContentWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::SoundEffectContentWriter::InputType",
             "CNA::Content::Pipeline::SoundEffectContentWriter::Write",
             "CNA::Content::Pipeline::CompressedSoundImporter::Identity",
             "CNA::Content::Pipeline::CompressedSoundImporter::SourceExtensions",
             "CNA::Content::Pipeline::CompressedSoundImporter::OutputTypes",
             "CNA::Content::Pipeline::CompressedSoundImporter::SelectedByNameOnly",
             "CNA::Content::Pipeline::CompressedSoundImporter::Import",
             "CNA::Content::Pipeline::RegisterSoundEffectContentPipeline"},
    .tags = {"content", "pipeline", "audio", "wav", "cnb", "importer", "processor", "writer"},
    .assets = {"sounds/tone_660.wav"},
})
