// SPDX-License-Identifier: MS-PL
// Effect::Parameters and Effect::Techniques: an effect's named inputs and its alternative shaders.
#include <memory>
#include <string>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectAnnotation.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectParameter.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectParameterClass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectParameterCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectParameterType.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechniqueCollection.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    std::string_view ClassName(EffectParameterClass c)
    {
        switch (c)
        {
        case EffectParameterClass::Scalar: return "Scalar";
        case EffectParameterClass::Vector: return "Vector";
        case EffectParameterClass::Matrix: return "Matrix";
        case EffectParameterClass::Object: return "Object";
        case EffectParameterClass::Struct: return "Struct";
        }
        return "?";
    }

    class ParametersDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            BasicEffect effect(getGraphicsDeviceProperty());

            // Every effect exposes its shader inputs by name. The stock effects' properties
            // (BasicEffect::DiffuseColor and so on) are conveniences over these parameters.
            EffectParameterCollection& parameters = effect.getParametersProperty();
            Context().Print("BasicEffect has {} parameters:", parameters.getCountProperty());
            for (int i = 0; i < parameters.getCountProperty(); ++i)
            {
                const EffectParameter* parameter = parameters[i];
                Context().Print("  {:<24} {}", parameter->getNameProperty(), ClassName(parameter->getParameterClassProperty()));
            }
            Context().Check(parameters.getCountProperty() > 0, "the effect has parameters");

            // ...and its shader variants as techniques, each with one or more passes.
            EffectTechniqueCollection& techniques = effect.getTechniquesProperty();
            Context().Print("{} technique(s):", techniques.getCountProperty());
            for (int i = 0; i < techniques.getCountProperty(); ++i)
                Context().Print("  {} ({} pass(es))", techniques[i]->getNameProperty(),
                                techniques[i]->getPassesProperty().getCountProperty());
            EffectTechnique* currentTechnique = effect.getCurrentTechniqueProperty();
            Context().Check(currentTechnique != nullptr && currentTechnique->getNameProperty() == "BasicEffect" &&
                                techniques["BasicEffect"] == currentTechnique && techniques["basiceffect"] == nullptr,
                            "the current technique has its authored name and exact name lookup identity");

            EffectPassCollection& passes = currentTechnique->getPassesProperty();
            EffectPass* firstPass = passes[0];
            Context().Check(firstPass != nullptr && firstPass->getNameProperty() == "P0" &&
                                passes["P0"] == firstPass && passes["p0"] == nullptr && passes[-1] == nullptr,
                            "pass index and exact name lookup return the same authored P0 pass");

            // Stock effects have no authored FX annotations. Add public reflection values to show
            // that technique and pass metadata remain scoped to their respective owner.
            currentTechnique->getAnnotationsProperty().Add(
                EffectAnnotation("Purpose", "", 1, 1, EffectParameterClass::Object,
                                 EffectParameterType::String, {}, "stock lighting"));
            firstPass->getAnnotationsProperty().Add(
                EffectAnnotation("Stage", "", 1, 1, EffectParameterClass::Object,
                                 EffectParameterType::String, {}, "forward"));
            Context().Check(currentTechnique->getAnnotationsProperty().getCountProperty() == 1 &&
                                currentTechnique->getAnnotationsProperty()["Purpose"]->GetValueString() ==
                                    "stock lighting" &&
                                currentTechnique->getAnnotationsProperty()["Stage"] == nullptr,
                            "technique annotations retain their own authored reflection values");
            Context().Check(firstPass->getAnnotationsProperty().getCountProperty() == 1 &&
                                firstPass->getAnnotationsProperty()["Stage"]->GetValueString() == "forward" &&
                                firstPass->getAnnotationsProperty()["Purpose"] == nullptr,
                            "pass annotations retain separate authored reflection values");

            // The properties are pushed into the parameters when a pass is applied, not when set --
            // and the stock effects store the diffuse colour premultiplied by Alpha.
            EffectParameter* diffuse = parameters["DiffuseColor"];
            effect.setDiffuseColorProperty(Vector3(1.0f, 0.0f, 0.0f));
            effect.setAlphaProperty(0.5f);
            const Vector4 beforeApply = diffuse->GetValueVector4();
            firstPass->Apply();
            const Vector4 afterApply = diffuse->GetValueVector4();
            Context().Print("DiffuseColor parameter: before Apply {}, after Apply {}", beforeApply.ToString(),
                            afterApply.ToString());
            Context().Check(beforeApply != afterApply, "setting a property does not touch the parameter yet");
            Context().Check(afterApply == Vector4(0.5f, 0.0f, 0.0f, 0.5f),
                            "after Apply it holds DiffuseColor * Alpha, with Alpha in W");
            Context().Check(parameters["NoSuchParameter"] == nullptr, "an unknown name gives null, not an exception");
        }
    };
}

CNA_DEMOS_GAME(ParametersDemo, {
    .id = "graphics.effect.parameters_and_techniques",
    .title = "Effect parameters and techniques",
    .description = "Inspects a BasicEffect's named parameters, technique, pass and scoped annotations, and shows "
                   "that properties reach the DiffuseColor parameter only when its pass is applied.",
    .apis = {"Effect::Parameters", "Effect::Techniques", "Effect::CurrentTechnique", "EffectParameterCollection::Item",
             "EffectParameter::Name", "EffectParameter::ParameterClass", "EffectParameter::GetValueVector4", "EffectPass::Apply",
             "EffectTechniqueCollection::Item", "EffectTechnique::Name", "EffectTechnique::Passes",
             "EffectTechnique::Annotations", "EffectPass::Name", "EffectPass::Annotations",
             "EffectPassCollection::Item",
             "EffectParameterClass", "EffectParameterType",
             "EffectPassCollection::Count", "EffectParameterCollection::Count", "EffectTechniqueCollection::Count"},
    .tags = {"effect", "shader", "parameters", "reflection"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
})
