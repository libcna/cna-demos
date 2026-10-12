// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"
CNA_DEMOS_STANDALONE({
    .id = "content.custom.multi_artifact_publication",
    .title = "Publish an encounter's compiled and support files",
    .description = "Uses the real content compiler to publish primary and additional CNBs plus a deployed briefing, "
                   "then checks manifest ownership, unchanged skips, missing outputs, changed support data, "
                   "recovery after a blocked deployment destination, conflicting node ownership and clean with or without its ownership manifest.",
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
                    "Serial publication and retry only; individual files may already change before a later output fails. "
                    "No whole-node rollback, crash-consistency, concurrent-publisher or runtime loader claim.",
})
