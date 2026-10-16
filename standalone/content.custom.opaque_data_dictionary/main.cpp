// SPDX-License-Identifier: MS-PL
// A processor-style parameter sheet retains heterogeneous values in XNA intermediate XML.
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/OpaqueDataDictionary.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/InvalidCastException.hpp"
#include "System/Xml/XmlReader.hpp"

using Microsoft::Xna::Framework::Vector3;
using Microsoft::Xna::Framework::Content::Pipeline::Box;
using Microsoft::Xna::Framework::Content::Pipeline::ContentObject;
using Microsoft::Xna::Framework::Content::Pipeline::InvalidContentException;
using Microsoft::Xna::Framework::Content::Pipeline::OpaqueDataDictionary;
using Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate::IntermediateSerializer;

namespace
{
    constexpr std::string_view XnaDocument =
        "<?xml version=\"1.0\" encoding=\"utf-16\"?><XnaContent "
        "xmlns:Pipeline=\"Microsoft.Xna.Framework.Content.Pipeline\" "
        "xmlns:Framework=\"Microsoft.Xna.Framework\"><Asset Type=\"Pipeline:OpaqueDataDictionary\">"
        "<Data Key=\"Count\" Type=\"int\">3</Data><Data Key=\"Name\">wall</Data>"
        "<Data Key=\"Scale\" Type=\"Framework:Vector3\">1 2 3</Data></Asset></XnaContent>";

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

        OpaqueDataDictionary parameters;
        check(parameters.GetContentAsXml().empty(), "an empty parameter dictionary has no XML document");

        parameters.SetValue<std::int32_t>("Count", 3);
        parameters.SetValue<std::string>("Name", "wall");
        parameters.SetValue<Vector3>("Scale", Vector3(1.0f, 2.0f, 3.0f));
        const std::vector<std::string> expectedKeys{"Count", "Name", "Scale"};
        check(parameters.getCountProperty() == 3 && parameters.getKeysProperty() == expectedKeys,
              "heterogeneous entries retain insertion order");
        check(parameters.GetValue<std::int32_t>("Count", -1) == 3 &&
                  parameters.GetValue<std::string>("Name", "missing") == "wall" &&
                  parameters.GetValue<Vector3>("Scale", Vector3()) == Vector3(1.0f, 2.0f, 3.0f),
              "typed lookup unboxes each processor parameter");
        check(parameters.GetValue<std::int32_t>("Missing", 17) == 17,
              "a missing parameter returns the caller's typed default");

        bool wrongTypeRejected = false;
        try
        {
            (void)parameters.GetValue<std::int32_t>("Name", 0);
        }
        catch (const System::InvalidCastException& error)
        {
            wrongTypeRejected = true;
            std::cout << "Rejected wrong value type: " << error.getMessageProperty() << '\n';
        }
        check(wrongTypeRejected, "requesting a stored value as another type throws InvalidCastException");

        const std::string xml = parameters.GetContentAsXml();
        std::cout << xml << '\n';
        check(xml == XnaDocument,
              "compact XML exactly matches XNA's utf-16 declaration, dynamic types and entry order");

        parameters.SetValue<std::string>("Name", "stone");
        check(parameters.getKeysProperty() == expectedKeys &&
                  parameters.GetValue<std::string>("Name", "missing") == "stone",
              "replacing a value preserves its original position");
        parameters.SetValue<std::string>("Name", "wall");

        const auto rejectsXml = [&](const std::string& input, std::string_view expectation) {
            bool rejected = false;
            try
            {
                std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(input));
                (void)IntermediateSerializer::Deserialize<OpaqueDataDictionary>(*reader, "");
            }
            catch (const InvalidContentException& error)
            {
                rejected = true;
                std::cout << "Rejected XML: " << error.what() << '\n';
            }
            check(rejected, expectation);
        };

        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        const OpaqueDataDictionary restored =
            IntermediateSerializer::Deserialize<OpaqueDataDictionary>(*reader, "");
        check(restored.getKeysProperty() == expectedKeys &&
                  restored.GetValue<std::int32_t>("Count", -1) == 3 &&
                  restored.GetValue<std::string>("Name", "missing") == "wall" &&
                  restored.GetValue<Vector3>("Scale", Vector3()) == Vector3(1.0f, 2.0f, 3.0f) &&
                  restored.GetContentAsXml() == XnaDocument,
              "deserialization retains entry order, dynamic types and the exact document");

        rejectsXml(Replaced(xml, " Key=\"Name\"", ""),
                   "deserialization rejects an entry without its Key attribute");
        rejectsXml(Replaced(xml, "Type=\"int\"", "Type=\"No.Such.Type\""),
                   "deserialization rejects an unknown dynamic Type");
        rejectsXml(Replaced(xml, "Key=\"Name\"", "Key=\"Count\""),
                   "deserialization rejects a duplicate key");

        const int originalCount = parameters.getCountProperty();
        bool emptyKeyRejected = false;
        try
        {
            parameters.SetValue<std::string>("", "unused");
        }
        catch (const System::ArgumentException&)
        {
            emptyKeyRejected = true;
        }
        check(emptyKeyRejected && parameters.getCountProperty() == originalCount,
              "an empty key is rejected without changing the dictionary");

        bool duplicateRejected = false;
        try
        {
            parameters.Add("Count", Box<std::int32_t>(4));
        }
        catch (const System::ArgumentException&)
        {
            duplicateRejected = true;
        }
        check(duplicateRejected && parameters.GetValue<std::int32_t>("Count", -1) == 3 &&
                  parameters.getCountProperty() == originalCount,
              "Add rejects a duplicate key and preserves the original entry");

        bool nullRejectedPrecisely = false;
        try
        {
            parameters.Add("Empty", ContentObject{});
        }
        catch (const System::ArgumentNullException& error)
        {
            nullRejectedPrecisely = true;
            std::cout << "Rejected null value: " << error.getMessageProperty() << '\n';
        }
        catch (const System::ArgumentException& error)
        {
            std::cout << "Observed broader null-value exception: " << error.getMessageProperty() << '\n';
        }
        check(nullRejectedPrecisely && parameters.getCountProperty() == originalCount,
              "a null value throws XNA's ArgumentNullException and leaves the dictionary unchanged");

        check(parameters.GetContentAsXml() == XnaDocument,
              "all refused operations leave the original parameter document unchanged");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Opaque-data dictionary demonstration failed: " << error.what() << '\n';
        return 1;
    }
}
