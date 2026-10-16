// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_framework_values",
    .title = "Format framework values in intermediate XML",
    .description = "Serializes one geometry configuration with vector, matrix, packed Color, integer rectangle, "
                   "BoundingBox, Ray and Curve values; normalizes accepted token whitespace/case, preserves empty "
                   "and keyed curves, and rejects wrong tuple, hexadecimal, structured and curve-key forms.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeDescriptor::Field"},
    .tags = {"content", "custom", "xml", "serialization", "geometry", "curve", "formatting", "pipeline", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. Uses a "
                    "representative value family rather than duplicating every tuple width and math type.",
})
