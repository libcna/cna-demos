// SPDX-License-Identifier: MS-PL
// ToString of the XNA math types: the text a log, a debug overlay or a save file gets.
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Ray.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    // Every type prints its fields as {Name:value ...}; composite types nest their parts' text.
    void Expect(CnaDemos::DemoContext& ctx, const char* type, const std::string& actual, const std::string& expected)
    {
        ctx.Print("{:<15} {}", type, actual);
        ctx.Check(actual == expected, std::string(type) + " prints " + expected);
    }

    void ToStringFormats(CnaDemos::DemoContext& ctx)
    {
        Expect(ctx, "Point", Point(3, -4).ToString(), "{X:3 Y:-4}");
        Expect(ctx, "Vector2", Vector2(1.5f, -2.0f).ToString(), "{X:1.5 Y:-2}");
        Expect(ctx, "Vector3", Vector3(1.0f, 2.0f, 3.0f).ToString(), "{X:1 Y:2 Z:3}");
        Expect(ctx, "Vector4", Vector4(1.0f, 2.0f, 3.0f, 4.0f).ToString(), "{X:1 Y:2 Z:3 W:4}");
        Expect(ctx, "Quaternion", Quaternion::Identity.ToString(), "{X:0 Y:0 Z:0 W:1}");
        Expect(ctx, "Rectangle", Rectangle(10, 20, 30, 40).ToString(), "{X:10 Y:20 Width:30 Height:40}");
        Expect(ctx, "Color", Color(255, 128, 0, 255).ToString(), "{R:255 G:128 B:0 A:255}");
        Expect(ctx, "Plane", Plane(Vector3::Up, -2.0f).ToString(), "{Normal:{X:0 Y:1 Z:0} D:-2}");
        Expect(ctx, "BoundingSphere", BoundingSphere(Vector3::Zero, 1.0f).ToString(),
               "{Center:{X:0 Y:0 Z:0} Radius:1}");
        // XNA builds these two with string.Format("{{Min:{0} Max:{1}}}", ...): the doubled braces
        // are format escapes and print as single ones (CNA-DEMOS-ISSUE-024).
        Expect(ctx, "BoundingBox", BoundingBox(-Vector3::One, Vector3::One).ToString(),
               "{Min:{X:-1 Y:-1 Z:-1} Max:{X:1 Y:1 Z:1}}");
        Expect(ctx, "Ray", Ray(Vector3::Zero, Vector3::Forward).ToString(),
               "{Position:{X:0 Y:0 Z:0} Direction:{X:0 Y:0 Z:-1}}");
        Expect(ctx, "Matrix", Matrix::getIdentityProperty().ToString(),
               "{ {M11:1 M12:0 M13:0 M14:0} {M21:0 M22:1 M23:0 M24:0} {M31:0 M32:0 M33:1 M34:0} "
               "{M41:0 M42:0 M43:0 M44:1} }");
    }
}

CNA_DEMOS_CONSOLE(ToStringFormats, {
    .id = "math.formatting.to_string",
    .title = "Math values as text (ToString)",
    .description = "Prints the XNA math types with ToString -- Point, the vectors, Quaternion, Rectangle, Color, "
                   "Plane, BoundingSphere, BoundingBox, Ray and Matrix -- and checks each against XNA's format.",
    .apis = {"Point::ToString", "Vector2::ToString", "Vector3::ToString", "Vector4::ToString", "Quaternion::ToString",
             "Rectangle::ToString", "Color::ToString", "Plane::ToString", "BoundingSphere::ToString",
             "BoundingBox::ToString", "Ray::ToString", "Matrix::ToString"},
    .tags = {"math", "string", "debug", "format"},
})
