// SPDX-License-Identifier: MS-PL
// CNAEXT glTF cameras: the cameras a scene file carries, used to view the model they came with.
#include <array>
#include <cmath>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "System/TimeSpan.hpp"

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
            // The same scene through the content pipeline and imported at run time from the .gltf.
            const Model compiled = getContentProperty().Load<Model>("models/camera_rig");
            rig_.emplace(getContentProperty().Load<Model>("models/loose_camera_rig"));
            const std::vector<ModelCameraEXT>& cameras = rig_->getCamerasEXTProperty();
            ctx.Check(cameras.size() == 3, "the run-time import has the scene's three glTF cameras");
            const std::vector<ModelCameraEXT>& compiledCameras = compiled.getCamerasEXTProperty();
            bool sameCameras = compiledCameras.size() == cameras.size();
            for (std::size_t i = 0; sameCameras && i < cameras.size(); ++i)
            {
                const ModelCameraEXT& a = compiledCameras[i];
                const ModelCameraEXT& b = cameras[i];
                sameCameras = a.Name == b.Name && a.SceneNodeIndex == b.SceneNodeIndex &&
                              a.IsPerspective == b.IsPerspective && a.Projection == b.Projection &&
                              a.WorldTransform == b.WorldTransform &&
                              a.HasAuthoredAspectRatio == b.HasAuthoredAspectRatio &&
                              a.HasInfiniteFarPlane == b.HasInfiniteFarPlane;
            }
            ctx.Check(sameCameras, "the compiled model keeps the same three cameras");
            if (cameras.size() != 3)
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
            const ModelCameraEXT& infinite = cameras[2];
            ctx.Check(front.Name == "Front" && front.IsPerspective && std::abs(front.FieldOfView - 0.7f) < 1e-5f &&
                          front.HasAuthoredAspectRatio && !front.HasInfiniteFarPlane,
                      "Front: perspective, yfov 0.7, authored aspect ratio, finite far plane");
            ctx.Check(top.Name == "Top" && !top.IsPerspective && !top.HasAuthoredAspectRatio &&
                          top.FieldOfView == 0.0f && top.NearPlaneDistance == 0.0f &&
                          top.FarPlaneDistance == 0.0f &&
                          top.Projection == Matrix::CreateOrthographic(6.0f, 7.2f, 0.1f, 20.0f),
                      "Top: exact orthographic projection with perspective-only fields at defaults");
            ctx.Check(infinite.Name == "Infinite" && infinite.IsPerspective &&
                          infinite.HasInfiniteFarPlane && !infinite.HasAuthoredAspectRatio &&
                          infinite.AspectRatio == 1.0f && std::abs(infinite.FieldOfView - 0.6f) < 1e-5f &&
                          std::abs(infinite.NearPlaneDistance - 0.2f) < 1e-5f &&
                          infinite.FarPlaneDistance == 0.0f,
                      "Infinite records its absent aspect ratio and far plane without inventing values");
            const Matrix rebuiltInfinite = CreateInfinitePerspectiveFieldOfViewEXT(
                infinite.FieldOfView, infinite.AspectRatio, infinite.NearPlaneDistance);
            ctx.Check(infinite.Projection == rebuiltInfinite,
                      "the public infinite projection builder exactly reconstructs the imported matrix");

            // A camera's live placement is its node's absolute transform (the node could be
            // animated); the view matrix is its inverse. At rest that equals the import snapshot.
            std::vector<Matrix> absolute(static_cast<std::size_t>(rig_->getBonesProperty().getCountProperty()));
            rig_->CopyAbsoluteBoneTransformsTo(absolute);
            const Vector3 frontPosition = absolute[static_cast<std::size_t>(front.SceneNodeIndex)].getTranslationProperty();
            ctx.Check(std::abs(frontPosition.Y - 1.8f) < 1e-4f && std::abs(frontPosition.Z - 6.0f) < 1e-4f,
                      "the Front camera's node stands at (0, 1.8, 6)");
            ctx.Check(absolute[static_cast<std::size_t>(front.SceneNodeIndex)] == front.WorldTransform,
                      "at rest the bone's absolute transform is the imported WorldTransform");

            const auto* animations = dynamic_cast<const ModelAnimationsEXT*>(rig_->getTagProperty());
            if (!ctx.Check(animations && animations->Clips.contains("Dolly"),
                           "the camera-node animation is exposed as the Dolly scene-node clip"))
                return;
            const AnimationClip& dolly = animations->Clips.at("Dolly");
            if (!ctx.Check(dolly.TargetSpace == ClipTargetSpaceEXT::SceneNode &&
                               dolly.Tracks.size() == 1 &&
                               dolly.Tracks[0].BoneIndex == front.SceneNodeIndex,
                           "Dolly targets the exact Model bone that carries Front"))
                return;
            const Matrix importedFront = front.WorldTransform;
            ApplyClipToBonesEXT(*rig_, dolly, System::TimeSpan::FromSeconds(1.0));
            rig_->CopyAbsoluteBoneTransformsTo(absolute);
            const Matrix liveFront = absolute[static_cast<std::size_t>(front.SceneNodeIndex)];
            const Vector3 livePosition = liveFront.getTranslationProperty();
            ctx.Check(std::abs(livePosition.X - 0.8f) < 1e-4f &&
                          std::abs(livePosition.Y - 1.8f) < 1e-4f &&
                          std::abs(livePosition.Z - 6.0f) < 1e-4f,
                      "sampling Dolly moves Front's live bone to (0.8, 1.8, 6)");
            ctx.Check(front.WorldTransform == importedFront && liveFront != front.WorldTransform,
                      "WorldTransform remains an import snapshot while the live camera bone moves");

            views_[0] = Matrix::Invert(liveFront);
            views_[1] = Matrix::Invert(absolute[static_cast<std::size_t>(top.SceneNodeIndex)]);
            projections_[0] = front.Projection;
            projections_[1] = top.Projection;
            ready_ = true;
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const Viewport full = device.getViewportProperty();
            device.Clear(Color(24, 24, 32));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            if (ready_)
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
        bool ready_ = false;
    };
}

CNA_DEMOS_GAME(GltfCamerasDemo, {
    .id = "graphics.model.gltf_cameras",
    .title = "Cameras from a glTF scene (CNAEXT)",
    .description = "Loads finite, orthographic and infinite glTF cameras, reconstructs the infinite projection, "
                   "then samples a camera-node animation and shows that live placement comes from its Model bone "
                   "while WorldTransform remains the import snapshot; the finite and top views share the screen. The "
                   "compiled model carries the same cameras as the run-time import.",
    .apis = {"ModelCameraEXT", "Model::getCamerasEXTProperty", "Model::CopyAbsoluteBoneTransformsTo",
             "CreateInfinitePerspectiveFieldOfViewEXT", "ModelAnimationsEXT", "ApplyClipToBonesEXT",
             "Matrix::Invert", "GraphicsDevice::Viewport", "Model::Draw"},
    .tags = {"3d", "model", "gltf", "camera", "animation", "split-screen", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/camera_rig.cnb", "models/loose_camera_rig.gltf"},
})
