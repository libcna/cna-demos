// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"
CNA_DEMOS_STANDALONE({
    .id = "content.custom.dependency_graph",
    .title = "Build dependencies versus runtime references",
    .description = "Compiles original budget nodes through CNA's real graph coordinator, contrasts strong build edges "
                   "with runtime-only references, inspects direct/effective fingerprints, loads the published budgets "
                   "checks refusal and repair of cyclic or missing-node build dependencies, and verifies two-worker overlap and deterministic artifacts.",
    .apis = {"CNA::Content::Pipeline::RunContentCompiler",
             "CNA::Content::Pipeline::ContentProcessorContext::AddContentBuildDependency",
             "CNA::Content::Pipeline::ContentProcessorContext::AddRuntimeReference",
             "CNA::Content::Pipeline::ContentBuildManifest::Parse",
             "CNA::Content::Pipeline::ContentBuildManifest::Find",
             "CNA::Content::Pipeline::ContentBuildManifestEntry::directFingerprint",
             "CNA::Content::Pipeline::ContentBuildManifestEntry::fingerprint",
             "ContentManager::RegisterCnbLoaderEXT", "ContentManager::Load"},
    .tags = {"content", "custom", "pipeline", "dependencies", "incremental", "cnaext"},
    .restrictions = "Requires CNA::ContentCompiler. Original private fixtures and outputs are removed. "
                    "Serial and two-worker runs with a bounded importer overlap probe; no writer invocation-order, arbitrary runtime cycles "
                    "same-output concurrent compiler invocations or whole-build transactional guarantee is claimed.",
})
