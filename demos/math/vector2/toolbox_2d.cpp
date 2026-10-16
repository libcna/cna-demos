// SPDX-License-Identifier: MS-PL
// Vector2's static helpers on everyday 2D game tasks: bounce, keep on screen, range checks, paths,
// easing and a hit test. (The Vector3 demos show each helper on its own; these are the same calls
// in two dimensions.)
#include <any>
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    bool Near(Vector2 a, Vector2 b) { return Vector2::Distance(a, b) < 1e-4f; }

    void Toolbox2D(CnaDemos::DemoContext& ctx)
    {
        // A ball moving down-right hits the floor (normal pointing up, screen y grows downwards).
        const Vector2 velocity(3.0f, 4.0f);
        const Vector2 bounced = Vector2::Reflect(velocity, Vector2(0.0f, -1.0f));
        ctx.Print("Reflect({}, floor) = {}", velocity.ToString(), bounced.ToString());
        ctx.Check(Near(bounced, Vector2(3.0f, -4.0f)), "Reflect bounces the ball off the floor: (3, -4)");

        // Keep a sprite on an 800x480 screen.
        const Vector2 kept = Vector2::Clamp(Vector2(-25.0f, 530.0f), Vector2::Zero, Vector2(800.0f, 480.0f));
        ctx.Check(Near(kept, Vector2(0.0f, 480.0f)), "Clamp keeps (-25, 530) on screen: (0, 480)");

        // Range checks: DistanceSquared avoids the square root when comparing with a radius.
        const Vector2 player(100.0f, 100.0f), enemy(130.0f, 140.0f);
        const float radius = 60.0f;
        ctx.Print("Distance {} (squared {})", Vector2::Distance(player, enemy), Vector2::DistanceSquared(player, enemy));
        ctx.Check(Vector2::Distance(player, enemy) == 50.0f &&
                      Vector2::DistanceSquared(player, enemy) < radius * radius,
                  "the enemy 50 px away is in range (DistanceSquared 2500 < 60^2)");

        // In front or behind: the sign of Dot with the facing direction.
        const Vector2 facing = Vector2::Normalize(Vector2(1.0f, 0.0f));
        ctx.Check(Vector2::Dot(facing, Vector2::Subtract(enemy, player)) > 0.0f &&
                      Vector2::Dot(facing, Vector2::Negate(Vector2::Subtract(enemy, player))) < 0.0f,
                  "Dot with the facing direction: the enemy is in front, its mirror image behind");

        // Easing a menu panel in: Lerp moves evenly, SmoothStep starts and ends slowly.
        const Vector2 from(-200.0f, 40.0f), to(40.0f, 40.0f);
        const Vector2 linear = Vector2::Lerp(from, to, 0.1f), eased = Vector2::SmoothStep(from, to, 0.1f);
        ctx.Print("10% of the way: Lerp {}, SmoothStep {}", linear.ToString(), eased.ToString());
        ctx.Check(eased.X < linear.X, "SmoothStep has moved less than Lerp at 10 %");

        const Vector2 interpolationA(2.0f, -4.0f), interpolationB(10.0f, 8.0f);
        Vector2 interpolationOutput;
        Vector2::Lerp(interpolationA, interpolationB, 1.5f, interpolationOutput);
        ctx.Check(Vector2::Lerp(interpolationA, interpolationB, -0.5f) == Vector2(-2.0f, -10.0f) &&
                      Vector2::Lerp(interpolationA, interpolationB, 1.5f) == Vector2(14.0f, 14.0f) &&
                      interpolationOutput == Vector2(14.0f, 14.0f),
                  "Lerp extrapolates below zero and above one, including its output form");
        Vector2::SmoothStep(interpolationA, interpolationB, -0.5f, interpolationOutput);
        ctx.Check(Vector2::SmoothStep(interpolationA, interpolationB, -0.5f) == interpolationA &&
                      Vector2::SmoothStep(interpolationA, interpolationB, 1.5f) == interpolationB &&
                      interpolationOutput == interpolationA,
                  "SmoothStep clamps amounts outside zero to one, including its output form");
        const Vector2 nanLerp = Vector2::Lerp(interpolationA, interpolationB, NAN);
        const Vector2 nanSmooth = Vector2::SmoothStep(interpolationA, interpolationB, NAN);
        ctx.Check(std::isnan(nanLerp.X) && std::isnan(nanLerp.Y) &&
                      std::isnan(nanSmooth.X) && std::isnan(nanSmooth.Y),
                  "Lerp and SmoothStep propagate a NaN amount to both components");

        // A path through waypoints: CatmullRom passes through the middle two points.
        const Vector2 p0(0.0f, 0.0f), p1(100.0f, 0.0f), p2(200.0f, 100.0f), p3(300.0f, 100.0f);
        ctx.Check(Near(Vector2::CatmullRom(p0, p1, p2, p3, 0.0f), p1) && Near(Vector2::CatmullRom(p0, p1, p2, p3, 1.0f), p2),
                  "CatmullRom passes through p1 at 0 and p2 at 1");
        // Hermite with explicit tangents, e.g. a jump arc leaving and landing horizontally.
        const Vector2 arcMiddle =
            Vector2::Hermite(Vector2(0.0f, 0.0f), Vector2(100.0f, 0.0f), Vector2(100.0f, 0.0f), Vector2(100.0f, 0.0f), 0.5f);
        ctx.Check(Near(arcMiddle, Vector2(50.0f, 0.0f)), "Hermite with matching tangents is straight: (50, 0) at 0.5");

        // A hit test: a point is inside a triangle when its barycentric weights are all positive.
        const Vector2 a(0.0f, 0.0f), b(100.0f, 0.0f), c(0.0f, 100.0f);
        const Vector2 inside = Vector2::Barycentric(a, b, c, 0.25f, 0.25f);
        ctx.Check(Near(inside, Vector2(25.0f, 25.0f)), "Barycentric(a, b, c, 0.25, 0.25) = (25, 25), inside the triangle");

        const Vector2 interpolationC(-6.0f, 12.0f), interpolationD(14.0f, -20.0f);
        const Vector2 barycentricOutside =
            Vector2::Barycentric(interpolationA, interpolationB, interpolationC, -0.5f, 1.5f);
        Vector2::Barycentric(interpolationA, interpolationB, interpolationC, -0.5f, 1.5f,
                             interpolationOutput);
        ctx.Check(barycentricOutside == Vector2(-14.0f, 14.0f) &&
                      interpolationOutput == barycentricOutside,
                  "Barycentric accepts weights outside the triangle and its output form agrees");

        const Vector2 catmullBefore =
            Vector2::CatmullRom(interpolationA, interpolationB, interpolationC, interpolationD, -0.5f);
        const Vector2 catmullAfter =
            Vector2::CatmullRom(interpolationA, interpolationB, interpolationC, interpolationD, 1.5f);
        Vector2::CatmullRom(interpolationA, interpolationB, interpolationC, interpolationD, 1.5f,
                            interpolationOutput);
        ctx.Check(catmullBefore == Vector2(-2.25f, 8.25f) &&
                      catmullAfter == Vector2(10.75f, -4.75f) && interpolationOutput == catmullAfter,
                  "CatmullRom extrapolates before and after its middle segment");

        const Vector2 tangentA(3.0f, -5.0f), tangentB(-7.0f, 11.0f);
        const Vector2 hermiteBefore =
            Vector2::Hermite(interpolationA, tangentA, interpolationB, tangentB, -0.5f);
        const Vector2 hermiteAfter =
            Vector2::Hermite(interpolationA, tangentA, interpolationB, tangentB, 1.5f);
        Vector2::Hermite(interpolationA, tangentA, interpolationB, tangentB, 1.5f,
                         interpolationOutput);
        ctx.Check(hermiteBefore == Vector2(9.25f, 9.5f) &&
                      hermiteAfter == Vector2(-4.75f, 6.5f) && interpolationOutput == hermiteAfter,
                  "Hermite extrapolates with its endpoint tangents and its output form agrees");

        // The bounding box of some points: Min and Max per component.
        const Vector2 points[] = {Vector2(30.0f, 70.0f), Vector2(-10.0f, 20.0f), Vector2(50.0f, -5.0f)};
        Vector2 low = points[0], high = points[0];
        for (const Vector2& p : points)
        {
            low = Vector2::Min(low, p);
            high = Vector2::Max(high, p);
        }
        ctx.Check(Near(low, Vector2(-10.0f, -5.0f)) && Near(high, Vector2(50.0f, 70.0f)),
                  "Min and Max give the points' bounding box: (-10, -5) to (50, 70)");

        // The static arithmetic forms, for code that prefers named calls to operators.
        ctx.Check(Near(Vector2::Add(a, b), a + b) && Near(Vector2::Multiply(b, 0.5f), b * 0.5f) &&
                      Near(Vector2::Divide(b, 4.0f), b / 4.0f) && Near(Vector2::Subtract(b, a), b - a) &&
                      Near(Vector2::Negate(b), -b),
                  "arithmetic helpers equal their binary and unary operators");

        const Vector2 samePlayer(100.0f, 100.0f);
        const Vector2 movedPlayer(101.0f, 100.0f);
        ctx.Check(player.Equals(samePlayer) && player.Equals(std::any{samePlayer}) &&
                      !player.Equals(std::any{42}) && !player.Equals(std::any{}),
                  "typed and boxed equality accept only the same two components");
        ctx.Check(player == samePlayer && player != movedPlayer &&
                      player.GetHashCode() == samePlayer.GetHashCode(),
                  "Vector2 equality operators and hashes follow both components");

        const Vector2 signedZeroUnitX(1.0f, -0.0f);
        const Vector2 signedZeroOrigin(-0.0f, 0.0f);
        ctx.Check(Vector2::UnitX.Equals(signedZeroUnitX) &&
                      Vector2::UnitX.Equals(std::any{signedZeroUnitX}) &&
                      Vector2::UnitX == signedZeroUnitX && !(Vector2::UnitX != signedZeroUnitX) &&
                      Vector2::Zero.Equals(signedZeroOrigin) && Vector2::Zero == signedZeroOrigin,
                  "positive and negative zero compare equal through every Vector2 equality form");
        ctx.Check(Vector2::UnitX.GetHashCode() == signedZeroUnitX.GetHashCode() &&
                      Vector2::Zero.GetHashCode() == signedZeroOrigin.GetHashCode(),
                  "XNA gives equal signed-zero Vector2 values equal hash codes");
    }
}

CNA_DEMOS_CONSOLE(Toolbox2D, {
    .id = "math.vector2.toolbox_2d",
    .title = "Vector2 helpers for 2D games",
    .description = "Vector2's static helpers on 2D game tasks, including interpolation versus extrapolation, "
                   "reflection, bounds, distance, paths, arithmetic and value identity.",
    .apis = {"Vector2::Reflect", "Vector2::Clamp", "Vector2::Distance", "Vector2::DistanceSquared", "Vector2::Dot",
             "Vector2::Normalize", "Vector2::Negate", "Vector2::Subtract", "Vector2::Lerp", "Vector2::SmoothStep",
             "Vector2::CatmullRom", "Vector2::Hermite", "Vector2::Barycentric", "Vector2::Min", "Vector2::Max",
             "Vector2::Add", "Vector2::Multiply", "Vector2::Divide", "Vector2::Equals", "Vector2::GetHashCode",
             "Vector2::op_Addition", "Vector2::op_Division", "Vector2::op_Equality", "Vector2::op_Inequality",
             "Vector2::op_Multiply", "Vector2::op_Subtraction", "Vector2::op_UnaryNegation"},
    .tags = {"vector", "2d", "math", "gameplay"},
    .restrictions = "CNA-DEMOS-ISSUE-120 records unequal hashes for equal signed-zero Vector2 values.",
    .knownIssue = "CNA-DEMOS-ISSUE-120",
})
