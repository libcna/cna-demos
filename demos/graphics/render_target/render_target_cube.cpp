// SPDX-License-Identifier: MS-PL
// RenderTargetCube: render the scene six times, once per cube face, then reflect it (dynamic environment map).
#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <utility>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/EnvironmentMapEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetBinding.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetCube.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // Each face is cleared to a colour of its own (dim "walls"), so the reflection shows which
    // face it samples; the orbiting cubes are drawn into every face on top of that.
    struct FaceView
    {
        CubeMapFace face;
        Vector3 forward;
        Vector3 up;
        Color wall;
    };

    // The look directions and up vectors XNA programs use for cube faces (right-handed view
    // matrices into the cube map's left-handed face layout, hence +Z looks down Forward).
    const std::array<FaceView, 6> Faces = {{
        {CubeMapFace::PositiveX, Vector3::Right, Vector3::Up, Color(90, 30, 30)},
        {CubeMapFace::NegativeX, Vector3::Left, Vector3::Up, Color(30, 80, 90)},
        {CubeMapFace::PositiveY, Vector3::Up, Vector3::Backward, Color(120, 150, 200)},
        {CubeMapFace::NegativeY, Vector3::Down, Vector3::Forward, Color(70, 50, 30)},
        {CubeMapFace::PositiveZ, Vector3::Forward, Vector3::Up, Color(30, 110, 40)},
        {CubeMapFace::NegativeZ, Vector3::Backward, Vector3::Up, Color(40, 40, 120)},
    }};

    class RenderTargetCubeDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            sphere_ = CnaDemos::MakeSphere(1.0f);
            cube_ = CnaDemos::MakeCube(0.3f);

            // Six 128x128 colour faces sharing one depth buffer format; rendered again every frame.
            environment_ = std::make_unique<RenderTargetCube>(device, 128, false, SurfaceFormat::Color,
                                                              DepthFormat::Depth24, 0,
                                                              RenderTargetUsage::PreserveContents);
            Context().Check(environment_->getDepthStencilFormatProperty() == DepthFormat::Depth24 &&
                                environment_->getMultiSampleCountProperty() == 0 &&
                                environment_->getRenderTargetUsageProperty() == RenderTargetUsage::PreserveContents,
                            "the cube target reports its applied depth, multisample and preservation state");
            environment_->ContentLost += [this](System::Object*, const System::EventArgs&) {
                ++contentLostEvents_;
            };
            environment_->NotifyContentLostEXT();
            Context().Check(environment_->getIsContentLostProperty() && contentLostEvents_ == 1,
                            "a content-loss notification raises ContentLost and marks the cube target lost");
            const RenderTargetBinding negativeZ(environment_.get(), CubeMapFace::NegativeZ);
            Context().Check(negativeZ.getRenderTargetProperty() == environment_.get() &&
                                negativeZ.getCubeMapFaceProperty() == CubeMapFace::NegativeZ,
                            "a cube binding retains its exact target and selected face");

            device.SetRenderTarget(environment_.get(), CubeMapFace::NegativeZ);
            device.Clear(Color::Gold);
            bool disposeRefused = false;
            try { environment_->Dispose(); }
            catch (const System::InvalidOperationException&) { disposeRefused = true; }
            const auto bindings = device.GetRenderTargets();
            Context().Check(disposeRefused && !environment_->getIsDisposedProperty() &&
                                bindings.size() == 1 &&
                                bindings[0].getRenderTargetProperty() == environment_.get() &&
                                bindings[0].getCubeMapFaceProperty() == CubeMapFace::NegativeZ,
                            "disposing a bound cube target is refused without changing its selected face");
            device.SetRenderTarget(static_cast<RenderTarget2D*>(nullptr));
            const Rectangle origin(0, 0, 1, 1);
            Color pixel = Color::Black;
            environment_->GetData(CubeMapFace::NegativeZ, 0, &origin, &pixel, 0, 1);
            Context().Check(pixel == Color::Gold,
                            "the cube face remains readable and exact after bound disposal is refused");
            CheckBoundDestruction(device);

            cubeEffect_ = std::make_unique<BasicEffect>(device);
            cubeEffect_->EnableDefaultLighting();

            white_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);
            chrome_ = std::make_unique<EnvironmentMapEffect>(device);
            chrome_->EnableDefaultLighting();
            chrome_->setTextureProperty(white_.get());
            chrome_->setDiffuseColorProperty(Vector3(0.1f, 0.1f, 0.1f));
            chrome_->setFresnelFactorProperty(0.0f);
            chrome_->setEnvironmentMapAmountProperty(1.0f);
            chrome_->setEnvironmentMapProperty(environment_.get()); // a RenderTargetCube is a TextureCube
        }

        void CheckBoundDestruction(GraphicsDevice& device)
        {
            const Rectangle backBufferBounds = device.getViewportProperty().getBoundsProperty();
            std::optional<RenderTargetCube> slot;
            slot.emplace(device, 24, false, SurfaceFormat::Color, DepthFormat::None, 0,
                         RenderTargetUsage::PreserveContents);
            const RenderTargetCube* const firstAddress = &*slot;
            device.SetRenderTarget(&*slot, CubeMapFace::PositiveY);
            slot.reset();

            bool presentRefused = false;
            try { device.Present(); }
            catch (const System::InvalidOperationException&) { presentRefused = true; }
            Context().Check(device.GetRenderTargets().empty() && presentRefused,
                            "destroying a bound cube removes its dangling public binding while Present stays refused");

            device.SetRenderTarget(static_cast<RenderTarget2D*>(nullptr));
            Context().Check(device.GetRenderTargets().empty() &&
                                device.getViewportProperty().getBoundsProperty() == backBufferBounds,
                            "an explicit unbind after bound cube destruction restores the complete back-buffer viewport");

            slot.emplace(device, 24, false, SurfaceFormat::Color, DepthFormat::None, 0,
                         RenderTargetUsage::PreserveContents);
            const bool addressReused = &*slot == firstAddress;
            device.SetRenderTarget(&*slot, CubeMapFace::NegativeX);
            const auto replacementBindings = device.GetRenderTargets();
            Context().Check(addressReused && replacementBindings.size() == 1 &&
                                replacementBindings[0].getRenderTargetProperty() == &*slot &&
                                replacementBindings[0].getCubeMapFaceProperty() == CubeMapFace::NegativeX &&
                                device.getViewportProperty().getBoundsProperty() == Rectangle(0, 0, 24, 24),
                            "a replacement cube at the destroyed target's address is genuinely rebound to its selected face");

            device.Clear(Color::CornflowerBlue);
            device.SetRenderTarget(static_cast<RenderTarget2D*>(nullptr));
            const Rectangle origin(0, 0, 1, 1);
            Color pixel = Color::Black;
            slot->GetData(CubeMapFace::NegativeX, 0, &origin, &pixel, 0, 1);
            Context().Check(pixel == Color::CornflowerBlue,
                            "the same-address cube replacement receives rendering through its selected face binding");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const float t = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty());
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            // 1. The environment, seen from the sphere's centre: one 90-degree view per face.
            const Matrix faceProjection = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver2, 1.0f, 0.1f, 50.0f);
            for (const FaceView& view : Faces)
            {
                device.SetRenderTarget(environment_.get(), view.face);
                if (!checkedContentRestore_)
                {
                    checkedContentRestore_ = true;
                    Context().Check(!environment_->getIsContentLostProperty(),
                                    "binding a cube face for rendering clears its lost-content state");
                }
                // After the first frame, rebind +Z without touching it. PreserveContents must keep
                // its green wall available to the reflective sphere below.
                if (FramesDrawn() > 0 && view.face == CubeMapFace::PositiveZ)
                    continue;
                device.Clear(view.wall);
                DrawOrbitingCubes(Matrix::CreateLookAt(Vector3::Zero, view.forward, view.up), faceProjection, t);
            }
            device.SetRenderTarget(static_cast<RenderTarget2D*>(nullptr));

            // 2. The scene from the camera, with a mirror sphere reflecting the six faces just drawn.
            const Matrix cameraView = Matrix::CreateLookAt(Vector3(0.0f, 1.5f, 6.5f), Vector3(0.0f, 0.5f, 0.0f), Vector3::Up);
            const Matrix cameraProjection = Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 50.0f);
            device.Clear(Color(16, 16, 22));
            DrawOrbitingCubes(cameraView, cameraProjection, t);
            chrome_->setViewProperty(cameraView);
            chrome_->setProjectionProperty(cameraProjection);
            for (EffectPass& pass : chrome_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, sphere_.vertices.data(), 0,
                                                 sphere_.VertexCount(), sphere_.indices.data(), 0,
                                                 sphere_.PrimitiveCount());
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const Viewport viewport = device.getViewportProperty();
                const Vector3 centre = viewport.Project(Vector3::Zero, cameraProjection, cameraView, Matrix::getIdentityProperty());
                const Vector3 top = viewport.Project(Vector3::Up, cameraProjection, cameraView, Matrix::getIdentityProperty());
                const int cx = static_cast<int>(centre.X);
                const int cy = static_cast<int>(centre.Y);
                const int radius = static_cast<int>(centre.Y - top.Y);

                // The sphere's centre faces the camera, so it mirrors what lies behind the camera:
                // the +Z face, cleared to green this frame.
                const Color middle = pixels.At(cx, cy);
                Context().Print("sphere centre {}", middle.ToString());
                Context().Check(middle.getGProperty() > middle.getRProperty() + 30 &&
                                    middle.getGProperty() > middle.getBProperty() + 30,
                                "the sphere reflects the +Z face rendered this frame");

                // The cubes orbit above the sphere, so their reflections belong in its upper half.
                // Count reflected pixels that are neither a wall colour nor dark, above and below.
                int above = 0;
                int below = 0;
                for (int y = cy - radius * 8 / 10; y <= cy + radius * 8 / 10; ++y)
                {
                    for (int x = cx - radius * 8 / 10; x <= cx + radius * 8 / 10; ++x)
                    {
                        if ((x - cx) * (x - cx) + (y - cy) * (y - cy) > radius * radius * 64 / 100 || y == cy)
                            continue;
                        const Color c = pixels.At(x, y);
                        bool wall = c.getRProperty() < 60 && c.getGProperty() < 60 && c.getBProperty() < 60;
                        for (const FaceView& view : Faces)
                            wall = wall || CnaDemos::Near(c, view.wall, 30);
                        if (!wall)
                            ++(y < cy ? above : below);
                    }
                }
                Context().Print("reflected cube pixels: {} above the centre, {} below", above, below);
                Context().Check(above > 30 && above > 3 * below,
                                "the cubes above the sphere are reflected in its upper half (faces sampled upright)");
            }
            else if (FramesDrawn() == 1)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const Viewport viewport = device.getViewportProperty();
                const Vector3 centre = viewport.Project(Vector3::Zero, cameraProjection, cameraView,
                                                        Matrix::getIdentityProperty());
                const Color middle = pixels.At(static_cast<int>(centre.X), static_cast<int>(centre.Y));
                Context().Print("preserved +Z sphere centre {}", middle.ToString());
                Context().Check(middle.getGProperty() > middle.getRProperty() + 30 &&
                                    middle.getGProperty() > middle.getBProperty() + 30,
                                "PreserveContents keeps the +Z face through an empty rebind");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        void DrawOrbitingCubes(const Matrix& view, const Matrix& projection, float t)
        {
            static const std::array<Vector3, 6> colors = {Vector3(1.0f, 0.3f, 0.2f), Vector3(1.0f, 0.8f, 0.2f),
                                                          Vector3(0.3f, 1.0f, 0.3f), Vector3(0.2f, 0.8f, 1.0f),
                                                          Vector3(0.4f, 0.4f, 1.0f), Vector3(1.0f, 0.4f, 1.0f)};
            cubeEffect_->setViewProperty(view);
            cubeEffect_->setProjectionProperty(projection);
            for (std::size_t i = 0; i < colors.size(); ++i)
            {
                const float angle = t * 0.8f + MathHelper::TwoPi * static_cast<float>(i) / 6.0f;
                cubeEffect_->setWorldProperty(Matrix::CreateRotationY(t * 2.0f) *
                                              Matrix::CreateTranslation(2.6f * std::cos(angle), 1.6f, 2.6f * std::sin(angle)));
                cubeEffect_->setDiffuseColorProperty(colors[i]);
                for (EffectPass& pass : cubeEffect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    getGraphicsDeviceProperty().DrawUserIndexedPrimitives(
                        PrimitiveType::TriangleList, cube_.vertices.data(), 0, cube_.VertexCount(), cube_.indices.data(), 0,
                        cube_.PrimitiveCount());
                }
            }
        }

        CnaDemos::Mesh sphere_;
        CnaDemos::Mesh cube_;
        std::unique_ptr<RenderTargetCube> environment_;
        std::unique_ptr<BasicEffect> cubeEffect_;
        std::unique_ptr<Texture2D> white_;
        std::unique_ptr<EnvironmentMapEffect> chrome_;
        int contentLostEvents_ = 0;
        bool checkedContentRestore_ = false;
    };
}

CNA_DEMOS_GAME(RenderTargetCubeDemo, {
    .id = "graphics.render_target.render_target_cube",
    .title = "Rendering into a cube map",
    .description = "Checks bound-face disposal and exact rendered-face readback, renders the scene six times a frame "
                   "into a RenderTargetCube, then samples it through a mirror sphere's EnvironmentMapEffect.",
    .apis = {"RenderTargetCube::RenderTargetCube", "RenderTargetBinding::RenderTargetBinding",
             "RenderTargetBinding::CubeMapFace", "GraphicsDevice::SetRenderTarget",
             "GraphicsDevice::GetRenderTargets", "GraphicsResource::Dispose",
             "GraphicsResource::IsDisposed", "CubeMapFace",
             "RenderTargetCube::ContentLost", "RenderTargetCube::IsContentLost",
             "RenderTargetCube::GetData",
             "RenderTargetCube::DepthStencilFormat", "RenderTargetCube::MultiSampleCount",
             "RenderTargetCube::RenderTargetUsage",
             "EnvironmentMapEffect::EnvironmentMap", "Matrix::CreatePerspectiveFieldOfView"},
    .tags = {"3d", "render-target", "cubemap", "reflection"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
})
