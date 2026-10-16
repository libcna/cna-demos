// SPDX-License-Identifier: MS-PL
// XNA's TypeConverters: math values to text and back, as a property grid or a config file needs.
#include <any>
#include <memory>
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
#include "System/ArgumentException.hpp"
#include "System/ComponentModel/TypeDescriptor.hpp"
#include "System/NotSupportedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Design;
using System::ComponentModel::TypeDescriptor;

namespace
{
    template <class T>
    void Show(CnaDemos::DemoContext& ctx, const char* name, const T& value)
    {
        const auto converter = TypeDescriptor::GetConverter(System::Type::From<T>());
        const bool parses = converter->CanConvertFrom(System::Type::From<std::string>());
        ctx.Print("{:<12} \"{}\"{}", name, converter->ConvertToInvariantString(std::any(value)),
                  parses ? "" : "   (text output only)");
    }

    void StringRoundTrip(CnaDemos::DemoContext& ctx)
    {
        // Each XNA math type names its converter (XNA's [TypeConverter] attribute), so tools find
        // it through TypeDescriptor without knowing the type in advance.
        const auto converter = TypeDescriptor::GetConverter(System::Type::From<Vector3>());
        ctx.Check(dynamic_cast<Vector3Converter*>(converter.get()) != nullptr,
                  "TypeDescriptor::GetConverter(Vector3) is a Vector3Converter");

        // Invariant text uses '.' and ", ": the form for files that must read back anywhere.
        const Vector3 position(1.5f, 2.25f, -3.75f);
        const std::string text = converter->ConvertToInvariantString(std::any(position));
        ctx.Print("Vector3 -> \"{}\"", text);
        ctx.Check(text == "1.5, 2.25, -3.75", "a Vector3 becomes \"1.5, 2.25, -3.75\"");
        ctx.Check(std::any_cast<Vector3>(converter->ConvertFromInvariantString(text)) == position,
                  "and the text reads back as the same Vector3");

        bool rejected = false;
        try
        {
            (void)converter->ConvertFromInvariantString("1.5, 2.25");
        }
        catch (const System::ArgumentException&)
        {
            rejected = true;
        }
        ctx.Check(rejected, "text with the wrong number of components throws ArgumentException");

        const Point point(12, -7);
        const PointConverter points;
        const Vector2 vector2(1.5f, -2.25f);
        const Vector2Converter vectors2;
        const Vector4 vector4(1.0f, 0.5f, 0.25f, 1.0f);
        const Vector4Converter vectors4;
        const Quaternion quaternion = Quaternion::Identity;
        const QuaternionConverter quaternions;
        ctx.Check(std::any_cast<Point>(points.ConvertFromInvariantString(points.ConvertToInvariantString(point))) == point &&
                      std::any_cast<Vector2>(vectors2.ConvertFromInvariantString(vectors2.ConvertToInvariantString(vector2))) == vector2 &&
                      std::any_cast<Vector4>(vectors4.ConvertFromInvariantString(vectors4.ConvertToInvariantString(vector4))) == vector4 &&
                      std::any_cast<Quaternion>(quaternions.ConvertFromInvariantString(
                          quaternions.ConvertToInvariantString(quaternion))) == quaternion,
                  "Point, Vector2, Vector4 and Quaternion invariant text round-trips exactly");

        const ColorConverter colors;
        ctx.Check(std::any_cast<Color>(colors.ConvertFromInvariantString("255, 128, 0, 255")) == Color(255, 128, 0, 255),
                  "a Color reads from \"R, G, B, A\"");
        ctx.Check(colors.ConvertToInvariantString(Color::CornflowerBlue) == "100, 149, 237, 255",
                  "Color writes its four byte channels in RGBA order");

        // Types with several parts (Rectangle, Matrix, bounding volumes) print but do not parse,
        // as in XNA; a property grid edits their parts instead.
        const RectangleConverter rectangles;
        bool notSupported = false;
        try
        {
            (void)rectangles.ConvertFromInvariantString("1, 2, 3, 4");
        }
        catch (const System::NotSupportedException&)
        {
            notSupported = true;
        }
        ctx.Check(!rectangles.CanConvertFrom(System::Type::From<std::string>()) && notSupported,
                  "RectangleConverter does not parse text (NotSupportedException)");

        const BoundingBoxConverter boxes;
        const MatrixConverter matrices;
        const PlaneConverter planes;
        const RayConverter rays;
        ctx.Check(rectangles.ConvertToInvariantString(Rectangle(1, 2, 30, 40)) ==
                          "{X:1 Y:2 Width:30 Height:40}" &&
                      boxes.ConvertToInvariantString(BoundingBox(Vector3(-1.0f), Vector3::One)) ==
                          "{{Min:{X:-1 Y:-1 Z:-1} Max:{X:1 Y:1 Z:1}}}" &&
                      planes.ConvertToInvariantString(Plane(Vector3::Up, -2.0f)) ==
                          "{Normal:{X:0 Y:1 Z:0} D:-2}" &&
                      rays.ConvertToInvariantString(Ray(Vector3::Zero, Vector3::Forward)) ==
                          "{{Position:{X:0 Y:0 Z:0} Direction:{X:0 Y:0 Z:-1}}}" &&
                      matrices.ConvertToInvariantString(Matrix::getIdentityProperty()).find("M44:1") != std::string::npos,
                  "non-parsing converters still emit each value's exact structural text");

        Show(ctx, "Point", Point(12, -7));
        Show(ctx, "Vector2", Vector2(1.5f, -2.25f));
        Show(ctx, "Vector3", position);
        Show(ctx, "Vector4", Vector4(1.0f, 0.5f, 0.25f, 1.0f));
        Show(ctx, "Quaternion", Quaternion::Identity);
        Show(ctx, "Plane", Plane(Vector3::Up, -2.0f));
        Show(ctx, "Ray", Ray(Vector3::Zero, Vector3::Forward));
        Show(ctx, "Color", Color::CornflowerBlue);
        Show(ctx, "Rectangle", Rectangle(1, 2, 30, 40));
        Show(ctx, "BoundingBox", BoundingBox(Vector3(-1.0f, -1.0f, -1.0f), Vector3::One));
        Show(ctx, "BoundSphere", BoundingSphere(Vector3(1.0f, 2.0f, 3.0f), 4.5f));
        Show(ctx, "Matrix", Matrix::getIdentityProperty());
    }
}

CNA_DEMOS_CONSOLE(StringRoundTrip, {
    .id = "design.type_converter.string_round_trip",
    .title = "Math values as text",
    .description = "Finds the Vector3Converter through TypeDescriptor, writes a Vector3 as invariant text and reads "
                   "it back, rejects malformed text, and lists which XNA math types parse text (Point, vectors, "
                   "Quaternion, Color) and which only print (Rectangle, Matrix, Plane, Ray, bounding volumes).",
    .apis = {"Vector3Converter::ConvertTo", "Vector3Converter::ConvertFrom", "Vector3Converter::Vector3Converter",
             "MathTypeConverter::CanConvertFrom", "ColorConverter::ColorConverter", "ColorConverter::ConvertFrom",
             "ColorConverter::ConvertTo", "PointConverter::PointConverter", "PointConverter::ConvertFrom",
             "PointConverter::ConvertTo", "Vector2Converter::Vector2Converter", "Vector2Converter::ConvertFrom",
             "Vector2Converter::ConvertTo", "Vector4Converter::Vector4Converter", "Vector4Converter::ConvertFrom",
             "Vector4Converter::ConvertTo", "QuaternionConverter::QuaternionConverter",
             "QuaternionConverter::ConvertFrom", "QuaternionConverter::ConvertTo",
             "RectangleConverter::RectangleConverter", "RectangleConverter::ConvertFrom",
             "RectangleConverter::ConvertTo", "BoundingBoxConverter::BoundingBoxConverter",
             "BoundingBoxConverter::ConvertTo", "MatrixConverter::MatrixConverter", "MatrixConverter::ConvertTo",
             "PlaneConverter::PlaneConverter", "PlaneConverter::ConvertTo", "RayConverter::RayConverter",
             "RayConverter::ConvertTo"},
    .tags = {"design", "type-converter", "text", "serialization", "tools"},
})
