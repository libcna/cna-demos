// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"
CNA_DEMOS_STANDALONE({
    .id = "content.custom.incremental_compiler",
    .title = "Embed an incremental score compiler",
    .description = "Runs CNA's real content compiler with a game-owned registry, inspects published score CNBs and its manifest, "
                   "then checks unchanged skips, primary/shared dependency edits, failure recovery, writer-version/output integrity, typed parameter fingerprints, removed-source collection and ownership-aware cleanup.",
    .apis = {"CNA::Content::Pipeline::RunContentCompiler", "CNA::Content::Pipeline::ContentBuildManifest::Parse",
             "CNA::Content::Pipeline::ContentBuildManifest::Find", "CNA::Content::Pipeline::ContentFileSha256",
             "CNA::Content::Pipeline::ContentProcessorContext::ResolveSourceDependency",
             "CNA::Content::Pipeline::ContentPipelineRegistry::IsFrozen",
             "CNA::Content::Pipeline::ContentPipelineRegistry::RegisterWriter",
             "CNA::Content::Pipeline::ContentBuildConfiguration::Parse", "CNA::Content::Pipeline::ContentBuildConfiguration::Find",
             "CNA::Content::Pipeline::ContentProcessorContext::Parameters"},
    .tags = {"content", "custom", "pipeline", "incremental", "dependencies", "cnaext"},
    .restrictions = "Requires CNA::ContentCompiler. Creates and removes original temporary score inputs and compiler output. "
                    "Checks serial node behavior and failed-import preservation; no crash-consistency or concurrent-publisher claim.",
})
