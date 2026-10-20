// SPDX-License-Identifier: MS-PL
// ISSUE-105: named-only components never participate in default routing.
#include "CNA/Content/Pipeline/ContentPipeline.hpp"
#include <iostream>
#include <stdexcept>
#include <typeinfo>

namespace Pipeline = CNA::Content::Pipeline;
namespace
{
    constexpr const char* inputType = "probe.Input";
    int checks = 0, failed = 0;
    void Check(bool condition, const std::string& label)
    {
        ++checks;
        if (!condition) ++failed;
        std::cout << (condition ? "PASS " : "FAIL ") << label << '\n';
    }
    class Importer final : public Pipeline::ContentImporter
    {
        std::string name_;
        bool named_;
    public:
        Importer(std::string name, bool named) : name_(std::move(name)), named_(named) {}
        Pipeline::ContentComponentIdentity Identity() const override { return {name_, "1"}; }
        std::vector<std::string> SourceExtensions() const override { return {".probe"}; }
        std::vector<std::string> OutputTypes() const override { return {inputType}; }
        bool SelectedByNameOnly() const override { return named_; }
        Pipeline::ContentValue Import(Pipeline::ContentImporterContext&) const override
        {
            throw std::logic_error("fixture must not execute importer");
        }
    };
    class Processor final : public Pipeline::ContentProcessor
    {
        std::string name_;
        bool named_;
    public:
        Processor(std::string name, bool named) : name_(std::move(name)), named_(named) {}
        Pipeline::ContentComponentIdentity Identity() const override { return {name_, "1"}; }
        std::string InputType() const override { return inputType; }
        std::string OutputType() const override { return "probe.Output"; }
        bool SelectedByNameOnly() const override { return named_; }
        void ValidateParameters(const Pipeline::ContentProcessorParameters&) const override {}
        Pipeline::ContentValue Process(const Pipeline::ContentValue&,
                                      Pipeline::ContentProcessorContext&) const override
        {
            throw std::logic_error("fixture must not execute processor");
        }
    };
    template<class Action>
    std::string Refusal(Action action)
    {
        try { action(); }
        catch (const std::logic_error& error)
        {
            if (typeid(error) == typeid(std::logic_error)) return error.what();
        }
        catch (...) { }
        return {};
    }
    template<class Add, class Resolve>
    void Exercise(const std::string& kind, Add add, Resolve resolve)
    {
        auto refusal = [&] { return Refusal([&] { (void)resolve(""); }); };
        Check(refusal().find("no " + kind) != std::string::npos, kind + " empty registry refuses default");
        add("probe.NamedOne", true);
        Check(refusal().find("no " + kind) != std::string::npos, kind + " sole named-only refuses default");
        Check(resolve("probe.NamedOne") == "probe.NamedOne", kind + " sole named-only resolves explicitly");
        Check(!Refusal([&] { (void)resolve("probe.Absent"); }).empty(), kind + " unknown explicit name is refused");
        add("probe.NamedTwo", true);
        Check(refusal().find("no " + kind) != std::string::npos, kind + " all named-only reports no default rather than ambiguity");
        Check(resolve("probe.NamedTwo") == "probe.NamedTwo", kind + " multiple named-only resolve explicitly");
        add("probe.ZOrdinary", false);
        Check(resolve("") == "probe.ZOrdinary", kind + " sole eligible ordinary route wins");
        Check(resolve("probe.NamedOne") == "probe.NamedOne", kind + " explicit named-only survives ordinary registration");
        add("probe.AOrdinary", false);
        const auto ambiguous = refusal();
        Check(ambiguous.find("ambiguous " + kind) != std::string::npos, kind + " two ordinary candidates remain ambiguous");
        const auto first = ambiguous.find("probe.AOrdinary"), last = ambiguous.find("probe.ZOrdinary");
        Check(first != std::string::npos && last != std::string::npos && first < last &&
              ambiguous.find("probe.Named") == std::string::npos,
              kind + " ambiguity lists only eligible candidates in stable order");
    }
}
int main()
{
    try
    {
        Pipeline::ContentPipelineRegistry importers;
        Check(!importers.HasImporterForSource("asset.probe"), "empty importer discovery is false");
        Exercise("importer",
                 [&](const std::string& name, bool named) { importers.RegisterImporter(std::make_shared<Importer>(name, named)); },
                 [&](const std::string& name) { return importers.ResolveImporter("ASSET.PROBE", name)->Identity().name; });
        Check(importers.HasImporterForSource("ASSET.PROBE"), "importer discovery remains case insensitive");
        Check(!Refusal([&] { (void)importers.ResolveImporter("asset.wrong", "probe.NamedOne"); }).empty(), "explicit importer must accept source extension");
        Pipeline::ContentPipelineRegistry namedDiscovery;
        namedDiscovery.RegisterImporter(std::make_shared<Importer>("probe.Only", true));
        Check(namedDiscovery.HasImporterForSource("asset.probe"), "named-only importer remains discoverable");
        Pipeline::ContentPipelineRegistry processors;
        Exercise("processor",
                 [&](const std::string& name, bool named) { processors.RegisterProcessor(std::make_shared<Processor>(name, named)); },
                 [&](const std::string& name) { return processors.ResolveProcessor(inputType, name)->Identity().name; });
        Check(!Refusal([&] { (void)processors.ResolveProcessor("probe.Wrong", "probe.NamedOne"); }).empty(), "explicit processor must accept input type");
        std::cout << "checks=" << checks << " failed=" << failed << '\n';
        return failed ? 1 : 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Unexpected fixture error: " << error.what() << '\n';
        return 2;
    }
}
