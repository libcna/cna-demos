// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.opaque_data_dictionary",
    .title = "Serialize heterogeneous processor parameters",
    .description = "Builds an ordered heterogeneous OpaqueDataDictionary, verifies typed and default lookup, "
                   "matches XNA's compact intermediate XML, round-trips its dynamic types, and rejects invalid "
                   "keys, values and XML type declarations without corrupting the original parameter sheet.",
    .apis = {"Microsoft::Xna::Framework::Content::Pipeline::OpaqueDataDictionary::OpaqueDataDictionary",
             "Microsoft::Xna::Framework::Content::Pipeline::OpaqueDataDictionary::SetValue",
             "Microsoft::Xna::Framework::Content::Pipeline::OpaqueDataDictionary::GetValue",
             "Microsoft::Xna::Framework::Content::Pipeline::OpaqueDataDictionary::GetContentAsXml",
             "Microsoft::Xna::Framework::Content::Pipeline::NamedValueDictionary::Add",
             "Microsoft::Xna::Framework::Content::Pipeline::NamedValueDictionary::Count",
             "Microsoft::Xna::Framework::Content::Pipeline::NamedValueDictionary::Keys",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::Deserialize"},
    .tags = {"content", "custom", "xml", "serialization", "dictionary", "pipeline", "cnaext"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild.",
})
