// SPDX-License-Identifier: MS-PL
// Model::Draw: a model from the content pipeline, drawn with one call.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ModelDrawDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            // content/models/robot_arm.gltf, compiled by CNA's content pipeline (CNA.GltfImporter ->
            // CNA.ModelProcessor). Like an XNA .fbx, it arrives with its meshes, bones and effects.
            arm_.emplace(getContentProperty().Load<Model>("models/robot_arm"));
            Context().Print("{} meshes, {} bones", arm_->getMeshesProperty().getCountProperty(),
                            arm_->getBonesProperty().getCountProperty());
            Context().Check(arm_->getMeshesProperty().getCountProperty() == 3, "the robot arm has its three meshes");

            view_ = Matrix::CreateLookAt(Vector3(5.0f, 4.0f, 7.0f), Vector3(0.0f, 1.6f, 0.0f), Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4,
                getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::CornflowerBlue);
            // SpriteBatch leaves its own render states behind; 3D drawing needs a depth test.
            device.setDepthStencilStateProperty(DepthStencilState::Default);

            // Draw sets each mesh's effects up -- World from the mesh's bone, plus view and
            // projection -- and draws every part.
            arm_->Draw(Matrix::getIdentityProperty(), view_, projection_);

            if (!checked_)
            {
                // The upper arm's front face, halfway up: orange (shading baked into the model).
                const Vector3 onScreen =
                    device.getViewportProperty().Project(Vector3(0.0f, 1.5f, 0.25f), projection_, view_, Matrix::getIdentityProperty());
                const Color pixel = CnaDemos::BackBufferPixels(device).At(static_cast<int>(onScreen.X), static_cast<int>(onScreen.Y));
                Context().Print("upper arm at ({:.0f}, {:.0f}): {} {} {}", onScreen.X, onScreen.Y, pixel.getRProperty(),
                                pixel.getGProperty(), pixel.getBProperty());
                Context().Check(pixel.getRProperty() > 100 && pixel.getRProperty() > pixel.getGProperty() * 3 / 2 &&
                                    pixel.getGProperty() > pixel.getBProperty() * 2,
                                "the upper arm is drawn in its orange vertex colours");
                checked_ = true;
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "models/robot_arm (glTF) -- Model::Draw", Vector2(20.0f, 20.0f), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::optional<Model> arm_;
        Matrix view_;
        Matrix projection_;
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(ModelDrawDemo, {
    .id = "graphics.model.draw",
    .title = "Draw a model",
    .description = "Loads a three-part robot arm compiled from glTF by CNA's content pipeline and draws it with "
                   "Model::Draw(world, view, projection); a pixel on the upper arm is checked.",
    .apis = {"ContentManager::Load", "Model::Draw", "Model::Meshes", "Model::Bones", "Viewport::Project",
             "ModelMeshCollection::Count"},
    .tags = {"model", "3d", "mesh", "gltf", "content"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "models/robot_arm.cnb"},
})
