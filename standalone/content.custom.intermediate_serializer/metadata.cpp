// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_serializer",
    .title = "Describe an intermediate XML asset",
    .description = "Uses public content descriptors and ContentSerializerAttribute to serialize a game-owned "
                   "encounter with renamed, optional, collection and ignored fields, then checks deserialization.",
    .apis = {"ContentSerializerAttribute::ContentSerializerAttribute", "ContentSerializerAttribute::ElementName",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeDescriptor::Field",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::Format",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::Optional",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::CollectionItemName",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::Ignore"},
    .tags = {"content", "custom", "xml", "serialization", "pipeline", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. An unavailable "
                    "tool is reported as NOT RUN.",
})
