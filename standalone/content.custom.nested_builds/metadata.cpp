// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.nested_builds",
    .title = "Build and load a stage's child asset",
    .description = "Uses BuildAndLoadAsset to compute a stage's health budget and BuildAsset to prepare a separate "
                   "profile CNB and a CNJ texture using a propagated source-root capability; checks dependencies, "
                   "duplicate requests and name collisions, then publishes outputs and resolves the profile at runtime, including separately scheduled matching project items.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::ContentProcessorContext::BuildAndLoadAsset",
             "Microsoft::Xna::Framework::Content::Pipeline::ContentProcessorContext::BuildAsset",
             "Microsoft::Xna::Framework::Content::Pipeline::ExternalReference::ExternalReference",
             "CNA::Content::Pipeline::RegisterXnaImporter", "CNA::Content::Pipeline::RegisterXnaProcessor",
             "CNA::Content::Pipeline::ContentPipeline::Build", "CNA::Content::Pipeline::ContentBuildRequest::siblings",
             "CNA::Content::Pipeline::ContentBuildRequest::externalSourceRoots",
             "CNA::Content::Pipeline::ContentDependency::sourceRoot",
             "CNA::Content::Cnb::CnbWriter::SetExternalReferences",
             "Microsoft::Xna::Framework::Content::ContentManager::RegisterCnbLoaderEXT", "ContentManager::Load"},
    .tags = {"content", "custom", "pipeline", "dependencies", "external-reference", "cnaext"},
    .assets = {"nested_builds/arena.stage", "nested_builds/guard.profile", "nested_builds/shared_texture.cnj",
               "textures/checker.png"},
    .restrictions = "Build-time tool requires CNA::ContentPipelineBuild. Publishes into a private temporary content root; "
                    "application-managed writes, not the CLI atomic publisher or incremental cache.",
})
