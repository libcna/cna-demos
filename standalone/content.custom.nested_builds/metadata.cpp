// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.nested_builds",
    .title = "Process and compile a stage's child asset",
    .description = "Uses BuildAndLoadAsset to compute a stage's health budget and BuildAsset to prepare a separate "
                   "profile CNB; checks source dependencies, runtime references, duplicate requests and conflicting output names.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::ContentProcessorContext::BuildAndLoadAsset",
             "Microsoft::Xna::Framework::Content::Pipeline::ContentProcessorContext::BuildAsset",
             "Microsoft::Xna::Framework::Content::Pipeline::ExternalReference::ExternalReference",
             "CNA::Content::Pipeline::RegisterXnaImporter", "CNA::Content::Pipeline::RegisterXnaProcessor",
             "CNA::Content::Pipeline::ContentPipeline::Build", "CNA::Content::Cnb::CnbWriter::SetExternalReferences"},
    .tags = {"content", "custom", "pipeline", "dependencies", "external-reference", "cnaext"},
    .assets = {"nested_builds/arena.stage", "nested_builds/guard.profile"},
    .restrictions = "Build-time tool requires CNA::ContentPipelineBuild. Inspects compiled bytes in memory; "
                    "does not publish files, demonstrate incremental caching or load these schemas at runtime.",
})
