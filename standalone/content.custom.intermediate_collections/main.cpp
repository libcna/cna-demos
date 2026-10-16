// SPDX-License-Identifier: MS-PL
// XNA intermediate XML selects packed or element-per-item collection shapes from the element type.
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/ContentObject.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Collections/Generic/OrderedDictionary.hpp"
#include "System/Object.hpp"
#include "System/Xml/XmlReader.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/XmlWriterSettings.hpp"

using Microsoft::Xna::Framework::Vector3;
using Microsoft::Xna::Framework::Content::Pipeline::InvalidContentException;
using Microsoft::Xna::Framework::Content::Pipeline::Box;
using Microsoft::Xna::Framework::Content::Pipeline::ContentObject;
using Microsoft::Xna::Framework::Content::Pipeline::Holds;
using Microsoft::Xna::Framework::Content::Pipeline::Unbox;
using namespace Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate;

namespace
{
    using Weights = System::Collections::Generic::OrderedDictionary<std::string, std::int32_t>;

    struct RouteManifest final : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.RouteManifest";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName);
            return name;
        }

        std::vector<std::int32_t> checkpoints;
        std::vector<Vector3> offsets;
        std::vector<std::string> labels;
        std::vector<std::optional<std::int32_t>> optionalStops;
        std::vector<ContentObject> payloads;
        Weights weights;
        std::vector<std::vector<std::int32_t>> lanes;
        std::vector<std::int32_t> empty;

        static void DescribeContent(ContentTypeDescriptor<RouteManifest>& type)
        {
            type.Field("Checkpoints", &RouteManifest::checkpoints);
            type.Field("Offsets", &RouteManifest::offsets);
            type.Field("Labels", &RouteManifest::labels);
            type.Field("OptionalStops", &RouteManifest::optionalStops);
            type.Field("Payloads", &RouteManifest::payloads);
            type.Field("Weights", &RouteManifest::weights);
            type.Field("Lanes", &RouteManifest::lanes);
            type.Field("Empty", &RouteManifest::empty);
        }
    };

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

        auto authored = std::make_shared<RouteManifest>();
        authored->checkpoints = {1, 2, 3};
        authored->offsets = {Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 5.0f, 6.0f)};
        authored->labels = {"north gate", " south "};
        authored->optionalStops = {7, std::nullopt, 9};
        authored->payloads = {Box<std::int32_t>(7), Box<std::string>("seal")};
        authored->weights.Add("beta", 2);
        authored->weights.Add("alpha", 1);
        authored->lanes = {{1, 2}, {3}};

        System::Xml::XmlWriterSettings settings;
        settings.Indent = false;
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<RouteManifest>(*writer, authored, "");
        const std::string xml = writer->ToString();
        std::cout << xml << '\n';

        const std::string packedScalars = "<Checkpoints>1 2 3</Checkpoints>";
        const std::string packedVectors = "<Offsets>1 2 3 4 5 6</Offsets>";
        const std::string itemStrings =
            "<Labels><Item>north gate</Item><Item> south </Item></Labels>";
        const std::string nullableItems =
            "<OptionalStops><Item>7</Item><Item Null=\"true\" /><Item>9</Item></OptionalStops>";
        const std::string dynamicItems =
            "<Payloads><Item Type=\"int\">7</Item><Item Type=\"string\">seal</Item></Payloads>";
        const std::string dictionary =
            "<Weights><Item><Key>beta</Key><Value>2</Value></Item>"
            "<Item><Key>alpha</Key><Value>1</Value></Item></Weights>";
        const std::string jagged = "<Lanes><Item>1 2</Item><Item>3</Item></Lanes>";

        check(xml.find(packedScalars) != std::string::npos && xml.find(packedVectors) != std::string::npos,
              "scalar and fixed-width vector collections use packed text");
        check(xml.find(itemStrings) != std::string::npos,
              "strings with spaces remain distinct element-per-item values");
        check(xml.find(nullableItems) != std::string::npos && xml.find(dynamicItems) != std::string::npos,
              "nullable and object items carry per-item Null and dynamic Type attributes");
        check(xml.find(dictionary) != std::string::npos,
              "dictionary items use Key before Value and preserve insertion order");
        check(xml.find(jagged) != std::string::npos && xml.find("<Empty />") != std::string::npos,
              "jagged inner lists use packed Item text and an empty packed list is self-closing");

        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        const auto restored = IntermediateSerializer::Deserialize<RouteManifest>(*reader, "");
        check(restored && restored->checkpoints == authored->checkpoints && restored->offsets == authored->offsets &&
                  restored->labels == authored->labels && restored->optionalStops == authored->optionalStops &&
                  restored->lanes == authored->lanes && restored->empty.empty(),
              "all collection shapes round-trip their values");
        check(restored && restored->payloads.size() == 2 && Holds<std::int32_t>(restored->payloads[0]) &&
                  Unbox<std::int32_t>(restored->payloads[0]) == 7 && Holds<std::string>(restored->payloads[1]) &&
                  Unbox<std::string>(restored->payloads[1]) == "seal",
              "dynamic object items round-trip their independent runtime types");
        check(restored && restored->weights.getCountProperty() == 2 &&
                  restored->weights.GetAt(0).Key == "beta" && restored->weights.GetAt(0).Value == 2 &&
                  restored->weights.GetAt(1).Key == "alpha" && restored->weights.GetAt(1).Value == 1,
              "dictionary round trip retains document insertion order");

        const auto rejects = [&](const std::string& malformed, std::string_view expectation) {
            bool rejected = false;
            try
            {
                std::unique_ptr<System::Xml::XmlReader> bad(System::Xml::XmlReader::CreateFromString(malformed));
                (void)IntermediateSerializer::Deserialize<RouteManifest>(*bad, "");
            }
            catch (const InvalidContentException& error)
            {
                rejected = true;
                std::cout << "Rejected collection XML: " << error.what() << '\n';
            }
            check(rejected, expectation);
        };

        rejects(Replaced(xml, packedScalars, "<Checkpoints><Item>1</Item></Checkpoints>"),
                "a packed scalar list rejects child Item elements");
        rejects(Replaced(xml, itemStrings, "<Labels>north gate south</Labels>"),
                "an element-per-item string list rejects packed text");
        rejects(Replaced(xml, "<Item Type=\"int\">7</Item>", "<Item>7</Item>"),
                "an object item rejects a missing dynamic Type");
        rejects(Replaced(xml, "<Item><Key>beta</Key><Value>2</Value></Item>",
                         "<Item><Value>2</Value><Key>beta</Key></Item>"),
                "a dictionary rejects Value before Key");
        rejects(Replaced(xml, "<Key>alpha</Key>", "<Key>beta</Key>"),
                "a dictionary rejects a duplicate key");
        rejects(Replaced(xml, packedVectors, "<Offsets>1 2 3 4</Offsets>"),
                "a packed Vector3 list rejects a partial final value");

        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Intermediate collection demonstration failed: " << error.what() << '\n';
        return 1;
    }
}
