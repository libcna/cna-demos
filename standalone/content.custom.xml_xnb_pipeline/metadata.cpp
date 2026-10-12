// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"
CNA_DEMOS_STANDALONE({
    .id = "content.custom.xml_xnb_pipeline",
    .title = "Compile custom XML into runtime XNB",
    .description = "Registers the game's XML serializer and XNB type writer, builds original profile XML through CNA's "
                   "source pipeline, and loads the published XNB with a versioned game reader before and after an authored edit.",
    .apis = {"CNA::Content::Pipeline::RegisterXnaXmlSourceContentPipeline",
             "CNA::Content::Pipeline::RunContentCompiler", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer::TypeSerializerFor",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentCompiler::AddTypeWriter", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::Write", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::GetRuntimeReader",
             "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::GetRuntimeType", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentTypeWriter::TypeVersion", "Microsoft::Xna::Framework::Content::Pipeline::Serialization::Compiler::ContentWriter::Write",
             "ContentTypeReader::Read", "ContentTypeReader::TypeVersion", "ContentManager::Load", "ContentManager::Unload"},
    .tags = {"content", "custom", "pipeline", "xml", "xnb", "cnaext"},
    .restrictions = "Requires CNA::ContentCompiler and CNA::ContentPipelineBuild. Uses original private XML fixtures. "
                    "Checks serial source compilation and runtime loading; no genuine-XNA oracle, GPU or arbitrary-object CNB claim.",
})
