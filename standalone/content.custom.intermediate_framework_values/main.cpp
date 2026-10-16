// SPDX-License-Identifier: MS-PL
// XNA intermediate XML gives framework values fixed component shapes and Color a distinct ARGB hex form.
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Ray.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Object.hpp"
#include "System/Xml/XmlReader.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/XmlWriterSettings.hpp"

using Microsoft::Xna::Framework::BoundingBox;
using Microsoft::Xna::Framework::Color;
using Microsoft::Xna::Framework::Curve;
using Microsoft::Xna::Framework::CurveContinuity;
using Microsoft::Xna::Framework::CurveKey;
using Microsoft::Xna::Framework::CurveLoopType;
using Microsoft::Xna::Framework::Matrix;
using Microsoft::Xna::Framework::Ray;
using Microsoft::Xna::Framework::Rectangle;
using Microsoft::Xna::Framework::Vector3;
using Microsoft::Xna::Framework::Content::Pipeline::InvalidContentException;
using namespace Microsoft::Xna::Framework::Content::Pipeline::Serialization::Intermediate;

namespace
{
    Curve MakeMotionCurve()
    {
        Curve curve;
        curve.setPreLoopProperty(CurveLoopType::Constant);
        curve.setPostLoopProperty(CurveLoopType::Linear);
        curve.getKeysProperty().Add(CurveKey(0.0f, 0.0f, 0.0f, 1.0f, CurveContinuity::Smooth));
        curve.getKeysProperty().Add(CurveKey(1.0f, 2.0f, 1.0f, 0.0f, CurveContinuity::Step));
        return curve;
    }

    struct GeometrySettings final : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.GeometrySettings";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName);
            return name;
        }

        Vector3 origin{1.0f, 2.0f, 3.0f};
        Matrix transform = Matrix::CreateTranslation(1.0f, 2.0f, 3.0f);
        Color tint{10, 20, 30, 40};
        Rectangle viewport{1, 2, 30, 40};
        BoundingBox bounds{Vector3(-1.0f, -2.0f, -3.0f), Vector3(1.0f, 2.0f, 3.0f)};
        Ray probe{Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)};
        Curve motion = MakeMotionCurve();

        static void DescribeContent(ContentTypeDescriptor<GeometrySettings>& type)
        {
            type.Field("Origin", &GeometrySettings::origin);
            type.Field("Transform", &GeometrySettings::transform);
            type.Field("Tint", &GeometrySettings::tint);
            type.Field("Viewport", &GeometrySettings::viewport);
            type.Field("Bounds", &GeometrySettings::bounds);
            type.Field("Probe", &GeometrySettings::probe);
            type.Field("Motion", &GeometrySettings::motion);
        }
    };

    std::string Serialize(const std::shared_ptr<GeometrySettings>& value)
    {
        System::Xml::XmlWriterSettings settings;
        settings.Indent = false;
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<GeometrySettings>(*writer, value, "");
        return writer->ToString();
    }

    std::shared_ptr<GeometrySettings> Deserialize(const std::string& xml)
    {
        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
        return IntermediateSerializer::Deserialize<GeometrySettings>(*reader, "");
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

        const auto authored = std::make_shared<GeometrySettings>();
        const std::string xml = Serialize(authored);
        std::cout << xml << '\n';

        const std::string origin = "<Origin>1 2 3</Origin>";
        const std::string transform =
            "<Transform>1 0 0 0 0 1 0 0 0 0 1 0 1 2 3 1</Transform>";
        const std::string tint = "<Tint>280A141E</Tint>";
        const std::string viewport = "<Viewport>1 2 30 40</Viewport>";
        const std::string bounds = "<Bounds><Min>-1 -2 -3</Min><Max>1 2 3</Max></Bounds>";
        const std::string probe =
            "<Probe><Position>0 0 0</Position><Direction>0 0 1</Direction></Probe>";
        const std::string motion =
            "<Motion><PreLoop>Constant</PreLoop><PostLoop>Linear</PostLoop>"
            "<Keys>0 0 0 1 Smooth 1 2 1 0 Step</Keys></Motion>";

        check(xml.find(origin) != std::string::npos && xml.find(transform) != std::string::npos,
              "vectors and matrices use fixed-width whitespace-separated component tuples");
        check(xml.find(tint) != std::string::npos && xml.find(viewport) != std::string::npos,
              "Color uses packed ARGB hexadecimal text while Rectangle uses four decimal integers");
        check(xml.find(bounds) != std::string::npos && xml.find(probe) != std::string::npos,
              "BoundingBox and Ray retain their named nested vector members");
        check(xml.find(motion) != std::string::npos,
              "Curve writes required loop modes and five packed tokens per key");

        const auto restored = Deserialize(xml);
        check(restored && restored->origin == authored->origin && restored->transform == authored->transform &&
                  restored->tint == authored->tint && restored->viewport == authored->viewport,
              "packed vector, matrix, Color and Rectangle values round-trip exactly");
        check(restored && restored->bounds == authored->bounds && restored->probe == authored->probe,
              "structured BoundingBox and Ray values round-trip exactly");
        check(restored && restored->motion.getPreLoopProperty() == CurveLoopType::Constant &&
                  restored->motion.getPostLoopProperty() == CurveLoopType::Linear &&
                  restored->motion.getKeysProperty().getCountProperty() == 2 &&
                  restored->motion.getKeysProperty().getItemProperty(0) ==
                      authored->motion.getKeysProperty().getItemProperty(0) &&
                  restored->motion.getKeysProperty().getItemProperty(1) ==
                      authored->motion.getKeysProperty().getItemProperty(1),
              "Curve loop modes and complete key records round-trip exactly");

        auto emptyCurveValue = std::make_shared<GeometrySettings>();
        emptyCurveValue->motion = Curve{};
        const std::string emptyCurveXml = Serialize(emptyCurveValue);
        const std::string emptyMotion =
            "<Motion><PreLoop>Constant</PreLoop><PostLoop>Constant</PostLoop><Keys /></Motion>";
        const auto emptyCurveRestored = Deserialize(emptyCurveXml);
        check(emptyCurveXml.find(emptyMotion) != std::string::npos && emptyCurveRestored &&
                  emptyCurveRestored->motion.getKeysProperty().getCountProperty() == 0 &&
                  emptyCurveRestored->motion.getPreLoopProperty() == CurveLoopType::Constant &&
                  emptyCurveRestored->motion.getPostLoopProperty() == CurveLoopType::Constant,
              "an empty Curve keeps both default loop modes and a self-closing Keys element");

        const std::string permissive = Replaced(
            Replaced(xml, origin, "<Origin>1\t2\n3</Origin>"), tint, "<Tint>280a141e</Tint>");
        const auto normalizedValue = Deserialize(permissive);
        const std::string normalized = Serialize(normalizedValue);
        check(normalizedValue && normalizedValue->origin == authored->origin &&
                  normalizedValue->tint == authored->tint && normalized.find(origin) != std::string::npos &&
                  normalized.find(tint) != std::string::npos,
              "accepted tuple whitespace and lower-case Color hex normalize to canonical output");

        const auto rejects = [&](const std::string& malformed, std::string_view expectation) {
            bool rejected = false;
            try
            {
                (void)Deserialize(malformed);
            }
            catch (const InvalidContentException& error)
            {
                rejected = true;
                std::cout << "Rejected framework value XML: " << error.what() << '\n';
            }
            check(rejected, expectation);
        };

        rejects(Replaced(xml, origin, "<Origin>1, 2, 3</Origin>"),
                "a vector tuple rejects comma separators");
        rejects(Replaced(xml, origin, "<Origin>1 2</Origin>"),
                "a vector tuple rejects too few components");
        rejects(Replaced(xml, transform,
                         "<Transform>1 0 0 0 0 1 0 0 0 0 1 0 1 2 3</Transform>"),
                "a matrix tuple rejects fewer than sixteen components");
        rejects(Replaced(xml, tint, "<Tint>0x280A141E</Tint>"),
                "Color rejects a hexadecimal prefix");
        rejects(Replaced(xml, tint, "<Tint>40 10 20 30</Tint>"),
                "Color rejects component-list text");
        rejects(Replaced(xml, viewport, "<Viewport>1.5 2 30 40</Viewport>"),
                "Rectangle rejects floating-point components");
        rejects(Replaced(xml, bounds, "<Bounds><Min>-1 -2 -3</Min></Bounds>"),
                "BoundingBox rejects a missing Max member");
        rejects(Replaced(xml, probe,
                         "<Probe><Position>0 0 0</Position><Direction>0 1</Direction></Probe>"),
                "Ray rejects a partial Direction vector");
        rejects(Replaced(xml, motion,
                         "<Motion><Keys>0 0 0 1 Smooth 1 2 1 0 Step</Keys></Motion>"),
                "Curve rejects missing loop-mode elements");
        rejects(Replaced(xml, "<Keys>0 0 0 1 Smooth 1 2 1 0 Step</Keys>",
                         "<Keys>0 0 0 1</Keys>"),
                "Curve rejects a partial five-token key record");
        rejects(Replaced(xml, "Smooth 1 2 1 0 Step", "Bouncy 1 2 1 0 Step"),
                "Curve rejects an unknown continuity name");

        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Intermediate framework-value demonstration failed: " << error.what() << '\n';
        return 1;
    }
}
