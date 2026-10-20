// SPDX-License-Identifier: MS-PL
// Run in an isolated process with a bounded deadline: pre-fix ISSUE-090 deadlocks.
#include <iostream>
#include <memory>
#include <string>
#include "Microsoft/Xna/Framework/Content/Pipeline/ContentItem.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Compiler/ContentCompiler.hpp"
namespace Xna = Microsoft::Xna::Framework::Content::Pipeline;
namespace Compiler = Xna::Serialization::Compiler;
struct ProbePayload : Xna::ContentItem
{
    static constexpr std::string_view XnaTypeName = "Repair.InitializationPayload";
    const std::string& GetTypeName() const override
    { static const std::string name(XnaTypeName); return name; }
};
struct OtherPayload : ProbePayload
{
    static constexpr std::string_view XnaTypeName = "Repair.OtherPayload";
    const std::string& GetTypeName() const override
    { static const std::string name(XnaTypeName); return name; }
};
template<class T> class InitializationWriter final : public Compiler::ContentTypeWriter<T>
{
public:
    inline static int totalCalls = 0;
    int calls = 0;
    std::shared_ptr<Compiler::ContentTypeWriterBase> nested;
    std::string GetRuntimeReader(Xna::TargetPlatform) const override { return "Repair.Reader, Repair"; }
protected:
    void Initialize(Compiler::ContentCompiler& compiler) override
    {
        ++calls; ++totalCalls;
        std::cout << "ENTER Initialize nested lookup" << std::endl;
        nested = compiler.GetTypeWriter(System::Type::From<std::string>());
        std::cout << "LEAVE Initialize nested lookup" << std::endl;
    }
    void Write(Compiler::ContentWriter& output, const std::shared_ptr<T>&) override
    { output.Write(std::string("retained")); }
};
int main()
{
    int checks = 0, failed = 0;
    const auto check = [&](bool value, const char* label) {
        ++checks;
        if (!value) ++failed;
        std::cout << (value ? "PASS " : "FAIL ") << label << std::endl;
    };
    try
    {
        using Writer = InitializationWriter<ProbePayload>;
        Compiler::ContentCompiler compiler;
        const auto nested = compiler.GetTypeWriter(System::Type::From<std::string>());
        check(nested != nullptr, "built-in lookup control before callback");
        const auto before = compiler.KnownTypeNames().size();
        const auto writer = compiler.AddTypeWriter<Writer>();
        check(writer->calls == 1, "callback exactly once");
        check(writer->nested == nested, "nested lookup preserves identity");
        check(compiler.GetTypeWriter(System::Type::From<ProbePayload>()) == writer, "registered identity");
        check(writer->calls == 1, "lookup does not initialize again");
        check(compiler.KnownTypeNames().size() == before + 1, "one registration published");
        bool duplicate = false;
        try { (void)compiler.AddTypeWriter<Writer>(); }
        catch (const Xna::PipelineException&) { duplicate = true; }
        check(duplicate && Writer::totalCalls == 1, "duplicate rejected before callback");
        (void)compiler.TypeWriterRegistry(Xna::TargetPlatform::Windows);
        bool frozen = false;
        try { (void)compiler.AddTypeWriter<InitializationWriter<OtherPayload>>(); }
        catch (const Xna::PipelineException&) { frozen = true; }
        check(frozen && InitializationWriter<OtherPayload>::totalCalls == 0, "frozen registration rejected before callback");
        Compiler::ContentCompiler second;
        const auto other = second.AddTypeWriter<Writer>();
        check(other != writer && other->calls == 1 && other->nested != nullptr, "independent compiler initialization");
    }
    catch (const std::exception& error)
    { ++failed; std::cerr << "UNEXPECTED " << error.what() << '\n'; }
    std::cout << "checks=" << checks << " failed=" << failed << '\n';
    return failed == 0 && checks == 9 ? 0 : 1;
}
