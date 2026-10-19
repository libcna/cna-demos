// SPDX-License-Identifier: MS-PL
// Vector3::Distance and DistanceSquared for range checks.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3Distance(CnaDemos::DemoContext& ctx)
    {
        const Vector3 player(0.0f, 0.0f, 0.0f);
        const Vector3 enemy(2.0f, 3.0f, 6.0f);

        const float distance = Vector3::Distance(player, enemy);
        const float distanceSquared = Vector3::DistanceSquared(player, enemy);
        ctx.Print("Distance        = {}", distance);
        ctx.Print("DistanceSquared = {}", distanceSquared);
        ctx.Check(distance == 7.0f, "the distance to (2, 3, 6) is 7");
        ctx.Check(distanceSquared == 49.0f, "the squared distance is 49");

        float distanceOutput = 0.0f;
        Vector3::Distance(player, enemy, distanceOutput);
        ctx.Check(distanceOutput == distance, "the output Distance form agrees for ordinary points");

        // A range test can compare squared values and skip the square root.
        const float range = 10.0f;
        const bool inRange = Vector3::DistanceSquared(player, enemy) <= range * range;
        ctx.Print("enemy within {} units: {}", range, inRange ? "yes" : "no");
        ctx.Check(inRange, "the enemy is within range");

        const Vector3 huge(1e20f, 0.0f, 0.0f);
        float hugeOutput = 0.0f;
        Vector3::Distance(Vector3::Zero, huge, hugeOutput);
        ctx.Check(std::isfinite(Vector3::Distance(Vector3::Zero, huge)) &&
                      std::abs((Vector3::Distance(Vector3::Zero, huge) / 1e20f) - 1.0f) < 1e-5f &&
                      hugeOutput == Vector3::Distance(Vector3::Zero, huge),
                  "XNA keeps value and output Distance finite across a huge displacement");
        ctx.Check(std::isinf(Vector3::DistanceSquared(Vector3::Zero, huge)),
                  "DistanceSquared exposes the huge displacement's overflow");

        const Vector3 tiny(1e-30f, 0.0f, 0.0f);
        ctx.Check(std::isfinite(Vector3::Distance(Vector3::Zero, tiny)) &&
                      std::abs((Vector3::Distance(Vector3::Zero, tiny) / 1e-30f) - 1.0f) < 1e-5f,
                  "XNA keeps Distance finite across a tiny displacement");
        ctx.Check(Vector3::DistanceSquared(Vector3::Zero, tiny) == 0.0f,
                  "DistanceSquared exposes the tiny displacement's underflow");
    }
}

CNA_DEMOS_CONSOLE(Vector3Distance, {
    .id = "math.vector3.distance",
    .title = "Distance between points",
    .description = "Compares Vector3 Distance and DistanceSquared for ordinary range checks and extreme finite "
                   "displacements through value and output forms.",
    .apis = {"Vector3::Distance", "Vector3::DistanceSquared"},
    .tags = {"vector", "range", "precision"},
})
