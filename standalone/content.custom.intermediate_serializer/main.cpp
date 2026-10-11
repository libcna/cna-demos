// SPDX-License-Identifier: MS-PL
// A build-time XML asset with named, optional and ignored game fields.
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
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
        std::optional<std::string> objective = "Open the gate";
        std::optional<std::string> hint = "Look for the key";
        std::vector<std::string> loot;
        std::int32_t runtimeScore = 0;

        static void DescribeContent(ContentTypeDescriptor<Encounter>& type)
        {
            type.Field("Name", &Encounter::name);
            // The public XNA attribute is a real format object; CNA's descriptor accepts it.
            ContentSerializerAttribute healthFormat;
            healthFormat.setElementNameProperty("HitPoints");
            type.Field("Health", &Encounter::health).Format(healthFormat);
            // The C++ carrier can represent absence, but the game's authored schema forbids it.
            type.Field("Objective", &Encounter::objective).AllowNull(false);
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
        authored->objective = "Recover the key";
        authored->hint.reset(); // omitted Optional restores the reader constructor default
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
        check(loaded && loaded->name == "Gatekeeper" && loaded->health == 90 && loaded->loot == authored->loot &&
              loaded->objective == "Recover the key",
              "the authored encounter values round trip through intermediate XML");
        check(loaded && loaded->hint == "Look for the key" && loaded->runtimeScore == 0,
              "absent Optional keeps its constructor default and ignored runtime state is not copied");

        // Absence means keep the default; an explicit Null means clear it. A present value wins.
        const auto lootStart = xml.find("<Loot>");
        if (lootStart == std::string::npos) throw std::runtime_error("Serialized XML has no Loot element");
        const std::pair<std::string, std::optional<std::string>> hintCases[] = {
            {"<Hint>Watch the doorway</Hint>", std::string("Watch the doorway")},
            {"<Hint Null=\"true\"/>", std::nullopt}};
        for (const auto& [element, expected] : hintCases)
        {
            std::string changed = xml;
            changed.insert(lootStart, element);
            std::unique_ptr<System::Xml::XmlReader> hintReader(System::Xml::XmlReader::CreateFromString(changed));
            const auto withHint = IntermediateSerializer::Deserialize<Encounter>(*hintReader, "");
            std::cout << "Authored override: " << element << '\n';
            check(withHint && withHint->hint == expected,
                  expected ? "a present optional hint overrides the default" : "explicit Null clears the default hint");
        }

        // Ignored means neither written nor accepted, not a hidden route to inject runtime state.
        std::string invalid = xml;
        const auto end = invalid.find("</Asset>");
        if (end == std::string::npos) throw std::runtime_error("Serialized intermediate XML has no Asset closing tag");
        invalid.insert(end, "<RuntimeScore>999</RuntimeScore>");
        std::string missing = xml;
        const std::string healthElement = "<HitPoints>90</HitPoints>";
        const auto healthStart = missing.find(healthElement);
        if (healthStart == std::string::npos) throw std::runtime_error("Serialized XML has no HitPoints element");
        missing.erase(healthStart, healthElement.size());
        std::string nullObjective = xml;
        const std::string objectiveElement = "<Objective>Recover the key</Objective>";
        const auto objectiveStart = nullObjective.find(objectiveElement);
        if (objectiveStart == std::string::npos) throw std::runtime_error("Serialized XML has no Objective element");
        nullObjective.replace(objectiveStart, objectiveElement.size(), "<Objective Null=\"true\"/>");
        const std::pair<std::string, std::string> rejectedInputs[] = {
            {invalid, "an ignored field in authored XML is rejected"},
            {missing, "a missing required HitPoints field is rejected despite its constructor default"},
            {nullObjective, "AllowNull(false) rejects an explicitly null objective even though its C++ carrier is nullable"}};
        for (const auto& [input, expectation] : rejectedInputs)
        {
            bool rejected = false;
            try
            {
                std::unique_ptr<System::Xml::XmlReader> badReader(System::Xml::XmlReader::CreateFromString(input));
                (void)IntermediateSerializer::Deserialize<Encounter>(*badReader, "");
            }
            catch (const InvalidContentException& error)
            {
                rejected = true;
                std::cout << "InvalidContentException: " << error.what() << '\n';
            }
            check(rejected, expectation);
        }
        authored->objective.reset();
        bool rejectedNullWrite = false;
        try
        {
            std::unique_ptr<System::Xml::XmlWriter> invalidWriter(System::Xml::XmlWriter::CreateToString(settings));
            IntermediateSerializer::Serialize<Encounter>(*invalidWriter, authored, "");
        }
        catch (const InvalidContentException& error)
        {
            rejectedNullWrite = true;
            std::cout << "Invalid objective on write: " << error.what() << '\n';
        }
        check(rejectedNullWrite, "AllowNull(false) also rejects writing an encounter without its objective");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Intermediate serialization failed: " << error.what() << '\n';
        return 1;
    }
}
