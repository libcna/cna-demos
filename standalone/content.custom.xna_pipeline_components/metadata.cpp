// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.custom.xna_pipeline_components",
    .title = "XNA component adapters and nested conversion",
    .description = "Registers XNA-shaped importer and processor classes with CNA's coordinator, converts route "
                   "units through another processor, records a sidecar dependency and compiles padded bounds; "
                   "checks strict versus warning/default parameter handling, processor dependencies and a generated route summary.",
    .apis = {"CNA::Content::Pipeline::RegisterXnaImporter", "CNA::Content::Pipeline::RegisterXnaProcessor",
             "CNA::Content::Pipeline::ContentPipeline::Build",
             "CNA::Content::Pipeline::ContentBuildEnvironment::strictness",
             "CNA::Content::Pipeline::ContentStrictness::XnaCompatible",
             "CNA::Content::Pipeline::ContentBuildResult::messages",
             "CNA::Content::Pipeline::ContentBuildResult::parameters",
             "Microsoft::Xna::Framework::Content::Pipeline::ContentImporter::Import",
             "Microsoft::Xna::Framework::Content::Pipeline::ContentImporterAttribute::DefaultProcessor",
             "Microsoft::Xna::Framework::Content::Pipeline::ContentImporterContext::AddDependency",
             "Microsoft::Xna::Framework::Content::Pipeline::ContentProcessor::Process",
             "Microsoft::Xna::Framework::Content::Pipeline::ContentProcessorContext::Convert",
             "Microsoft::Xna::Framework::Content::Pipeline::ContentProcessorContext::AddDependency",
             "Microsoft::Xna::Framework::Content::Pipeline::ContentProcessorContext::AddOutputFile",
             "Microsoft::Xna::Framework::Content::Pipeline::ProcessorParameterBindings::Add",
             "Microsoft::Xna::Framework::Content::Pipeline::OpaqueDataDictionary::SetValue"},
    .tags = {"content", "custom", "pipeline", "parameters", "dependencies", "cnaext"},
    .assets = {"custom/patrol.route", "custom/patrol.scale"},
    .restrictions = "Build-time tool in a separate process; requires CNA::ContentPipelineBuild. "
                    "ISSUE-048: returned effective parameters retain dropped entries. Compiled CNB bytes are inspected in memory. Temporary source/output files are removed; explicit builds do not establish incremental caching.",
    .knownIssue = "CNA-DEMOS-ISSUE-047",
})
