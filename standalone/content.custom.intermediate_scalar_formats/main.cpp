// SPDX-License-Identifier: MS-PL
// XNA intermediate XML uses invariant, type-specific scalar spellings rather than C++ stream defaults.
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "System/DateTime.hpp"
#include "System/DateTimeKind.hpp"
#include "System/Decimal.hpp"
#include "System/Object.hpp"
#include "System/TimeSpan.hpp"
#include "System/Xml/XmlReader.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/XmlWriterSettings.hpp"

using Microsoft::Xna::Framework::Content::Pipeline::InvalidContentException;
using namespace Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate;

namespace CnaDemos
{
    enum class Mood { Happy, Sad, Angry };
    enum class Toppings { None = 0, Cheese = 1, Ham = 2, Olives = 4 };

    System::Decimal ParseDecimal(std::string_view text)
    {
        System::Decimal value;
        if (!System::Decimal::TryParse(std::string(text), value))
        {
            throw std::logic_error("Invalid demonstration Decimal literal");
        }
        return value;
    }

    System::DateTime UtcDateTime(int year, int month, int day, int hour, int minute, int second)
    {
        const System::DateTime unspecified(year, month, day, hour, minute, second);
        return System::DateTime(unspecified.getTicksProperty(), System::DateTimeKind::Utc);
    }

    struct ScalarSettings final : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.ScalarSettings";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName);
            return name;
        }

        bool enabled = true;
        std::int32_t count = -123456789;
        float negativeZero = -0.0f;
        float notANumber = std::numeric_limits<float>::quiet_NaN();
        float positiveInfinity = std::numeric_limits<float>::infinity();
        float third = 1.0f / 3.0f;
        double doubleThird = 1.0 / 3.0;
        System::Decimal cost = ParseDecimal("12.34");
        System::TimeSpan duration = System::TimeSpan(0, 1, 30);
        System::DateTime timestamp = UtcDateTime(2010, 9, 16, 12, 30, 45);
        Mood mood = Mood::Sad;
        Toppings toppings = static_cast<Toppings>(
            static_cast<int>(Toppings::Cheese) | static_cast<int>(Toppings::Olives));

        static void DescribeContent(ContentTypeDescriptor<ScalarSettings>& type)
        {
            type.Field("Enabled", &ScalarSettings::enabled);
            type.Field("Count", &ScalarSettings::count);
            type.Field("NegativeZero", &ScalarSettings::negativeZero);
            type.Field("NotANumber", &ScalarSettings::notANumber);
            type.Field("PositiveInfinity", &ScalarSettings::positiveInfinity);
            type.Field("Third", &ScalarSettings::third);
            type.Field("DoubleThird", &ScalarSettings::doubleThird);
            type.Field("Cost", &ScalarSettings::cost);
            type.Field("Duration", &ScalarSettings::duration);
            type.Field("Timestamp", &ScalarSettings::timestamp);
            type.Field("Mood", &ScalarSettings::mood);
            type.Field("Toppings", &ScalarSettings::toppings);
        }
    };
}

CNA_XNA_CONTENT_ENUM(CnaDemos::Mood, "CnaDemos.Mood", false,
                     {CnaDemos::Mood::Happy, "Happy"},
                     {CnaDemos::Mood::Sad, "Sad"},
                     {CnaDemos::Mood::Angry, "Angry"});
CNA_XNA_CONTENT_ENUM(CnaDemos::Toppings, "CnaDemos.Toppings", true,
                     {CnaDemos::Toppings::None, "None"},
                     {CnaDemos::Toppings::Cheese, "Cheese"},
                     {CnaDemos::Toppings::Ham, "Ham"},
                     {CnaDemos::Toppings::Olives, "Olives"});

namespace
{
    std::string Replaced(std::string text, std::string_view from, std::string_view to)
    {
        const std::size_t at = text.find(from);
        if (at == std::string::npos) throw std::logic_error("Expected XML marker was not found");
        text.replace(at, from.size(), to);
        return text;
    }
}

int main()
{
    try
    {
        bool passed = true;
        const auto check = [&](bool condition, std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = condition && passed;
        };

        const auto authored = std::make_shared<CnaDemos::ScalarSettings>();
        System::Xml::XmlWriterSettings settings;
        settings.Indent = false;
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<CnaDemos::ScalarSettings>(*writer, authored, "");
        const std::string xml = writer->ToString();
        std::cout << xml << '\n';

        check(xml.find("<Enabled>true</Enabled><Count>-123456789</Count>") != std::string::npos,
              "Boolean and integer values use invariant lower-case and decimal text");
        check(xml.find("<NegativeZero>-0</NegativeZero><NotANumber>NaN</NotANumber>"
                       "<PositiveInfinity>INF</PositiveInfinity>") != std::string::npos,
              "special floating values use XNA's canonical signed-zero, NaN and infinity spellings");
        check(xml.find("<Third>0.333333343</Third><DoubleThird>0.33333333333333331</DoubleThird>") !=
                  std::string::npos,
              "Single and Double use the measured XNA round-trip precision");
        check(xml.find("<Cost>12.34</Cost><Duration>PT1M30S</Duration>"
                       "<Timestamp>2010-09-16T12:30:45Z</Timestamp>") != std::string::npos,
              "Decimal, TimeSpan and UTC DateTime use invariant XNA text");
        check(xml.find("<Mood>Sad</Mood><Toppings>Cheese, Olives</Toppings>") != std::string::npos,
              "an enum uses its exact member name and flags use comma-space spelling");

        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        const auto restored = IntermediateSerializer::Deserialize<CnaDemos::ScalarSettings>(*reader, "");
        check(restored && restored->enabled && restored->count == authored->count &&
                  restored->cost == authored->cost && restored->duration == authored->duration &&
                  restored->timestamp == authored->timestamp &&
                  restored->timestamp.getKindProperty() == System::DateTimeKind::Utc,
              "ordinary numeric and temporal values round-trip exactly, including DateTime kind");
        check(restored && std::signbit(restored->negativeZero) && std::isnan(restored->notANumber) &&
                  std::isinf(restored->positiveInfinity) && restored->positiveInfinity > 0.0f &&
                  restored->third == authored->third && restored->doubleThird == authored->doubleThird,
              "special values and both floating round-trip bit values are retained");
        check(restored && restored->mood == CnaDemos::Mood::Sad &&
                  restored->toppings == authored->toppings,
              "enum and combined flags round-trip their typed values");

        const auto rejects = [&](const std::string& malformed, std::string_view expectation) {
            bool rejected = false;
            try
            {
                std::unique_ptr<System::Xml::XmlReader> bad(System::Xml::XmlReader::CreateFromString(malformed));
                (void)IntermediateSerializer::Deserialize<CnaDemos::ScalarSettings>(*bad, "");
            }
            catch (const InvalidContentException& error)
            {
                rejected = true;
                std::cout << "Rejected scalar XML: " << error.what() << '\n';
            }
            check(rejected, expectation);
        };

        rejects(Replaced(xml, "<Enabled>true</Enabled>", "<Enabled>True</Enabled>"),
                "Boolean input is case-sensitive");
        rejects(Replaced(xml, "<Count>-123456789</Count>", "<Count>3000000000</Count>"),
                "an out-of-range Int32 is rejected");
        rejects(Replaced(xml, "<NotANumber>NaN</NotANumber>", "<NotANumber>nan</NotANumber>"),
                "a noncanonical lower-case NaN is rejected");
        rejects(Replaced(xml, "<Cost>12.34</Cost>", "<Cost>1e2</Cost>"),
                "Decimal exponent notation is rejected");
        rejects(Replaced(xml, "<Duration>PT1M30S</Duration>", "<Duration>00:01:30</Duration>"),
                "a .NET-style TimeSpan is rejected in favor of ISO 8601 duration text");
        rejects(Replaced(xml, "<Mood>Sad</Mood>", "<Mood>sad</Mood>"),
                "enum member names are case-sensitive");
        rejects(Replaced(xml, "<Toppings>Cheese, Olives</Toppings>",
                         "<Toppings>Cheese Olives</Toppings>"),
                "flags require a comma between member names");

        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Intermediate scalar demonstration failed: " << error.what() << '\n';
        return 1;
    }
}
