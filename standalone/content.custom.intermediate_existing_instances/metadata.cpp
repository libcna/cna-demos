// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"
CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_existing_instances",
    .title = "Fill existing objects from ordinary and raw XML",
    .description = "A custom XML serializer fills preallocated stat slots through ordinary and raw object reads, "
                   "preserves identity and runtime-only data, contrasts Null and Type handling, and selects a field-specific compact serializer.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateReader::ReadObject",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateReader::ReadRawObject",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateWriter::WriteObject",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateWriter::WriteRawObject",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::AddTypeSerializer"},
    .tags = {"content", "custom", "xml", "serialization", "identity", "cnaext"},
    .restrictions = "Requires CNA::ContentPipelineBuild. Uses original in-memory XML and shared reference carriers. "
                    "No XNB writer, GPU or genuine-XNA validation is implied by this XML example.",
})
