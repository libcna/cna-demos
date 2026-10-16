// SPDX-License-Identifier: MS-PL
// A described C++ configuration can keep validated properties and constructor-owned collections.
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "System/Object.hpp"
#include "System/Xml/XmlReader.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/XmlWriterSettings.hpp"

using Microsoft::Xna::Framework::Content::Pipeline::InvalidContentException;
using namespace Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate;

namespace
{
    struct NamedConfiguration : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.NamedConfiguration";
        const std::string& GetTypeName() const override
        {
            static const std::string typeName(XnaTypeName);
            return typeName;
        }
        std::string name = "Untitled";
        static void DescribeContent(ContentTypeDescriptor<NamedConfiguration>& type)
        {
            type.Field("Name", &NamedConfiguration::name);
        }
    };

    struct SpawnPoint
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.SpawnPoint";
        int x = 0;
        int y = 0;
        static void DescribeContent(ContentTypeDescriptor<SpawnPoint>& type)
        {
            type.Field("X", &SpawnPoint::x);
            type.Field("Y", &SpawnPoint::y);
        }
    };

    class DungeonConfiguration final : public NamedConfiguration
    {
    public:
        static constexpr std::string_view XnaTypeName = "CnaDemos.DungeonConfiguration";
        const std::string& GetTypeName() const override
        {
            static const std::string typeName(XnaTypeName);
            return typeName;
        }
        int getDifficulty() const { return difficulty_; }
        void setDifficulty(int value)
        {
            if (value < 1 || value > 5) throw InvalidContentException("Dungeon difficulty must be between 1 and 5.");
            difficulty_ = value;
            ++difficultyAssignments_;
        }
        int getDifficultyAssignments() const { return difficultyAssignments_; }
        std::vector<std::string>& getRooms() { return rooms_; } // no collection setter
        SpawnPoint spawn;
        std::vector<int> route;
        int reward = 0;

        static void DescribeContent(ContentTypeDescriptor<DungeonConfiguration>& type)
        {
            type.Property("Difficulty", &DungeonConfiguration::getDifficulty, &DungeonConfiguration::setDifficulty);
            type.ReadOnlyProperty("Rooms", &DungeonConfiguration::getRooms).CollectionItemName("Room");
            type.Field("Spawn", &DungeonConfiguration::spawn).FlattenContent();
            type.Field("Route", &DungeonConfiguration::route).FlattenContent().CollectionItemName("Step");
            type.Field("Reward", &DungeonConfiguration::reward);
            // BaseType places inherited members first even when declared after these bindings.
            type.BaseType<NamedConfiguration>();
        }

    private:
        int difficulty_ = 1;
        int difficultyAssignments_ = 0; // runtime observation, not described as content
        std::vector<std::string> rooms_{"Entrance"};
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
        auto authored = std::make_shared<DungeonConfiguration>();
        authored->name = "Vault";
        authored->setDifficulty(3);
        authored->getRooms() = {"Shop", "Arena"};
        authored->spawn = {12, -4};
        authored->route = {4, 8};
        authored->reward = 25;
        System::Xml::XmlWriterSettings settings;
        settings.Indent = true;
        settings.NewLineChars = "\n";
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<DungeonConfiguration>(*writer, authored, "");
        const auto xml = writer->ToString();
        std::cout << xml << '\n';
        const auto nameAt = xml.find("<Name>Vault</Name>");
        const auto difficultyAt = xml.find("<Difficulty>3</Difficulty>");
        const auto roomsAt = xml.find("<Rooms>");
        check(nameAt != std::string::npos && difficultyAt != std::string::npos && roomsAt != std::string::npos &&
              nameAt < difficultyAt && difficultyAt < roomsAt,
              "the inherited name is emitted before the derived property and collection");

        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        const auto loaded = IntermediateSerializer::Deserialize<DungeonConfiguration>(*reader, "");
        check(loaded && loaded->name == "Vault" && loaded->getDifficulty() == 3 && loaded->getDifficultyAssignments() == 1,
              "the inherited field is restored and the property is assigned through its setter exactly once");
        if (!loaded) return 1;
        check(loaded->getRooms() == std::vector<std::string>{"Entrance", "Shop", "Arena"} &&
              authored->getRooms() == std::vector<std::string>{"Shop", "Arena"},
              "the get-only collection appends XML rooms to its constructor default, without changing the source");
        check(xml.find("<Spawn") == std::string::npos && xml.find("<X>12</X>") != std::string::npos &&
              xml.find("<Y>-4</Y>") != std::string::npos && loaded->spawn.x == 12 && loaded->spawn.y == -4,
              "flattened spawn coordinates keep their values while omitting the Spawn wrapper");
        check(xml.find("</Y>4 8<Reward>25</Reward>") != std::string::npos &&
              xml.find("<Route") == std::string::npos && xml.find("<Step") == std::string::npos,
              "a flattened packed collection writes bare sibling text and ignores CollectionItemName");
        check(loaded->route == authored->route && loaded->reward == 25,
              "flattened packed text round-trips before the following named member");

        auto emptyRoute = std::make_shared<DungeonConfiguration>();
        emptyRoute->name = "Empty";
        emptyRoute->setDifficulty(2);
        emptyRoute->getRooms().clear();
        emptyRoute->spawn = {1, 2};
        emptyRoute->reward = 5;
        std::unique_ptr<System::Xml::XmlWriter> emptyWriter(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<DungeonConfiguration>(*emptyWriter, emptyRoute, "");
        const auto emptyXml = emptyWriter->ToString();
        bool emptyFlattenedRejected = false;
        try
        {
            std::unique_ptr<System::Xml::XmlReader> emptyReader(System::Xml::XmlReader::CreateFromString(emptyXml));
            (void)IntermediateSerializer::Deserialize<DungeonConfiguration>(*emptyReader, "");
        }
        catch (const InvalidContentException& error)
        {
            emptyFlattenedRejected = std::string(error.what()).find("ReadContentAsString") != std::string::npos;
            std::cout << "Rejected empty flattened route: " << error.what() << '\n';
        }
        check(emptyXml.find("<Route") == std::string::npos && emptyXml.find("<Step") == std::string::npos &&
              emptyXml.find("<Reward>5</Reward>") != std::string::npos && emptyFlattenedRejected,
              "an empty flattened packed collection has no XML spelling and blocks the following member on read");

        std::string invalid = xml;
        if (difficultyAt == std::string::npos) throw std::runtime_error("Expected authored difficulty missing");
        invalid.replace(difficultyAt, std::string_view("<Difficulty>3</Difficulty>").size(), "<Difficulty>0</Difficulty>");
        bool rejected = false;
        try
        {
            std::unique_ptr<System::Xml::XmlReader> badReader(System::Xml::XmlReader::CreateFromString(invalid));
            (void)IntermediateSerializer::Deserialize<DungeonConfiguration>(*badReader, "");
        }
        catch (const InvalidContentException& error)
        {
            rejected = true;
            std::cout << "Rejected difficulty: " << error.what() << '\n';
        }
        check(rejected, "deserialization honors the game's property validation instead of bypassing its setter");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Property serialization failed: " << error.what() << '\n';
        return 1;
    }
}
