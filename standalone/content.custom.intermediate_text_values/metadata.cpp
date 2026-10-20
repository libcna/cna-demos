// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_text_values",
    .title = "Preserve intermediate XML text values",
    .description = "Serializes one text resource with empty, null, padded, multiline, escaped and Unicode strings "
                   "plus whitespace/XML-significant characters; round-trips exact values, normalizes CDATA and "
                   "entities, and rejects invalid character widths and NUL output.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeDescriptor::Field"},
    .tags = {"content", "custom", "xml", "serialization", "strings", "unicode", "formatting", "pipeline", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. Exercises "
                    "UTF-8 XML text and XNA's UTF-16 code-unit character contract, not locale conversion.",
})
