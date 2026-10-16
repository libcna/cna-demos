// SPDX-License-Identifier: MS-PL
// Plane construction, public fields, products, identity and text.
#include <any>
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void PlaneClassify(CnaDemos::DemoContext& ctx)
    {
        const Plane zero;
        const Plane floor(Vector3::Up, -2.0f);
        ctx.Check(zero.Normal == Vector3::Zero && zero.D == 0.0f &&
                      floor.Normal == Vector3::Up && floor.D == -2.0f,
                  "default and normal-distance construction preserve the public fields");

        const Plane fromVector(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
        const Plane fromScalars(1.0f, 2.0f, 3.0f, 4.0f);
        ctx.Check(fromVector == fromScalars && fromVector.Normal == Vector3(1.0f, 2.0f, 3.0f) &&
                      fromVector.D == 4.0f,
                  "Vector4 and four-scalar constructors assign the same coefficients");

        const Vector3 a(0.0f, 2.0f, 0.0f);
        const Vector3 b(0.0f, 2.0f, 1.0f);
        const Vector3 c(1.0f, 2.0f, 0.0f);
        const Plane fromPoints(a, b, c);
        const Plane reverseWinding(a, c, b);
        ctx.Check(fromPoints == floor && fromPoints.DotCoordinate(a) == 0.0f &&
                      fromPoints.DotCoordinate(b) == 0.0f && fromPoints.DotCoordinate(c) == 0.0f,
                  "three counter-clockwise points produce the expected unit plane");
        ctx.Check(reverseWinding.Normal == -floor.Normal && reverseWinding.D == -floor.D,
                  "reversing point winding reverses all four plane coefficients");

        const Plane degenerate(Vector3::Zero, Vector3::UnitX, Vector3(2.0f, 0.0f, 0.0f));
        ctx.Check(std::isnan(degenerate.Normal.X) && std::isnan(degenerate.Normal.Y) &&
                      std::isnan(degenerate.Normal.Z) && std::isnan(degenerate.D),
                  "collinear construction follows XNA by producing four NaN coefficients");

        const Plane coefficients(Vector3(1.0f, 2.0f, 3.0f), 4.0f);
        const Vector4 homogeneous(-2.0f, 3.0f, -4.0f, 5.0f);
        const Vector3 coordinate(-2.0f, 3.0f, -4.0f);
        float dotOutput = 99.0f;
        float coordinateOutput = 99.0f;
        float normalOutput = 99.0f;
        coefficients.Dot(homogeneous, dotOutput);
        coefficients.DotCoordinate(coordinate, coordinateOutput);
        coefficients.DotNormal(coordinate, normalOutput);
        ctx.Check(coefficients.Dot(homogeneous) == 12.0f && dotOutput == 12.0f,
                  "Dot value and output forms include the Vector4 W coefficient");
        ctx.Check(coefficients.DotCoordinate(coordinate) == -4.0f && coordinateOutput == -4.0f,
                  "DotCoordinate value and output forms add the plane distance");
        ctx.Check(coefficients.DotNormal(coordinate) == -8.0f && normalOutput == -8.0f,
                  "DotNormal value and output forms use only the normal");

        const Plane authored(Vector3(-1.0f, 2.0f, -3.0f), 4.0f);
        const Plane equal(authored.Normal, authored.D);
        ctx.Check(authored.Equals(equal) && authored.Equals(std::any{equal}) && authored == equal &&
                      !(authored != equal) && !authored.Equals(std::any{42}) &&
                      !authored.Equals(std::any{}),
                  "typed, boxed and operator equality agree and reject other boxed values");
        ctx.Check(Plane(Vector3(-1.0f, 2.0f, -2.0f), 4.0f) != authored &&
                      Plane(authored.Normal, 5.0f) != authored,
                  "changing either the normal or distance makes a Plane unequal");
        ctx.Check(authored.GetHashCode() == equal.GetHashCode(), "equal planes hash equally");

        ctx.Print("authored plane = {}", authored.ToString());
        ctx.Check(authored.ToString() == "{Normal:{X:-1 Y:2 Z:-3} D:4}",
                  "ToString uses XNA's exact Normal and D representation");
    }
}

CNA_DEMOS_CONSOLE(PlaneClassify, {
    .id = "math.plane.classify_points",
    .title = "Plane construction and products",
    .description = "Constructs Planes from every public coefficient form and point winding, checks degenerate points, "
                   "value/output dot products, value identity, hashing and exact text.",
    .apis = {"Plane::Plane", "Plane::Normal", "Plane::D", "Plane::Dot", "Plane::DotCoordinate",
             "Plane::DotNormal", "Plane::Equals", "Plane::GetHashCode", "Plane::ToString",
             "Plane::op_Equality", "Plane::op_Inequality"},
    .tags = {"plane", "construction", "distance", "value-semantics", "formatting"},
})
