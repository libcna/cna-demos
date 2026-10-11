// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_type_serializer",
    .title = "Custom intermediate serializer for dice",
    .description = "Registers a game-owned ContentTypeSerializer for compact NdM dice values, uses it at the "
                   "root and inside a described attack, and rejects malformed or out-of-domain values.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::AddTypeSerializer",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateReader::ReadText",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateWriter::Xml"},
    .tags = {"content", "custom", "xml", "serialization", "validation", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. "
                    "Authored dice definitions use compact NdM syntax.",
})
