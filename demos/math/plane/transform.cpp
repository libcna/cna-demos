// SPDX-License-Identifier: MS-PL
// Plane matrix/quaternion transforms and all normalization forms.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    bool NearPlane(const Plane& actual, const Plane& expected, double epsilon)
    {
        return Vector3::Distance(actual.Normal, expected.Normal) <= epsilon &&
               std::abs(actual.D - expected.D) <= epsilon;
    }

    void PlaneTransform(CnaDemos::DemoContext& ctx)
    {
        const Plane ground(Vector3::Up, 0.0f);
        const Matrix translation = Matrix::CreateTranslation(0.0f, 5.0f, 0.0f);
        const Plane raised = Plane::Transform(ground, translation);
        ctx.Print("ground raised by 5 = {}", raised.ToString());
        ctx.Check(NearPlane(raised, Plane(Vector3::Up, -5.0f), 1e-6) &&
                      raised.DotCoordinate(Vector3(3.0f, 5.0f, -2.0f)) == 0.0f,
                  "matrix transformation moves the ground plane to height five");

        Plane matrixOutput(Vector3::Zero, 99.0f);
        Plane::Transform(ground, translation, matrixOutput);
        Plane matrixAlias = ground;
        Plane::Transform(matrixAlias, translation, matrixAlias);
        ctx.Check(matrixOutput == raised && matrixAlias == raised,
                  "matrix output transformation matches the value form and permits input aliasing");

        const Plane coefficients(Vector3(1.0f, 2.0f, 3.0f), 4.0f);
        const Plane nonuniform = Plane::Transform(coefficients, Matrix::CreateScale(2.0f, 3.0f, 4.0f));
        ctx.Check(NearPlane(nonuniform, Plane(0.5f, 2.0f / 3.0f, 0.75f, 4.0f), 1e-6),
                  "matrix transformation applies inverse transpose under nonuniform scale");

        const Quaternion quarterTurn =
            Quaternion::CreateFromAxisAngle(Vector3::UnitZ, MathHelper::PiOver2);
        const Plane wall = Plane::Transform(ground, quarterTurn);
        ctx.Print("ground rotated 90 about Z = {}", wall.ToString());
        ctx.Check(Vector3::Distance(wall.Normal, -Vector3::UnitX) <= 1e-6 && wall.D == 0.0f,
                  "quaternion transformation rotates the normal and preserves D");

        Plane quaternionOutput(Vector3::Zero, 99.0f);
        Plane::Transform(ground, quarterTurn, quaternionOutput);
        Plane quaternionAlias = ground;
        Plane::Transform(quaternionAlias, quarterTurn, quaternionAlias);
        ctx.Check(NearPlane(quaternionOutput, wall, 1e-6) && NearPlane(quaternionAlias, wall, 1e-6),
                  "quaternion output transformation matches the value form and permits input aliasing");

        Plane member(0.0f, 2.0f, 0.0f, -4.0f);
        member.Normalize();
        const Plane input(0.0f, 4.0f, 0.0f, -8.0f);
        const Plane value = Plane::Normalize(input);
        Plane output(Vector3::Zero, 99.0f);
        Plane::Normalize(input, output);
        Plane alias = input;
        Plane::Normalize(alias, alias);
        const Plane expected(Vector3::Up, -2.0f);
        ctx.Check(member == expected && value == expected && output == expected && alias == expected &&
                      input == Plane(0.0f, 4.0f, 0.0f, -8.0f),
                  "member, value, output and aliased normalization agree without changing the source");

        Plane zeroMember;
        zeroMember.Normalize();
        const Plane zeroValue = Plane::Normalize(Plane());
        ctx.Check(std::isnan(zeroMember.Normal.X) && std::isnan(zeroMember.Normal.Y) &&
                      std::isnan(zeroMember.Normal.Z) && std::isnan(zeroMember.D) &&
                      std::isnan(zeroValue.Normal.X) && std::isnan(zeroValue.Normal.Y) &&
                      std::isnan(zeroValue.Normal.Z) && std::isnan(zeroValue.D),
                  "normalizing the zero plane follows XNA by producing four NaN coefficients");
    }
}

CNA_DEMOS_CONSOLE(PlaneTransform, {
    .id = "math.plane.transform",
    .title = "Transform and normalize a plane",
    .description = "Transforms Planes by translation, nonuniform scale and quaternion rotation through value/output "
                   "forms with aliasing, then exercises every normalization form and the zero boundary.",
    .apis = {"Plane::Transform", "Plane::Normalize", "Plane::Plane"},
    .tags = {"plane", "transform", "normalization", "aliasing"},
})
