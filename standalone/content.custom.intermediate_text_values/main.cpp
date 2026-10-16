// SPDX-License-Identifier: MS-PL
// XNA intermediate XML preserves string text while chars must contain exactly one UTF-16 code unit.
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "System/ArgumentException.hpp"
#include "System/Object.hpp"
#include "System/Xml/XmlReader.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/XmlWriterSettings.hpp"

using Microsoft::Xna::Framework::Content::Pipeline::InvalidContentException;
using namespace Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate;

namespace
{
    struct TextResource final : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.TextResource";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName);
            return name;
        }

        std::string empty;
        std::optional<std::string> missing;
        std::string padded = "  padded  ";
        std::string lines = "line1\nline2\nline3";
        std::string escaped = "<tag> & \"quotes\" 'apostrophes'";
        std::string unicode = "café 中文 😀";
        char16_t tab = u'\t';
        char16_t lessThan = u'<';
        char16_t ampersand = u'&';
        char16_t space = u' ';

        static void DescribeContent(ContentTypeDescriptor<TextResource>& type)
        {
            type.Field("Empty", &TextResource::empty);
            type.Field("Missing", &TextResource::missing);
            type.Field("Padded", &TextResource::padded);
            type.Field("Lines", &TextResource::lines);
            type.Field("Escaped", &TextResource::escaped);
            type.Field("Unicode", &TextResource::unicode);
            type.Field("Tab", &TextResource::tab);
            type.Field("LessThan", &TextResource::lessThan);
            type.Field("Ampersand", &TextResource::ampersand);
            type.Field("Space", &TextResource::space);
        }
    };

    std::string Serialize(const std::shared_ptr<TextResource>& value)
    {
        System::Xml::XmlWriterSettings settings;
        settings.Indent = false;
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<TextResource>(*writer, value, "");
        return writer->ToString();
    }

    std::shared_ptr<TextResource> Deserialize(const std::string& xml)
    {
        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        return IntermediateSerializer::Deserialize<TextResource>(*reader, "");
    }

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

        const auto authored = std::make_shared<TextResource>();
        const std::string xml = Serialize(authored);
        std::cout << xml << '\n';

        const std::string padded = "<Padded>  padded  </Padded>";
        const std::string lessThan = "<LessThan>&lt;</LessThan>";
        check(xml.find("<Empty></Empty><Missing Null=\"true\" />") != std::string::npos,
              "empty and null strings remain distinct XML forms");
        check(xml.find(padded) != std::string::npos &&
                  xml.find("<Lines>line1\nline2\nline3</Lines>") != std::string::npos,
              "leading/trailing spaces and embedded line feeds are preserved");
        check(xml.find("<Escaped>&lt;tag&gt; &amp; \"quotes\" 'apostrophes'</Escaped>") !=
                  std::string::npos,
              "XML markup characters are escaped without changing quotes or apostrophes in text");
        check(xml.find("<Unicode>café 中文 😀</Unicode>") != std::string::npos,
              "UTF-8 text preserves BMP and supplementary Unicode characters");
        check(xml.find("<Tab>\t</Tab>") != std::string::npos &&
                  xml.find(lessThan) != std::string::npos &&
                  xml.find("<Ampersand>&amp;</Ampersand><Space> </Space>") != std::string::npos,
              "tab, space and XML-significant characters retain one-character values");

        const auto restored = Deserialize(xml);
        check(restored && restored->empty.empty() && !restored->missing &&
                  restored->padded == authored->padded && restored->lines == authored->lines &&
                  restored->escaped == authored->escaped && restored->unicode == authored->unicode &&
                  restored->tab == u'\t' && restored->lessThan == u'<' &&
                  restored->ampersand == u'&' && restored->space == u' ',
              "all reference strings and character code units round-trip exactly");

        const std::string alternate = Replaced(
            Replaced(xml, padded, "<Padded><![CDATA[n<>]]></Padded>"),
            lessThan, "<LessThan>&#x3C;</LessThan>");
        const auto alternateValue = Deserialize(alternate);
        const std::string normalized = Serialize(alternateValue);
        check(alternateValue && alternateValue->padded == "n<>" && alternateValue->lessThan == u'<' &&
                  normalized.find("<Padded>n&lt;&gt;</Padded>") != std::string::npos &&
                  normalized.find(lessThan) != std::string::npos,
              "CDATA and numeric entities normalize to ordinary escaped text");

        const auto rejects = [&](const std::string& malformed, std::string_view expectation) {
            bool rejected = false;
            try
            {
                (void)Deserialize(malformed);
            }
            catch (const InvalidContentException& error)
            {
                rejected = true;
                std::cout << "Rejected text XML: " << error.what() << '\n';
            }
            check(rejected, expectation);
        };

        rejects(Replaced(xml, "<Tab>\t</Tab>", "<Tab></Tab>"),
                "an empty character element is rejected");
        rejects(Replaced(xml, "<Tab>\t</Tab>", "<Tab>ab</Tab>"),
                "a two-character element is rejected");
        rejects(Replaced(xml, "<Tab>\t</Tab>", "<Tab>😀</Tab>"),
                "a supplementary character is rejected because it needs two UTF-16 code units");

        bool rejectedNul = false;
        bool matchedXnaNulException = false;
        try
        {
            System::Xml::XmlWriterSettings settings;
            settings.Indent = false;
            std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
            IntermediateSerializer::Serialize<char16_t>(*writer, u'\0', "");
        }
        catch (const System::ArgumentException& error)
        {
            rejectedNul = true;
            matchedXnaNulException = true;
            std::cout << "Rejected NUL output: " << error.what() << '\n';
        }
        catch (const System::Xml::XmlException& error)
        {
            rejectedNul = true;
            std::cout << "Rejected NUL output with XmlException: " << error.what() << '\n';
        }
        check(rejectedNul, "the XML writer rejects the U+0000 character");
        check(matchedXnaNulException,
              "ISSUE-147: NUL output preserves XNA's ArgumentException contract");

        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Intermediate text demonstration failed: " << error.what() << '\n';
        return 1;
    }
}
