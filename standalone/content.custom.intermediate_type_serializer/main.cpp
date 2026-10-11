// SPDX-License-Identifier: MS-PL
// A game-owned dice expression has compact syntax and explicit domain validation.
#include <charconv>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "System/Xml/XmlReader.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/XmlWriterSettings.hpp"

using Microsoft::Xna::Framework::Content::ContentSerializerAttribute;
using Microsoft::Xna::Framework::Content::Pipeline::InvalidContentException;
using namespace Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate;

namespace
{
    struct DiceRoll
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.DiceRoll";
        int count = 1;
        int sides = 6;
        bool operator==(const DiceRoll&) const = default;
    };

    class DiceRollSerializer final : public ContentTypeSerializer<DiceRoll>
    {
    public:
        DiceRollSerializer() : ContentTypeSerializer<DiceRoll>("dice") {}

    protected:
        void Serialize(IntermediateWriter& output, const DiceRoll& value,
                       const ContentSerializerAttribute&) override
        {
            Validate(value);
            output.getXmlProperty().WriteString(std::to_string(value.count) + "d" + std::to_string(value.sides));
        }

        DiceRoll Deserialize(IntermediateReader& input, const ContentSerializerAttribute& format,
                             DiceRoll) override
        {
            const std::string text = input.ReadText(format);
            const auto separator = text.find('d');
            if (separator == std::string::npos) throw InvalidContentException("Dice must use the compact NdM syntax.");
            DiceRoll value;
            const char* first = text.data();
            const char* middle = first + separator;
            const char* end = first + text.size();
            const auto count = std::from_chars(first, middle, value.count);
            const auto sides = std::from_chars(middle + 1, end, value.sides);
            if (count.ec != std::errc{} || count.ptr != middle || sides.ec != std::errc{} || sides.ptr != end)
                throw InvalidContentException("Dice require two complete integer tokens separated by d.");
            Validate(value);
            return value;
        }

    private:
        static void Validate(const DiceRoll& value)
        {
            if (value.count < 1 || value.count > 64 || value.sides < 2 || value.sides > 256)
                throw InvalidContentException("Dice require 1..64 rolls and 2..256 sides.");
        }
    };

    struct Attack
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.Attack";
        std::string name;
        DiceRoll damage;
        static void DescribeContent(ContentTypeDescriptor<Attack>& type)
        {
            type.Field("Name", &Attack::name);
            type.Field("Damage", &Attack::damage);
        }
    };
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
        // Registration replaces the reflection discovery that XNA performs for custom serializers.
        auto& serializer = IntermediateSerializer::AddTypeSerializer<DiceRollSerializer>();
        check(IntermediateSerializer::FindTypeSerializer(System::Type::From<DiceRoll>()) == &serializer &&
              serializer.getXmlTypeNameProperty() == "dice", "the registered value serializer has the dice XML shortcut");

        const DiceRoll roll{2, 6};
        System::Xml::XmlWriterSettings settings;
        settings.Indent = true;
        settings.NewLineChars = "\n";
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<DiceRoll>(*writer, roll, "");
        const std::string xml = writer->ToString();
        std::cout << xml << '\n';
        check(xml.find("Type=\"dice\"") != std::string::npos && xml.find(">2d6</Asset>") != std::string::npos,
              "a root dice value uses its compact syntax and registered type shortcut");
        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        check(IntermediateSerializer::Deserialize<DiceRoll>(*reader, "") == roll,
              "the custom parser restores both roll count and side count");

        std::unique_ptr<System::Xml::XmlWriter> attackWriter(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<Attack>(*attackWriter, Attack{"Spark", roll}, "");
        const std::string attackXml = attackWriter->ToString();
        std::cout << attackXml << '\n';
        std::unique_ptr<System::Xml::XmlReader> attackReader(System::Xml::XmlReader::CreateFromString(attackXml));
        const auto attack = IntermediateSerializer::Deserialize<Attack>(*attackReader, "");
        check(attack.name == "Spark" && attack.damage == roll &&
              attackXml.find("<Damage>2d6</Damage>") != std::string::npos,
              "described parent fields dispatch to the same registered custom serializer");

        const auto payload = xml.find("2d6");
        if (payload == std::string::npos) throw std::runtime_error("Serialized dice payload missing");
        for (const std::string invalid : {"0d6", "2d6junk"})
        {
            std::string malformed = xml;
            malformed.replace(payload, 3, invalid);
            bool rejected = false;
            try
            {
                std::unique_ptr<System::Xml::XmlReader> badReader(System::Xml::XmlReader::CreateFromString(malformed));
                (void)IntermediateSerializer::Deserialize<DiceRoll>(*badReader, "");
            }
            catch (const InvalidContentException& error)
            {
                rejected = true;
                std::cout << "Rejected " << invalid << ": " << error.what() << '\n';
            }
            check(rejected, invalid == "0d6" ? "zero rolls violate the game's dice domain"
                                              : "trailing text cannot masquerade as a valid dice expression");
        }
        bool invalidObjectRejected = false;
        try
        {
            std::unique_ptr<System::Xml::XmlWriter> badWriter(System::Xml::XmlWriter::CreateToString());
            IntermediateSerializer::Serialize<DiceRoll>(*badWriter, DiceRoll{2, 1}, "");
        }
        catch (const InvalidContentException&)
        {
            invalidObjectRejected = true;
        }
        check(invalidObjectRejected, "serialization also rejects invalid game objects before emitting their payload");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Custom intermediate serializer failed: " << error.what() << '\n';
        return 1;
    }
}
