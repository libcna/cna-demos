// SPDX-License-Identifier: MS-PL
// SetVertexBuffers: one vertex built from several buffers, so each attribute can be updated alone.
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DynamicVertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SetDataOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBufferBinding.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Slices = 12;
    constexpr int VertexCount = Slices * 3; // a fan of separate triangles around the centre

    // Stream 0: positions only. Stream 1: colours only, packed RGBA (the struct must be plain data:
    // CNA's Color is a class, so a colour stream holds its PackedValue).
    const VertexDeclaration& PositionStream()
    {
        static const VertexDeclaration declaration{
            VertexElement(0, VertexElementFormat::Vector3, VertexElementUsage::Position, 0)};
        return declaration;
    }

    const VertexDeclaration& ColorStream()
    {
        static const VertexDeclaration declaration{
            VertexElement(0, VertexElementFormat::Color, VertexElementUsage::Color, 0)};
        return declaration;
    }

    class SeveralStreamsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(-400.0f, 400.0f, -240.0f, 240.0f, 0.0f, 1.0f));

            // The shape never changes: its positions are uploaded once, to a static buffer.
            std::vector<Vector3> positions;
            positions.emplace_back(9000.0f, 9000.0f, 0.0f); // skipped by VertexOffset 1
            for (int i = 0; i < Slices; ++i)
            {
                const float a0 = MathHelper::TwoPi * static_cast<float>(i) / Slices;
                const float a1 = MathHelper::TwoPi * static_cast<float>(i + 1) / Slices;
                positions.emplace_back(0.0f, 0.0f, 0.0f);
                positions.emplace_back(200.0f * std::cos(a0), 200.0f * std::sin(a0), 0.0f);
                positions.emplace_back(200.0f * std::cos(a1), 200.0f * std::sin(a1), 0.0f);
            }
            positionBuffer_ = std::make_unique<VertexBuffer>(device, PositionStream(), VertexCount + 1,
                                                             BufferUsage::WriteOnly);
            positionBuffer_->SetData(positions.data(), VertexCount + 1);

            // The colours change every frame: a dynamic buffer of their own.
            colorBuffer_ = std::make_unique<DynamicVertexBuffer>(device, ColorStream(), VertexCount + 1,
                                                                 BufferUsage::WriteOnly);
            colors_.resize(VertexCount + 1);
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            const float t = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty());
            colors_[0] = Color::Black.getPackedValueProperty(); // skipped by VertexOffset 1
            for (int i = 0; i < Slices; ++i)
            {
                const float hue = static_cast<float>(i) / Slices * MathHelper::TwoPi + t;
                const Color rim(0.5f + 0.5f * std::cos(hue), 0.5f + 0.5f * std::cos(hue - 2.094f),
                                0.5f + 0.5f * std::cos(hue + 2.094f));
                colors_[static_cast<std::size_t>(i * 3 + 1)] = Color::White.getPackedValueProperty();
                colors_[static_cast<std::size_t>(i * 3 + 2)] = rim.getPackedValueProperty();
                colors_[static_cast<std::size_t>(i * 3 + 3)] = rim.getPackedValueProperty();
            }
            // Only the colour stream is rewritten; the positions stay where they are on the GPU.
            colorBuffer_->SetData(colors_.data(), 0, VertexCount + 1, SetDataOptions::Discard);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(26, 28, 40));
            device.setRasterizerStateProperty(RasterizerState::CullNone);

            // Both buffers feed the same vertices: element N of each stream forms vertex N.
            device.SetVertexBuffers({VertexBufferBinding(positionBuffer_.get(), 1),
                                     VertexBufferBinding(colorBuffer_.get(), 1)});
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawPrimitives(PrimitiveType::TriangleList, 0, Slices);
            }

            if (FramesDrawn() == 0)
            {
                const auto bindings = device.GetVertexBuffers();
                Context().Check(bindings.size() == 2 &&
                                    bindings[0].getVertexBufferProperty() == positionBuffer_.get() &&
                                    bindings[1].getVertexBufferProperty() == colorBuffer_.get() &&
                                    bindings[0].getVertexOffsetProperty() == 1 &&
                                    bindings[1].getVertexOffsetProperty() == 1 &&
                                    bindings[0].getInstanceFrequencyProperty() == 0 &&
                                    bindings[1].getInstanceFrequencyProperty() == 0,
                                "two exact per-vertex bindings retain their buffers and skip one prefix element");
                // The centre comes from stream 1's white, the rim from its hues, the shape from stream 0.
                const CnaDemos::BackBufferPixels pixels(device);
                const Color centre = pixels.At(400, 240);
                const Color outside = pixels.At(20, 20);
                Context().Print("centre {}, outside {}", centre.ToString(), outside.ToString());
                Context().Check(centre.getRProperty() > 200 && centre.getGProperty() > 200 && centre.getBProperty() > 200,
                                "the centre vertex takes its white from the colour stream");
                Context().Check(CnaDemos::Near(outside, Color(26, 28, 40), 2), "the shape from the position stream");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<VertexBuffer> positionBuffer_;
        std::unique_ptr<DynamicVertexBuffer> colorBuffer_;
        std::vector<std::uint32_t> colors_;
    };
}

CNA_DEMOS_GAME(SeveralStreamsDemo, {
    .id = "graphics.vertex_buffer.several_streams",
    .title = "Several vertex streams",
    .description = "Builds each vertex from two buffers bound together with SetVertexBuffers -- positions in a static "
                   "buffer uploaded once, colours in a dynamic buffer rewritten every frame -- so the animated "
                   "attribute is updated without touching the other.",
    .apis = {"GraphicsDevice::SetVertexBuffers", "GraphicsDevice::GetVertexBuffers",
             "VertexBufferBinding::VertexBufferBinding", "VertexBufferBinding::VertexBuffer",
             "VertexBufferBinding::VertexOffset", "DynamicVertexBuffer::SetData", "VertexElementFormat::Color",
             "Color::PackedValue", "GraphicsDevice::DrawPrimitives"},
    .tags = {"3d", "vertex-streams", "buffer", "animation"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 30,
})
