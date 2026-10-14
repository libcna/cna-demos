// SPDX-License-Identifier: MS-PL
// A game-owned dice expression has compact syntax and explicit domain validation.
#include <charconv>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

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
        [[nodiscard]] bool ObjectIsEmpty(const DiceRoll& value) const override { return value.count == 0; }

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

    class AlternateDiceRollSerializer final : public ContentTypeSerializer<DiceRoll>
    {
    public:
        AlternateDiceRollSerializer() : ContentTypeSerializer<DiceRoll>("alternate-dice") {}
    protected:
        void Serialize(IntermediateWriter& output, const DiceRoll& value,
                       const ContentSerializerAttribute&) override
        {
            output.getXmlProperty().WriteString(std::to_string(value.count) + "x" + std::to_string(value.sides));
        }
        DiceRoll Deserialize(IntermediateReader&, const ContentSerializerAttribute&, DiceRoll) override
        {
            return {9, 9};
        }
    };

    struct CoinFlip
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.CoinFlip";
        int count = 1;
        bool operator==(const CoinFlip&) const = default;
    };

    // Deliberately collides with DiceRollSerializer's XML shortcut while owning another type.
    class CoinFlipSerializer final : public ContentTypeSerializer<CoinFlip>
    {
    public:
        CoinFlipSerializer() : ContentTypeSerializer<CoinFlip>("dice") {}
    protected:
        void Serialize(IntermediateWriter& output, const CoinFlip& value,
                       const ContentSerializerAttribute&) override
        {
            output.getXmlProperty().WriteString(std::to_string(value.count) + "c");
        }
        CoinFlip Deserialize(IntermediateReader& input, const ContentSerializerAttribute& format,
                             CoinFlip) override
        {
            const std::string text = input.ReadText(format);
            if (text.size() < 2 || text.back() != 'c')
                throw InvalidContentException("Coin flips must use the compact Nc syntax.");
            CoinFlip value;
            const auto parsed = std::from_chars(text.data(), text.data() + text.size() - 1, value.count);
            if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() - 1 || value.count < 1)
                throw InvalidContentException("Coin flips require one positive integer before c.");
            return value;
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
        bool operator==(const Attack&) const = default;
    };

    struct OptionalDice
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.OptionalDice";
        DiceRoll bonus{0, 6};
        DiceRoll attack{1, 4};
        static void DescribeContent(ContentTypeDescriptor<OptionalDice>& type)
        {
            type.Field("Bonus", &OptionalDice::bonus).Optional();
            type.Field("Attack", &OptionalDice::attack);
        }
        bool operator==(const OptionalDice&) const = default;
    };

    struct TypedEnvelope
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.TypedEnvelope";
        DiceRoll payload;
        Attack encounter;
        bool operator==(const TypedEnvelope&) const = default;
    };

    class TypedEnvelopeSerializer final : public ContentTypeSerializer<TypedEnvelope>
    {
    public:
        TypedEnvelopeSerializer() : ContentTypeSerializer<TypedEnvelope>("typedEnvelope") {}
        [[nodiscard]] int InitializeCalls() const noexcept { return initializeCalls_; }
        [[nodiscard]] int ScanCalls() const noexcept { return scanCalls_; }
        [[nodiscard]] bool ScanSawInitialization() const noexcept { return scanSawInitialization_; }

    protected:
        void Initialize(IntermediateSerializer& serializer) override
        {
            ++initializeCalls_;
            attackSerializer_ = &serializer.GetTypeSerializer(System::Type::From<Attack>());
        }

        void ScanChildren(IntermediateSerializer& serializer, const ChildCallback& callback,
                          const TypedEnvelope& value) override
        {
            ++scanCalls_;
            scanSawInitialization_ = attackSerializer_ != nullptr;
            // Object declares a dynamic child, causing the prepass to reserve the CnaDemos alias
            // that WriteTypeName needs after the root namespace declarations have been emitted.
            auto& objectSerializer = serializer.GetTypeSerializer(System::Type::From<System::Object>());
            callback(objectSerializer, ToContentObject<Attack>(value.encounter));
        }

        void Serialize(IntermediateWriter& output, const TypedEnvelope& value,
                       const ContentSerializerAttribute&) override
        {
            auto& payloadSerializer =
                output.getSerializerProperty().GetTypeSerializer(System::Type::From<DiceRoll>());
            auto& xml = output.getXmlProperty();
            xml.WriteStartElement("Payload");
            output.WriteTypeName(System::Type::From<DiceRoll>());
            ContentSerializerAttribute raw;
            raw.setElementNameProperty("Payload");
            raw.setFlattenContentProperty(true);
            output.WriteRawObject<DiceRoll>(value.payload, raw, payloadSerializer);
            xml.WriteEndElement();

            auto& encounterSerializer = attackSerializer_ != nullptr
                ? *attackSerializer_
                : output.getSerializerProperty().GetTypeSerializer(System::Type::From<Attack>());
            xml.WriteStartElement("Encounter");
            output.WriteTypeName(System::Type::From<Attack>());
            ContentSerializerAttribute encounterRaw;
            encounterRaw.setElementNameProperty("Encounter");
            encounterRaw.setFlattenContentProperty(true);
            output.WriteRawObject<Attack>(value.encounter, encounterRaw, encounterSerializer);
            xml.WriteEndElement();
        }

        TypedEnvelope Deserialize(IntermediateReader& input, const ContentSerializerAttribute&,
                                  TypedEnvelope) override
        {
            if (!input.MoveToElement("Payload"))
                throw InvalidContentException("Typed envelope requires a Payload element.");
            const std::string spelledType = input.getXmlProperty().GetAttribute("Type");
            const System::Type payloadType = input.ReadTypeName();
            if (payloadType != System::Type::From<DiceRoll>())
                throw InvalidContentException("Typed envelope does not accept payload type '" + spelledType + "'.");
            auto& payloadSerializer = input.getSerializerProperty().GetTypeSerializer(payloadType);
            ContentSerializerAttribute payload;
            payload.setElementNameProperty("Payload");
            TypedEnvelope result;
            result.payload = input.ReadObject<DiceRoll>(payload, payloadSerializer);

            if (!input.MoveToElement("Encounter"))
                throw InvalidContentException("Typed envelope requires an Encounter element.");
            const System::Type encounterType = input.ReadTypeName();
            if (encounterType != System::Type::From<Attack>())
                throw InvalidContentException("Typed envelope requires an Attack encounter.");
            auto& encounterSerializer = input.getSerializerProperty().GetTypeSerializer(encounterType);
            ContentSerializerAttribute encounter;
            encounter.setElementNameProperty("Encounter");
            result.encounter = input.ReadObject<Attack>(encounter, encounterSerializer);
            return result;
        }

    private:
        ContentTypeSerializerBase* attackSerializer_ = nullptr;
        int initializeCalls_ = 0;
        int scanCalls_ = 0;
        bool scanSawInitialization_ = false;
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
              serializer.getTargetTypeProperty() == System::Type::From<DiceRoll>() &&
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

        const auto roundTripOptional = [&](const OptionalDice& value) {
            std::unique_ptr<System::Xml::XmlWriter> optionalWriter(
                System::Xml::XmlWriter::CreateToString(settings));
            IntermediateSerializer::Serialize<OptionalDice>(*optionalWriter, value, "");
            const std::string document = optionalWriter->ToString();
            std::unique_ptr<System::Xml::XmlReader> optionalReader(
                System::Xml::XmlReader::CreateFromString(document));
            return std::pair(document, IntermediateSerializer::Deserialize<OptionalDice>(*optionalReader, ""));
        };
        const auto [emptyOptionalXml, emptyOptional] = roundTripOptional(OptionalDice{});
        check(emptyOptionalXml.find("<Bonus>") == std::string::npos &&
              emptyOptionalXml.find("<Attack>1d4</Attack>") != std::string::npos && emptyOptional == OptionalDice{},
              "ObjectIsEmpty omits an empty optional custom value without suppressing required peers");
        OptionalDice boosted;
        boosted.bonus = {2, 12};
        const auto [presentOptionalXml, presentOptional] = roundTripOptional(boosted);
        check(presentOptionalXml.find("<Bonus>2d12</Bonus>") != std::string::npos && presentOptional == boosted,
              "a nonempty optional custom value still serializes and round trips");

        std::unique_ptr<System::Xml::XmlWriter> attackWriter(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<Attack>(*attackWriter, Attack{"Spark", roll}, "");
        const std::string attackXml = attackWriter->ToString();
        std::cout << attackXml << '\n';
        std::unique_ptr<System::Xml::XmlReader> attackReader(System::Xml::XmlReader::CreateFromString(attackXml));
        const auto attack = IntermediateSerializer::Deserialize<Attack>(*attackReader, "");
        check(attack.name == "Spark" && attack.damage == roll &&
              attackXml.find("<Damage>2d6</Damage>") != std::string::npos,
              "described parent fields dispatch to the same registered custom serializer");

        auto& envelopeSerializer = dynamic_cast<TypedEnvelopeSerializer&>(
            IntermediateSerializer::AddTypeSerializer<TypedEnvelopeSerializer>());
        const TypedEnvelope envelope{{3, 8}, {"Ambush", {2, 10}}};
        std::unique_ptr<System::Xml::XmlWriter> envelopeWriter(
            System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<TypedEnvelope>(*envelopeWriter, envelope, "");
        const std::string envelopeXml = envelopeWriter->ToString();
        std::cout << envelopeXml << '\n';
        check(envelopeXml.find("<Payload Type=\"dice\">3d8</Payload>") != std::string::npos,
              "the envelope writes a nested dynamic type name and compact payload");
        check(envelopeSerializer.ScanCalls() == 1 &&
              envelopeXml.find("xmlns:CnaDemos=\"CnaDemos\"") != std::string::npos &&
              envelopeXml.find("<Encounter Type=\"CnaDemos:Attack\">") != std::string::npos,
              "ScanChildren reserves the namespace used by the nested custom Attack type");
        check(envelopeSerializer.InitializeCalls() == 1 && envelopeSerializer.ScanSawInitialization(),
              "Initialize runs before child scanning so nested serializers can be cached (CNA-DEMOS-ISSUE-089)");
        const auto readEnvelope = [](const std::string& document) {
            std::unique_ptr<System::Xml::XmlReader> typedReader(
                System::Xml::XmlReader::CreateFromString(document));
            return IntermediateSerializer::Deserialize<TypedEnvelope>(*typedReader, "");
        };
        check(readEnvelope(envelopeXml) == envelope,
              "reader context resolves the nested type and dispatches its registered serializer");

        const auto payloadStart = envelopeXml.find("<Payload");
        const auto payloadEnd = envelopeXml.find("</Payload>", payloadStart);
        if (payloadStart == std::string::npos || payloadEnd == std::string::npos)
            throw std::runtime_error("Typed envelope payload missing.");
        auto missingPayload = envelopeXml;
        missingPayload.erase(payloadStart, payloadEnd + std::string_view("</Payload>").size() - payloadStart);
        bool missingPayloadRejected = false;
        try { (void)readEnvelope(missingPayload); }
        catch (const InvalidContentException& error)
        {
            missingPayloadRejected = std::string(error.what()).find("requires a Payload") != std::string::npos;
        }
        check(missingPayloadRejected, "MoveToElement exposes a missing required typed payload");

        const auto typeAt = envelopeXml.find(" Type=\"dice\"", payloadStart);
        if (typeAt == std::string::npos) throw std::runtime_error("Typed payload Type attribute missing.");
        auto missingType = envelopeXml;
        missingType.erase(typeAt, std::string_view(" Type=\"dice\"").size());
        bool missingTypeRejected = false;
        try { (void)readEnvelope(missingType); }
        catch (const InvalidContentException& error)
        {
            missingTypeRejected = std::string(error.what()).find("missing a \"Type\"") != std::string::npos;
        }
        check(missingTypeRejected, "ReadTypeName rejects a nested payload without its Type attribute");

        auto unknownType = envelopeXml;
        unknownType.replace(typeAt + std::string_view(" Type=\"").size(), 4, "unknownDice");
        bool unknownTypeRejected = false;
        try { (void)readEnvelope(unknownType); }
        catch (const std::exception& error)
        {
            unknownTypeRejected = std::string(error.what()).find("Cannot find type") != std::string::npos;
        }
        check(unknownTypeRejected, "ReadTypeName rejects an unregistered nested payload type");

        auto unrelatedType = envelopeXml;
        unrelatedType.replace(typeAt + std::string_view(" Type=\"").size(), 4, "string");
        bool unrelatedTypeRejected = false;
        try { (void)readEnvelope(unrelatedType); }
        catch (const InvalidContentException& error)
        {
            unrelatedTypeRejected = std::string(error.what()).find("payload type 'string'") != std::string::npos;
        }
        check(unrelatedTypeRejected,
              "the custom schema rejects a registered but unrelated nested payload type before dispatch");

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

        auto& repeated = IntermediateSerializer::AddTypeSerializer<AlternateDiceRollSerializer>();
        check(&repeated == &serializer && IntermediateSerializer::FindTypeSerializer("alternate-dice") == nullptr,
              "re-registering the same target type keeps the first serializer and ignores the later shortcut");

        bool collisionRejected = false;
        ContentTypeSerializerBase* coinSerializer = nullptr;
        try
        {
            coinSerializer = &IntermediateSerializer::AddTypeSerializer<CoinFlipSerializer>();
        }
        catch (const std::exception& error)
        {
            collisionRejected = true;
            std::cout << "Rejected colliding shortcut: " << error.what() << '\n';
        }
        bool coinRoundTrip = false;
        if (coinSerializer != nullptr)
        {
            try
            {
                std::unique_ptr<System::Xml::XmlWriter> coinWriter(System::Xml::XmlWriter::CreateToString(settings));
                IntermediateSerializer::Serialize<CoinFlip>(*coinWriter, CoinFlip{3}, "");
                const std::string coinXml = coinWriter->ToString();
                std::cout << coinXml << '\n';
                std::unique_ptr<System::Xml::XmlReader> coinReader(System::Xml::XmlReader::CreateFromString(coinXml));
                coinRoundTrip = IntermediateSerializer::Deserialize<CoinFlip>(*coinReader, "") == CoinFlip{3};
            }
            catch (const std::exception& error)
            {
                std::cout << "Accepted shortcut collision cannot round trip: " << error.what() << '\n';
            }
        }
        const bool completeCoinRegistration = coinSerializer != nullptr &&
            IntermediateSerializer::FindTypeSerializer(System::Type::From<CoinFlip>()) == coinSerializer &&
            IntermediateSerializer::FindTypeSerializer(std::string(CoinFlip::XnaTypeName)) == coinSerializer &&
            IntermediateSerializer::FindTypeSerializer("dice") == coinSerializer && coinRoundTrip;
        check(collisionRejected || completeCoinRegistration,
              "a conflicting XML shortcut is rejected or installed as one complete round-trippable mapping (CNA-DEMOS-ISSUE-088)");
        check(IntermediateSerializer::FindTypeSerializer(System::Type::From<DiceRoll>()) == &serializer &&
                  IntermediateSerializer::FindTypeSerializer("dice") == &serializer,
              "the original dice type and shortcut remain paired after both registration attempts");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Custom intermediate serializer failed: " << error.what() << '\n';
        return 1;
    }
}
