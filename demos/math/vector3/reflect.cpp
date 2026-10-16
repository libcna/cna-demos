// SPDX-License-Identifier: MS-PL
// Bouncing a direction off a surface with Vector3::Reflect.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3Reflect(CnaDemos::DemoContext& ctx)
    {
        // A ball moving down and to the right hits the floor, whose normal is Up.
        const Vector3 velocity(3.0f, -4.0f, 0.0f);
        const Vector3 bounced = Vector3::Reflect(velocity, Vector3::Up);
        ctx.Print("Reflect({}, Up) = {}", velocity.ToString(), bounced.ToString());

        // The component along the normal flips; the tangential component is kept.
        ctx.Check(bounced == Vector3(3.0f, 4.0f, 0.0f), "the vertical component flips");
        ctx.CheckNear(bounced.Length(), velocity.Length(), 1e-6, "reflection preserves speed");

        // The normal must be unit length; here a 45-degree wall.
        const Vector3 wallNormal = Vector3::Normalize(Vector3(-1.0f, 1.0f, 0.0f));
        const Vector3 offWall = Vector3::Reflect(Vector3::UnitX, wallNormal);
        ctx.Print("Reflect(UnitX, normalize(-1, 1, 0)) = {}", offWall.ToString());
        ctx.CheckNear(Vector3::Distance(offWall, Vector3::UnitY), 0.0, 1e-6, "a 45-degree wall turns +X into +Y");

        const Vector3 incident(3.0f, 4.0f, 5.0f);
        ctx.Check(Vector3::Reflect(incident, Vector3::Zero) == incident,
                  "a zero normal leaves the reflected vector unchanged");

        const Vector3 scaledNormal(0.0f, -2.0f, 0.0f);
        const Vector3 scaledReflection = Vector3::Reflect(incident, scaledNormal);
        Vector3 reflectionOutput;
        Vector3::Reflect(incident, scaledNormal, reflectionOutput);
        Vector3 reflectionAlias = scaledNormal;
        Vector3::Reflect(incident, reflectionAlias, reflectionAlias);
        ctx.Check(scaledReflection == Vector3(3.0f, -28.0f, 5.0f) &&
                      reflectionOutput == scaledReflection && reflectionAlias == scaledReflection,
                  "Reflect does not normalize its normal and its output may alias that normal");
    }
}

CNA_DEMOS_CONSOLE(Vector3Reflect, {
    .id = "math.vector3.reflect",
    .title = "Reflect a vector",
    .description = "Bounces a velocity off a surface with Vector3::Reflect, which flips the component along "
                   "the (unit) surface normal.",
    .apis = {"Vector3::Reflect"},
    .tags = {"bounce", "physics"},
})
