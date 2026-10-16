// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_external_references",
    .title = "Relocate XML external asset references",
    .description = "Serializes external profile filenames beside embedded and shared profiles, proves Resources precede "
                   "ExternalReferences, distinguishes equal references from shared instances, relocates the document, "
                   "and rejects reversed, unresolved or mistyped references.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::ExternalReference::ExternalReference",
             "Microsoft::Xna::Framework::Content::Pipeline::ExternalReference::Filename"},
    .tags = {"content", "custom", "xml", "external-reference", "relocation", "cnaext"},
    .restrictions = "Build-time XML reference records only; requires CNA::ContentPipelineBuild. "
                    "Does not import referenced files, track build dependencies or publish compiled assets.",
    .knownIssue = "CNA-DEMOS-ISSUE-046",
})
