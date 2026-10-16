// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_scalar_formats",
    .title = "Preserve invariant intermediate XML scalars",
    .description = "Serializes one authored settings object with canonical Boolean, integer, floating-point, "
                   "Decimal, TimeSpan, DateTime, enum and flags text; round-trips exact values and rejects "
                   "representative noncanonical, out-of-range and misspelled inputs.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeDescriptor::Field"},
    .tags = {"content", "custom", "xml", "serialization", "scalars", "formatting", "pipeline", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. Uses a "
                    "representative invariant scalar set rather than enumerating every numeric width.",
})
