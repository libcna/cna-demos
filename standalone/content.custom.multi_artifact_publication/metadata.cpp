// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"
CNA_DEMOS_STANDALONE({
    .id = "content.custom.multi_artifact_publication",
    .title = "Publish an encounter's compiled and support files",
    .description = "Uses the real content compiler to publish primary and additional CNBs plus a deployed briefing, "
                   "then checks manifest ownership, unchanged skips, missing outputs, changed support data, "
                   "recovery after a blocked deployment destination, same-root thread/process invocation refusal and stale-lease recovery, "
                   "conflicting node ownership and clean with or without its ownership manifest.",
    .apis = {"CNA::Content::Pipeline::RunContentCompiler",
             "CNA::Content::Pipeline::ContentTypeWriter::OutputSchemaIdentities",
             "CNA::Content::Pipeline::ContentWriteResult::additionalOutputs",
             "CNA::Content::Pipeline::ContentProcessorContext::AddDeploymentFile",
             "CNA::Content::Pipeline::ContentProcessorContext::AddRuntimeReference",
             "CNA::Content::Pipeline::ContentBuildManifest::Parse",
             "CNA::Content::Pipeline::ContentBuildManifest::Find",
             "CNA::Content::Pipeline::ContentFileSha256",
             "CNA::Content::Cnb::CnbWriter::SetExternalReferences"},
    .tags = {"content", "custom", "pipeline", "deployment", "incremental", "cnaext"},
    .restrictions = "Requires CNA::ContentCompiler. Original private fixtures are removed. "
                    "Individual files may already change before a later output fails. Concurrent coverage includes "
                    "in-process and independent-process owners plus a pre-write process exit; no mid-publication crash-consistency claim. "
                    "No whole-node rollback or runtime loader claim.",
})
