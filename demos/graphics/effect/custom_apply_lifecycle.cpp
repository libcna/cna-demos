// SPDX-License-Identifier: MS-PL
// Derive from Effect to observe the protected OnApply hook and public disposal contract.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/Effect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "System/EventArgs.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class TrackedEffect final : public Effect
    {
    public:
        explicit TrackedEffect(GraphicsDevice& device) : Effect(device) {}

        [[nodiscard]] int ApplyCount() const { return applyCount_; }

    protected:
        void OnApply() override { ++applyCount_; }

    private:
        int applyCount_ = 0;
    };

    template<typename Action>
    bool ThrowsDisposed(Action&& action)
    {
        try
        {
            action();
            return false;
        }
        catch (const System::ObjectDisposedException&)
        {
            return true;
        }
    }

    class CustomApplyLifecycleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            TrackedEffect effect(getGraphicsDeviceProperty());
            EffectTechnique* technique = effect.getCurrentTechniqueProperty();
            EffectPass* pass = technique->getPassesProperty()[0];

            Context().Check(effect.getParametersProperty().getCountProperty() == 0 &&
                                effect.getTechniquesProperty().getCountProperty() == 1 &&
                                technique->getNameProperty() == "Default" && pass != nullptr &&
                                pass->getNameProperty() == "P0",
                            "a base Effect starts with an empty parameter set and Default/P0 reflection graph");

            effect.Apply();
            Context().Check(effect.ApplyCount() == 1,
                            "Effect::Apply invokes the derived OnApply hook once");
            pass->Apply();
            Context().Check(effect.ApplyCount() == 2,
                            "applying the current pass invokes the same OnApply hook once");

            int disposingEvents = 0;
            effect.Disposing += [&disposingEvents](System::Object*, const System::EventArgs&) {
                ++disposingEvents;
            };
            effect.Dispose();
            Context().Check(effect.getIsDisposedProperty() && disposingEvents == 1,
                            "Dispose marks the effect and raises one Disposing event");
            Context().Check(ThrowsDisposed([&effect] { effect.Apply(); }) &&
                                ThrowsDisposed([pass] { pass->Apply(); }) &&
                                effect.ApplyCount() == 2,
                            "disposed effects reject direct and pass Apply before calling OnApply");

            effect.Dispose();
            Context().Check(disposingEvents == 1,
                            "disposing an Effect twice is idempotent");
            Context().Print("OnApply calls {}, Disposing events {}", effect.ApplyCount(), disposingEvents);
        }
    };
}

CNA_DEMOS_GAME(CustomApplyLifecycleDemo, {
    .id = "graphics.effect.custom_apply_lifecycle",
    .title = "Custom effect apply and lifetime",
    .description = "Derives from Effect, observes OnApply through direct and pass application, then checks disposal events, refusal and idempotence.",
    .apis = {"Effect::Effect", "Effect::OnApply", "Effect::Dispose", "Effect::Apply",
             "Effect::Parameters", "Effect::Techniques", "Effect::CurrentTechnique",
             "EffectPass::Apply", "GraphicsResource::Disposing", "GraphicsResource::IsDisposed"},
    .tags = {"effect", "apply", "lifetime", "dispose", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
})
