// SPDX-License-Identifier: MS-PL
// MathTypeConverter::GetProperties and CreateInstance: editing a value part by part, as a property grid does.
#include <any>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Design.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Ray.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "System/Collections/Hashtable.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Design;

namespace
{
    class ConfigurableMathConverter final : public MathTypeConverter
    {
    public:
        void SetStringConversion(bool enabled) { supportStringConvert = enabled; }

        void SetPropertyDescriptions(const System::ComponentModel::PropertyDescriptorCollection& descriptions)
        {
            propertyDescriptions = descriptions;
        }
    };

    void Properties(CnaDemos::DemoContext& ctx)
    {
        const MathTypeConverter common;
        ctx.Check(common.CanConvertFrom(System::Type::From<std::string>()) &&
                      common.GetCreateInstanceSupported() && common.GetPropertiesSupported(),
                  "the base math converter enables text, component properties and value recreation");

        // A grid lists the parts of a value ("expandable"), each with a name and a type.
        const ColorConverter colors;
        ctx.Check(colors.GetPropertiesSupported() && colors.GetCreateInstanceSupported(),
                  "math converters expose parts and can create values from them");
        const auto parts = colors.GetProperties(std::any(Color::Orange));
        std::string names;
        for (int i = 0; i < parts.getCountProperty(); ++i)
        {
            const auto& part = parts.getItem(i);
            names += (i ? ", " : "") + part->getNameProperty() + "=" +
                std::to_string(std::any_cast<SharpRuntime::bytecs>(part->GetValue(std::any(Color::Orange))));
        }
        ctx.Print("Color parts: {}", names);
        ctx.Check(parts.getCountProperty() == 4 && parts.getItem(0)->getNameProperty() == "R",
                  "a Color has the parts R, G, B and A");

        ConfigurableMathConverter configured;
        configured.SetStringConversion(false);
        const bool disabled = !configured.CanConvertFrom(System::Type::From<std::string>());
        configured.SetStringConversion(true);
        ctx.Check(disabled && configured.CanConvertFrom(System::Type::From<std::string>()),
                  "a subclass controls public string-conversion capability through supportStringConvert");
        configured.SetPropertyDescriptions(parts);
        const auto configuredParts = configured.GetProperties(std::any(Color::Orange));
        ctx.Check(configuredParts.getCountProperty() == parts.getCountProperty() &&
                      configuredParts.getItem(0) == parts.getItem(0),
                  "a subclass publishes its propertyDescriptions through GetProperties");

        // Editing one part changes the boxed value in place.
        const Vector3Converter vectors;
        std::any boxed = Vector3(1.0f, 2.0f, 3.0f);
        vectors.GetProperties(boxed).getItem("Y")->SetValue(boxed, std::any(20.0f));
        ctx.Check(std::any_cast<Vector3>(boxed) == Vector3(1.0f, 20.0f, 3.0f), "setting Y edits the Vector3");

        // A grid rebuilds a value from all its parts with CreateInstance.
        System::Collections::Hashtable edited;
        edited.setItem("R", std::any(SharpRuntime::bytecs{10}));
        edited.setItem("G", std::any(SharpRuntime::bytecs{200}));
        edited.setItem("B", std::any(SharpRuntime::bytecs{30}));
        edited.setItem("A", std::any(SharpRuntime::bytecs{255}));
        ctx.Check(std::any_cast<Color>(colors.CreateInstance(edited)) == Color(10, 200, 30, 255),
                  "CreateInstance builds the Color from the edited parts");

        System::Collections::Hashtable pointValues;
        pointValues.setItem("X", std::any(12));
        pointValues.setItem("Y", std::any(-7));
        const PointConverter points;
        System::Collections::Hashtable vector2Values;
        vector2Values.setItem("X", std::any(1.5f));
        vector2Values.setItem("Y", std::any(-2.25f));
        const Vector2Converter vectors2;
        System::Collections::Hashtable vector3Values;
        vector3Values.setItem("X", std::any(1.0f));
        vector3Values.setItem("Y", std::any(20.0f));
        vector3Values.setItem("Z", std::any(3.0f));
        System::Collections::Hashtable vector4Values;
        vector4Values.setItem("X", std::any(1.0f));
        vector4Values.setItem("Y", std::any(2.0f));
        vector4Values.setItem("Z", std::any(3.0f));
        vector4Values.setItem("W", std::any(4.0f));
        const Vector4Converter vectors4;
        ctx.Check(std::any_cast<Point>(points.CreateInstance(pointValues)) == Point(12, -7) &&
                      std::any_cast<Vector2>(vectors2.CreateInstance(vector2Values)) == Vector2(1.5f, -2.25f) &&
                      std::any_cast<Vector3>(vectors.CreateInstance(vector3Values)) == Vector3(1.0f, 20.0f, 3.0f) &&
                      std::any_cast<Vector4>(vectors4.CreateInstance(vector4Values)) == Vector4(1.0f, 2.0f, 3.0f, 4.0f),
                  "CreateInstance rebuilds integer and floating-point component vectors");

        System::Collections::Hashtable quaternionValues;
        quaternionValues.setItem("X", std::any(0.0f));
        quaternionValues.setItem("Y", std::any(0.0f));
        quaternionValues.setItem("Z", std::any(0.0f));
        quaternionValues.setItem("W", std::any(1.0f));
        const QuaternionConverter quaternions;
        System::Collections::Hashtable rectangleValues;
        rectangleValues.setItem("X", std::any(1));
        rectangleValues.setItem("Y", std::any(2));
        rectangleValues.setItem("Width", std::any(30));
        rectangleValues.setItem("Height", std::any(40));
        const RectangleConverter rectangles;
        ctx.Check(std::any_cast<Quaternion>(quaternions.CreateInstance(quaternionValues)) == Quaternion::Identity &&
                      std::any_cast<Rectangle>(rectangles.CreateInstance(rectangleValues)) == Rectangle(1, 2, 30, 40),
                  "CreateInstance rebuilds orientation and integer rectangle values");

        System::Collections::Hashtable boxValues;
        boxValues.setItem("Min", std::any(Vector3(-1.0f)));
        boxValues.setItem("Max", std::any(Vector3::One));
        const BoundingBoxConverter boxes;
        System::Collections::Hashtable sphereValues;
        sphereValues.setItem("Center", std::any(Vector3(1.0f, 2.0f, 3.0f)));
        sphereValues.setItem("Radius", std::any(4.5f));
        const BoundingSphereConverter spheres;
        System::Collections::Hashtable planeValues;
        planeValues.setItem("Normal", std::any(Vector3::Up));
        planeValues.setItem("D", std::any(-2.0f));
        const PlaneConverter planes;
        System::Collections::Hashtable rayValues;
        rayValues.setItem("Position", std::any(Vector3::Zero));
        rayValues.setItem("Direction", std::any(Vector3::Forward));
        const RayConverter rays;
        ctx.Check(std::any_cast<BoundingBox>(boxes.CreateInstance(boxValues)) ==
                          BoundingBox(Vector3(-1.0f), Vector3::One) &&
                      std::any_cast<BoundingSphere>(spheres.CreateInstance(sphereValues)) ==
                          BoundingSphere(Vector3(1.0f, 2.0f, 3.0f), 4.5f) &&
                      std::any_cast<Plane>(planes.CreateInstance(planeValues)) == Plane(Vector3::Up, -2.0f) &&
                      std::any_cast<Ray>(rays.CreateInstance(rayValues)) == Ray(Vector3::Zero, Vector3::Forward),
                  "CreateInstance rebuilds compound geometry from named parts");

        // Matrix offers its sixteen elements plus a Translation part for convenience.
        const MatrixConverter matrices;
        std::any transform = Matrix::getIdentityProperty();
        const auto matrixParts = matrices.GetProperties(transform);
        ctx.Print("Matrix parts: {} (first: {})", matrixParts.getCountProperty(), matrixParts.getItem(0)->getNameProperty());
        matrixParts.getItem("Translation")->SetValue(transform, std::any(Vector3(7.0f, 8.0f, 9.0f)));
        ctx.Check(matrixParts.getCountProperty() == 17 &&
                      std::any_cast<Matrix>(transform).getTranslationProperty() == Vector3(7.0f, 8.0f, 9.0f),
                  "Matrix has 17 parts; editing Translation moves the matrix");
        System::Collections::Hashtable matrixValues;
        for (int i = 0; i < matrixParts.getCountProperty(); ++i)
        {
            const auto& part = matrixParts.getItem(i);
            if (part->getNameProperty() != "Translation")
            {
                matrixValues.setItem(part->getNameProperty(), part->GetValue(transform));
            }
        }
        ctx.Check(std::any_cast<Matrix>(matrices.CreateInstance(matrixValues)) == std::any_cast<Matrix>(transform),
                  "Matrix CreateInstance rebuilds all sixteen stored elements");
    }
}

CNA_DEMOS_CONSOLE(Properties, {
    .id = "design.type_converter.properties",
    .title = "Editing values part by part",
    .description = "Lists a Color's parts with ColorConverter::GetProperties, edits a Vector3's Y through its "
                   "property descriptor, rebuilds a Color with CreateInstance from edited parts, and moves a Matrix "
                   "through its extra Translation part -- what a property grid does.",
    .apis = {"MathTypeConverter::MathTypeConverter", "MathTypeConverter::GetProperties",
             "MathTypeConverter::GetPropertiesSupported",
             "MathTypeConverter::GetCreateInstanceSupported", "MathTypeConverter::propertyDescriptions",
             "MathTypeConverter::supportStringConvert", "ColorConverter::CreateInstance",
             "PointConverter::CreateInstance", "Vector2Converter::CreateInstance",
             "Vector3Converter::CreateInstance", "Vector4Converter::CreateInstance",
             "QuaternionConverter::CreateInstance", "RectangleConverter::CreateInstance",
             "BoundingBoxConverter::CreateInstance", "BoundingSphereConverter::CreateInstance",
             "PlaneConverter::CreateInstance", "RayConverter::CreateInstance", "MatrixConverter::CreateInstance"},
    .tags = {"design", "type-converter", "property-grid", "editor", "tools"},
})
