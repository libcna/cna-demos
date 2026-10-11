// SPDX-License-Identifier: MS-PL
// Read/write typed effect parameters; inspect array shape rather than assuming it.
#include <vector>
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectParameter.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectParameterCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "System/InvalidCastException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ParameterValues final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            auto& device = getGraphicsDeviceProperty();
            BasicEffect effect(device);
            auto& parameters = effect.getParametersProperty();
            EffectParameter* power = parameters["SpecularPower"];
            ctx.Check(power->getParameterTypeProperty() == EffectParameterType::Single &&
                      power->getParameterClassProperty() == EffectParameterClass::Scalar,
                      "SpecularPower is a Single scalar");
            power->SetValue(24.0f);
            ctx.CheckNear(power->GetValueSingle(), 24.0, 1e-6, "Single round trip");
            parameters["ShaderIndex"]->SetValue(3);
            ctx.Check(parameters["ShaderIndex"]->GetValueInt32() == 3, "Int32 round trip");

            EffectParameter* diffuse = parameters["DiffuseColor"];
            const Vector4 tint(0.2f, 0.4f, 0.6f, 0.8f);
            diffuse->SetValue(tint);
            ctx.Check(diffuse->GetValueVector4() == tint, "Vector4 round trip");
            ctx.Check(diffuse->getRowCountProperty() == 1 && diffuse->getColumnCountProperty() == 4,
                      "DiffuseColor has four columns in one row");

            const Matrix transform = Matrix::CreateRotationY(0.3f) * Matrix::CreateTranslation(2.0f, 3.0f, 4.0f);
            EffectParameter* world = parameters["World"];
            world->SetValue(transform);
            ctx.CheckNear(CnaDemos::MaxDifference(world->GetValueMatrix(), transform), 0.0, 1e-6,
                          "matrix round trip preserves the translation and rotation");
            ctx.CheckNear(CnaDemos::MaxDifference(world->GetValueMatrixTranspose(), Matrix::Transpose(transform)),
                          0.0, 1e-6, "transpose getter transposes the stored matrix");
            world->SetValueTranspose(transform);
            ctx.CheckNear(CnaDemos::MaxDifference(world->GetValueMatrixTranspose(), transform), 0.0, 1e-6,
                          "transpose setter and getter round trip");

            Texture2D texture(device, 1, 1);
            parameters["Texture"]->SetValue(&texture);
            ctx.Check(parameters["Texture"]->GetValueTexture2D() == &texture, "texture parameter retains the supplied object");
            parameters["Texture"]->SetValue(static_cast<Texture2D*>(nullptr));
            // These are parameter-storage checks. Applying a stock effect may overwrite values
            // from its cached properties, as parameters_and_techniques demonstrates.

            SkinnedEffect skinned(device);
            EffectParameter* bones = skinned.getParametersProperty()["Bones"];
            const std::vector<Matrix> transforms{Matrix::getIdentityProperty(), transform};
            bones->SetValue(transforms);
            const auto values = bones->GetValueMatrixArray(2);
            const auto transposed = bones->GetValueMatrixTransposeArray(2);
            ctx.Check(values.size() == 2 && transposed.size() == 2, "both array getters return two matrices");
            if (values.size() == 2 && transposed.size() == 2)
                for (std::size_t i = 0; i < transforms.size(); ++i)
                {
                    ctx.CheckNear(CnaDemos::MaxDifference(values[i], transforms[i]), 0.0, 1e-6,
                                  "each bone matrix round trips independently");
                    ctx.CheckNear(CnaDemos::MaxDifference(transposed[i], Matrix::Transpose(transforms[i])),
                                  0.0, 1e-6, "each bone transpose matches");
                }

            // Keep incompatible behavior visible: these expectations must not be changed to
            // accept CNA's current stock-effect storage implementation.
            bool rejected = false;
            try { ctx.Print("Vector4 read as Single returned {}", diffuse->GetValueSingle()); }
            catch (const System::InvalidCastException&) { rejected = true; }
            ctx.Check(rejected, "a Vector4 parameter rejects a scalar getter (ISSUE-034)");
            ctx.Print("Bones: {} elements, {} rows, {} columns", bones->getElementsProperty().getCountProperty(),
                      bones->getRowCountProperty(), bones->getColumnCountProperty());
            // Do not infer reflected rows/columns from the HLSL spelling: the stored FNA
            // fixture reports 3x4 for the shader's float4x3 declaration. The array is unambiguous.
            ctx.Check(bones->getElementsProperty().getCountProperty() == SkinnedEffect::MaxBones,
                      "Bones reflects 72 individual matrix elements (ISSUE-035)");
        }
    };
}

CNA_DEMOS_GAME(ParameterValues, {
    .id = "graphics.effect.parameter_values",
    .title = "Typed effect parameter values",
    .description = "Writes and reads scalar, vector, matrix, texture and bone-array effect parameters, including "
                   "transpose access and reflected shape, and exposes stock-effect compatibility gaps.",
    .apis = {"Effect::Parameters", "EffectParameter::ParameterType", "EffectParameter::ParameterClass",
             "EffectParameter::RowCount", "EffectParameter::ColumnCount", "EffectParameter::Elements",
             "EffectParameter::SetValue", "EffectParameter::SetValueTranspose", "EffectParameter::GetValueSingle",
             "EffectParameter::GetValueInt32", "EffectParameter::GetValueVector4", "EffectParameter::GetValueMatrix",
             "EffectParameter::GetValueMatrixTranspose", "EffectParameter::GetValueTexture2D",
             "EffectParameter::GetValueMatrixArray", "EffectParameter::GetValueMatrixTransposeArray"},
    .tags = {"effect", "parameters", "reflection", "matrix"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .restrictions = "CNA-DEMOS-ISSUE-034: stock Vector4 accepts scalar reads; ISSUE-035: Bones has no reflected array elements.",
    .knownIssue = "CNA-DEMOS-ISSUE-034",
})
