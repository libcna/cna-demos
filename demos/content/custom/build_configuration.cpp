// SPDX-License-Identifier: MS-PL
// Parse the public, versioned content-build configuration without running a compiler.
#include <cstdint>
#include <filesystem>
#include <functional>
#include <limits>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Pipeline/ContentBuildConfiguration.hpp"

namespace Pipeline = CNA::Content::Pipeline;

namespace
{
    bool RefusesConfiguration(
        const std::string& json,
        const std::filesystem::path& source,
        const std::vector<std::string_view>& fragments)
    {
        try
        {
            (void)Pipeline::ContentBuildConfiguration::Parse(json, source);
        }
        catch (const std::runtime_error& error)
        {
            const std::string message = error.what();
            for (const std::string_view fragment : fragments)
            {
                if (message.find(fragment) == std::string::npos) return false;
            }
            return true;
        }
        return false;
    }

    std::string ConfigurationWithAsset(std::string asset)
    {
        return "{\"format\":\"CNA.ContentPipeline.Config\",\"version\":1,\"assets\":{" +
            std::move(asset) + "}}";
    }

    void BuildConfiguration(CnaDemos::DemoContext& ctx)
    {
        ctx.Check(Pipeline::ContentBuildConfigurationVersion == 1u &&
                      std::string(Pipeline::ContentBuildConfigurationFileName) ==
                          ".cna-content.json",
                  "the configuration version and conventional filename are stable");

        const auto empty = Pipeline::ContentBuildConfiguration::Parse(
            R"json({"format":"CNA.ContentPipeline.Config","version":1,"assets":{}})json");
        ctx.Check(empty.Entries().empty() && empty.SourceRoots().Empty() &&
                      !empty.OutputFormat().has_value() && empty.Find("missing.score") == nullptr,
                  "a minimal document has no overrides and leaves the command line to choose the output format");

        const std::string complete = R"json({
            "assets": {
                "zeta/empty.score": {},
                "alpha/full.score": {
                    "logicalName": "levels/intro",
                    "importer": "Demo.Importer",
                    "processor": "Demo.Processor",
                    "writer": "Demo.Writer",
                    "outputFormat": "cnb",
                    "parameters": {
                        "Text": {"type": "string", "value": "exact"},
                        "Unsigned": {"type": "u64", "value": "18446744073709551615"},
                        "Ratio": {"type": "f64", "value": "0.125"},
                        "Signed": {"type": "i64", "value": "-9223372036854775808"},
                        "Enabled": {"type": "bool", "value": true}
                    }
                }
            },
            "sourceRoots": {"z-root": "../z", "a-root": "../a"},
            "outputFormat": "xnb",
            "version": 1,
            "format": "CNA.ContentPipeline.Config"
        })json";
        const auto parsed = Pipeline::ContentBuildConfiguration::Parse(
            complete, "project/.cna-content.json");
        const auto* full = parsed.Find("alpha/full.score");
        const auto* defaults = parsed.Find("zeta/empty.score");
        ctx.Check(parsed.Entries().size() == 2u && parsed.Entries().begin()->first ==
                      "alpha/full.score" && full != nullptr && defaults != nullptr &&
                      full->source == "alpha/full.score" && defaults->source ==
                      "zeta/empty.score",
                  "asset entries are keyed and returned in deterministic source-path order");

        ctx.Check(full != nullptr && full->logicalName == "levels/intro" &&
                      full->importer == "Demo.Importer" &&
                      full->processor == "Demo.Processor" && full->writer == "Demo.Writer" &&
                      full->outputFormat == Pipeline::ContentOutputFormat::Cnb &&
                      defaults != nullptr && defaults->logicalName.empty() &&
                      defaults->importer.empty() && defaults->processor.empty() &&
                      defaults->writer.empty() && !defaults->outputFormat.has_value() &&
                      defaults->parameters.Empty(),
                  "one asset retains every override while an empty entry retains safe defaults");

        const auto parameterIs = [&](const char* name, const auto& expected)
        {
            if (full == nullptr) return false;
            const auto* value = full->parameters.Find(name);
            using T = std::decay_t<decltype(expected)>;
            const auto* typed = value == nullptr ? nullptr : std::get_if<T>(value);
            return typed != nullptr && *typed == expected;
        };
        ctx.Check(full != nullptr && full->parameters.Values().size() == 5u &&
                      parameterIs("Enabled", true) &&
                      parameterIs("Signed", std::numeric_limits<std::int64_t>::min()) &&
                      parameterIs("Unsigned", std::numeric_limits<std::uint64_t>::max()) &&
                      parameterIs("Ratio", 0.125) &&
                      parameterIs("Text", std::string("exact")),
                  "all five persisted parameter types retain their exact variant values");

        const auto& roots = parsed.SourceRoots().Entries();
        ctx.Check(parsed.OutputFormat() == Pipeline::ContentOutputFormat::Xnb &&
                      roots.size() == 2u && roots.begin()->first == "a-root" &&
                      roots.begin()->second == std::filesystem::path("../a") &&
                      parsed.SourceRoots().Find("z-root") != nullptr &&
                      *parsed.SourceRoots().Find("z-root") == std::filesystem::path("../z"),
                  "the project format and authored external roots remain explicit and alias-ordered");

        const bool malformed = RefusesConfiguration(
            "{", "settings/broken.json", {"settings/broken.json", "JSON"});
        const bool wrongKind = RefusesConfiguration(
            R"json({"format":"Other","version":1,"assets":{}})json", {},
            {"<memory>", "unknown format identity"});
        const bool wrongVersion = RefusesConfiguration(
            R"json({"format":"CNA.ContentPipeline.Config","version":2,"assets":{}})json",
            "settings.json", {"settings.json", "version", "not supported"});
        const bool unknownRoot = RefusesConfiguration(
            R"json({"format":"CNA.ContentPipeline.Config","version":1,"extra":0,"assets":{}})json",
            "settings.json", {"root", "unknown field 'extra'"});
        const bool duplicateRoot = RefusesConfiguration(
            R"json({"format":"CNA.ContentPipeline.Config","format":"CNA.ContentPipeline.Config","version":1,"assets":{}})json",
            "settings.json", {"root", "repeats field 'format'"});
        ctx.Check(malformed && wrongKind && wrongVersion && unknownRoot && duplicateRoot,
                  "syntax, identity, version and strict root-schema failures retain configuration context");

        const bool parentSource = RefusesConfiguration(
            ConfigurationWithAsset(R"json("../escape.score":{})json"), "settings.json",
            {"asset entry '../escape.score'", "must not contain '..'"});
        const bool absoluteSource = RefusesConfiguration(
            ConfigurationWithAsset(R"json("/absolute.score":{})json"), "settings.json",
            {"asset entry '/absolute.score'", "relative path"});
        const bool nativeSeparator = RefusesConfiguration(
            ConfigurationWithAsset(R"json("folder\\file.score":{})json"), "settings.json",
            {"source key", "using '/'"});
        const bool unnormalizedSource = RefusesConfiguration(
            ConfigurationWithAsset(R"json("folder/./file.score":{})json"), "settings.json",
            {"normalized generic UTF-8"});
        ctx.Check(parentSource && absoluteSource && nativeSeparator && unnormalizedSource,
                  "asset keys reject traversal, absolute, native-separator and unnormalized spellings");

        const bool unsafeLogicalName = RefusesConfiguration(
            ConfigurationWithAsset(R"json("ok.score":{"logicalName":"../escape"})json"),
            "settings.json", {"logicalName", "contains a '..' segment"});
        const bool emptyImporter = RefusesConfiguration(
            ConfigurationWithAsset(R"json("ok.score":{"importer":""})json"),
            "settings.json", {"field 'importer'", "non-empty string"});
        const bool badAssetFormat = RefusesConfiguration(
            ConfigurationWithAsset(R"json("ok.score":{"outputFormat":"zip"})json"),
            "settings.json", {"outputFormat", "cnb", "xnb"});
        const bool unknownAssetField = RefusesConfiguration(
            ConfigurationWithAsset(R"json("ok.score":{"surprise":true})json"),
            "settings.json", {"asset entry 'ok.score'", "unknown field 'surprise'"});
        ctx.Check(unsafeLogicalName && emptyImporter && badAssetFormat && unknownAssetField,
                  "per-asset names, component overrides, formats and fields are validated before use");

        const bool wrongBoolean = RefusesConfiguration(
            ConfigurationWithAsset(
                R"json("ok.score":{"parameters":{"P":{"type":"bool","value":"true"}}})json"),
            "settings.json", {"parameter 'P'", "true or false"});
        const bool numericJsonNumber = RefusesConfiguration(
            ConfigurationWithAsset(
                R"json("ok.score":{"parameters":{"P":{"type":"i64","value":7}}})json"),
            "settings.json", {"parameter 'P'", "numeric values must be strings"});
        const bool unknownParameterType = RefusesConfiguration(
            ConfigurationWithAsset(
                R"json("ok.score":{"parameters":{"P":{"type":"i32","value":"7"}}})json"),
            "settings.json", {"unknown parameter type 'i32'"});
        const bool nonFinite = RefusesConfiguration(
            ConfigurationWithAsset(
                R"json("ok.score":{"parameters":{"P":{"type":"f64","value":"nan"}}})json"),
            "settings.json", {"invalid finite floating-point value"});
        const bool repeatedParameter = RefusesConfiguration(
            ConfigurationWithAsset(
                R"json("ok.score":{"parameters":{"P":{"type":"bool","value":true},"P":{"type":"bool","value":false}}})json"),
            "settings.json", {"repeats parameter 'P'"});
        ctx.Check(wrongBoolean && numericJsonNumber && unknownParameterType && nonFinite &&
                      repeatedParameter,
                  "typed parameters reject mismatched JSON, lossy numeric syntax, unknown types, non-finite values and duplicates");

        const bool invalidAlias = RefusesConfiguration(
            R"json({"format":"CNA.ContentPipeline.Config","version":1,"sourceRoots":{"9bad":"../shared"},"assets":{}})json",
            "settings.json", {"alias '9bad'", "begin with a lowercase ASCII letter"});
        const bool emptyRoot = RefusesConfiguration(
            R"json({"format":"CNA.ContentPipeline.Config","version":1,"sourceRoots":{"shared":""},"assets":{}})json",
            "settings.json", {"alias 'shared'", "non-empty string"});
        const bool repeatedAlias = RefusesConfiguration(
            R"json({"format":"CNA.ContentPipeline.Config","version":1,"sourceRoots":{"shared":"../a","shared":"../b"},"assets":{}})json",
            "settings.json", {"repeats alias 'shared'"});
        ctx.Check(invalidAlias && emptyRoot && repeatedAlias,
                  "source-root capabilities reject invalid aliases, empty paths and duplicate grants");
    }
}

CNA_DEMOS_CONSOLE(BuildConfiguration, {
    .id = "content.custom.build_configuration",
    .title = "Parse content build configuration (CNAEXT)",
    .description = "Parses the complete public .cna-content.json shape, including ordered assets, external roots, container overrides and every typed processor parameter, then checks strict schema and path refusals.",
    .apis = {"CNA::Content::Pipeline::ContentBuildConfigurationVersion",
             "CNA::Content::Pipeline::ContentBuildConfigurationFileName",
             "CNA::Content::Pipeline::ContentAssetBuildConfiguration",
             "CNA::Content::Pipeline::ContentBuildConfiguration::Parse",
             "CNA::Content::Pipeline::ContentBuildConfiguration::Find",
             "CNA::Content::Pipeline::ContentBuildConfiguration::Entries",
             "CNA::Content::Pipeline::ContentBuildConfiguration::OutputFormat",
             "CNA::Content::Pipeline::ContentBuildConfiguration::SourceRoots",
             "CNA::Content::Pipeline::ContentSourceRootCapabilities::Find",
             "CNA::Content::Pipeline::ContentSourceRootCapabilities::Entries",
             "CNA::Content::Pipeline::ContentProcessorParameters::Find",
             "CNA::Content::Pipeline::ContentProcessorParameters::Values",
             "CNA::Content::Pipeline::ContentProcessorParameters::Empty"},
    .tags = {"content", "pipeline", "configuration", "validation", "cnaext"},
    .restrictions = "Pure in-memory parsing. Existing compiler demos cover applying the configuration to incremental builds; no filesystem resolution or compilation is claimed here.",
})
