// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_shared_resources",
    .title = "Shared loot tables in intermediate XML",
    .description = "Serializes shared and inline loot tables, then checks reference fixups, object identity, "
                   "equal but independent resources, null tables and edits through an alias after loading; "
                   "rejects malformed references and closes a cyclic fallback graph with explicit C++ ownership cleanup.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::SharedResource"},
    .tags = {"content", "custom", "xml", "serialization", "ownership", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. "
                    "Cyclic shared_ptr back edges are explicitly broken; external references are not demonstrated.",
})
