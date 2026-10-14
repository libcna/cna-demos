// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"
CNA_DEMOS_STANDALONE({
    .id = "content.custom.xml_xnb_pipeline",
    .title = "Compile custom XML into runtime XNB",
    .description = "Registers the game's XML serializer and XNB type writer, builds original profile XML through CNA's "
                   "source pipeline, exercises writer identity, lifecycle, target context and nested dispatch, then loads the "
                   "published XNB with a versioned game reader before and after an authored edit.",
    .apis = {"CNA::Content::Pipeline::RegisterXnaXmlSourceContentPipeline",
             "CNA::Content::Pipeline::RunContentCompiler", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::TypeSerializerFor",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentCompiler::AddTypeWriter", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentCompiler::GetTypeWriter",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::Write", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::Initialize", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::ShouldCompressContent",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::GetRuntimeReader", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::GetRuntimeType", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::TargetType", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::TypeVersion", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::CanDeserializeIntoExistingObject",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentWriter::Write", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentWriter::WriteObject", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentWriter::WriteRawObject", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentWriter::TargetPlatform", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentWriter::TargetProfile",
             "ContentTypeReader::Read", "ContentTypeReader::TypeVersion", "ContentManager::Load", "ContentManager::Unload"},
    .tags = {"content", "custom", "pipeline", "xml", "xnb", "cnaext"},
    .restrictions = "Requires CNA::ContentCompiler and CNA::ContentPipelineBuild. Uses original private XML fixtures. "
                    "Checks serial source compilation and runtime loading. ISSUE-090: Initialize deadlocks when it follows its "
                    "documented nested-writer caching contract. No genuine-XNA oracle, GPU or arbitrary-object CNB claim.",
    .knownIssue = "CNA-DEMOS-ISSUE-090",
})
