// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_shared_resources",
    .title = "Shared loot tables in intermediate XML",
    .description = "Serializes shared and inline loot tables, then checks reference fixups, object identity, "
                   "equal but independent resources, constructor defaults for empty/null members, opaque IDs and "
                   "edits through an alias after loading; rejects malformed references/resources and non-shared cycles, "
                   "closes resource and root self-cycles, and "
                   "verifies explicit C++ ownership cleanup.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::SharedResource",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateWriter::MaxNestingDepth"},
    .tags = {"content", "custom", "xml", "serialization", "ownership", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. "
                    "Cyclic shared_ptr back edges are explicitly broken; external references are not demonstrated.",
})
