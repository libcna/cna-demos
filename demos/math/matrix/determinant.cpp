// SPDX-License-Identifier: MS-PL
// What Matrix::Determinant tells about a transform.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Plane.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixDeterminant(CnaDemos::DemoContext& ctx)
    {
        // The determinant is the factor by which a transform scales volume.
        const float scaled = Matrix::CreateScale(2.0f, 3.0f, 4.0f).Determinant();
        const float rotated = Matrix::CreateRotationY(1.2f).Determinant();
        const float mirrored = Matrix::CreateReflection(Plane(Vector3::Up, 0.0f)).Determinant();
        const float flattened = Matrix::CreateScale(1.0f, 0.0f, 1.0f).Determinant();

        ctx.Print("scale (2, 3, 4)  : {}", scaled);
        ctx.Print("rotation         : {}", rotated);
        ctx.Print("mirror           : {}", mirrored);
        ctx.Print("flatten Y to 0   : {}", flattened);

        ctx.CheckNear(scaled, 24.0, 1e-5, "scaling by 2, 3, 4 multiplies volume by 24");
        ctx.CheckNear(rotated, 1.0, 1e-5, "a rotation keeps volume");
        ctx.Check(mirrored < 0.0f, "a reflection flips handedness (negative determinant)");
        ctx.Check(flattened == 0.0f, "a flattening transform has determinant 0 and no inverse");
    }
}

CNA_DEMOS_CONSOLE(MatrixDeterminant, {
    .id = "math.matrix.determinant",
    .title = "Matrix determinant",
    .description = "Uses Matrix::Determinant to read how a transform scales volume, whether it mirrors, and "
                   "whether it can be inverted.",
    .apis = {"Matrix::Determinant", "Matrix::CreateReflection", "Matrix::CreateScale"},
    .tags = {"matrix", "volume", "handedness"},
})
