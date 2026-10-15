// SPDX-License-Identifier: MS-PL
// DepthBufferFunction: draw only where something is in front -- an "X-ray" silhouette through a wall.
#include <array>
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ClearOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/CompareFunction.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    std::array<VertexPositionColor, 6> Quad(Rectangle r, float z, Color color)
    {
        const auto x0 = static_cast<float>(r.X), y0 = static_cast<float>(r.Y);
        const auto x1 = static_cast<float>(r.X + r.Width), y1 = static_cast<float>(r.Y + r.Height);
        const VertexPositionColor a(Vector3(x0, y0, z), color), b(Vector3(x1, y0, z), color),
            c(Vector3(x0, y1, z), color), d(Vector3(x1, y1, z), color);
        return {a, b, c, b, d, c};
    }

    class DepthFunctionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 10.0f));

            // Only where the depth buffer is NEARER than the object -- i.e. where it is hidden.
            // Not written, so the silhouette never hides anything itself.
            xray_.setDepthBufferEnableProperty(true);
            xray_.setDepthBufferFunctionProperty(CompareFunction::Greater);
            xray_.setDepthBufferWriteEnableProperty(false);

            const auto comparison = [](DepthStencilState& state, CompareFunction function) {
                state.setDepthBufferEnableProperty(true);
                state.setDepthBufferWriteEnableProperty(false);
                state.setDepthBufferFunctionProperty(function);
            };
            comparison(less_, CompareFunction::Less);
            comparison(lessEqual_, CompareFunction::LessEqual);
            comparison(greater_, CompareFunction::Greater);
            comparison(greaterEqual_, CompareFunction::GreaterEqual);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            if (FramesDrawn() == 0)
                CheckEqualityBoundaries(device);
            device.Clear(Color::Black);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.setBlendStateProperty(BlendState::Opaque);

            const auto wall = Quad(Rectangle(300, 40, 200, 400), -2.0f, Color::SaddleBrown);
            const auto hero = Quad(Rectangle(180, 180, 300, 120), -6.0f, Color::Gold);
            const auto silhouette = Quad(Rectangle(180, 180, 300, 120), -6.0f, Color::Cyan);

            // 1. The scene, with the normal depth test: the wall hides part of the hero.
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            Draw(wall);
            Draw(hero);

            // 2. The hero again, in a flat colour, passing only where it is behind something.
            device.setDepthStencilStateProperty(xray_);
            Draw(silhouette);

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                Context().Check(p.At(240, 240) == Color::Gold, "the visible part of the hero is drawn normally");
                Context().Check(p.At(400, 240) == Color::Cyan, "the part behind the wall shows as the silhouette");
                Context().Check(p.At(400, 100) == Color::SaddleBrown, "the wall elsewhere is untouched");
            }

            Game::Draw(gameTime);
        }

    private:
        void CheckEqualityBoundaries(GraphicsDevice& device)
        {
            const std::array<DepthStencilState*, 4> comparisons = {
                &less_, &lessEqual_, &greater_, &greaterEqual_};
            const std::array<const char*, 4> names = {"Less", "LessEqual", "Greater", "GreaterEqual"};
            const std::array<bool, 4> equalityPasses = {false, true, false, true};

            device.Clear(ClearOptions::Target | ClearOptions::DepthBuffer, Color::Black, 1.0f, 0);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            for (std::size_t i = 0; i < comparisons.size(); ++i)
                Draw(Quad(Rectangle(40 + static_cast<int>(i) * 190, 200, 150, 100), -5.0f, Color::Crimson));
            for (std::size_t i = 0; i < comparisons.size(); ++i)
            {
                device.setDepthStencilStateProperty(*comparisons[i]);
                Draw(Quad(Rectangle(40 + static_cast<int>(i) * 190, 200, 150, 100), -5.0f, Color::Lime));
            }

            const CnaDemos::BackBufferPixels pixels(device);
            for (std::size_t i = 0; i < comparisons.size(); ++i)
            {
                const Color expected = equalityPasses[i] ? Color::Lime : Color::Crimson;
                Context().Check(pixels.At(115 + static_cast<int>(i) * 190, 250) == expected,
                                std::string(names[i]) + (equalityPasses[i] ? " accepts" : " rejects") +
                                    " equal depth");
            }
        }

        void Draw(const std::array<VertexPositionColor, 6>& quad)
        {
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                getGraphicsDeviceProperty().DrawUserPrimitives(PrimitiveType::TriangleList, quad.data(), 0, 2);
            }
        }

        std::unique_ptr<BasicEffect> effect_;
        DepthStencilState xray_;
        DepthStencilState less_;
        DepthStencilState lessEqual_;
        DepthStencilState greater_;
        DepthStencilState greaterEqual_;
    };
}

CNA_DEMOS_GAME(DepthFunctionDemo, {
    .id = "graphics.depth_stencil_state.depth_function",
    .title = "Depth compare function (X-ray)",
    .description = "Draws an object hidden partly behind a wall, then again with DepthBufferFunction Greater and no "
                   "depth writes, so only its hidden part appears -- as a silhouette.",
    .apis = {"DepthStencilState::DepthBufferFunction", "DepthStencilState::DepthBufferWriteEnable",
             "DepthStencilState::DepthBufferEnable", "CompareFunction::Greater", "CompareFunction::GreaterEqual",
             "CompareFunction::Less", "CompareFunction::LessEqual"},
    .tags = {"3d", "depth", "silhouette", "xray", "state"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
