// SPDX-License-Identifier: MS-PL
// Give a CNJ texture explicit access to shared art without changing its authored reference.
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>
#include <nlohmann/json.hpp>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "CNA/Content/Pipeline/CnjContentPipeline.hpp"
#include "CNA/Content/Pipeline/Texture2DContentPipeline.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Pipeline = CNA::Content::Pipeline;
namespace Cnb = CNA::Content::Cnb;
using Microsoft::Xna::Framework::TitleLocation;

namespace
{
    struct TemporarySources
    {
        std::filesystem::path path;
        ~TemporarySources() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };

    void SourceRootCapabilities(CnaDemos::DemoContext& ctx)
    {
        const auto root = std::filesystem::temp_directory_path() / ("cna-demos-shared-art-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(root)) throw std::runtime_error("Private shared-art directory already exists");
        {
            TemporarySources cleanup{root};
            const auto canonical = std::filesystem::canonical(root);
            const auto source = canonical / "source";
            const auto shared = canonical / "shared";
            const auto relocated = canonical / "relocated";
            for (const auto& path : {source, shared, relocated}) std::filesystem::create_directory(path);
            const auto assets = std::filesystem::path(TitleLocation::Path()) / "assets/textures";
            // Distinct original fixtures at the same basename expose which root was actually read.
            std::filesystem::copy_file(assets / "checker.png", shared / "image.png");
            std::filesystem::copy_file(assets / "checker.png", relocated / "image.png");
            std::filesystem::copy_file(assets / "sprite_strip.png", source / "image.png");
            const auto checker = Pipeline::DecodeImportedImage(assets / "checker.png");
            const auto strip = Pipeline::DecodeImportedImage(assets / "sprite_strip.png");
            const auto recipe = source / "texture.cnj";
            const auto writeRecipe = [&](const std::string& reference)
            {
                std::ofstream file(recipe, std::ios::trunc);
                file << nlohmann::json{{"cnjVersion", 1}, {"type", "Texture2D"}, {"sourceFile", reference}};
                file.close();
                if (!file) throw std::runtime_error("Could not write the original CNJ texture recipe");
            };
            auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
            Pipeline::RegisterTexture2DContentPipeline(*registry);
            Pipeline::RegisterCnjContentPipeline(*registry);
            const Pipeline::ContentPipeline pipeline(registry);
            Pipeline::ContentBuildRequest request;
            request.sourceRoot = source;
            request.source = recipe;
            request.logicalName = "ui/shared_image";
            request.externalSourceRoots.Add("art", shared);
            request.parameters.Set(Pipeline::TextureFormatParameter, std::string("Color"));
            request.parameters.Set(Pipeline::TexturePremultiplyAlphaParameter, false);
            writeRecipe("@art/image.png");
            const auto built = pipeline.Build(request);
            ctx.Print("Authored CNJ sourceFile: @art/image.png");
            for (const auto& dependency : built.dependencies)
                ctx.Print("Dependency: {} (source root '{}')", dependency.identity, dependency.sourceRoot);
            const auto matchesPixels = [](const Pipeline::ContentBuildResult& result,
                                          const Pipeline::ImportedImage& expected)
            {
                const auto document = Cnb::CnbDocument::Parse(result.output.bytes, "shared-art result");
                const auto texture = Cnb::DecodeTexture2DFromCnb(document);
                return texture.width == expected.width && texture.height == expected.height &&
                    texture.depth == 1 && texture.faceCount == 1 && texture.mipCount == 1 &&
                    texture.representations.size() == 1 &&
                    texture.representations[0].format == Cnb::CnbTextureFormat::Rgba8 &&
                    texture.representations[0].levels == std::vector<std::vector<std::uint8_t>>{expected.rgbaPixels};
            };
            ctx.Check(matchesPixels(built, checker),
                      "the built-in CNJ pipeline compiles exact pixels from the explicitly granted shared-art root");
            const std::vector<Pipeline::ContentDependency> expectedDependencies{
                {Pipeline::ContentDependencyKind::PrimarySource, recipe.generic_string(), ""},
                {Pipeline::ContentDependencyKind::SourceFile, (shared / "image.png").generic_string(), "art"}};
            ctx.Check(built.dependencies == expectedDependencies && built.runtimeReferences.empty() &&
                      built.deploymentFiles.empty(),
                      "the primary recipe and external source dependency retain distinct paths and the art capability alias");

            auto moved = request;
            moved.externalSourceRoots = {};
            moved.externalSourceRoots.Add("art", relocated);
            const auto remapped = pipeline.Build(moved);
            auto remappedDependencies = expectedDependencies;
            remappedDependencies[1].identity = (relocated / "image.png").generic_string();
            ctx.Check(remapped.output.bytes == built.output.bytes && remapped.dependencies == remappedDependencies,
                      "remapping the same authored alias preserves compiled bytes while recording the actual new source path");

            writeRecipe("image.png");
            const auto local = pipeline.Build(request);
            auto localDependencies = expectedDependencies;
            localDependencies[1] = {Pipeline::ContentDependencyKind::SourceFile,
                                    (source / "image.png").generic_string(), ""};
            ctx.Check(matchesPixels(local, strip) && local.output.bytes != built.output.bytes &&
                      local.dependencies == localDependencies,
                      "an unqualified reference stays relative to its recipe despite an available shared-art capability");

            const auto refused = [&](const Pipeline::ContentBuildRequest& candidate,
                                     Pipeline::ContentPipelineStage stage, const std::string& reason)
            {
                try { (void)pipeline.Build(candidate); }
                catch (const Pipeline::ContentPipelineError& error)
                {
                    ctx.Print("Expected {} refusal: {}", Pipeline::ContentPipelineStageName(stage), error.what());
                    return error.Stage() == stage && error.Source() == recipe &&
                        error.LogicalName() == request.logicalName &&
                        error.Component() == (stage == Pipeline::ContentPipelineStage::Import ? "CNA.CnjImporter" : "") &&
                        std::string(error.what()).find(reason) != std::string::npos;
                }
                return false;
            };
            writeRecipe("@art/image.png");
            auto ungranted = request;
            ungranted.externalSourceRoots = {};
            ctx.Check(refused(ungranted, Pipeline::ContentPipelineStage::Import, "unknown root"),
                      "the same authored alias is refused when this request has not granted it");
            writeRecipe("@art/../source/image.png");
            ctx.Check(refused(request, Pipeline::ContentPipelineStage::Import, "must not contain '.' or '..'"),
                      "an alias-qualified parent traversal cannot read the real image in a neighboring root");
            writeRecipe((shared / "image.png").generic_string());
            ctx.Check(refused(request, Pipeline::ContentPipelineStage::Import, "must be relative"),
                      "an absolute authored filename cannot bypass the explicit alias spelling even with a grant");
            writeRecipe("@art/image.png");
            auto overlapping = request;
            overlapping.externalSourceRoots = {};
            overlapping.externalSourceRoots.Add("art", source);
            ctx.Check(refused(overlapping, Pipeline::ContentPipelineStage::Selection, "must not equal"),
                      "a capability equal to the primary source root is refused before importer selection");
            const auto repaired = pipeline.Build(request);
            ctx.Check(repaired.output.bytes == built.output.bytes && repaired.dependencies == expectedDependencies,
                      "restoring the valid recipe and request reproduces the original shared-art build after refusals");
        }
        ctx.Check(!std::filesystem::exists(root), "private recipes and all copied original image sources are removed");
    }
}

CNA_DEMOS_CONSOLE(SourceRootCapabilities, {
    .id = "content.custom.source_root_capabilities",
    .title = "Explicit shared-art source roots (CNAEXT)",
    .description = "Compiles a CNJ texture through an explicit external source-root alias; checks exact pixels, relocation, local references and refused capability/path requests.",
    .apis = {"CNA::Content::Pipeline::ContentSourceRootCapabilities::Add",
             "CNA::Content::Pipeline::ContentBuildRequest::externalSourceRoots",
             "CNA::Content::Pipeline::ContentDependency::sourceRoot",
             "CNA::Content::Pipeline::RegisterCnjContentPipeline",
             "CNA::Content::Pipeline::RegisterTexture2DContentPipeline",
             "CNA::Content::Pipeline::ContentPipeline::Build",
             "CNA::Content::Cnb::DecodeTexture2DFromCnb"},
    .tags = {"content", "pipeline", "sources", "cnj", "cnaext"},
    .assets = {"textures/checker.png", "textures/sprite_strip.png"},
    .restrictions = "In-memory CPU compilation and exact decoded pixels; no GPU rendering, CLI fingerprints, nested capability propagation or exhaustive filesystem containment claim.",
})
