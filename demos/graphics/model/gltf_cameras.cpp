// SPDX-License-Identifier: MS-PL
// CNAEXT glTF cameras: the cameras a scene file carries, used to view the model they came with.
#include <array>
#include <cmath>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class GltfCamerasDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            // Through the content pipeline the cameras do not survive (CNA-DEMOS-ISSUE-023)...
            const Model compiled = getContentProperty().Load<Model>("models/camera_rig");
            ctx.Check(compiled.getCamerasEXTProperty().size() == 2,
                      "the compiled model keeps the scene's two cameras (CNA-DEMOS-ISSUE-023)");
            // ...so the rest of the demo uses the same scene imported at run time from the .gltf.
            rig_.emplace(getContentProperty().Load<Model>("models/loose_camera_rig"));
            const std::vector<ModelCameraEXT>& cameras = rig_->getCamerasEXTProperty();
            ctx.Check(cameras.size() == 2, "the run-time import has the scene's two glTF cameras");
            if (cameras.size() != 2)
            {
                return;
            }
            for (const ModelCameraEXT& camera : cameras)
            {
                ctx.Print("{}: {}, node {}, fov {:.2f}, near {:.2f}, far {:.1f}, authored aspect {}", camera.Name,
                          camera.IsPerspective ? "perspective" : "orthographic", camera.SceneNodeIndex,
                          camera.FieldOfView, camera.NearPlaneDistance, camera.FarPlaneDistance,
                          camera.HasAuthoredAspectRatio);
            }
            const ModelCameraEXT& front = cameras[0];
            const ModelCameraEXT& top = cameras[1];
            ctx.Check(front.Name == "Front" && front.IsPerspective && std::abs(front.FieldOfView - 0.7f) < 1e-5f &&
                          front.HasAuthoredAspectRatio && !front.HasInfiniteFarPlane,
                      "Front: perspective, yfov 0.7, authored aspect ratio, finite far plane");
            ctx.Check(top.Name == "Top" && !top.IsPerspective, "Top: orthographic");

            // A camera's live placement is its node's absolute transform (the node could be
            // animated); the view matrix is its inverse. At rest that equals the import snapshot.
            std::vector<Matrix> absolute(static_cast<std::size_t>(rig_->getBonesProperty().getCountProperty()));
            rig_->CopyAbsoluteBoneTransformsTo(absolute);
            for (std::size_t i = 0; i < cameras.size(); ++i)
            {
                const Matrix placement = absolute[static_cast<std::size_t>(cameras[i].SceneNodeIndex)];
                views_[i] = Matrix::Invert(placement);
                projections_[i] = cameras[i].Projection;
            }
            const Vector3 frontPosition = absolute[static_cast<std::size_t>(front.SceneNodeIndex)].getTranslationProperty();
            ctx.Check(std::abs(frontPosition.Y - 1.8f) < 1e-4f && std::abs(frontPosition.Z - 6.0f) < 1e-4f,
                      "the Front camera's node stands at (0, 1.8, 6)");
            ctx.Check(absolute[static_cast<std::size_t>(front.SceneNodeIndex)] == front.WorldTransform,
                      "at rest the bone's absolute transform is the imported WorldTransform");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const Viewport full = device.getViewportProperty();
            device.Clear(Color(24, 24, 32));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            if (rig_->getCamerasEXTProperty().size() == 2)
            {
                // Each half of the screen (400x480, the aspect ratio the file's cameras were made for)
                // shows the scene through one of its cameras.
                for (int i = 0; i < 2; ++i)
                {
                    Viewport half = full;
                    half.setXProperty(i * full.getWidthProperty() / 2);
                    half.setWidthProperty(full.getWidthProperty() / 2);
                    device.setViewportProperty(half);
                    rig_->Draw(Matrix::getIdentityProperty(), views_[static_cast<std::size_t>(i)],
                               projections_[static_cast<std::size_t>(i)]);
                }
                device.setViewportProperty(full);
            }

            if (FramesDrawn() == 0)
            {
                // The front view sees the floor below the middle; the top view sees it all around.
                const CnaDemos::BackBufferPixels pixels(device);
                const Color frontFloor = pixels.At(200, 400);
                const Color topCentre = pixels.At(600, 240);
                Context().Print("front view at (200, 400) {}, top view centre {}", frontFloor.ToString(),
                                topCentre.ToString());
                Context().Check(!CnaDemos::Near(frontFloor, Color(24, 24, 32), 8) &&
                                    !CnaDemos::Near(topCentre, Color(24, 24, 32), 8),
                                "both cameras see the scene");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::optional<Model> rig_;
        std::array<Matrix, 2> views_;
        std::array<Matrix, 2> projections_;
    };
}

CNA_DEMOS_GAME(GltfCamerasDemo, {
    .id = "graphics.model.gltf_cameras",
    .title = "Cameras from a glTF scene (CNAEXT)",
    .description = "Loads a glTF scene with a perspective and an orthographic camera, reads them from "
                   "Model::CamerasEXT (projection, field of view, planes, authored aspect ratio) and shows the "
                   "scene through each in one half of the screen, the view taken from the camera node's bone "
                   "(from the run-time import: the compiled model loses its cameras, CNA-DEMOS-ISSUE-023).",
    .apis = {"ModelCameraEXT", "Model::getCamerasEXTProperty", "Model::CopyAbsoluteBoneTransformsTo",
             "Matrix::Invert", "GraphicsDevice::Viewport", "Model::Draw"},
    .tags = {"3d", "model", "gltf", "camera", "split-screen", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/camera_rig.cnb", "models/loose_camera_rig.gltf"},
    .knownIssue = "CNA-DEMOS-ISSUE-023",
})
