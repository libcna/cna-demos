// SPDX-License-Identifier: MS-PL
// DynamicVertexBuffer streaming options: a three-segment ring carries a waving strip.
#include <cmath>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DynamicVertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SetDataOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/EventArgs.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Columns = 64;
    constexpr int SegmentVertices = 2 * Columns;
    constexpr int RingSegments = 3;

    class DynamicVertexBufferDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 1.0f));

            buffer_ = std::make_unique<DynamicVertexBuffer>(device, VertexPositionColor::getVertexDeclarationStatic(),
                                                            RingSegments * SegmentVertices, BufferUsage::WriteOnly);
            // A dynamic buffer's contents can be lost (device reset); the game must then refill it.
            // Here it is refilled every frame anyway, so the handler only notes it.
            buffer_->ContentLost += [this](System::Object*, const System::EventArgs&) { ++contentLost_; };
            strip_.resize(SegmentVertices);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setRasterizerStateProperty(RasterizerState::CullNone);

            // New positions each frame (driven by the frame number, so every run draws the same frames).
            const float phase = static_cast<float>(FramesDrawn()) * 0.15f;
            for (int i = 0; i < Columns; ++i)
            {
                const float x = 40.0f + static_cast<float>(i) * 720.0f / (Columns - 1);
                const float wave = 60.0f * std::sin(phase + static_cast<float>(i) * 0.2f);
                const Color color(static_cast<int>(4 * i), 128, 255 - static_cast<int>(4 * i));
                strip_[static_cast<std::size_t>(2 * i)] = VertexPositionColor(Vector3(x, 200.0f + wave, 0.0f), color);
                strip_[static_cast<std::size_t>(2 * i + 1)] = VertexPositionColor(Vector3(x, 280.0f + wave, 0.0f), Color::White);
            }

            const int segment = FramesDrawn() % RingSegments;
            const int stride = VertexPositionColor::getVertexDeclarationStatic().getVertexStrideProperty();
            const SetDataOptions options = FramesDrawn() == 0
                ? SetDataOptions::None
                : (segment == 0 ? SetDataOptions::Discard : SetDataOptions::NoOverwrite);
            buffer_->SetData(segment * SegmentVertices * stride, strip_.data(), 0,
                             static_cast<int>(strip_.size()), stride, options);

            device.SetVertexBuffer(buffer_.get());
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawPrimitives(PrimitiveType::TriangleStrip, segment * SegmentVertices,
                                      SegmentVertices - 2);
            }

            if (FramesDrawn() == 0)
            {
                bool ordinaryRefused = false;
                try
                {
                    buffer_->SetData(SegmentVertices * stride, strip_.data(), 0,
                                     static_cast<int>(strip_.size()), stride, SetDataOptions::None);
                }
                catch (const System::InvalidOperationException&) { ordinaryRefused = true; }

                bool noOverwriteAccepted = true;
                try
                {
                    buffer_->SetData(SegmentVertices * stride, strip_.data(), 0,
                                     static_cast<int>(strip_.size()), stride,
                                     SetDataOptions::NoOverwrite);
                }
                catch (...) { noOverwriteAccepted = false; }
                Context().Check(ordinaryRefused && noOverwriteAccepted,
                                "None refuses a bound update while NoOverwrite accepts a disjoint ring segment");
            }

            device.SetVertexBuffer(nullptr);

            Game::Draw(gameTime);
        }

        void EndRun() override
        {
            Context().Print("frames {}, IsContentLost {}, ContentLost raised {} time(s)", FramesDrawn(),
                            buffer_->getIsContentLostProperty(), contentLost_);
            Context().Check(!buffer_->getIsContentLostProperty(), "the contents were not lost during the run");
            Game::EndRun();
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<DynamicVertexBuffer> buffer_;
        std::vector<VertexPositionColor> strip_;
        int contentLost_ = 0;
    };
}

CNA_DEMOS_GAME(DynamicVertexBufferDemo, {
    .id = "graphics.vertex_buffer.dynamic_vertex_buffer",
    .title = "Dynamic vertex buffer",
    .description = "Streams a waving strip through a three-segment DynamicVertexBuffer ring with None, "
                   "NoOverwrite and Discard, and watches its ContentLost event.",
    .apis = {"DynamicVertexBuffer::DynamicVertexBuffer", "DynamicVertexBuffer::SetData", "DynamicVertexBuffer::ContentLost",
             "DynamicVertexBuffer::IsContentLost", "SetDataOptions::None", "SetDataOptions::Discard",
             "SetDataOptions::NoOverwrite", "PrimitiveType::TriangleStrip"},
    .tags = {"3d", "buffer", "dynamic", "animation"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 60,
    .deterministic = true,
})
