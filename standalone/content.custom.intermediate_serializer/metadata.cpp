// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_serializer",
    .title = "Describe an intermediate XML asset",
    .description = "Uses public content descriptors and ContentSerializerAttribute to serialize a game-owned "
                   "encounter with renamed, optional, collection and ignored fields. Distinguishes absent, "
                   "present, false-null and explicit-null hints, preserves implicit member names and optional collection "
                   "defaults, validates tolerant metadata/node handling and strict XnaContent envelope shapes/order, "
                   "refuses a null "
                   "required objective on read/write, and rejects missing or ignored fields.",
    .apis = {"ContentSerializerAttribute::ContentSerializerAttribute", "ContentSerializerAttribute::AllowNull",
             "ContentSerializerAttribute::Clone", "ContentSerializerAttribute::CollectionItemName",
             "ContentSerializerAttribute::ElementName", "ContentSerializerAttribute::FlattenContent",
             "ContentSerializerAttribute::HasCollectionItemName", "ContentSerializerAttribute::Optional",
             "ContentSerializerAttribute::SharedResource",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeDescriptor::Field",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::Format",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::Optional",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::AllowNull",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::CollectionItemName",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::Ignore"},
    .tags = {"content", "custom", "xml", "serialization", "pipeline", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. An unavailable "
                    "tool is reported as NOT RUN. ISSUE-148: CNA rejects an in-element processing instruction and "
                    "accepts self-closing empty sections and an in-memory UTF-8 BOM that XNA rejects.",
    .knownIssue = "CNA-DEMOS-ISSUE-148",
})
