// SPDX-License-Identifier: MS-PL
// Keeping an accumulated orientation unit length with Quaternion::Normalize.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void QuaternionNormalize(CnaDemos::DemoContext& ctx)
    {
        // A spaceship turning a little every frame: thousands of multiplications accumulate
        // rounding error, and a non-unit quaternion starts to scale what it rotates.
        const Quaternion perFrame = Quaternion::CreateFromYawPitchRoll(0.013f, 0.007f, 0.011f);
        Quaternion orientation = Quaternion::Identity;
        for (int frame = 0; frame < 100000; ++frame)
        {
            orientation = orientation * perFrame;
        }
        ctx.Print("length after 100000 updates          = {:.7f}", orientation.Length());

        const Quaternion fixed = Quaternion::Normalize(orientation);
        ctx.Print("length after Quaternion::Normalize   = {:.7f}", fixed.Length());
        ctx.CheckNear(fixed.Length(), 1.0, 1e-6, "Normalize restores unit length");

        orientation.Normalize();   // the in-place member form
        ctx.Check(orientation == fixed, "member Normalize() matches the static form");
        ctx.CheckNear(Vector3::Transform(Vector3::UnitX, fixed).Length(), 1.0, 1e-5, "rotating no longer scales vectors");

        const Quaternion raw(1.0f, 2.0f, 3.0f, 4.0f);
        const Quaternion rawNormalized = Quaternion::Normalize(raw);
        Quaternion rawByReference;
        Quaternion::Normalize(raw, rawByReference);
        ctx.Check(rawByReference == rawNormalized && std::abs(rawNormalized.Length() - 1.0f) < 1e-6f,
                  "the output overload matches static normalization of a nonunit value");

        const Quaternion zero;
        const Quaternion zeroStatic = Quaternion::Normalize(zero);
        Quaternion zeroMember = zero;
        zeroMember.Normalize();
        ctx.Check(std::isnan(zeroStatic.X) && std::isnan(zeroStatic.Y) &&
                      std::isnan(zeroStatic.Z) && std::isnan(zeroStatic.W) &&
                      std::isnan(zeroMember.X) && std::isnan(zeroMember.Y) &&
                      std::isnan(zeroMember.Z) && std::isnan(zeroMember.W),
                  "zero normalization exposes four NaNs in static and member forms");

        const Quaternion tiny = Quaternion::Normalize(Quaternion(0.0f, 0.0f, 0.0f, 1e-20f));
        ctx.Print("normalized tiny value = {}", tiny.ToString());
        ctx.Check(tiny.X == 0.0f && tiny.Y == 0.0f && tiny.Z == 0.0f &&
                      std::abs(tiny.W - 1.0f) < 1e-5f,
                  "a tiny representable W component normalizes to identity");

        const Quaternion huge = Quaternion::Normalize(Quaternion(1e20f, 0.0f, 0.0f, 0.0f));
        ctx.Print("normalized huge value = {}", huge.ToString());
        ctx.Check(huge == Quaternion(1.0f, 0.0f, 0.0f, 0.0f),
                  "XNA normalizes a huge finite component without overflowing its squared length");
    }
}

CNA_DEMOS_CONSOLE(QuaternionNormalize, {
    .id = "math.quaternion.normalize",
    .title = "Normalize a quaternion",
    .description = "Normalizes accumulated drift, ordinary nonunit values, zero and extreme finite magnitudes "
                   "through the static, member and output forms.",
    .apis = {"Quaternion::Normalize", "Quaternion::Length", "Quaternion::op_Multiply"},
    .tags = {"quaternion", "precision", "drift"},
    .restrictions = "CNA-DEMOS-ISSUE-118 records overflow while normalizing a huge finite quaternion.",
    .knownIssue = "CNA-DEMOS-ISSUE-118",
})
