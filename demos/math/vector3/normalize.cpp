// SPDX-License-Identifier: MS-PL
// Making a unit-length direction: instance Normalize() versus static Vector3::Normalize().
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using Microsoft::Xna::Framework::Vector3;

namespace
{
    void Vector3Normalize(CnaDemos::DemoContext& ctx)
    {
        const Vector3 velocity(3.0f, 0.0f, 4.0f);

        // The static form returns a new vector and leaves its argument alone.
        const Vector3 direction = Vector3::Normalize(velocity);
        ctx.Print("Normalize({}) = {}  (length {})", velocity.ToString(), direction.ToString(), direction.Length());
        ctx.CheckNear(direction.Length(), 1.0, 1e-6, "a normalized vector has length 1");
        ctx.Check(direction == Vector3(0.6f, 0.0f, 0.8f), "the direction of (3, 0, 4) is (0.6, 0, 0.8)");

        // The member form normalizes in place.
        Vector3 inPlace = velocity;
        inPlace.Normalize();
        ctx.Check(inPlace == direction, "member Normalize() gives the same result in place");

        // Direction times speed rebuilds the original vector.
        ctx.Check(direction * velocity.Length() == velocity, "direction * length restores the vector");
    }
}

CNA_DEMOS_CONSOLE(Vector3Normalize, {
    .id = "math.vector3.normalize",
    .title = "Normalize a Vector3",
    .description = "Turns a vector into a unit-length direction with the static Vector3::Normalize and the "
                   "in-place member Normalize().",
    .apis = {"Vector3::Normalize", "Vector3::Length"},
    .tags = {"vector", "direction", "unit-vector"},
})
