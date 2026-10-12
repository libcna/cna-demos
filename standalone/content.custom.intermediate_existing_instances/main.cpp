// SPDX-License-Identifier: MS-PL
// Populate preallocated game objects through ordinary and raw intermediate XML reads.
#include <charconv>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

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
    struct StatSlot : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.StatSlot";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName); return name;
        }
        int health = -1;
        int ownerToken = 0; // runtime-only association, deliberately absent from the XML descriptor
        static void DescribeContent(ContentTypeDescriptor<StatSlot>& type) { type.Field("Health", &StatSlot::health); }
    };
    // A field-specific hint writes the same type as compact text without replacing its global descriptor.
    class CompactStatSerializer final : public ContentTypeSerializer<StatSlot>
    {
    public:
        CompactStatSerializer() : ContentTypeSerializer<StatSlot>("compactSlot") {}
        bool getCanDeserializeIntoExistingObjectProperty() const override { return true; }
    protected:
        void Serialize(IntermediateWriter& output, const TargetCarrier& value,
                       const ContentSerializerAttribute&) override
        {
            output.getXmlProperty().WriteString(std::to_string(value->health));
        }
        TargetCarrier Deserialize(IntermediateReader& input, const ContentSerializerAttribute& format,
                                  TargetCarrier existing) override
        {
            const auto text = input.ReadText(format);
            int health = 0;
            const auto parsed = std::from_chars(text.data(), text.data() + text.size(), health);
            if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
                throw InvalidContentException("A compact stat slot requires one complete integer.");
            if (!existing) existing = std::make_shared<StatSlot>();
            existing->health = health;
            return existing;
        }
    };
    struct Slots
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.StatSlots";
        std::shared_ptr<StatSlot> raw, ordinary, nullable, compact;
        // Keep the preallocated objects visible so main can test identity even when Null replaces a slot.
        std::shared_ptr<StatSlot> rawBefore, ordinaryBefore, nullableBefore, compactBefore;
    };
    class SlotsSerializer final : public ContentTypeSerializer<Slots>
    {
    public:
        SlotsSerializer() : ContentTypeSerializer<Slots>("statSlots") {}
    protected:
        void Serialize(IntermediateWriter& output, const Slots& value, const ContentSerializerAttribute&) override
        {
            ContentSerializerAttribute raw; raw.setElementNameProperty("Raw");
            ContentSerializerAttribute ordinary; ordinary.setElementNameProperty("Ordinary"); ordinary.setAllowNullProperty(true);
            ContentSerializerAttribute nullable; nullable.setElementNameProperty("Nullable"); nullable.setAllowNullProperty(true);
            output.WriteRawObject<StatSlot>(value.raw, raw);
            output.WriteObject<StatSlot>(value.ordinary, ordinary);
            output.WriteObject<StatSlot>(value.nullable, nullable);
            ContentSerializerAttribute compact; compact.setElementNameProperty("Compact");
            output.WriteRawObject<StatSlot>(value.compact, compact, compactSerializer_);
        }
        Slots Deserialize(IntermediateReader& input, const ContentSerializerAttribute&, Slots) override
        {
            Slots result;
            result.rawBefore = std::make_shared<StatSlot>(); result.rawBefore->ownerToken = 101;
            result.ordinaryBefore = std::make_shared<StatSlot>(); result.ordinaryBefore->ownerToken = 202;
            result.nullableBefore = std::make_shared<StatSlot>(); result.nullableBefore->ownerToken = 303;
            ContentSerializerAttribute raw; raw.setElementNameProperty("Raw");
            ContentSerializerAttribute ordinary; ordinary.setElementNameProperty("Ordinary"); ordinary.setAllowNullProperty(true);
            ContentSerializerAttribute nullable; nullable.setElementNameProperty("Nullable"); nullable.setAllowNullProperty(true);
            result.raw = input.ReadRawObject<StatSlot>(raw, result.rawBefore);
            result.ordinary = input.ReadObject<StatSlot>(ordinary, result.ordinaryBefore);
            result.nullable = input.ReadObject<StatSlot>(nullable, result.nullableBefore);
            result.compactBefore = std::make_shared<StatSlot>(); result.compactBefore->ownerToken = 404;
            ContentSerializerAttribute compact; compact.setElementNameProperty("Compact");
            result.compact = input.ReadRawObject<StatSlot>(compact, compactSerializer_, result.compactBefore);
            return result;
        }
    private:
        CompactStatSerializer compactSerializer_;
    };
    Slots ReadSlots(const std::string& xml)
    {
        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        return IntermediateSerializer::Deserialize<Slots>(*reader, "");
    }
}
int main()
{
    try
    {
        bool passed = true;
        const auto check = [&](bool condition, std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = passed && condition;
        };
        (void)IntermediateSerializer::AddTypeSerializer<SlotsSerializer>();
        (void)IntermediateSerializer::TypeSerializerFor<StatSlot>();
        (void)IntermediateSerializer::TypeSerializerFor<std::string>(); // known but unrelated Type control
        Slots original;
        original.raw = std::make_shared<StatSlot>(); original.raw->health = 65; original.raw->ownerToken = 999;
        original.ordinary = std::make_shared<StatSlot>(); original.ordinary->health = 90; original.ordinary->ownerToken = 999;
        original.compact = original.raw; // another field encoding; neither inline route promises shared-resource identity
        System::Xml::XmlWriterSettings settings; settings.Indent = true; settings.NewLineChars = "\n";
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<Slots>(*writer, original, "");
        const std::string xml = writer->ToString(); std::cout << xml << '\n';
        check(xml.find("<Raw>") != std::string::npos && xml.find("<Ordinary>") != std::string::npos &&
              xml.find("<Nullable Null=\"true\"") != std::string::npos && xml.find("999") == std::string::npos,
              "raw and ordinary payloads have named wrappers while the nullable ordinary slot emits Null");
        check(xml.find("<Compact>65</Compact>") != std::string::npos && xml.find("<Health>90</Health>") != std::string::npos,
              "an explicit raw serializer hint uses compact text for one field while the ordinary descriptor remains active");
        const auto loaded = ReadSlots(xml);
        check(loaded.raw && loaded.raw == loaded.rawBefore && loaded.raw->health == 65 && loaded.raw->ownerToken == 101,
              "ReadRawObject fills the preallocated reference and preserves its nonserialized owner token");
        check(loaded.ordinary && loaded.ordinary == loaded.ordinaryBefore && loaded.ordinary->health == 90 &&
              loaded.ordinary->ownerToken == 202,
              "ReadObject also fills an existing non-null object without replacing its identity");
        check(!loaded.nullable && loaded.nullableBefore && loaded.nullableBefore->health == -1 &&
              loaded.nullableBefore->ownerToken == 303,
              "ordinary Null returns an empty reference without filling the supplied instance");
        check(loaded.raw != original.raw && loaded.ordinary != original.ordinary && original.raw->ownerToken == 999,
              "loaded slots are independent of the authoring objects and do not persist their runtime-only state");
        check(loaded.compact && loaded.compact == loaded.compactBefore && loaded.compact->health == 65 &&
              loaded.compact->ownerToken == 404,
              "the explicit raw reader hint fills its supplied instance through the field-specific compact codec");
        const auto rawAt = xml.find("<Raw>");
        if (rawAt == std::string::npos) throw std::runtime_error("Expected original raw wrapper missing.");
        auto rawAttributes = xml;
        rawAttributes.replace(rawAt, 5, "<Raw Type=\"string\" Null=\"true\">");
        const auto rawLoaded = ReadSlots(rawAttributes);
        check(rawLoaded.raw && rawLoaded.raw == rawLoaded.rawBefore && rawLoaded.raw->health == 65 &&
              rawLoaded.raw->ownerToken == 101 && rawLoaded.ordinary && rawLoaded.ordinary->health == 90,
              "the raw reader ignores Type and Null attributes and still consumes its declared payload");
        const auto ordinaryAt = xml.find("<Ordinary>");
        if (ordinaryAt == std::string::npos) throw std::runtime_error("Expected original ordinary wrapper missing.");
        auto wrongType = xml; wrongType.replace(ordinaryAt, 10, "<Ordinary Type=\"string\">");
        bool rejected = false;
        try { (void)ReadSlots(wrongType); }
        catch (const InvalidContentException& error)
        {
            rejected = std::string(error.what()).find("subclass") != std::string::npos;
            std::cout << "Ordinary type refused: " << error.what() << '\n';
        }
        check(rejected, "the ordinary reader refuses a known XML type unrelated to the declared slot");
        auto ordinaryNull = xml; ordinaryNull.replace(ordinaryAt, 10, "<Ordinary Null=\"true\">");
        const auto nullLoaded = ReadSlots(ordinaryNull);
        check(!nullLoaded.ordinary && nullLoaded.ordinaryBefore && nullLoaded.ordinaryBefore->health == -1 &&
              nullLoaded.raw && nullLoaded.raw->health == 65 && !nullLoaded.nullable,
              "ordinary Null skips its authored payload and leaves subsequent fields correctly aligned");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Existing-instance intermediate XML failed: " << error.what() << '\n';
        return 1;
    }
}
