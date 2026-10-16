// SPDX-License-Identifier: MS-PL
// One authored spawn list keeps the data and dynamic type of each concrete spawn definition.
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "System/Object.hpp"
#include "System/Xml/XmlReader.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/XmlWriterSettings.hpp"

using namespace Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate;
using Microsoft::Xna::Framework::Content::Pipeline::InvalidContentException;

namespace
{
    struct Spawn : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.Spawn";
        const std::string& GetTypeName() const override = 0; // a generic Spawn cannot be instantiated
        std::string name;
        static void DescribeContent(ContentTypeDescriptor<Spawn>& type) { type.Field("Name", &Spawn::name); }
    };
    struct EnemySpawn final : Spawn
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.EnemySpawn";
        const std::string& GetTypeName() const override
        {
            static const std::string typeName(XnaTypeName); return typeName;
        }
        int health = 0;
        static void DescribeContent(ContentTypeDescriptor<EnemySpawn>& type)
        {
            type.BaseType<Spawn>();
            type.Field("Health", &EnemySpawn::health);
        }
    };
    struct TreasureSpawn final : Spawn
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.TreasureSpawn";
        const std::string& GetTypeName() const override
        {
            static const std::string typeName(XnaTypeName); return typeName;
        }
        int coins = 0;
        static void DescribeContent(ContentTypeDescriptor<TreasureSpawn>& type)
        {
            type.BaseType<Spawn>();
            type.Field("Coins", &TreasureSpawn::coins);
        }
    };
    struct SpawnList : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.SpawnList";
        const std::string& GetTypeName() const override
        {
            static const std::string typeName(XnaTypeName); return typeName;
        }
        std::vector<std::shared_ptr<Spawn>> spawns;
        static void DescribeContent(ContentTypeDescriptor<SpawnList>& type)
        {
            type.Field("Spawns", &SpawnList::spawns).CollectionItemName("Spawn");
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
        // A base-typed member does not enumerate every derived game type automatically.
        // Register the concrete serializers before resolving dynamic XML Type declarations.
        (void)IntermediateSerializer::TypeSerializerFor<EnemySpawn>();
        (void)IntermediateSerializer::TypeSerializerFor<TreasureSpawn>();
        auto enemy = std::make_shared<EnemySpawn>(); enemy->name = "Gatekeeper"; enemy->health = 90;
        auto treasure = std::make_shared<TreasureSpawn>(); treasure->name = "Vault"; treasure->coins = 35;
        auto authored = std::make_shared<SpawnList>(); authored->spawns = {enemy, treasure, nullptr};
        System::Xml::XmlWriterSettings settings; settings.Indent = true; settings.NewLineChars = "\n";
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<SpawnList>(*writer, authored, "");
        const auto xml = writer->ToString(); std::cout << xml << '\n';

        // Inspect the actual public XML attributes without assuming a generated namespace prefix.
        std::vector<std::string> declaredTypes;
        bool nullElement = false;
        std::unique_ptr<System::Xml::XmlReader> inspection(System::Xml::XmlReader::CreateFromString(xml));
        while (inspection->Read())
        {
            if (inspection->getNodeTypeProperty() == System::Xml::XmlNodeType::Element &&
                inspection->getLocalNameProperty() == "Spawn")
            {
                const auto type = inspection->GetAttribute("Type");
                if (!type.empty()) declaredTypes.push_back(type);
                nullElement |= inspection->GetAttribute("Null") == "true";
            }
        }
        const bool concreteTypes = declaredTypes.size() == 2 && declaredTypes[0].ends_with("EnemySpawn") &&
                                   declaredTypes[1].ends_with("TreasureSpawn");
        check(concreteTypes, "base-typed list elements emit their concrete enemy and treasure Type declarations");
        check(nullElement, "a null spawn is represented explicitly rather than constructing the abstract base");
        if (!concreteTypes) return 1; // keep malformed-input probes tied to the XML actually emitted
        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        const auto loaded = IntermediateSerializer::Deserialize<SpawnList>(*reader, "");
        check(loaded && loaded->spawns.size() == 3 && !loaded->spawns[2], "the mixed list preserves order, length and its null entry");
        if (!loaded || loaded->spawns.size() != 3) return 1;
        const auto loadedEnemy = std::dynamic_pointer_cast<EnemySpawn>(loaded->spawns[0]);
        const auto loadedTreasure = std::dynamic_pointer_cast<TreasureSpawn>(loaded->spawns[1]);
        check(loadedEnemy && loadedEnemy->name == "Gatekeeper" && loadedEnemy->health == 90 && loadedEnemy != enemy,
              "enemy loading preserves inherited and derived data in a fresh concrete EnemySpawn");
        check(loadedTreasure && loadedTreasure->name == "Vault" && loadedTreasure->coins == 35 && loadedTreasure != treasure,
              "treasure loading preserves its own derived schema through the same abstract base");

        const std::string originalType = "Type=\"" + declaredTypes[0] + "\"";
        const auto at = xml.find(originalType);
        if (at == std::string::npos) throw std::runtime_error("Emitted concrete type not found");
        const std::string prefix = declaredTypes[0].substr(0, declaredTypes[0].size() - std::string_view("EnemySpawn").size());
        struct InvalidType { std::string type, expectation, diagnostic; };
        const InvalidType badTypes[] = {
            {prefix + "MissingSpawn", "an unregistered concrete XML type is refused", "MissingSpawn"},
            {"string", "a known XML type unrelated to the declared Spawn base is refused", "subclass"},
            {prefix + "Spawn", "the abstract Spawn base cannot be instantiated as a concrete list element", "abstract"}};
        for (const auto& [type, expectation, diagnostic] : badTypes)
        {
            std::string changed = xml; changed.replace(at, originalType.size(), "Type=\"" + type + "\"");
            bool rejected = false;
            try
            {
                std::unique_ptr<System::Xml::XmlReader> badReader(System::Xml::XmlReader::CreateFromString(changed));
                (void)IntermediateSerializer::Deserialize<SpawnList>(*badReader, "");
            }
            catch (const InvalidContentException& error)
            {
                // Check the intended type-resolution gate, not an incidental payload parse error.
                rejected = std::string(error.what()).find(diagnostic) != std::string::npos;
                std::cout << "Rejected type: " << error.what() << '\n';
            }
            check(rejected, expectation);
        }

        const auto rejectsChangedType = [&](std::string changed, std::string_view expectation,
                                            std::string_view diagnostic) {
            bool rejected = false;
            try
            {
                std::unique_ptr<System::Xml::XmlReader> badReader(System::Xml::XmlReader::CreateFromString(changed));
                (void)IntermediateSerializer::Deserialize<SpawnList>(*badReader, "");
            }
            catch (const InvalidContentException& error)
            {
                rejected = std::string(error.what()).find(diagnostic) != std::string::npos;
                std::cout << "Rejected type form: " << error.what() << '\n';
            }
            check(rejected, expectation);
        };

        std::string missingType = xml;
        missingType.erase(at, originalType.size());
        rejectsChangedType(std::move(missingType),
                           "an abstract declared member without Type cannot choose a concrete instance", "abstract");

        std::string undeclaredAlias = xml;
        undeclaredAlias.replace(at, originalType.size(), "Type=\"Missing:EnemySpawn\"");
        rejectsChangedType(std::move(undeclaredAlias),
                           "a Type prefix must be declared in XML namespace scope", "EnemySpawn");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Polymorphic spawn serialization failed: " << error.what() << '\n';
        return 1;
    }
}
