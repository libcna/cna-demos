// SPDX-License-Identifier: MS-PL
// Recovering scale, rotation and translation with Matrix::Decompose.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixDecompose(CnaDemos::DemoContext& ctx)
    {
        const Quaternion originalRotation = Quaternion::CreateFromYawPitchRoll(0.5f, 0.25f, 0.0f);
        const Matrix world = Matrix::CreateScale(2.0f, 3.0f, 4.0f) * Matrix::CreateFromQuaternion(originalRotation)
                             * Matrix::CreateTranslation(7.0f, 8.0f, 9.0f);

        Vector3 scale;
        Quaternion rotation;
        Vector3 translation;
        const bool ok = world.Decompose(scale, rotation, translation);

        ctx.Print("scale       = {}", scale.ToString());
        ctx.Print("rotation    = {}", rotation.ToString());
        ctx.Print("translation = {}", translation.ToString());
        ctx.Check(ok, "Decompose succeeds for a scale-rotate-translate matrix");
        ctx.CheckNear(Vector3::Distance(scale, Vector3(2.0f, 3.0f, 4.0f)), 0.0, 1e-4, "the scale is recovered");
        ctx.CheckNear(CnaDemos::RotationDifference(rotation, originalRotation), 0.0, 1e-5, "the rotation is recovered");
        ctx.Check(translation == Vector3(7.0f, 8.0f, 9.0f), "the translation is the matrix's fourth row");

        // A matrix that collapses an axis cannot be decomposed.
        Vector3 s;
        Quaternion r;
        Vector3 t;
        ctx.Check(!Matrix::CreateScale(1.0f, 0.0f, 1.0f).Decompose(s, r, t), "a flattened matrix reports failure");
    }
}

CNA_DEMOS_CONSOLE(MatrixDecompose, {
    .id = "math.matrix.decompose",
    .title = "Decompose a matrix",
    .description = "Splits a world matrix back into scale, rotation (as a Quaternion) and translation with "
                   "Matrix::Decompose, which reports failure for degenerate matrices.",
    .apis = {"Matrix::Decompose", "Matrix::CreateFromQuaternion", "Quaternion::CreateFromYawPitchRoll"},
    .tags = {"matrix", "decompose", "animation"},
})
