// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"
CNA_DEMOS_STANDALONE({
    .id = "content.custom.incremental_compiler",
    .title = "Embed an incremental score compiler",
    .description = "Runs CNA's real content compiler with game-owned registries, inspects published score CNBs and manifests, "
                   "then checks pre-built registry service refusal, concurrent disjoint factory calls, unchanged skips, generated and aliased dependency edits/remapping, failure recovery, writer-version/output integrity, typed parameter fingerprints, removed-source collection and ownership-aware cleanup.",
    .apis = {"CNA::Content::Pipeline::RunContentCompiler", "CNA::Content::Pipeline::ContentBuildManifest::Parse",
             "CNA::Content::Pipeline::ContentBuildManifest::Find", "CNA::Content::Pipeline::ContentFileSha256",
             "CNA::Content::Pipeline::ContentProcessorContext::ResolveSourceDependency",
             "CNA::Content::Pipeline::ContentProcessorContext::AddGeneratedDependency",
             "CNA::Content::Pipeline::ContentProcessorContext::SourceRoot",
             "CNA::Content::Pipeline::ContentPipelineRegistry::IsFrozen",
             "CNA::Content::Pipeline::ContentPipelineRegistry::RegisterWriter",
             "CNA::Content::Pipeline::ContentBuildConfiguration::Parse", "CNA::Content::Pipeline::ContentBuildConfiguration::Find",
             "CNA::Content::Pipeline::ContentBuildConfiguration::SourceRoots",
             "CNA::Content::Pipeline::ContentProcessorContext::Parameters"},
    .tags = {"content", "custom", "pipeline", "incremental", "dependencies", "cnaext"},
    .restrictions = "Requires CNA::ContentCompiler. Creates and removes original temporary score inputs and compiler output. "
                    "Checks concurrent factory calls only for disjoint roots; no dynamic-plugin or crash-consistency claim.",
})
