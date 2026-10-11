// SPDX-License-Identifier: MS-PL
// A build-time XML asset with named, optional and ignored game fields.
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "Microsoft/Xna/Framework/Content/ContentSerializerAttribute.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "System/Object.hpp"
#include "System/Xml/XmlReader.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/XmlWriterSettings.hpp"

using Microsoft::Xna::Framework::Content::ContentSerializerAttribute;
using Microsoft::Xna::Framework::Content::Pipeline::InvalidContentException;
using namespace Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate;

namespace
{
    struct Encounter : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.Encounter";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName);
            return name;
        }
        std::string name;
        std::int32_t health = 0;
        std::optional<std::string> hint;
        std::vector<std::string> loot;
        std::int32_t runtimeScore = 0;

        static void DescribeContent(ContentTypeDescriptor<Encounter>& type)
        {
            type.Field("Name", &Encounter::name);
            // The public XNA attribute is a real format object; CNA's descriptor accepts it.
            ContentSerializerAttribute healthFormat;
            healthFormat.setElementNameProperty("HitPoints");
            type.Field("Health", &Encounter::health).Format(healthFormat);
            type.Field("Hint", &Encounter::hint).Optional();
            type.Field("Loot", &Encounter::loot).CollectionItemName("Drop");
            type.Field("RuntimeScore", &Encounter::runtimeScore).Ignore();
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
        auto authored = std::make_shared<Encounter>();
        authored->name = "Gatekeeper";
        authored->health = 90;
        authored->loot = {"Coin", "Key"};
        authored->runtimeScore = 1234; // runtime-only state must never enter the source asset

        System::Xml::XmlWriterSettings settings;
        settings.Indent = true;
        settings.NewLineChars = "\n";
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<Encounter>(*writer, authored, "");
        const std::string xml = writer->ToString();
        std::cout << xml << '\n';
        check(xml.find("<HitPoints>90</HitPoints>") != std::string::npos &&
              xml.find("<Health>") == std::string::npos, "the format attribute renames Health to HitPoints");
        check(xml.find("<Drop>Coin</Drop>") != std::string::npos,
              "the collection uses the authored item name Drop");
        check(xml.find("<Hint") == std::string::npos && xml.find("RuntimeScore") == std::string::npos,
              "absent optional and ignored fields are omitted from the XML");

        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        const auto loaded = IntermediateSerializer::Deserialize<Encounter>(*reader, "");
        check(loaded && loaded->name == "Gatekeeper" && loaded->health == 90 && loaded->loot == authored->loot,
              "the authored encounter values round trip through intermediate XML");
        check(loaded && !loaded->hint && loaded->runtimeScore == 0,
              "omitted fields keep their constructor defaults instead of copying runtime state");

        // Ignored means neither written nor accepted, not a hidden route to inject runtime state.
        std::string invalid = xml;
        const auto end = invalid.find("</Asset>");
        if (end == std::string::npos) throw std::runtime_error("Serialized intermediate XML has no Asset closing tag");
        invalid.insert(end, "<RuntimeScore>999</RuntimeScore>");
        bool rejected = false;
        try
        {
            std::unique_ptr<System::Xml::XmlReader> badReader(System::Xml::XmlReader::CreateFromString(invalid));
            (void)IntermediateSerializer::Deserialize<Encounter>(*badReader, "");
        }
        catch (const InvalidContentException& error)
        {
            rejected = true;
            std::cout << "Ignored field rejected: " << error.what() << '\n';
        }
        check(rejected, "an ignored field in authored XML is rejected with InvalidContentException");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Intermediate serialization failed: " << error.what() << '\n';
        return 1;
    }
}
