// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.intermediate_polymorphism",
    .title = "A polymorphic XML spawn list",
    .description = "Serializes enemy and treasure definitions through an abstract Spawn list, restores concrete types "
                   "and inherited fields, preserves nulls, and refuses unknown, unrelated or abstract type declarations.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Serialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::TypeSerializerFor",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::ContentTypeDescriptor::BaseType"},
    .tags = {"content", "custom", "xml", "serialization", "polymorphism", "cnaext"},
    .restrictions = "Build-time tool requires CNA::ContentPipelineBuild; explicit derived-type registration, no XNB output or genuine-XNA execution.",
})
