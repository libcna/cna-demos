// SPDX-License-Identifier: MS-PL
// Equal authored loot tables need not be the same shared resource.
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <string>
#include <string_view>
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
    struct LootTable : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.LootTable";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName);
            return name;
        }
        std::vector<std::string> items;
        std::shared_ptr<LootTable> fallback;
        static void DescribeContent(ContentTypeDescriptor<LootTable>& type)
        {
            type.Field("Items", &LootTable::items).CollectionItemName("Drop");
            type.Field("Fallback", &LootTable::fallback).SharedResource();
        }
    };

    // XML reference fixups do not collect C++ shared_ptr cycles. Keep a scoped owner of both
    // participating nodes so even an early return or exception breaks their owning back edges.
    struct UnlinkFallbacks
    {
        std::shared_ptr<LootTable> first, second;
        ~UnlinkFallbacks()
        {
            if (first) first->fallback.reset();
            if (second) second->fallback.reset();
        }
    };

    struct LevelLoot : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.LevelLoot";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName);
            return name;
        }
        std::shared_ptr<LootTable> guard;
        std::shared_ptr<LootTable> patrol;
        std::shared_ptr<LootTable> boss;
        std::shared_ptr<LootTable> inlineFirst;
        std::shared_ptr<LootTable> inlineSecond;
        static void DescribeContent(ContentTypeDescriptor<LevelLoot>& type)
        {
            type.Field("Guard", &LevelLoot::guard).SharedResource();
            type.Field("Patrol", &LevelLoot::patrol).SharedResource();
            type.Field("Boss", &LevelLoot::boss).SharedResource();
            // The same C++ reference without SharedResource is serialized inline, twice.
            type.Field("InlineFirst", &LevelLoot::inlineFirst);
            type.Field("InlineSecond", &LevelLoot::inlineSecond);
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
        auto common = std::make_shared<LootTable>();
        common->items = {"Coin", "Key"};
        auto authored = std::make_shared<LevelLoot>();
        authored->guard = authored->patrol = common;
        authored->boss = std::make_shared<LootTable>();
        authored->boss->items = common->items; // equal value, independently owned object
        authored->inlineFirst = authored->inlineSecond = common;

        System::Xml::XmlWriterSettings settings;
        settings.Indent = true;
        settings.NewLineChars = "\n";
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<LevelLoot>(*writer, authored, "");
        const auto xml = writer->ToString();
        std::cout << xml << '\n';
        check(xml.find("<Resources>") != std::string::npos,
              "shared tables are emitted into the intermediate XML Resources section");

        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        const auto loaded = IntermediateSerializer::Deserialize<LevelLoot>(*reader, "");
        const bool complete = loaded && loaded->guard && loaded->patrol && loaded->boss &&
                              loaded->inlineFirst && loaded->inlineSecond;
        check(complete, "all table references are fixed up before Deserialize returns");
        if (!complete) return 1;
        check(loaded->guard == loaded->patrol && loaded->guard != common,
              "guard and patrol share one newly loaded table, not an authored pointer");
        check(loaded->boss != loaded->guard && loaded->boss->items == loaded->guard->items,
              "equal table values do not merge two distinct shared resources");
        check(loaded->inlineFirst != loaded->inlineSecond && loaded->inlineFirst != loaded->guard &&
              loaded->inlineFirst->items == common->items && loaded->inlineSecond->items == common->items,
              "inline fields become independent objects despite sharing the authored pointer");
        loaded->guard->items.push_back("Potion");
        check(loaded->patrol->items.size() == 3 && loaded->boss->items.size() == 2 &&
              loaded->inlineFirst->items.size() == 2 && common->items.size() == 2,
              "a shared table edit reaches its alias while independent and authored tables stay unchanged");
        // A missing table is a null reference, not another shared resource with empty contents.
        authored->boss.reset();
        std::unique_ptr<System::Xml::XmlWriter> nullWriter(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<LevelLoot>(*nullWriter, authored, "");
        const auto nullXml = nullWriter->ToString();
        std::unique_ptr<System::Xml::XmlReader> nullReader(System::Xml::XmlReader::CreateFromString(nullXml));
        const auto withoutBoss = IntermediateSerializer::Deserialize<LevelLoot>(*nullReader, "");
        check(withoutBoss && !withoutBoss->boss && withoutBoss->guard &&
              withoutBoss->guard == withoutBoss->patrol && nullXml.find("#Resource2") == std::string::npos,
              "a null shared table stays null without adding a resource or breaking the other aliases");

        // Corrupt only the reference graph, keeping otherwise valid authored XML. No Internal
        // parser is needed: IntermediateSerializer must reject the bad graph before returning it.
        const auto replaceOne = [](std::string& document, std::string_view from, std::string_view to) {
            const auto at = document.find(from);
            if (at == std::string::npos) throw std::runtime_error("Expected authored XML fragment missing");
            document.replace(at, from.size(), to);
        };
        std::vector<std::pair<std::string, std::string>> invalidGraphs;
        std::string missing = xml;
        replaceOne(missing, "<Guard>#Resource1</Guard>", "<Guard>#Missing</Guard>");
        invalidGraphs.emplace_back(std::move(missing), "a reference to a missing resource is rejected");
        std::string duplicate = xml;
        replaceOne(duplicate, "ID=\"#Resource2\"", "ID=\"#Resource1\"");
        replaceOne(duplicate, "<Boss>#Resource2</Boss>", "<Boss>#Resource1</Boss>");
        invalidGraphs.emplace_back(std::move(duplicate), "duplicate resource IDs are rejected even when every reference resolves");
        std::string wrongType = xml;
        const auto resourceBegin = wrongType.find("<Resource ID=\"#Resource1\"");
        const auto resourceEnd = wrongType.find("</Resource>", resourceBegin);
        if (resourceBegin == std::string::npos || resourceEnd == std::string::npos)
            throw std::runtime_error("Expected authored resource missing");
        wrongType.replace(resourceBegin, resourceEnd + std::string_view("</Resource>").size() - resourceBegin,
                          "<Resource ID=\"#Resource1\" Type=\"string\">Coin</Resource>");
        invalidGraphs.emplace_back(std::move(wrongType), "a resolved resource must have the declared loot-table type");
        for (const auto& [invalidXml, message] : invalidGraphs)
        {
            bool rejected = false;
            try
            {
                std::unique_ptr<System::Xml::XmlReader> badReader(System::Xml::XmlReader::CreateFromString(invalidXml));
                (void)IntermediateSerializer::Deserialize<LevelLoot>(*badReader, "");
            }
            catch (const InvalidContentException& error)
            {
                rejected = true;
                std::cout << "Rejected graph: " << error.what() << '\n';
            }
            check(rejected, message);
        }
        // A cyclic graph is serialized through Resources, rather than recursive inline copies.
        // The containing LevelLoot stays outside the cycle: both edges join actual resources.
        std::weak_ptr<LootTable> authoredFirst, authoredSecond, restoredFirst, restoredSecond;
        bool restoredOwnersObserved = false;
        {
            auto first = std::make_shared<LootTable>();
            auto second = std::make_shared<LootTable>();
            UnlinkFallbacks unlinkAuthored{first, second};
            first->items = {"Coin"}; second->items = {"Key"};
            first->fallback = second; second->fallback = first;
            authoredFirst = first; authoredSecond = second;
            auto cyclic = std::make_shared<LevelLoot>();
            cyclic->guard = cyclic->patrol = first;
            cyclic->boss = second;
            std::unique_ptr<System::Xml::XmlWriter> cycleWriter(System::Xml::XmlWriter::CreateToString(settings));
            IntermediateSerializer::Serialize<LevelLoot>(*cycleWriter, cyclic, "");
            const auto cycleXml = cycleWriter->ToString();
            std::cout << "Cyclic fallback tables:\n" << cycleXml << '\n';
            int resources = 0;
            for (std::size_t at = 0; (at = cycleXml.find("<Resource ID=", at)) != std::string::npos; ++at) ++resources;
            check(resources == 2, "a two-table cycle emits each shared resource once instead of recursing inline");
            std::unique_ptr<System::Xml::XmlReader> cycleReader(System::Xml::XmlReader::CreateFromString(cycleXml));
            const auto restored = IntermediateSerializer::Deserialize<LevelLoot>(*cycleReader, "");
            UnlinkFallbacks unlinkRestored{restored ? restored->guard : nullptr, restored ? restored->boss : nullptr};
            const bool closed = restored && restored->guard && restored->boss &&
                restored->guard == restored->patrol && restored->guard->fallback == restored->boss &&
                restored->boss->fallback == restored->guard;
            check(closed, "forward and backward fixups close on the same loaded resource identities");
            if (closed)
            {
                restoredFirst = restored->guard; restoredSecond = restored->boss;
                restoredOwnersObserved = true;
                check(restored->guard != first && restored->boss != second &&
                      restored->guard->items == first->items && restored->boss->items == second->items,
                      "the cyclic graph contains fresh resources with both authored table values");
                restored->guard->items.push_back("Potion");
                check(restored->boss->fallback->items.size() == 2 && restored->patrol->items.size() == 2 &&
                      restored->boss->items.size() == 1 && first->items.size() == 1,
                      "an edit through the loaded cycle reaches aliases but preserves independent and authored tables");
            }
            else
            {
                check(false, "the cyclic graph contains fresh resources with both authored table values");
                check(false, "an edit through the loaded cycle reaches aliases but preserves independent and authored tables");
            }
        } // explicit unlinking precedes ordinary shared_ptr destruction
        check(authoredFirst.expired() && authoredSecond.expired(),
              "scoped unlinking releases both authored cycle nodes");
        check(restoredOwnersObserved && restoredFirst.expired() && restoredSecond.expired(),
              "scoped unlinking releases both loaded cycle nodes");

        // XNA does not special-case the root when a shared edge points back to it. The root is
        // written again as a resource, and the resource copy's edge closes on that resource.
        std::weak_ptr<LootTable> authoredRoot, loadedRoot, loadedResource;
        bool rootCycleObserved = false;
        {
            auto root = std::make_shared<LootTable>();
            UnlinkFallbacks unlinkAuthored{root, nullptr};
            root->items = {"Crown"};
            root->fallback = root;
            authoredRoot = root;

            std::unique_ptr<System::Xml::XmlWriter> rootWriter(
                System::Xml::XmlWriter::CreateToString(settings));
            IntermediateSerializer::Serialize<LootTable>(*rootWriter, root, "");
            const auto rootXml = rootWriter->ToString();
            std::cout << "Root self-cycle:\n" << rootXml << '\n';
            int rootResources = 0;
            for (std::size_t at = 0; (at = rootXml.find("<Resource ID=", at)) != std::string::npos; ++at)
                ++rootResources;
            const auto firstReference = rootXml.find("<Fallback>#Resource1</Fallback>");
            const auto secondReference = firstReference == std::string::npos
                ? std::string::npos
                : rootXml.find("<Fallback>#Resource1</Fallback>", firstReference + 1);
            check(rootResources == 1 && firstReference != std::string::npos && secondReference != std::string::npos,
                  "a root self-cycle writes one resource copy and both edges target it");

            std::unique_ptr<System::Xml::XmlReader> rootReader(
                System::Xml::XmlReader::CreateFromString(rootXml));
            const auto restored = IntermediateSerializer::Deserialize<LootTable>(*rootReader, "");
            UnlinkFallbacks unlinkRestored{restored, restored ? restored->fallback : nullptr};
            rootCycleObserved = restored && restored->fallback && restored != restored->fallback &&
                restored->fallback->fallback == restored->fallback && restored->items == root->items &&
                restored->fallback->items == root->items;
            check(rootCycleObserved,
                  "root and resource load as distinct copies while the self-cycle closes on the resource");
            if (restored)
            {
                loadedRoot = restored;
                loadedResource = restored->fallback;
            }
        }
        check(rootCycleObserved && authoredRoot.expired() && loadedRoot.expired() && loadedResource.expired(),
              "scoped unlinking releases the authored root and both loaded identities");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Shared-resource serialization failed: " << error.what() << '\n';
        return 1;
    }
}
