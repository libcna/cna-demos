// SPDX-License-Identifier: MS-PL
// Vector3::Distance and DistanceSquared for range checks.
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

        // A range test can compare squared values and skip the square root.
        const float range = 10.0f;
        const bool inRange = Vector3::DistanceSquared(player, enemy) <= range * range;
        ctx.Print("enemy within {} units: {}", range, inRange ? "yes" : "no");
        ctx.Check(inRange, "the enemy is within range");
    }
}

CNA_DEMOS_CONSOLE(Vector3Distance, {
    .id = "math.vector3.distance",
    .title = "Distance between points",
    .description = "Measures the distance between two points with Vector3::Distance and compares squared "
                   "distances with Vector3::DistanceSquared to avoid a square root.",
    .apis = {"Vector3::Distance", "Vector3::DistanceSquared"},
    .tags = {"vector", "range"},
})
