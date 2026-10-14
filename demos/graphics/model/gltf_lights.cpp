// SPDX-License-Identifier: MS-PL
// glTF KHR_lights_punctual: the lights a scene declares become the model's effect lights.
#include <optional>
#include <string>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DirectionalLight.hpp"
#include "Microsoft/Xna/Framework/Graphics/Effect.hpp"
#include "Microsoft/Xna/Framework/Graphics/IEffectLights.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    bool Near(const Vector3& a, const Vector3& b) { return Vector3::Distance(a, b) < 0.02f; }

    class GltfLightsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            GraphicsDevice& device = getGraphicsDeviceProperty();
            // CNA documents PbrEffect as an unlit fallback on these renderers.
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer == "SOFTWARE" || renderer == "HEADLESS")
                Context().Skip(std::string(renderer) + " draws PbrEffect as an unlit fallback");

            // lit_sphere.gltf: a white metallic-roughness sphere and two directional lights,
            // "Warm" shining towards +X and "Cool" towards -X (tools/gltf_models.py).
            sphere_.emplace(getContentProperty().Load<Model>("models/lit_sphere"));
            ModelMeshPart* part = *sphere_->getMeshesProperty()[0]->getMeshPartsProperty().begin();
            Effect* effect = part->getEffectProperty();
            ctx.Check(dynamic_cast<PbrEffect*>(effect) != nullptr, "the metallic-roughness material is a PbrEffect");

            // The importer turned each glTF light into one of the effect's three directional
            // lights (IEffectLights): direction from the light's node, colour times intensity.
            auto* lights = dynamic_cast<IEffectLights*>(effect);
            ctx.Check(lights != nullptr, "the effect has IEffectLights");
            if (lights == nullptr)
                return;
            const DirectionalLight& warm = lights->getDirectionalLight0Property();
            const DirectionalLight& cool = lights->getDirectionalLight1Property();
            ctx.Print("light 0: direction {}, colour {}", warm.getDirectionProperty().ToString(),
                      warm.getDiffuseColorProperty().ToString());
            ctx.Print("light 1: direction {}, colour {}", cool.getDirectionProperty().ToString(),
                      cool.getDiffuseColorProperty().ToString());
            ctx.Check(warm.getEnabledProperty() && Near(warm.getDirectionProperty(), Vector3::UnitX) &&
                          Near(warm.getDiffuseColorProperty(), Vector3(1.0f, 0.35f, 0.2f)),
                      "light 0 is \"Warm\": towards +X, (1, 0.35, 0.2)");
            ctx.Check(cool.getEnabledProperty() && Near(cool.getDirectionProperty(), -Vector3::UnitX) &&
                          Near(cool.getDiffuseColorProperty(), Vector3(0.2f, 0.45f, 1.0f)),
                      "light 1 is \"Cool\": towards -X, (0.2, 0.45, 1)");
            ctx.Check(!lights->getDirectionalLight2Property().getEnabledProperty(),
                      "the third light stays off (the file declares two)");

            view_ = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 3.5f), Vector3::Zero, Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(20, 20, 24));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            sphere_->Draw(Matrix::getIdentityProperty(), view_, projection_);
            if (FramesDrawn() == 0)
            {
                // The warm light travels towards +X, so it lights the sphere's left side, which
                // faces -X; the cool one the right side.
                const CnaDemos::BackBufferPixels p(device);
                const Color left = p.At(400 - 120, 240);
                const Color right = p.At(400 + 120, 240);
                Context().Print("left side {}, right side {}", left.ToString(), right.ToString());
                Context().Check(left.getRProperty() > left.getBProperty() + 40 &&
                                    right.getBProperty() > right.getRProperty() + 40,
                                "the left side is lit warm, the right side cool");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::optional<Model> sphere_;
        Matrix view_;
        Matrix projection_;
    };
}

CNA_DEMOS_GAME(GltfLightsDemo, {
    .id = "graphics.model.gltf_lights",
    .title = "Lights from a glTF scene",
    .description = "Loads a glTF sphere whose scene declares two KHR_lights_punctual directional lights; the "
                   "importer sets them as the PbrEffect's DirectionalLight0/1 (IEffectLights), and the sphere is "
                   "lit warm from the left and cool from the right.",
    .apis = {"ContentManager::Load", "Model::Draw", "ModelMeshPart::Effect", "IEffectLights::DirectionalLight0",
             "IEffectLights::DirectionalLight1", "IEffectLights::DirectionalLight2", "DirectionalLight::Direction",
             "DirectionalLight::DiffuseColor", "DirectionalLight::Enabled", "PbrEffect"},
    .tags = {"model", "gltf", "light", "pbr"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/lit_sphere.cnb"},
    .restrictions = "Skipped on SOFTWARE and HEADLESS, where CNA draws PbrEffect as an unlit fallback.",
})
