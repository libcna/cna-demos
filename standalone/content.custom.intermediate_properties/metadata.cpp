// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_properties",
    .title = "Properties and inherited XML configuration",
    .description = "Describes a dungeon configuration with a validated property, a get-only room collection, "
                   "inherited name and flattened spawn coordinates; checks setter dispatch, constructor defaults "
                   "and base-before-derived XML ordering.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeDescriptor::Property",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeDescriptor::ReadOnlyProperty",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeDescriptor::BaseType",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentMemberDescriptor::FlattenContent"},
    .tags = {"content", "custom", "xml", "serialization", "properties", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. "
                    "Describes a concrete derived type; polymorphic base-typed loading is separate.",
})
