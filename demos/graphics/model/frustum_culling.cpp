// SPDX-License-Identifier: MS-PL
// Culling with ModelMesh::BoundingSphere: skip meshes outside the camera's BoundingFrustum.
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
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
    constexpr int GridSize = 9;  // 9 x 9 robot arms, 6 units apart

    class FrustumCullingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            arm_.emplace(getContentProperty().Load<Model>("models/robot_arm"));
            absolute_.resize(static_cast<std::size_t>(arm_->getBonesProperty().getCountProperty()));
            arm_->CopyAbsoluteBoneTransformsTo(absolute_);
            projection_ = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4,
                getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.5f, 60.0f);

            // From the centre of the field looking along -Z, only part of it is in view.
            const int drawn = DrawScene(0.0f, false);
            Context().Print("looking along -Z from the middle: {} of {} meshes inside the frustum", drawn,
                            GridSize * GridSize * 3);
            Context().Check(drawn > 0 && drawn < GridSize * GridSize * 3 / 2,
                            "fewer than half of the meshes pass the frustum test");
        }

        void Update(GameTime& gameTime) override
        {
            angle_ = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty()) * 0.4f;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::CornflowerBlue);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            const int drawn = DrawScene(angle_, true);

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "meshes drawn: " + std::to_string(drawn) + " of " +
                                                 std::to_string(GridSize * GridSize * 3),
                                     Vector2(20.0f, 20.0f), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        // Returns how many meshes were inside the view frustum (and draws them when asked).
        int DrawScene(float angle, bool draw)
        {
            const Vector3 eye(0.0f, 3.0f, 0.0f);
            const Vector3 forward(-std::sin(angle), -0.15f, -std::cos(angle));
            const Matrix view = Matrix::CreateLookAt(eye, eye + forward, Vector3::Up);
            // The frustum is the volume the camera sees.
            const BoundingFrustum frustum(view * projection_);

            int inside = 0;
            for (int gz = 0; gz < GridSize; ++gz)
            {
                for (int gx = 0; gx < GridSize; ++gx)
                {
                    const Matrix world = Matrix::CreateTranslation(static_cast<float>(gx - GridSize / 2) * 6.0f, 0.0f,
                                                                   static_cast<float>(gz - GridSize / 2) * 6.0f);
                    for (ModelMesh* mesh : arm_->getMeshesProperty())
                    {
                        // The mesh's sphere is in its bone's space: move it into the world first.
                        const Matrix meshWorld = absolute_[static_cast<std::size_t>(mesh->getParentBoneProperty()->getIndexProperty())] * world;
                        const BoundingSphere sphere = mesh->getBoundingSphereProperty().Transform(meshWorld);
                        if (!frustum.Intersects(sphere))
                            continue;
                        ++inside;
                        if (!draw)
                            continue;
                        for (Effect* effect : mesh->getEffectsProperty())
                        {
                            auto* basic = dynamic_cast<BasicEffect*>(effect);
                            basic->setWorldProperty(meshWorld);
                            basic->setViewProperty(view);
                            basic->setProjectionProperty(projection_);
                        }
                        mesh->Draw();
                    }
                }
            }
            return inside;
        }

        std::optional<Model> arm_;
        std::vector<Matrix> absolute_;
        Matrix projection_;
        float angle_ = 0.0f;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(FrustumCullingDemo, {
    .id = "graphics.model.frustum_culling",
    .title = "Frustum culling with bounding spheres",
    .description = "Draws a 9x9 field of robot arms from a turning camera, skipping every mesh whose "
                   "ModelMesh::BoundingSphere, moved into the world, misses the camera's BoundingFrustum; the count "
                   "of drawn meshes is shown.",
    .apis = {"ModelMesh::BoundingSphere", "BoundingSphere::Transform", "BoundingFrustum::BoundingFrustum",
             "BoundingFrustum::Intersects", "Model::CopyAbsoluteBoneTransformsTo", "ModelMesh::Draw", "Model::Meshes",
             "ModelMesh::ParentBone"},
    .tags = {"model", "culling", "frustum", "bounding-sphere", "performance", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .assets = {"fonts/pixel_font.cnb", "models/robot_arm.cnb"},
})
