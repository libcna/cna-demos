// SPDX-License-Identifier: MS-PL
// XNA's TypeConverters: math values to text and back, as a property grid or a config file needs.
#include <any>
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
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

        const ColorConverter colors;
        ctx.Check(std::any_cast<Color>(colors.ConvertFromInvariantString("255, 128, 0, 255")) == Color(255, 128, 0, 255),
                  "a Color reads from \"R, G, B, A\"");

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
             "MathTypeConverter::CanConvertFrom", "ColorConverter::ConvertFrom", "RectangleConverter::ConvertFrom",
             "PointConverter::ConvertTo", "Vector2Converter::ConvertTo", "QuaternionConverter::ConvertTo",
             "BoundingBoxConverter::ConvertTo", "MatrixConverter::ConvertTo",
             "RectangleConverter", "Vector4Converter", "PlaneConverter", "RayConverter"},
    .tags = {"design", "type-converter", "text", "serialization", "tools"},
})
