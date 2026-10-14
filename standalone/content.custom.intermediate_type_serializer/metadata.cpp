// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_type_serializer",
    .title = "Custom intermediate serializer for dice",
    .description = "Registers a game-owned ContentTypeSerializer for compact NdM dice values, uses it at the "
                   "root and inside a described attack, rejects malformed values, and checks duplicate type and "
                   "XML-shortcut registration behavior.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::AddTypeSerializer",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateReader::ReadText",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateWriter::Xml"},
    .tags = {"content", "custom", "xml", "serialization", "validation", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. "
                    "Authored dice definitions use compact NdM syntax. ISSUE-088: a second type can silently "
                    "claim an already-owned XML shortcut and emit a document it cannot read back.",
    .knownIssue = "CNA-DEMOS-ISSUE-088",
})
