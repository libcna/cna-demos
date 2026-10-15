// SPDX-License-Identifier: MS-PL
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "CNA/Content/Pipeline/EffectContentPipeline.hpp"
#include "CNA/Content/Pipeline/EffectSourceContentPipeline.hpp"
#include "CNA/Content/Pipeline/XnbOutputContentPipeline.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Pipeline = CNA::Content::Pipeline;
namespace
{
    enum class CompilerMode { Success, Unavailable, Failed, InvalidBytes, ShaderModel3 };

    std::vector<std::uint8_t> ReadBytes(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        if (!input) throw std::runtime_error("Could not read the committed effect binary.");
        return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    }

    void WriteText(const std::filesystem::path& path, const std::string_view text)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path, std::ios::binary);
        output << text;
        output.close();
        if (!output) throw std::runtime_error("Could not write a private effect source.");
    }

    class RecordingCompiler final : public Pipeline::EffectCompilerService
    {
    public:
        RecordingCompiler(std::vector<std::uint8_t> bytecode, const CompilerMode mode)
            : bytecode_(std::move(bytecode)), mode_(mode)
        {
        }

        Pipeline::EffectCompilerIdentity Identity() const override
        {
            return {"cna-demos-controlled", "1.0", "fx_2_0"};
        }

        bool Available() const override { return mode_ != CompilerMode::Unavailable; }

        std::string UnavailableReason() const override
        {
            return mode_ == CompilerMode::Unavailable ? "controlled compiler is unavailable" : std::string{};
        }

        Pipeline::EffectCompileResult Compile(const Pipeline::EffectCompileRequest& request) const override
        {
            ++calls_;
            lastRequest_ = request;
            Pipeline::EffectCompileResult result;
            if (mode_ == CompilerMode::Unavailable) return result;
            if (mode_ == CompilerMode::Failed)
            {
                result.diagnostics.push_back({request.source.generic_string(), 9, 2, "X3000",
                                              "controlled rejection", true});
                return result;
            }
            result.succeeded = true;
            result.bytecode = mode_ == CompilerMode::InvalidBytes
                                  ? std::vector<std::uint8_t>{'n', 'o', 't', '-', 'f', 'x'}
                                  : bytecode_;
            if (mode_ == CompilerMode::ShaderModel3)
            {
                while ((result.bytecode.size() & 3u) != 0u) result.bytecode.push_back(0u);
                result.bytecode.insert(result.bytecode.end(), {0x00u, 0x03u, 0xFEu, 0xFFu});
            }
            result.diagnostics.push_back({request.source.filename().generic_string(), 3, 4,
                                          "X3550", "controlled warning", false});
            return result;
        }

        int Calls() const { return calls_; }
        const std::optional<Pipeline::EffectCompileRequest>& LastRequest() const { return lastRequest_; }

    private:
        std::vector<std::uint8_t> bytecode_;
        CompilerMode mode_;
        mutable int calls_ = 0;
        mutable std::optional<Pipeline::EffectCompileRequest> lastRequest_;
    };

    struct TemporaryDirectory
    {
        std::filesystem::path path;
        ~TemporaryDirectory() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };

    std::shared_ptr<Pipeline::ContentPipelineRegistry> MakeRegistry(
        const std::shared_ptr<const Pipeline::EffectCompilerService>& compiler,
        const bool debugByDefault = false)
    {
        auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
        Pipeline::RegisterEffectSourceContentPipeline(*registry, compiler, debugByDefault);
        Pipeline::RegisterXnbOutputContentPipeline(*registry, {});
        return registry;
    }

    Pipeline::ContentBuildRequest Request(const std::filesystem::path& sourceRoot)
    {
        Pipeline::ContentBuildRequest request;
        request.sourceRoot = sourceRoot;
        request.source = "probe.fx";
        request.logicalName = "effects/probe";
        request.outputFormat = Pipeline::ContentOutputFormat::Xnb;
        return request;
    }

    template<typename F>
    std::string Refusal(F&& action)
    {
        try { action(); }
        catch (const std::exception& error) { return error.what(); }
        return {};
    }
}

int main()
{
    try
    {
        bool passed = true;
        const auto check = [&](const bool condition, const std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = condition && passed;
        };

        Pipeline::EffectSourceProfile parsed = Pipeline::EffectSourceProfile::HiDef;
        check(std::string(Pipeline::EffectSourceProfileName(Pipeline::EffectSourceProfile::Reach)) == "reach" &&
                  std::string(Pipeline::EffectSourceProfileName(Pipeline::EffectSourceProfile::HiDef)) == "hidef" &&
                  Pipeline::TryParseEffectSourceProfile("reach", parsed) &&
                  parsed == Pipeline::EffectSourceProfile::Reach &&
                  Pipeline::TryParseEffectSourceProfile("hidef", parsed) &&
                  parsed == Pipeline::EffectSourceProfile::HiDef &&
                  !Pipeline::TryParseEffectSourceProfile("HiDef", parsed) &&
                  parsed == Pipeline::EffectSourceProfile::HiDef,
              "effect source profiles have exact stable names and strict nonmutating parsing");

        const auto diagnostics = Pipeline::ParseEffectCompilerDiagnostics(
            "C:\\fx\\probe.fx(12,7): error X3000: syntax error\r\n"
            "probe.fx(5): warning X3550: value ignored\n"
            "loader could not find a dependency\n\n");
        check(diagnostics.size() == 3u &&
                  diagnostics[0] == Pipeline::EffectCompilerDiagnostic{
                      "C:\\fx\\probe.fx", 12, 7, "X3000", "syntax error", true} &&
                  diagnostics[1] == Pipeline::EffectCompilerDiagnostic{
                      "probe.fx", 5, 0, "X3550", "value ignored", false} &&
                  diagnostics[2] == Pipeline::EffectCompilerDiagnostic{
                      {}, 0, 0, {}, "loader could not find a dependency", true},
              "compiler diagnostics retain Windows paths, optional columns, severity, codes and fallback lines");
        check(diagnostics[0].ToString() ==
                  "C:\\fx\\probe.fx(12,7): error X3000: syntax error" &&
                  diagnostics[1].ToString() == "probe.fx(5): warning X3550: value ignored" &&
                  diagnostics[2].ToString() == "error: loader could not find a dependency" &&
                  Pipeline::EffectCompilerIdentity{"demo", "1.2.3", "fx_2_0"}.ToString() ==
                      "demo-1.2.3-fx_2_0" &&
                  Pipeline::EffectCompilerIdentity{}.ToString() == "none",
              "diagnostic and compiler identities render their complete stable forms");

        const auto scanned = Pipeline::ScanEffectSourceIncludes(
            "// #include \"ignored-a.fxh\"\n"
            "float4 banner = \"#include ignored-b.fxh\";\n"
            "  #include \"common.fxh\"\n"
            "/* #include \"ignored-c.fxh\" */\n"
            "#include <nested/light.fxh>\n");
        check(scanned == std::vector<std::pair<std::string, int>>{
                             {"common.fxh", 3}, {"nested/light.fxh", 5}},
              "the public include scanner accepts quote/angle directives and ignores comments and strings");

        Pipeline::ExternalEffectCompilerOptions missingOptions;
        missingOptions.executable = std::filesystem::temp_directory_path() /
            "cna-demos-definitely-missing-fxc";
        const auto missingCompiler = Pipeline::MakeExternalEffectCompiler(missingOptions);
        Pipeline::EffectCompileRequest missingRequest;
        missingRequest.source = "missing.fx";
        const auto missingResult = missingCompiler->Compile(missingRequest);
        check(missingOptions == Pipeline::ExternalEffectCompilerOptions{missingOptions.executable, {}} &&
                  missingCompiler && !missingCompiler->Available() &&
                  missingCompiler->Identity().ToString() == "none" &&
                  missingCompiler->Identity().targetProfile == "fx_2_0" &&
                  missingCompiler->UnavailableReason().find("no usable effect compiler") != std::string::npos &&
                  !missingResult.succeeded && missingResult.bytecode.empty() &&
                  missingResult.diagnostics.size() == 1u && missingResult.diagnostics.front().isError,
              "the external factory returns a stable unavailable service for an explicitly missing executable");

        const std::filesystem::path title = Microsoft::Xna::Framework::TitleLocation::Path();
        const auto realBytecode = ReadBytes(title / "assets/effects/CnaConformanceEffect.fxb");
        check(Pipeline::IsCompiledEffectBinary(realBytecode),
              "the controlled service input is a committed genuine Effect Framework binary");

        const auto directory = std::filesystem::temp_directory_path() /
            ("cna-demos-effect-source-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(directory))
            throw std::runtime_error("Could not reserve a private effect workspace.");
        {
            TemporaryDirectory cleanup{directory};
            WriteText(directory / "probe.fx",
                      "#include \"common.fxh\"\nfloat4 Main() : COLOR0 { return Shared(); }\n");
            WriteText(directory / "common.fxh",
                      "#include \"nested/light.fxh\"\nfloat4 Shared() { return Light(); }\n");
            WriteText(directory / "nested/light.fxh",
                      "float4 Light() { return float4(1, 0, 0, 1); }\n");

            const auto compiler = std::make_shared<RecordingCompiler>(realBytecode, CompilerMode::Success);
            const auto registry = MakeRegistry(compiler, true);
            const auto importer = registry->ResolveImporter("probe.fx");
            const auto processor = registry->ResolveProcessor(Pipeline::ImportedEffectSourceType);
            const auto writer = registry->ResolveWriter(Pipeline::ProcessedCompiledEffectType, {},
                                                        Pipeline::ContentOutputFormat::Xnb);
            check(importer->Identity() == Pipeline::ContentComponentIdentity{"CNA.EffectSourceImporter", "1"} &&
                      importer->SourceExtensions() == std::vector<std::string>{".fx"} &&
                      importer->OutputTypes() == std::vector<std::string>{Pipeline::ImportedEffectSourceType} &&
                      processor->Identity() == Pipeline::ContentComponentIdentity{
                          "CNA.EffectSourceProcessor",
                          "1+cna-demos-controlled-1.0-fx_2_0+debug"} &&
                      processor->InputType() == Pipeline::ImportedEffectSourceType &&
                      processor->OutputType() == Pipeline::ProcessedCompiledEffectType &&
                      writer->OutputFormat() == Pipeline::ContentOutputFormat::Xnb &&
                      registry->AbsentWriterReason(Pipeline::ContentOutputFormat::Cnb,
                                                   Pipeline::ProcessedCompiledEffectType) ==
                          Pipeline::CompiledEffectHasNoCnbSchemaReason(),
                  "effect source registration exposes its exact importer, processor, XNB writer and CNB absence");

            auto request = Request(directory);
            request.environment.targetProfile =
                Microsoft::Xna::Framework::Graphics::GraphicsProfile::Reach;
            request.parameters.Set(Pipeline::EffectProfileParameter, std::string("hidef"));
            request.parameters.Set(Pipeline::EffectDefinesParameter,
                                   std::string("ZETA=9; FLAG ; ALPHA=1"));
            request.parameters.Set(Pipeline::EffectDebugParameter, false);
            const Pipeline::ContentPipeline pipeline(registry);
            const auto built = pipeline.Build(request);
            check(built.importer.name == "CNA.EffectSourceImporter" &&
                      built.processor.name == "CNA.EffectSourceProcessor" &&
                      built.writer.name == "CNA.XnbEffectWriter" &&
                      built.output.assetTypeName == "Microsoft.Xna.Framework.Graphics.Effect" &&
                      built.output.bytes.size() > realBytecode.size() &&
                      built.output.bytes[0] == 'X' && built.output.bytes[1] == 'N' &&
                      built.output.bytes[2] == 'B',
                  "the authored source reaches the registered compiler service and a real Effect XNB writer");

            const auto& captured = compiler->LastRequest();
            check(compiler->Calls() == 1 && captured &&
                      captured->source == std::filesystem::canonical(directory / "probe.fx") &&
                      captured->includeDirectories ==
                          std::vector<std::filesystem::path>{std::filesystem::canonical(directory)} &&
                      captured->defines == std::map<std::string, std::string>{
                          {"ALPHA", "1"}, {"FLAG", ""}, {"ZETA", "9"}} &&
                      captured->profile == Pipeline::EffectSourceProfile::HiDef &&
                      !captured->debugInformation,
                  "profile, sorted defines, source/include paths and explicit debug policy reach the service exactly");

            bool primary = false, common = false, nested = false;
            for (const auto& dependency : built.dependencies)
            {
                primary |= dependency.kind == Pipeline::ContentDependencyKind::PrimarySource &&
                    dependency.identity.ends_with("/probe.fx");
                common |= dependency.kind == Pipeline::ContentDependencyKind::SourceFile &&
                    dependency.identity.ends_with("/common.fxh");
                nested |= dependency.kind == Pipeline::ContentDependencyKind::SourceFile &&
                    dependency.identity.ends_with("/nested/light.fxh");
            }
            check(built.dependencies.size() == 3u && primary && common && nested,
                  "the importer records the primary source and complete transitive include tree");

            bool importInfo = false, compileInfo = false, compilerWarning = false;
            for (const auto& message : built.messages)
            {
                importInfo |= message.level == Pipeline::ContentLogLevel::Info &&
                    message.stage == Pipeline::ContentPipelineStage::Import &&
                    message.text.find("2 include(s)") != std::string::npos;
                compileInfo |= message.level == Pipeline::ContentLogLevel::Info &&
                    message.stage == Pipeline::ContentPipelineStage::Process &&
                    message.text.find("cna-demos-controlled-1.0-fx_2_0 at profile hidef") != std::string::npos;
                compilerWarning |= message.level == Pipeline::ContentLogLevel::Warning &&
                    message.stage == Pipeline::ContentPipelineStage::Process &&
                    message.text == "probe.fx(3,4): warning X3550: controlled warning";
            }
            check(built.messages.size() == 3u && importInfo && compileInfo && compilerWarning,
                  "import, compile and structured warning diagnostics retain their exact stages and text");

            Pipeline::EffectSourceProcessor parameterProbe(compiler);
            Pipeline::ContentProcessorParameters valid;
            valid.Set(Pipeline::EffectProfileParameter, std::string("reach"));
            valid.Set(Pipeline::EffectDefinesParameter, std::string("NAME=value; FLAG"));
            valid.Set(Pipeline::EffectDebugParameter, true);
            parameterProbe.ValidateParameters(valid);
            Pipeline::ContentProcessorParameters unknown; unknown.Set("unknown", true);
            Pipeline::ContentProcessorParameters badProfile;
            badProfile.Set(Pipeline::EffectProfileParameter, std::string("ultra"));
            Pipeline::ContentProcessorParameters duplicate;
            duplicate.Set(Pipeline::EffectDefinesParameter, std::string("A=1;A=2"));
            Pipeline::ContentProcessorParameters badDebug;
            badDebug.Set(Pipeline::EffectDebugParameter, std::string("maybe"));
            const auto nullProcessor = Refusal([] { (void)Pipeline::EffectSourceProcessor(nullptr); });
            const auto unknownParameter = Refusal([&] { parameterProbe.ValidateParameters(unknown); });
            const auto badProfileText = Refusal([&] { parameterProbe.ValidateParameters(badProfile); });
            const auto duplicateDefine = Refusal([&] { parameterProbe.ValidateParameters(duplicate); });
            const auto badDebugText = Refusal([&] { parameterProbe.ValidateParameters(badDebug); });
            check(nullProcessor.find("compiler must not be null") != std::string::npos &&
                      unknownParameter.find("does not recognize parameter 'unknown'") != std::string::npos &&
                      badProfileText.find("must be 'reach' or 'hidef'") != std::string::npos &&
                      duplicateDefine.find("defines 'A' twice") != std::string::npos &&
                      badDebugText.find("must be true or false") != std::string::npos,
                  "processor construction and parameter validation reject null, unknown and malformed values exactly");

            const auto unavailable = std::make_shared<RecordingCompiler>(realBytecode, CompilerMode::Unavailable);
            const Pipeline::ContentPipeline unavailablePipeline(MakeRegistry(unavailable));
            const auto unavailableText = Refusal([&] { (void)unavailablePipeline.Build(Request(directory)); });
            check(unavailable->Calls() == 0 &&
                      unavailableText.find("controlled compiler is unavailable") != std::string::npos,
                  "an unavailable compiler refuses the process stage without invoking Compile");

            const auto failed = std::make_shared<RecordingCompiler>(realBytecode, CompilerMode::Failed);
            const Pipeline::ContentPipeline failedPipeline(MakeRegistry(failed));
            const auto failedText = Refusal([&] { (void)failedPipeline.Build(Request(directory)); });
            check(failed->Calls() == 1 && failedText.find("cna-demos-controlled-1.0-fx_2_0") != std::string::npos &&
                      failedText.find("probe.fx(9,2): error X3000: controlled rejection") != std::string::npos,
                  "a failed compiler result reports its identity and structured error diagnostic");

            const auto invalid = std::make_shared<RecordingCompiler>(realBytecode, CompilerMode::InvalidBytes);
            const Pipeline::ContentPipeline invalidPipeline(MakeRegistry(invalid));
            const auto invalidText = Refusal([&] { (void)invalidPipeline.Build(Request(directory)); });
            check(invalid->Calls() == 1 &&
                      invalidText.find("do not begin with an Effect Framework 9.1 signature") != std::string::npos,
                  "successful service output without the Effect Framework signature is refused before writing");

            const auto shader3 = std::make_shared<RecordingCompiler>(realBytecode, CompilerMode::ShaderModel3);
            const Pipeline::ContentPipeline shader3Pipeline(MakeRegistry(shader3));
            const auto shader3Text = Refusal([&] { (void)shader3Pipeline.Build(Request(directory)); });
            check(shader3->Calls() == 1 &&
                      shader3Text.find("Reach profile does not support vertex shader model 3.0") != std::string::npos,
                  "Reach rejects compiler output containing a shader-model-three token");

            const auto phone = std::make_shared<RecordingCompiler>(realBytecode, CompilerMode::Success);
            const Pipeline::ContentPipeline phonePipeline(MakeRegistry(phone));
            auto phoneRequest = Request(directory);
            phoneRequest.environment.targetPlatform = Pipeline::ContentTargetPlatform::WindowsPhone;
            const auto phoneText = Refusal([&] { (void)phonePipeline.Build(phoneRequest); });
            check(phone->Calls() == 0 &&
                      phoneText.find("Windows Phone platform does not support custom shaders") != std::string::npos,
                  "Windows Phone refuses custom effect source before invoking the compiler service");

            auto cnbRequest = Request(directory);
            cnbRequest.outputFormat = Pipeline::ContentOutputFormat::Cnb;
            const auto cnbText = Refusal([&] { (void)pipeline.Build(cnbRequest); });
            check(compiler->Calls() == 2 &&
                      cnbText.find("CNB container reserves an Effect identifier") != std::string::npos,
                  "the source route compiles, then reports its documented deliberate CNB absence");
        }
        check(!std::filesystem::exists(directory), "the demo removes its private effect sources");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Effect source-pipeline audit failed: " << error.what() << '\n';
        return 1;
    }
}
