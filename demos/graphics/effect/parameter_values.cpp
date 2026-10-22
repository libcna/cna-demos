// SPDX-License-Identifier: MS-PL
// Read/write typed effect parameters; inspect array shape rather than assuming it.
#include <bit>
#include <vector>
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/MathChecks.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectAnnotation.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectParameter.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectParameterCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture3D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
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

            // CNA also exposes the XNA parameter value surface as an independent storage object.
            // Group the scalar/array families here instead of creating one property-list demo per
            // getter; distinct values make ordering, width and conversion observable.
            EffectParameter flags("Flags", "FLAGS", 1, 1, EffectParameterClass::Scalar,
                                  EffectParameterType::Bool);
            flags.SetValue(true);
            const bool booleanScalar = flags.GetValueBoolean();
            flags.SetValue(std::vector<bool>{true, false, true});
            const auto booleans = flags.GetValueBooleanArray(3);
            ctx.Check(booleanScalar && booleans == std::vector<bool>({true, false, true}),
                      "Boolean scalar and ordered array round trip");

            EffectParameter integers("Indices", "INDICES", 1, 1, EffectParameterClass::Scalar,
                                     EffectParameterType::Int32);
            integers.SetValue(std::vector<int>{7, -3, 11});
            EffectParameter singles("Weights", "WEIGHTS", 1, 1, EffectParameterClass::Scalar,
                                    EffectParameterType::Single);
            singles.SetValue(std::vector<float>{0.25f, 0.5f, 1.0f});
            ctx.Check(integers.GetValueInt32Array(3) == std::vector<int>({7, -3, 11}) &&
                          singles.GetValueSingleArray(3) == std::vector<float>({0.25f, 0.5f, 1.0f}),
                      "Int32 and Single arrays retain exact order and values");

            EffectParameter vector2("Offsets", "OFFSETS", 1, 2, EffectParameterClass::Vector,
                                    EffectParameterType::Single);
            const Vector2 offset(2.0f, -4.0f);
            const std::vector<Vector2> offsets{Vector2(1.0f, 2.0f), Vector2(-3.0f, 4.0f)};
            vector2.SetValue(offset);
            const Vector2 offsetScalar = vector2.GetValueVector2();
            vector2.SetValue(offsets);
            ctx.Check(offsetScalar == offset && vector2.GetValueVector2Array(2) == offsets,
                      "Vector2 scalar and ordered array round trip");

            EffectParameter vector3("Directions", "DIRECTIONS", 1, 3, EffectParameterClass::Vector,
                                    EffectParameterType::Single);
            const Vector3 direction(1.0f, -2.0f, 3.0f);
            const std::vector<Vector3> directions{Vector3(1.0f, 2.0f, 3.0f), Vector3(-4.0f, 5.0f, -6.0f)};
            vector3.SetValue(direction);
            const Vector3 directionScalar = vector3.GetValueVector3();
            vector3.SetValue(directions);
            ctx.Check(directionScalar == direction && vector3.GetValueVector3Array(2) == directions,
                      "Vector3 scalar and ordered array round trip");

            EffectParameter vector4("Planes", "PLANES", 1, 4, EffectParameterClass::Vector,
                                    EffectParameterType::Single);
            const std::vector<Vector4> planes{Vector4(1.0f, 0.0f, 0.0f, -2.0f),
                                              Vector4(0.0f, 1.0f, 0.0f, 3.0f)};
            vector4.SetValue(planes);
            ctx.Check(vector4.GetValueVector4Array(2) == planes, "Vector4 array retains both planes");

            EffectParameter rotationParameter("Rotations", "ROTATIONS", 1, 4, EffectParameterClass::Vector,
                                              EffectParameterType::Single);
            const Quaternion rotation(0.1f, 0.2f, 0.3f, 0.9f);
            const std::vector<Quaternion> rotationValues{Quaternion(0.0f, 0.0f, 0.0f, 1.0f),
                                                         Quaternion(0.5f, -0.5f, 0.5f, 0.5f)};
            rotationParameter.SetValue(rotation);
            const Quaternion rotationScalar = rotationParameter.GetValueQuaternion();
            rotationParameter.SetValue(rotationValues);
            ctx.Check(rotationScalar == rotation && rotationParameter.GetValueQuaternionArray(2) == rotationValues,
                      "Quaternion scalar and ordered array round trip as float4 values");

            EffectParameter label("Label", "DISPLAYNAME", 1, 1, EffectParameterClass::Object,
                                  EffectParameterType::String);
            label.SetValue(std::string("polished copper"));
            ctx.Check(label.GetValueString() == "polished copper", "String storage retains authored text");

            EffectParameter genericTexture("GenericTexture", "TEXTURE", 1, 1, EffectParameterClass::Object,
                                           EffectParameterType::Texture);
            genericTexture.SetValue(&texture);
            EffectParameter oneDimensionalTexture("Lookup", "LOOKUP", 1, 1, EffectParameterClass::Object,
                                                  EffectParameterType::Texture1D);
            ctx.Check(genericTexture.GetValueTexture2D() == &texture &&
                          oneDimensionalTexture.getParameterTypeProperty() == EffectParameterType::Texture1D,
                      "generic Texture stores a concrete resource while Texture1D remains explicit reflection metadata");

            Texture3D volume(device, 2, 2, 2, false, SurfaceFormat::Color);
            TextureCube cube(device, 2, false, SurfaceFormat::Color);
            EffectParameter volumeInput("Volume", "VOLUME0", 1, 1, EffectParameterClass::Object,
                                        EffectParameterType::Texture3D);
            EffectParameter cubeInput("Environment", "ENVIRONMENT0", 1, 1, EffectParameterClass::Object,
                                      EffectParameterType::TextureCube);
            volumeInput.SetValue(&volume);
            cubeInput.SetValue(&cube);
            ctx.Check(volumeInput.GetValueTexture3D() == &volume && cubeInput.GetValueTextureCube() == &cube,
                      "Texture3D and TextureCube parameters retain exact resource identities");

            EffectParameter material("Material", "MATERIAL0", 1, 1, EffectParameterClass::Struct,
                                     EffectParameterType::Void);
            material.getStructureMembersProperty().Add(
                EffectParameter("Tint", "COLOR0", 1, 4, EffectParameterClass::Vector,
                                EffectParameterType::Single));
            material.getAnnotationsProperty().Add(
                EffectAnnotation("UiName", "", 1, 1, EffectParameterClass::Object,
                                 EffectParameterType::String, {}, "Surface tint"));
            material.getAnnotationsProperty().Add(
                EffectAnnotation("Enabled", "FEATURE_ENABLED", 1, 1, EffectParameterClass::Scalar,
                                 EffectParameterType::Bool, {std::bit_cast<float>(1)}));
            material.getAnnotationsProperty().Add(
                EffectAnnotation("Priority", "SORT_PRIORITY", 1, 1, EffectParameterClass::Scalar,
                                 EffectParameterType::Int32, {std::bit_cast<float>(42)}));
            material.getAnnotationsProperty().Add(
                EffectAnnotation("Roughness", "ROUGHNESS", 1, 1, EffectParameterClass::Scalar,
                                 EffectParameterType::Single, {0.35f}));
            material.getAnnotationsProperty().Add(
                EffectAnnotation("UvScale", "UV_SCALE", 1, 2, EffectParameterClass::Vector,
                                 EffectParameterType::Single, {2.0f, 0.5f}));
            material.getAnnotationsProperty().Add(
                EffectAnnotation("Axis", "AXIS", 1, 3, EffectParameterClass::Vector,
                                 EffectParameterType::Single, {1.0f, -2.0f, 3.0f}));
            material.getAnnotationsProperty().Add(
                EffectAnnotation("Color", "COLOR", 1, 4, EffectParameterClass::Vector,
                                 EffectParameterType::Single, {0.1f, 0.2f, 0.3f, 0.4f}));
            const Matrix annotationTransform(1.0f, 2.0f, 3.0f, 4.0f,
                                             5.0f, 6.0f, 7.0f, 8.0f,
                                             9.0f, 10.0f, 11.0f, 12.0f,
                                             13.0f, 14.0f, 15.0f, 16.0f);
            material.getAnnotationsProperty().Add(
                EffectAnnotation("Transform", "LOCAL_TO_WORLD", 4, 4, EffectParameterClass::Matrix,
                                 EffectParameterType::Single,
                                 {1.0f, 5.0f, 9.0f, 13.0f, 2.0f, 6.0f, 10.0f, 14.0f,
                                  3.0f, 7.0f, 11.0f, 15.0f, 4.0f, 8.0f, 12.0f, 16.0f}));
            ctx.Check(power->getParameterClassProperty() == EffectParameterClass::Scalar &&
                          diffuse->getParameterClassProperty() == EffectParameterClass::Vector &&
                          world->getParameterClassProperty() == EffectParameterClass::Matrix &&
                          label.getParameterClassProperty() == EffectParameterClass::Object &&
                          material.getParameterClassProperty() == EffectParameterClass::Struct,
                      "loaded and authored parameters distinguish all five parameter classes");
            ctx.Check(flags.getParameterTypeProperty() == EffectParameterType::Bool &&
                          integers.getParameterTypeProperty() == EffectParameterType::Int32 &&
                          singles.getParameterTypeProperty() == EffectParameterType::Single &&
                          label.getParameterTypeProperty() == EffectParameterType::String &&
                          genericTexture.getParameterTypeProperty() == EffectParameterType::Texture &&
                          oneDimensionalTexture.getParameterTypeProperty() == EffectParameterType::Texture1D &&
                          parameters["Texture"]->getParameterTypeProperty() == EffectParameterType::Texture2D &&
                          volumeInput.getParameterTypeProperty() == EffectParameterType::Texture3D &&
                          cubeInput.getParameterTypeProperty() == EffectParameterType::TextureCube &&
                          material.getParameterTypeProperty() == EffectParameterType::Void,
                      "loaded and authored parameters distinguish all ten parameter types");
            const EffectParameter* tintMember = material.getStructureMembersProperty().GetParameterBySemantic("color0");
            const EffectAnnotation* uiName = material.getAnnotationsProperty()["UiName"];
            ctx.Check(material.getSemanticProperty() == "MATERIAL0" && tintMember != nullptr &&
                          tintMember->getNameProperty() == "Tint" && uiName != nullptr &&
                          uiName->GetValueString() == "Surface tint",
                      "structure members support case-insensitive semantic lookup and attached string annotations");
            const EffectAnnotation* enabled = material.getAnnotationsProperty()["Enabled"];
            const EffectAnnotation* priority = material.getAnnotationsProperty()["Priority"];
            const EffectAnnotation* roughness = material.getAnnotationsProperty()["Roughness"];
            ctx.Check(enabled->GetValueBoolean() && priority->GetValueInt32() == 42 &&
                          roughness->GetValueSingle() == 0.35f &&
                          priority->getParameterClassProperty() == EffectParameterClass::Scalar &&
                          priority->getParameterTypeProperty() == EffectParameterType::Int32,
                      "Boolean, Int32 and Single annotations retain scalar values and type metadata");
            ctx.Check(material.getAnnotationsProperty()["UvScale"]->GetValueVector2() == Vector2(2.0f, 0.5f) &&
                          material.getAnnotationsProperty()["Axis"]->GetValueVector3() == Vector3(1.0f, -2.0f, 3.0f) &&
                          material.getAnnotationsProperty()["Color"]->GetValueVector4() ==
                              Vector4(0.1f, 0.2f, 0.3f, 0.4f),
                      "Vector2, Vector3 and Vector4 annotations retain exact component widths");
            const EffectAnnotation* transformAnnotation = material.getAnnotationsProperty()["Transform"];
            ctx.Check(transformAnnotation->getSemanticProperty() == "LOCAL_TO_WORLD" &&
                          transformAnnotation->getRowCountProperty() == 4 &&
                          transformAnnotation->getColumnCountProperty() == 4 &&
                          transformAnnotation->getParameterClassProperty() == EffectParameterClass::Matrix &&
                          transformAnnotation->getParameterTypeProperty() == EffectParameterType::Single &&
                          CnaDemos::MaxDifference(transformAnnotation->GetValueMatrix(), annotationTransform) < 1e-6f,
                      "matrix annotation reports exact shape/type/semantic metadata and column-major value");

            SkinnedEffect skinned(device);
            EffectParameter* bones = skinned.getParametersProperty()["Bones"];
            const std::vector<Matrix> transforms{Matrix::getIdentityProperty(), transform};
            bones->SetValue(transforms);
            const auto values = bones->GetValueMatrixArray(2);
            const auto transposed = bones->GetValueMatrixTransposeArray(2);
            const auto boneTransforms = skinned.GetBoneTransforms(2);
            ctx.Check(values.size() == 2 && transposed.size() == 2, "both array getters return two matrices");
            if (values.size() == 2 && transposed.size() == 2)
                for (std::size_t i = 0; i < transforms.size(); ++i)
                {
                    // Genuine XNA exposes a 4x3 raw matrix: its fourth column is zero.
                    // GetBoneTransforms, not the parameter getter, restores affine M44.
                    Matrix rawExpected = transforms[i];
                    rawExpected.M14 = rawExpected.M24 = rawExpected.M34 = rawExpected.M44 = 0;
                    ctx.CheckNear(CnaDemos::MaxDifference(values[i], rawExpected), 0.0, 1e-6,
                                  "each raw bone matrix preserves its 4x3 components");
                    ctx.CheckNear(CnaDemos::MaxDifference(transposed[i], Matrix::Transpose(rawExpected)),
                                  0.0, 1e-6, "each bone transpose matches");
                    ctx.Check(boneTransforms.size() == 2 &&
                                  CnaDemos::MaxDifference(boneTransforms[i], transforms[i]) < 1e-6,
                              "GetBoneTransforms restores each complete affine transform");
                }

            // Keep incompatible behavior visible: these expectations must not be changed to
            // accept CNA's current stock-effect storage implementation.
            bool rejected = false;
            try { ctx.Print("Vector4 read as Single returned {}", diffuse->GetValueSingle()); }
            catch (const System::InvalidCastException&) { rejected = true; }
            ctx.Check(rejected, "a Vector4 parameter rejects a scalar getter (ISSUE-034)");
            ctx.Print("Bones: {} elements, {} rows, {} columns", bones->getElementsProperty().getCountProperty(),
                      bones->getRowCountProperty(), bones->getColumnCountProperty());
            // The genuine XNA oracle records 4x3 (FNA's register reflection reports 3x4).
            // See docs/repair-evidence/graphics/STOCK_PARAMETER_TRIAGE.md.
            ctx.Check(bones->getElementsProperty().getCountProperty() == SkinnedEffect::MaxBones,
                      "Bones reflects 72 individual matrix elements (ISSUE-035)");
        }
    };
}

CNA_DEMOS_GAME(ParameterValues, {
    .id = "graphics.effect.parameter_values",
    .title = "Typed effect parameter values",
    .description = "Writes and reads scalar/array, vector, quaternion, string, texture and matrix parameter families, "
                   "including a structured reflection graph, transpose access and stock-effect compatibility gaps.",
    .apis = {"Effect::Parameters", "EffectParameter::ParameterType", "EffectParameter::ParameterClass",
             "EffectParameter::RowCount", "EffectParameter::ColumnCount", "EffectParameter::Elements",
             "EffectParameter::SetValue", "EffectParameter::SetValueTranspose", "EffectParameter::GetValueSingle",
             "EffectParameter::GetValueInt32", "EffectParameter::GetValueVector4", "EffectParameter::GetValueMatrix",
             "EffectParameter::GetValueMatrixTranspose", "EffectParameter::GetValueTexture2D",
             "EffectParameter::GetValueMatrixArray", "EffectParameter::GetValueMatrixTransposeArray",
             "EffectParameter::GetValueBoolean", "EffectParameter::GetValueBooleanArray",
             "EffectParameter::GetValueInt32Array", "EffectParameter::GetValueSingleArray",
             "EffectParameter::GetValueVector2", "EffectParameter::GetValueVector2Array",
             "EffectParameter::GetValueVector3", "EffectParameter::GetValueVector3Array",
             "EffectParameter::GetValueVector4Array", "EffectParameter::GetValueQuaternion",
             "EffectParameter::GetValueQuaternionArray", "EffectParameter::GetValueString",
             "EffectParameter::GetValueTexture3D", "EffectParameter::GetValueTextureCube",
             "EffectParameter::Semantic", "EffectParameter::StructureMembers", "EffectParameter::Annotations",
             "EffectParameterCollection::GetParameterBySemantic", "EffectAnnotation::Name",
             "EffectAnnotation::GetValueString", "EffectAnnotation::GetValueBoolean",
             "EffectAnnotation::GetValueInt32", "EffectAnnotation::GetValueSingle",
             "EffectAnnotation::GetValueVector2", "EffectAnnotation::GetValueVector3",
             "EffectAnnotation::GetValueVector4", "EffectAnnotation::GetValueMatrix",
             "EffectAnnotation::Semantic", "EffectAnnotation::RowCount", "EffectAnnotation::ColumnCount",
             "EffectAnnotation::ParameterClass", "EffectAnnotation::ParameterType",
             "EffectAnnotationCollection::Count",
             "EffectAnnotationCollection::Item", "EffectParameterClass::Scalar", "EffectParameterClass::Vector",
             "EffectParameterClass::Matrix", "EffectParameterClass::Object", "EffectParameterClass::Struct",
             "EffectParameterType::Bool", "EffectParameterType::Int32", "EffectParameterType::Single",
             "EffectParameterType::String", "EffectParameterType::Texture", "EffectParameterType::Texture1D",
             "EffectParameterType::Texture2D", "EffectParameterType::Texture3D", "EffectParameterType::TextureCube",
             "EffectParameterType::Void"},
    .tags = {"effect", "parameters", "reflection", "matrix"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
})
