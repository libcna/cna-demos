// SPDX-License-Identifier: MS-PL
// Vector3::Dot: projection length, angle between vectors, facing tests.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void Vector3Dot(CnaDemos::DemoContext& ctx)
    {
        // For unit vectors the dot product is the cosine of the angle between them.
        const Vector3 forward = Vector3::Forward;
        const Vector3 diagonal = Vector3::Normalize(Vector3(1.0f, 0.0f, -1.0f));

        const float cosine = Vector3::Dot(forward, diagonal);
        const float degrees = MathHelper::ToDegrees(std::acos(cosine));
        ctx.Print("Dot(Forward, normalize(1, 0, -1)) = {:.4f} -> {:.1f} degrees", cosine, degrees);
        ctx.CheckNear(degrees, 45.0, 1e-3, "the vectors are 45 degrees apart");

        // Sign tells which side: > 0 in front, 0 perpendicular, < 0 behind.
        ctx.Check(Vector3::Dot(Vector3::Forward, Vector3::Right) == 0.0f, "perpendicular vectors have a zero dot product");
        ctx.Check(Vector3::Dot(Vector3::Forward, Vector3::Backward) < 0.0f, "opposite vectors have a negative dot product");

        // With a unit axis, Dot is the length of the projection onto that axis.
        const Vector3 point(3.0f, 4.0f, 5.0f);
        ctx.Print("height of {} above the ground = Dot(point, Up) = {}", point.ToString(), Vector3::Dot(point, Vector3::Up));
        ctx.Check(Vector3::Dot(point, Vector3::Up) == 4.0f, "projection onto Up is the Y component");

        float byReference = 0.0f;
        Vector3::Dot(point, Vector3::One, byReference);
        ctx.Check(byReference == 12.0f, "the by-reference overload writes 3 + 4 + 5");
    }
}

CNA_DEMOS_CONSOLE(Vector3Dot, {
    .id = "math.vector3.dot",
    .title = "Vector3 dot product",
    .description = "Uses Vector3::Dot to measure the angle between directions, decide which side a vector "
                   "points to, and project a point onto an axis.",
    .apis = {"Vector3::Dot", "Vector3::Normalize", "MathHelper::ToDegrees"},
    .tags = {"vector", "angle", "projection"},
})
