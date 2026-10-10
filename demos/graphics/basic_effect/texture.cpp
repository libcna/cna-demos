// SPDX-License-Identifier: MS-PL
// BasicEffect::TextureEnabled: a textured cube -- texture coordinates from the vertices, colour from the texture.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class TextureDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            texture_ = getContentProperty().Load<Texture2D>("textures/checker");
            cube_ = CnaDemos::MakeCube(1.0f);

            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setTextureEnabledProperty(true);
            effect_->setTextureProperty(&texture_);
            effect_->setWorldProperty(Matrix::CreateRotationY(MathHelper::ToRadians(35.0f)) *
                                      Matrix::CreateRotationX(MathHelper::ToRadians(25.0f)));
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 4.5f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::SteelBlue);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, cube_.vertices.data(), 0,
                                                 cube_.VertexCount(), cube_.indices.data(), 0, cube_.PrimitiveCount());
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                const Color centre = p.At(400, 240);
                Context().Print("pixel at the centre: {}", centre.ToString());
                Context().Check(centre != Color::SteelBlue, "the cube covers the centre");
                Context().Check(centre.getRProperty() == centre.getGProperty() ||
                                    centre.getBProperty() > centre.getRProperty(),
                                "the colour comes from the checker texture (white or slate blue)");
            }

            Game::Draw(gameTime);
        }

    private:
        Texture2D texture_;
        CnaDemos::Mesh cube_;
        std::unique_ptr<BasicEffect> effect_;
    };
}

CNA_DEMOS_GAME(TextureDemo, {
    .id = "graphics.basic_effect.texture",
    .title = "Textured cube",
    .description = "Draws a cube with BasicEffect::TextureEnabled and a Texture; each vertex carries the texture "
                   "coordinate used for its corner of the face.",
    .apis = {"BasicEffect::TextureEnabled", "BasicEffect::Texture", "VertexPositionNormalTexture::VertexPositionNormalTexture",
             "GraphicsDevice::DrawUserIndexedPrimitives"},
    .tags = {"3d", "texture", "effect", "cube"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
