// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_type_serializer",
    .title = "Custom intermediate serializer for dice",
    .description = "Registers a game-owned ContentTypeSerializer for compact NdM dice values, uses it at the "
                   "root, in a described attack and in a scanned dynamic typed envelope; rejects malformed values, "
                   "and checks initialization plus duplicate type and XML-shortcut registration behavior.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::Initialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::ObjectIsEmpty",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::ScanChildren",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::TargetType",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::XmlTypeName",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::AddTypeSerializer",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::GetTypeSerializer",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateReader::ReadText",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateReader::MoveToElement",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateReader::ReadTypeName",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateReader::Serializer",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateReader::Xml",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateWriter::Serializer",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateWriter::WriteTypeName",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateWriter::Xml"},
    .tags = {"content", "custom", "xml", "serialization", "validation", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. "
                    "Authored dice definitions use compact NdM syntax. ISSUE-088: a second type can silently "
                    "claim an already-owned XML shortcut and emit a document it cannot read back. ISSUE-089: "
                    "registered intermediate serializers are never initialized.",
    .knownIssue = "CNA-DEMOS-ISSUE-088",
})
