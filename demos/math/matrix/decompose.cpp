// SPDX-License-Identifier: MS-PL
// Recovering scale, rotation and translation with Matrix::Decompose.
#include <array>
#include <cmath>

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

        const std::array<Quaternion, 3> quarterTurns = {
            Quaternion::CreateFromAxisAngle(Vector3::UnitX, MathHelper::PiOver2),
            Quaternion::CreateFromAxisAngle(Vector3::UnitY, MathHelper::PiOver2),
            Quaternion::CreateFromAxisAngle(Vector3::UnitZ, MathHelper::PiOver2),
        };
        bool quarterTurnsRecovered = true;
        for (const Quaternion& expected : quarterTurns)
        {
            Vector3 quarterScale;
            Quaternion quarterRotation;
            Vector3 quarterTranslation;
            const bool quarterOk = Matrix::CreateFromQuaternion(expected).Decompose(
                quarterScale, quarterRotation, quarterTranslation);
            quarterTurnsRecovered = quarterTurnsRecovered && quarterOk &&
                Vector3::Distance(quarterScale, Vector3::One) < 1e-5f &&
                CnaDemos::RotationDifference(quarterRotation, expected) < 1e-5f &&
                quarterTranslation == Vector3::Zero;
        }
        ctx.Check(quarterTurnsRecovered,
                  "axis-aligned quarter turns retain their exact orientation and unit scale");

        // XNA assigns the negative sign to Z for this reflection. The signed scale and returned
        // rotation must therefore rebuild the original matrix, while the determinant remains the
        // direct way to detect its reversed handedness.
        const Matrix mirrored = Matrix::CreateScale(-2.0f, 3.0f, 4.0f) * Matrix::CreateRotationY(0.7f) *
                                Matrix::CreateTranslation(7.0f, 8.0f, 9.0f);
        Vector3 mirroredScale;
        Quaternion mirroredRotation;
        Vector3 mirroredTranslation;
        const bool mirroredOk = mirrored.Decompose(mirroredScale, mirroredRotation, mirroredTranslation);
        ctx.Check(mirroredOk && Vector3::Distance(mirroredScale, Vector3(2.0f, 3.0f, -4.0f)) < 1e-4f &&
                      mirroredTranslation == Vector3(7.0f, 8.0f, 9.0f),
                  "XNA preserves a reflection as one negative scale component");
        const Matrix mirroredRebuilt = Matrix::CreateScale(mirroredScale) *
                                       Matrix::CreateFromQuaternion(mirroredRotation) *
                                       Matrix::CreateTranslation(mirroredTranslation);
        ctx.CheckNear(CnaDemos::MaxDifference(mirroredRebuilt, mirrored), 0.0, 1e-4,
                      "the reflected decomposition recomposes the original matrix");
        ctx.Check(mirrored.Determinant() < 0.0f,
                  "the determinant directly identifies the reflected transform");

        // Genuine Windows XNA 4.0 treats a zero scale axis as a completed decomposition: it
        // returns the scale and translation, reports success and publishes a NaN quaternion.
        Vector3 flatScale;
        Quaternion flatRotation;
        Vector3 flatTranslation;
        const bool flatOk = (Matrix::CreateScale(1.0f, 0.0f, 1.0f) *
                             Matrix::CreateTranslation(7.0f, 8.0f, 9.0f))
                                .Decompose(flatScale, flatRotation, flatTranslation);
        ctx.Check(flatOk && flatScale == Vector3(1.0f, 0.0f, 1.0f) &&
                      std::isnan(flatRotation.X) && std::isnan(flatRotation.Y) &&
                      std::isnan(flatRotation.Z) && std::isnan(flatRotation.W) &&
                      flatTranslation == Vector3(7.0f, 8.0f, 9.0f),
                  "XNA reports a flattened transform with an undefined NaN rotation");
    }
}

CNA_DEMOS_CONSOLE(MatrixDecompose, {
    .id = "math.matrix.decompose",
    .title = "Decompose a matrix",
    .description = "Splits ordinary, quarter-turn, reflected and flattened world matrices into scale, rotation and "
                   "translation, including XNA's signed-reflection and undefined-rotation boundaries.",
    .apis = {"Matrix::Decompose", "Matrix::Determinant", "Matrix::CreateFromQuaternion",
             "Quaternion::CreateFromYawPitchRoll"},
    .tags = {"matrix", "decompose", "animation"},
    .restrictions = "CNA-DEMOS-ISSUE-115 records mirrored and zero-scale results that differ from Windows XNA 4.0.",
    .knownIssue = "CNA-DEMOS-ISSUE-115",
})
