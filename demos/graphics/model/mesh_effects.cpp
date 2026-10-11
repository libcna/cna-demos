// SPDX-License-Identifier: MS-PL
// A model's effects are ordinary effects: change their settings per mesh (tint, transparency).
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class MeshEffectsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            arm_.emplace(getContentProperty().Load<Model>("models/robot_arm"));
            view_ = Matrix::CreateLookAt(Vector3(0.0f, 2.5f, 12.0f), Vector3(0.0f, 1.8f, 0.0f), Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4,
                getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::CornflowerBlue);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setBlendStateProperty(BlendState::AlphaBlend);

            // Three copies of one model with different effect settings. The settings belong to
            // the model's effects, so set them before each Draw.
            DrawCopy(-3.5f, Vector3::One, 1.0f);                     // as authored
            DrawCopy(0.0f, Vector3(0.4f, 1.0f, 0.4f), 1.0f);         // DiffuseColor tints the vertex colours
            DrawCopy(3.5f, Vector3::One, 0.35f);                     // Alpha makes it see-through

            if (!checked_)
            {
                // The same spot on each copy's upper arm.
                const auto at = [&](float x) {
                    const Vector3 p = device.getViewportProperty().Project(Vector3(x, 1.5f, 0.3f), projection_, view_,
                                                                           Matrix::getIdentityProperty());
                    return CnaDemos::BackBufferPixels(device).At(static_cast<int>(p.X), static_cast<int>(p.Y));
                };
                const Color original = at(-3.5f);
                const Color tinted = at(0.0f);
                const Color faded = at(3.5f);
                Context().Print("upper arm: original {} {} {}, tinted {} {} {}, faded {} {} {}", original.getRProperty(),
                                original.getGProperty(), original.getBProperty(), tinted.getRProperty(), tinted.getGProperty(),
                                tinted.getBProperty(), faded.getRProperty(), faded.getGProperty(), faded.getBProperty());
                Context().Check(tinted.getRProperty() < original.getRProperty() / 2 && tinted.getGProperty() >= original.getGProperty() - 2,
                                "DiffuseColor (0.4, 1, 0.4) cuts red and keeps green");
                Context().Check(faded.getBProperty() > original.getBProperty() + 40,
                                "with Alpha 0.35 the blue background shows through");
                checked_ = true;
            }
            device.setBlendStateProperty(BlendState::Opaque);
            Game::Draw(gameTime);
        }

    private:
        void DrawCopy(float x, const Vector3& tint, float alpha)
        {
            for (ModelMesh* mesh : arm_->getMeshesProperty())
            {
                for (Effect* effect : mesh->getEffectsProperty())
                {
                    auto* basic = dynamic_cast<BasicEffect*>(effect);
                    basic->setDiffuseColorProperty(tint);
                    basic->setAlphaProperty(alpha);
                }
            }
            arm_->Draw(Matrix::CreateTranslation(x, 0.0f, 0.0f), view_, projection_);
        }

        std::optional<Model> arm_;
        Matrix view_;
        Matrix projection_;
        bool checked_ = false;
    };
}

CNA_DEMOS_GAME(MeshEffectsDemo, {
    .id = "graphics.model.mesh_effects",
    .title = "Change a model's effect settings",
    .description = "Draws the robot arm three times, changing the BasicEffect of every mesh in between: as authored, "
                   "tinted with DiffuseColor, and see-through with Alpha; pixels on the three copies are compared.",
    .apis = {"ModelMesh::Effects", "BasicEffect::DiffuseColor", "BasicEffect::Alpha", "Model::Draw",
             "BlendState::AlphaBlend",
             "ModelEffectCollection", "Model::Meshes"},
    .tags = {"model", "effect", "tint", "transparency", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/robot_arm.cnb"},
})
