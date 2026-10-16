// SPDX-License-Identifier: MS-PL
// Vector4 interpolation boundaries: extrapolation, clamping, NaN and output forms.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using Microsoft::Xna::Framework::Vector4;

namespace
{
    bool AllNaN(const Vector4& value)
    {
        return std::isnan(value.X) && std::isnan(value.Y) &&
               std::isnan(value.Z) && std::isnan(value.W);
    }

    void Vector4Interpolation(CnaDemos::DemoContext& ctx)
    {
        const Vector4 a(2.0f, -4.0f, 1.0f, 6.0f);
        const Vector4 b(10.0f, 8.0f, -3.0f, -2.0f);
        Vector4 output;

        Vector4::Lerp(a, b, 1.5f, output);
        ctx.Check(Vector4::Lerp(a, b, -0.5f) == Vector4(-2.0f, -10.0f, 3.0f, 10.0f) &&
                      Vector4::Lerp(a, b, 1.5f) == Vector4(14.0f, 14.0f, -5.0f, -6.0f) &&
                      output == Vector4(14.0f, 14.0f, -5.0f, -6.0f),
                  "Lerp extrapolates below zero and above one, with matching output form");
        ctx.Check(AllNaN(Vector4::Lerp(a, b, NAN)),
                  "a NaN Lerp amount propagates through all four components");

        Vector4::SmoothStep(a, b, -0.5f, output);
        ctx.Check(Vector4::SmoothStep(a, b, -0.5f) == a &&
                      Vector4::SmoothStep(a, b, 1.5f) == b && output == a,
                  "SmoothStep clamps amounts outside zero to one, including its output form");
        ctx.Check(AllNaN(Vector4::SmoothStep(a, b, NAN)),
                  "a NaN SmoothStep amount propagates through all four components");

        const Vector4 c(-6.0f, 12.0f, 5.0f, -10.0f);
        const Vector4 d(14.0f, -20.0f, -7.0f, 18.0f);
        Vector4::Barycentric(a, b, c, -0.5f, 1.5f, output);
        ctx.Check(Vector4::Barycentric(a, b, c, -0.5f, 1.5f) ==
                          Vector4(-14.0f, 14.0f, 9.0f, -14.0f) &&
                      output == Vector4(-14.0f, 14.0f, 9.0f, -14.0f),
                  "Barycentric accepts outside weights in value and output forms");

        Vector4::CatmullRom(a, b, c, d, 1.5f, output);
        ctx.Check(Vector4::CatmullRom(a, b, c, d, -0.5f) ==
                          Vector4(-2.25f, 8.25f, 3.5f, -4.75f) &&
                      Vector4::CatmullRom(a, b, c, d, 1.5f) ==
                          Vector4(10.75f, -4.75f, -4.5f, 6.25f) &&
                      output == Vector4(10.75f, -4.75f, -4.5f, 6.25f),
                  "CatmullRom extrapolates on both sides, with matching output form");

        const Vector4 tangentA(3.0f, -5.0f, 2.0f, 7.0f);
        const Vector4 tangentB(-7.0f, 11.0f, -6.0f, -9.0f);
        Vector4::Hermite(a, tangentA, b, tangentB, 1.5f, output);
        ctx.Check(Vector4::Hermite(a, tangentA, b, tangentB, -0.5f) ==
                          Vector4(9.25f, 9.5f, -3.0f, -6.5f) &&
                      Vector4::Hermite(a, tangentA, b, tangentB, 1.5f) ==
                          Vector4(-4.75f, 6.5f, -5.0f, -1.5f) &&
                      output == Vector4(-4.75f, 6.5f, -5.0f, -1.5f),
                  "Hermite extrapolates on both sides, with matching output form");
    }
}

CNA_DEMOS_CONSOLE(Vector4Interpolation, {
    .id = "math.vector4.interpolation",
    .title = "Vector4 interpolation boundaries",
    .description = "Compares Vector4 Lerp, SmoothStep, Barycentric, CatmullRom and Hermite outside their usual "
                   "ranges, with NaN and output-overload behavior.",
    .apis = {"Vector4::Lerp", "Vector4::SmoothStep", "Vector4::Barycentric", "Vector4::CatmullRom",
             "Vector4::Hermite"},
    .tags = {"vector", "interpolation", "spline", "precision"},
})
