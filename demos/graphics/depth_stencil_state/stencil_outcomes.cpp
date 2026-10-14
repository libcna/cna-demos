// SPDX-License-Identifier: MS-PL
// The four stencil failure outcomes: stencil-test and depth-test failure, on both face windings.
#include <array>
#include <memory>
#include <string>

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
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    struct Case
    {
        const char* name;
        bool counterClockwise;
        Color color;
    };

    const std::array<Case, 4> Cases = {{
        {"clockwise stencil fail", false, Color::Crimson},
        {"clockwise depth fail", false, Color::LimeGreen},
        {"counter-clockwise stencil fail", true, Color::CornflowerBlue},
        {"counter-clockwise depth fail", true, Color::Gold},
    }};

    std::array<VertexPositionColor, 3> Triangle(float x, float y, bool counterClockwise)
    {
        const VertexPositionColor top(Vector3(x, y + 0.65f, 0.0f), Color::White);
        const VertexPositionColor left(Vector3(x - 0.75f, y - 0.65f, 0.0f), Color::White);
        const VertexPositionColor right(Vector3(x + 0.75f, y - 0.65f, 0.0f), Color::White);
        // XNA's screen convention treats top/right/left as clockwise.
        return counterClockwise ? std::array<VertexPositionColor, 3>{top, left, right}
                                : std::array<VertexPositionColor, 3>{top, right, left};
    }

    class StencilOutcomesDemo final : public CnaDemos::DemoGame
    {
    public:
        StencilOutcomesDemo() { Graphics().setPreferredDepthStencilFormatProperty(DepthFormat::Depth24Stencil8); }

    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 5.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreateOrthographic(8.0f, 4.8f, 0.1f, 10.0f));
            batch_ = std::make_unique<SpriteBatch>(device);
            pixel_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
            noColor_.setColorWriteChannelsProperty(ColorWriteChannels::None);

            for (std::size_t i = 0; i < write_.size(); ++i)
            {
                DepthStencilState& write = write_[i];
                write.setStencilEnableProperty(true);
                write.setReferenceStencilProperty(static_cast<int>(i + 1));
                write.setDepthBufferWriteEnableProperty(false);
                if (Cases[i].counterClockwise)
                {
                    write.setTwoSidedStencilModeProperty(true);
                    write.setCounterClockwiseStencilFunctionProperty(
                        i == 2 ? CompareFunction::Never : CompareFunction::Always);
                    write.setCounterClockwiseStencilFailProperty(
                        i == 2 ? StencilOperation::Replace : StencilOperation::Keep);
                    write.setCounterClockwiseStencilDepthBufferFailProperty(
                        i == 3 ? StencilOperation::Replace : StencilOperation::Keep);
                }
                else
                {
                    write.setStencilFunctionProperty(i == 0 ? CompareFunction::Never : CompareFunction::Always);
                    write.setStencilFailProperty(i == 0 ? StencilOperation::Replace : StencilOperation::Keep);
                    write.setStencilDepthBufferFailProperty(
                        i == 1 ? StencilOperation::Replace : StencilOperation::Keep);
                }
                const bool depthFailure = i == 1 || i == 3;
                write.setDepthBufferEnableProperty(depthFailure);
                if (depthFailure)
                    write.setDepthBufferFunctionProperty(CompareFunction::Never);

                DepthStencilState& show = show_[i];
                show.setDepthBufferEnableProperty(false);
                show.setStencilEnableProperty(true);
                show.setStencilFunctionProperty(CompareFunction::Equal);
                show.setReferenceStencilProperty(static_cast<int>(i + 1));
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(ClearOptions::Target | ClearOptions::DepthBuffer | ClearOptions::Stencil,
                         Color::Black, 1.0f, 0);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.setBlendStateProperty(noColor_);

            for (std::size_t i = 0; i < Cases.size(); ++i)
            {
                const auto vertices = Triangle(X(i), Y(i), Cases[i].counterClockwise);
                device.setDepthStencilStateProperty(write_[i]);
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices.data(), 0, 1);
                }
            }

            // Each stencil value reveals only the triangle whose selected failure operation wrote it.
            for (std::size_t i = 0; i < Cases.size(); ++i)
            {
                batch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, nullptr, &show_[i], nullptr);
                batch_->Draw(*pixel_, Rectangle(0, 0, 800, 480), Cases[i].color);
                batch_->End();
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                for (std::size_t i = 0; i < Cases.size(); ++i)
                {
                    const Vector3 centre = device.getViewportProperty().Project(
                        Vector3(X(i), Y(i) - 0.2f, 0.0f), effect_->getProjectionProperty(),
                        effect_->getViewProperty(), Matrix::getIdentityProperty());
                    Context().Check(pixels.At(static_cast<int>(centre.X), static_cast<int>(centre.Y)) == Cases[i].color,
                                    std::string(Cases[i].name) + " selects its Replace operation");
                }
                Context().Check(pixels.At(400, 240) == Color::Black,
                                "pixels outside the four triangles keep stencil zero and remain clear");
            }

            Game::Draw(gameTime);
        }

    private:
        static float X(std::size_t i) { return i % 2 == 0 ? -2.0f : 2.0f; }
        static float Y(std::size_t i) { return i < 2 ? 1.2f : -1.2f; }

        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<SpriteBatch> batch_;
        std::unique_ptr<Texture2D> pixel_;
        std::array<DepthStencilState, 4> write_;
        std::array<DepthStencilState, 4> show_;
        BlendState noColor_;
    };
}

CNA_DEMOS_GAME(StencilOutcomesDemo, {
    .id = "graphics.depth_stencil_state.stencil_outcomes",
    .title = "Stencil and depth failure outcomes",
    .description = "Draws clockwise and counter-clockwise triangles whose stencil-test or depth-test failure writes "
                   "a distinct stencil value, then reveals each value as a different colour.",
    .apis = {"DepthStencilState::StencilFail", "DepthStencilState::StencilDepthBufferFail",
             "DepthStencilState::CounterClockwiseStencilFail",
             "DepthStencilState::CounterClockwiseStencilDepthBufferFail", "StencilOperation::Replace",
             "DepthStencilState::TwoSidedStencilMode", "CompareFunction::Never"},
    .tags = {"stencil", "depth", "failure", "two-sided", "winding", "state"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::StencilBuffer,
    .frames = 3,
    .deterministic = true,
})
