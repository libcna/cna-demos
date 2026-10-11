// SPDX-License-Identifier: MS-PL
// DepthStencilState::TwoSidedStencilMode: a stencil shadow volume counted in one pass -- front
// (clockwise) faces increment, back (counter-clockwise) faces decrement -- instead of two culled passes.
#include <array>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ClearOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/ColorWriteChannels.hpp"
#include "Microsoft/Xna/Framework/Graphics/CompareFunction.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/StencilOperation.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // A horizontal quad (seen from above) as two clockwise triangles.
    std::array<VertexPositionColor, 6> Quad(float x0, float x1, float y, float z0, float z1, Color color)
    {
        const VertexPositionColor a(Vector3(x0, y, z0), color), b(Vector3(x1, y, z0), color),
            c(Vector3(x1, y, z1), color), d(Vector3(x0, y, z1), color);
        return {a, b, c, a, c, d};
    }

    // The shadow volume of the floating tile: a box from just below the tile to below the floor.
    // Each face is clockwise seen from outside, so its outside is its front.
    std::array<VertexPositionColor, 36> ShadowVolume()
    {
        constexpr float Top = 0.95f;
        constexpr float Bottom = -0.5f;
        const auto p = [](float x, float y, float z) { return VertexPositionColor(Vector3(x, y, z), Color::White); };
        // Corners of each face in clockwise order, as a viewer outside that face sees them.
        const std::array<std::array<VertexPositionColor, 4>, 6> faces = {{
            {p(-1, Top, 1), p(1, Top, 1), p(1, Bottom, 1), p(-1, Bottom, 1)},       // front (+z)
            {p(1, Top, -1), p(-1, Top, -1), p(-1, Bottom, -1), p(1, Bottom, -1)},   // back (-z)
            {p(1, Top, 1), p(1, Top, -1), p(1, Bottom, -1), p(1, Bottom, 1)},       // right (+x)
            {p(-1, Top, -1), p(-1, Top, 1), p(-1, Bottom, 1), p(-1, Bottom, -1)},   // left (-x)
            {p(-1, Top, -1), p(1, Top, -1), p(1, Top, 1), p(-1, Top, 1)},           // top (+y)
            {p(1, Bottom, -1), p(-1, Bottom, -1), p(-1, Bottom, 1), p(1, Bottom, 1)}, // bottom (-y)
        }};
        std::array<VertexPositionColor, 36> triangles;
        std::size_t n = 0;
        for (const auto& f : faces)
        {
            for (int index : {0, 1, 2, 0, 2, 3})
            {
                triangles[n++] = f[static_cast<std::size_t>(index)];
            }
        }
        return triangles;
    }

    const Color Floor(170, 190, 150);
    const Color Tile(240, 140, 40);

    // Equal within the rounding a GPU may add to vertex colours.
    bool Near(Color a, Color b)
    {
        const auto close = [](int x, int y) { return x - y <= 2 && y - x <= 2; };
        return close(a.getRProperty(), b.getRProperty()) && close(a.getGProperty(), b.getGProperty()) &&
               close(a.getBProperty(), b.getBProperty());
    }

    class TwoSidedStencilDemo final : public CnaDemos::DemoGame
    {
    public:
        TwoSidedStencilDemo() { Graphics().setPreferredDepthStencilFormatProperty(DepthFormat::Depth24Stencil8); }

    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            pixel_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setViewProperty(
                Matrix::CreateLookAt(Vector3(0.0f, 2.0f, 6.0f), Vector3(0.0f, 0.0f, -2.0f), Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));

            // Every volume pass tests depth against the scene but writes neither depth nor colour.
            const auto volumePass = [](DepthStencilState& state) {
                state.setDepthBufferWriteEnableProperty(false);
                state.setStencilEnableProperty(true);
                state.setStencilFunctionProperty(CompareFunction::Always);
            };
            // One pass: clockwise faces count up, counter-clockwise faces count down.
            volumePass(twoSided_);
            twoSided_.setTwoSidedStencilModeProperty(true);
            twoSided_.setStencilPassProperty(StencilOperation::Increment);
            twoSided_.setCounterClockwiseStencilFunctionProperty(CompareFunction::Always);
            twoSided_.setCounterClockwiseStencilPassProperty(StencilOperation::Decrement);
            // The classic way: the front faces in one pass, the back faces in another.
            volumePass(frontFaces_);
            frontFaces_.setStencilPassProperty(StencilOperation::Increment);
            volumePass(backFaces_);
            backFaces_.setStencilPassProperty(StencilOperation::Decrement);
            noColor_.setColorWriteChannelsProperty(ColorWriteChannels::None);

            // Then darken where the count is not zero: the floor inside the volume.
            inShadow_.setDepthBufferEnableProperty(false);
            inShadow_.setStencilEnableProperty(true);
            inShadow_.setStencilFunctionProperty(CompareFunction::NotEqual);
            inShadow_.setReferenceStencilProperty(0);
        }

        void Draw(const GameTime& gameTime) override
        {
            if (FramesDrawn() == 0)
            {
                // Under the tile, on the floor seen through the volume's upper part, and away from it.
                Render(false);
                const std::array<Color, 3> twoPasses = Sample();
                Render(true);
                const std::array<Color, 3> onePass = Sample();
                Context().Print("one pass: under the tile {}, through the volume {}, outside {}", onePass[0].ToString(),
                                onePass[1].ToString(), onePass[2].ToString());
                Context().Check(onePass[0].getRProperty() < Floor.getRProperty() * 3 / 4,
                                "the floor under the tile is in shadow (count 1)");
                Context().Check(Near(onePass[1], Floor),
                                "the floor seen through the volume above it is lit (+1 at the front, -1 at the back)");
                Context().Check(Near(onePass[2], Floor), "the floor away from the volume is lit (count 0)");
                Context().Check(Near(onePass[0], twoPasses[0]) && Near(onePass[1], twoPasses[1]) &&
                                    Near(onePass[2], twoPasses[2]),
                                "TwoSidedStencilMode gives the same image as two culled passes");
            }
            Render(true);
            Game::Draw(gameTime);
        }

    private:
        void Render(bool twoSided)
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(ClearOptions::Target | ClearOptions::DepthBuffer | ClearOptions::Stencil, Color(28, 30, 40),
                         1.0f, 0);

            // The scene: the floor and the floating tile, filling the depth buffer.
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            const auto floor = Quad(-6.0f, 6.0f, 0.0f, -10.0f, 4.0f, Floor);
            const auto tile = Quad(-1.0f, 1.0f, 1.0f, -1.0f, 1.0f, Tile);
            DrawTriangles(floor.data(), 2);
            DrawTriangles(tile.data(), 2);

            // The shadow volume, into the stencil buffer only.
            const auto volume = ShadowVolume();
            device.setBlendStateProperty(noColor_);
            if (twoSided)
            {
                device.setDepthStencilStateProperty(twoSided_);
                device.setRasterizerStateProperty(RasterizerState::CullNone); // both sides in one draw
                DrawTriangles(volume.data(), 12);
            }
            else
            {
                device.setDepthStencilStateProperty(frontFaces_);
                device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
                DrawTriangles(volume.data(), 12);
                device.setDepthStencilStateProperty(backFaces_);
                device.setRasterizerStateProperty(RasterizerState::CullClockwise);
                DrawTriangles(volume.data(), 12);
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::AlphaBlend, nullptr, &inShadow_, nullptr);
            spriteBatch_->Draw(*pixel_, Rectangle(0, 0, 800, 480), Color(0, 0, 0, 128));
            spriteBatch_->End();
        }

        void DrawTriangles(const VertexPositionColor* vertices, int triangles)
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices, 0, triangles);
            }
        }

        std::array<Color, 3> Sample()
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const CnaDemos::BackBufferPixels pixels(device);
            const auto at = [&](Vector3 point) {
                const Vector3 p = device.getViewportProperty().Project(point, effect_->getProjectionProperty(),
                                                                       effect_->getViewProperty(),
                                                                       Matrix::getIdentityProperty());
                return pixels.At(static_cast<int>(p.X), static_cast<int>(p.Y));
            };
            return {at(Vector3(0.0f, 0.0f, 0.5f)), at(Vector3(0.0f, 0.0f, -2.5f)), at(Vector3(3.0f, 0.0f, 1.5f))};
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> pixel_;
        std::unique_ptr<BasicEffect> effect_;
        DepthStencilState twoSided_;
        DepthStencilState frontFaces_;
        DepthStencilState backFaces_;
        DepthStencilState inShadow_;
        BlendState noColor_;
    };
}

CNA_DEMOS_GAME(TwoSidedStencilDemo, {
    .id = "graphics.depth_stencil_state.two_sided_stencil",
    .title = "Two-sided stencil: a shadow volume",
    .description = "The shadow volume under a floating tile is counted in one draw with TwoSidedStencilMode "
                   "(clockwise faces Increment, counter-clockwise faces Decrement): the floor inside the volume is "
                   "shadowed, the floor seen through it is not, and the result matches two culled passes.",
    .apis = {"DepthStencilState::TwoSidedStencilMode", "DepthStencilState::CounterClockwiseStencilFunction",
             "DepthStencilState::CounterClockwiseStencilPass", "StencilOperation::Decrement",
             "DepthStencilState::DepthBufferWriteEnable", "CompareFunction::NotEqual", "RasterizerState::CullClockwise",
             "RasterizerState::CullNone", "DepthStencilState::StencilEnable", "DepthStencilState::StencilFunction",
             "DepthStencilState::StencilPass", "DepthStencilState::ReferenceStencil"},
    .tags = {"stencil", "shadow", "shadow-volume", "3d", "state"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::StencilBuffer,
    .frames = 3,
    .deterministic = true,
})
