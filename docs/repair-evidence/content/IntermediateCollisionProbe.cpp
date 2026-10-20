// SPDX-License-Identifier: MS-PL
// ISSUE-088: bounded, actual-library registration atomicity and round-trip probe.
#include <iostream>
#include <memory>
#include <string>
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "System/InvalidOperationException.hpp"
namespace Intermediate = Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate;
using Microsoft::Xna::Framework::Content::ContentSerializerAttribute;
struct FirstPayload { static constexpr std::string_view XnaTypeName = "Repair.FirstPayload"; std::string text; };
struct SecondPayload { static constexpr std::string_view XnaTypeName = "Repair.SecondPayload"; std::string text; };
struct ThirdPayload { static constexpr std::string_view XnaTypeName = "Repair.ThirdPayload"; std::string text; };
template<typename T> class AliasSerializer final : public Intermediate::ContentTypeSerializer<T>
{
public:
    explicit AliasSerializer(std::string alias) : Intermediate::ContentTypeSerializer<T>(std::move(alias)) {}
protected:
    void Serialize(Intermediate::IntermediateWriter& output, const T& value, const ContentSerializerAttribute&) override
    { output.getXmlProperty().WriteString(value.text); }
    T Deserialize(Intermediate::IntermediateReader& input, const ContentSerializerAttribute& format, T) override
    { return T{input.ReadText(format)}; }
};
template<typename T> std::string Write(const T& value)
{
    std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString());
    Intermediate::IntermediateSerializer::Serialize<T>(*writer, value, "");
    return writer->ToString();
}
int main()
{
    using S = Intermediate::IntermediateSerializer;
    int checks = 0, failed = 0;
    const auto check = [&](bool value, const char* label) {
        ++checks;
        if (!value) ++failed;
        std::cout << (value ? "PASS " : "FAIL ") << label << '\n';
    };
    try
    {
        (void)S::TypeSerializerFor<std::string>();
        auto& first = S::RegisterTypeSerializer(std::make_unique<AliasSerializer<FirstPayload>>("repairCollision"));
        check(S::FindTypeSerializer(System::Type::From<FirstPayload>()) == &first, "first target identity");
        check(S::FindTypeSerializer(std::string("repairCollision")) == &first, "first alias identity");
        const auto original = Write(FirstPayload{"first"});
        bool rejected = false;
        try { (void)S::RegisterTypeSerializer(std::make_unique<AliasSerializer<SecondPayload>>("repairCollision")); }
        catch (const System::InvalidOperationException&) { rejected = true; }
        check(rejected, "conflicting shortcut rejected with InvalidOperationException");
        check(S::FindTypeSerializer(System::Type::From<SecondPayload>()) == nullptr, "rejected target unpublished");
        check(S::FindTypeSerializerForCarrier(typeid(SecondPayload)) == nullptr, "rejected carrier unpublished");
        check(S::FindTypeSerializer(std::string(SecondPayload::XnaTypeName)) == nullptr, "rejected full name unpublished");
        check(S::FindTypeSerializer(std::string("repairCollision")) == &first, "original shortcut preserved");

        auto replacement = std::make_unique<AliasSerializer<SecondPayload>>("repairDistinct");
        const auto* expected = replacement.get();
        auto& second = S::RegisterTypeSerializer(std::move(replacement));
        check(&second == expected, "rejected target can register a fresh serializer");
        check(S::FindTypeSerializer(std::string("repairDistinct")) == &second, "new shortcut published");
        check(Write(FirstPayload{"first"}) == original, "original XML byte-identical");
        bool roundTrip = false;
        try
        {
            std::unique_ptr<System::Xml::XmlReader> reader(
                System::Xml::XmlReader::CreateFromString(Write(SecondPayload{"second"})));
            roundTrip = S::Deserialize<SecondPayload>(*reader, "").text == "second";
        }
        catch (const std::exception& error) { std::cout << "ROUNDTRIP " << error.what() << '\n'; }
        check(roundTrip, "recovered target round trips exactly");

        auto& duplicate = S::RegisterTypeSerializer(std::make_unique<AliasSerializer<FirstPayload>>("unusedAlias"));
        check(&duplicate == &first, "existing target retains CNA first-registration identity");
        check(S::FindTypeSerializer(std::string("unusedAlias")) == nullptr, "duplicate target publishes no new alias");

        rejected = false;
        try { (void)S::RegisterTypeSerializer(std::make_unique<AliasSerializer<ThirdPayload>>(""),
                                             {"unusedAdditional", "repairCollision"}, false); }
        catch (const System::InvalidOperationException&) { rejected = true; }
        check(rejected, "additional-name collision rejected");
        check(S::FindTypeSerializer(System::Type::From<ThirdPayload>()) == nullptr, "additional-name rejection leaves target absent");
        check(S::FindTypeSerializer(std::string("unusedAdditional")) == nullptr, "additional-name rejection publishes no earlier alias");
    }
    catch (const std::exception& error) { ++failed; std::cout << "UNEXPECTED " << error.what() << '\n'; }
    std::cout << "checks=" << checks << " failed=" << failed << '\n';
    return failed == 0 && checks == 16 ? 0 : 1;
}
