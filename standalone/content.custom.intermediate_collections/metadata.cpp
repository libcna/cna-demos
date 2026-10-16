// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_collections",
    .title = "Choose intermediate XML collection shapes",
    .description = "Serializes one route manifest with packed scalar and vector sequences, element-per-item "
                   "strings, nullable and dynamic objects, insertion-ordered dictionary entries, jagged lists "
                   "and an empty collection; then round-trips and rejects representative malformed shapes.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeDescriptor::Field"},
    .tags = {"content", "custom", "xml", "serialization", "collections", "dictionary", "pipeline", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. Uses a "
                    "representative collection family rather than enumerating every serializable element type.",
})
