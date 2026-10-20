// SPDX-License-Identifier: MS-PL
// ISSUE-047 host fixture: a real processor writes and registers a support file.
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "CNA/Content/Cnb/CnbCurveCodec.hpp"
#include "CNA/Content/Pipeline/ContentCompiler.hpp"
#include "CNA/Content/Pipeline/XnaPipelineBridge.hpp"
#include "CNA/Internal/PathUtf8.hpp"

namespace
{
    namespace Pipeline = CNA::Content::Pipeline;
    namespace Cnb = CNA::Content::Cnb;
    constexpr const char* ImportedType = "Repair.GeneratedOutput.Input";
    constexpr const char* ProcessedType = "Repair.GeneratedOutput.Curve";
    constexpr const char* CurveType = "Microsoft.Xna.Framework.Curve";

    class Importer final : public Pipeline::ContentImporter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override
        {
            return {"Repair.GeneratedOutput.Importer", "1"};
        }

        std::vector<std::string> SourceExtensions() const override
        {
            return {".generated-output"};
        }

        std::vector<std::string> OutputTypes() const override { return {ImportedType}; }

        Pipeline::ContentValue Import(Pipeline::ContentImporterContext& context) const override
        {
            std::ifstream stream(context.SourcePath(), std::ios::binary);
            if (!stream) { throw std::runtime_error("cannot open generated-output input"); }
            const std::string text{std::istreambuf_iterator<char>(stream), {}};
            if (stream.bad()) { throw std::runtime_error("cannot read generated-output input"); }
            return Pipeline::ContentValue::Create(ImportedType, text);
        }
    };

    class Processor final : public Pipeline::ContentProcessor
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override
        {
            return {"Repair.GeneratedOutput.Processor", "1"};
        }

        std::string InputType() const override { return ImportedType; }
        std::string OutputType() const override { return ProcessedType; }

        void ValidateParameters(const Pipeline::ContentProcessorParameters& parameters) const override
        {
            if (!parameters.Values().empty())
            {
                throw std::invalid_argument("generated-output fixture takes no parameters");
            }
        }

        Pipeline::ContentValue Process(const Pipeline::ContentValue& input,
                                       Pipeline::ContentProcessorContext& context) const override
        {
            const std::string& text = input.Get<std::string>();
            const auto newline = text.find('\n');
            if (newline == std::string::npos)
            {
                throw std::invalid_argument("input requires a relative report path and payload");
            }
            const auto name = CNA::Internal::PathFromUtf8(text.substr(0, newline));
            if (name.empty() || name.has_root_name() || name.has_root_directory())
            {
                throw std::invalid_argument("report path must be relative");
            }
            for (const auto& component : name)
            {
                if (component == "." || component == "..")
                {
                    throw std::invalid_argument("report path must not contain dot segments");
                }
            }
            const auto& outputRoot = context.Environment().outputDirectory;
            if (outputRoot.empty())
            {
                throw std::invalid_argument("compiler must supply its output directory");
            }
            const auto report = outputRoot / name;
            std::filesystem::create_directories(report.parent_path());
            std::ofstream stream(report, std::ios::binary);
            stream.exceptions(std::ios::badbit | std::ios::failbit);
            stream << text.substr(newline + 1);
            stream.close();

            Pipeline::XnaBridgeProcessorContext bridge(context);
            bridge.AddOutputFile(CNA::Internal::PathToUtf8(report));
            return Pipeline::ContentValue::Create(
                ProcessedType, Microsoft::Xna::Framework::Curve{});
        }
    };

    class Writer final : public Pipeline::ContentTypeWriter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override
        {
            return {"Repair.GeneratedOutput.Writer", "1"};
        }

        std::string InputType() const override { return ProcessedType; }

        std::vector<Pipeline::ContentWriterSchemaIdentity> OutputSchemaIdentities() const override
        {
            return {{Cnb::CnbAssetTypeId::Curve, Cnb::CnbCurveSchemaVersion, CurveType,
                     {"CNA.EncodeCurveToCnb", "1"}}};
        }

        Pipeline::ContentWriteResult Write(const Pipeline::ContentValue& input,
                                           const std::string& logicalName) const override
        {
            Pipeline::ContentWriteResult result;
            result.bytes = Cnb::EncodeCurveToCnb(
                input.Get<Microsoft::Xna::Framework::Curve>(), logicalName);
            result.assetTypeId = Cnb::CnbAssetTypeId::Curve;
            result.assetTypeName = CurveType;
            result.assetSchemaVersion = Cnb::CnbCurveSchemaVersion;
            return result;
        }
    };

    template<typename Character>
    int Run(int argc, Character** argv)
    {
        std::vector<std::filesystem::path> arguments;
        for (int index = 1; index < argc; ++index) { arguments.emplace_back(argv[index]); }
        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        registry->RegisterImporter(std::make_shared<Importer>());
        registry->RegisterProcessor(std::make_shared<Processor>());
        registry->RegisterWriter(std::make_shared<Writer>());
        return Pipeline::RunContentCompiler(arguments, std::move(registry));
    }
}

#if defined(_WIN32)
int wmain(int argc, wchar_t** argv) { return Run(argc, argv); }
#else
int main(int argc, char** argv) { return Run(argc, argv); }
#endif
