// SPDX-License-Identifier: MS-PL
// A three-tap image filter: vec2 offsets and scalar weights are different uniform arrays.
#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "CNA/ShaderDialectEXT.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerStateCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    struct Vertex { float x, y, z; };
    constexpr const char* VertexSource = R"(
layout(location = 0) in vec3 aPosition;
out vec2 uv;
void main() {
    gl_Position = vec4(aPosition, 1.0);
    uv = (aPosition.xy + vec2(1.0)) * 0.5;
}
)";
    constexpr const char* FragmentSource = R"(
in vec2 uv;
uniform sampler2D sourceImage;
uniform vec2 texelSize;
uniform vec2 offsets[3];
uniform float weights[3];
out vec4 FragColor;
void main() {
    vec3 filtered = vec3(0.0);
    for (int tap = 0; tap < 3; ++tap)
        filtered += texture(sourceImage, uv + offsets[tap] * texelSize).rgb * weights[tap];
    FragColor = vec4(filtered, 1.0);
}
)";
    class SampleKernelDemo final : public CnaDemos::DemoGame
    {
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const auto dialect = device.GetShaderDialectEXT();
            const bool es = dialect == CNA::ShaderDialectEXT::GlslEs;
            if ((!es && dialect != CNA::ShaderDialectEXT::GlslDesktop) || !device.ExecutesShaderEffectSourceEXT())
                Context().Skip("the sample kernel needs executable GLSL source");
            const std::string header = es ? "#version 300 es\nprecision highp float;\n" : "#version 330 core\n";
            shader_ = std::make_unique<ShaderEffect>(device, header + VertexSource, header + FragmentSource);
            if (!Context().Check(shader_->IsEffectValid(), "the three-tap image filter shader compiles"))
                throw std::runtime_error(shader_->GetCompileErrorEXT());
            source_ = std::make_unique<Texture2D>(device, 7, 5);
            std::array<Color, 35> texels;
            texels.fill(Color::Black);
            texels[2 * 7 + 3] = Color::White; // a single impulse isolates each tap's contribution
            source_->SetData(texels.data(), static_cast<int>(texels.size()));
            declaration_.emplace(static_cast<int>(sizeof(Vertex)), std::vector<VertexElement>{
                VertexElement(0, VertexElementFormat::Vector3, VertexElementUsage::Position, 0)});
            const std::array<Vertex, 6> quad{{{-1,-1,0}, {1,-1,0}, {-1,1,0},
                                             {-1,1,0}, {1,-1,0}, {1,1,0}}};
            vertices_ = std::make_unique<VertexBuffer>(device, *declaration_, 6, BufferUsage::WriteOnly);
            vertices_->SetData(quad.data(), 6);
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            const auto full = device.getViewportProperty();
            device.Clear(Color(24, 28, 36));
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            device.getSamplerStatesProperty()(1, SamplerState::PointClamp);
            for (int panel = 0; panel < 3; ++panel)
            {
                // Counts refer to 3 scalar weights and 3 vec2 offsets (six explicit floats).
                std::array<float, 3> weights = panel == 0 ? std::array<float, 3>{0, 1, 0} :
                    panel == 1 ? std::array<float, 3>{.25f, .5f, .25f} :
                                 std::array<float, 3>{.125f, .75f, .125f};
                std::array<float, 6> offsets = panel == 2 ? std::array<float, 6>{0, -1, 0, 0, 0, 1} :
                                                                          std::array<float, 6>{-1, 0, 0, 0, 1, 0};
                device.setViewportProperty(Viewport(30 + panel * 250, 150, 210, 150));
                shader_->Apply();
                shader_->SetTexture(1, *source_);
                shader_->SetUniformInt("sourceImage", 1);
                shader_->SetUniformVec2("texelSize", 1.0f / 7, 1.0f / 5);
                shader_->SetUniformVec2Array("offsets", offsets.data(), 3);
                shader_->SetUniformFloatArray("weights", weights.data(), 3);
                offsets.fill(0); weights.fill(0); // caller storage no longer supplies the draw
                device.DrawPrimitives(PrimitiveType::TriangleList, 0, 2);
            }
            device.setViewportProperty(full);
            batch_->Begin();
            batch_->DrawString(*font_, "One image, three sample kernels", Vector2(30, 35), Color::White);
            constexpr std::array<const char*, 3> labels{"Original", "Horizontal 1:2:1", "Vertical 1:6:1"};
            for (int panel = 0; panel < 3; ++panel)
                batch_->DrawString(*font_, labels[panel], Vector2(static_cast<float>(30 + panel * 250), 115), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels screen(device);
                const auto sample = [&](int panel, int x, int y) {
                    return screen.At(30 + panel * 250 + 105 + x * 30, 225 + y * 30);
                };
                bool clearBorders = true;
                for (int panel = 0; panel < 3; ++panel)
                {
                    const int center = panel == 0 ? 255 : panel == 1 ? 128 : 191;
                    const int side = panel == 0 ? 0 : panel == 1 ? 64 : 32;
                    bool correct = CnaDemos::Near(sample(panel, 0, 0), Color(center, center, center), 1);
                    for (int sign : {-1, 1})
                    {
                        const auto along = panel == 2 ? sample(panel, 0, sign) : sample(panel, sign, 0);
                        const auto across = panel == 2 ? sample(panel, sign, 0) : sample(panel, 0, sign);
                        correct = correct && CnaDemos::Near(along, Color(side, side, side), 1) &&
                                             CnaDemos::Near(across, Color::Black, 0);
                        clearBorders = clearBorders && CnaDemos::Near(sample(panel, sign * 2, 0), Color::Black, 0) &&
                                                       CnaDemos::Near(sample(panel, 0, sign * 2), Color::Black, 0);
                    }
                    Context().Check(correct, std::string(labels[panel]) +
                        " uses the current offset and weight arrays after caller storage is overwritten");
                }
                Context().Check(clearBorders, "all three kernels preserve black texels beyond their one-texel support");
            }
            DemoGame::Draw(time);
        }
        std::unique_ptr<Texture2D> source_;
        std::unique_ptr<ShaderEffect> shader_;
        std::optional<VertexDeclaration> declaration_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(SampleKernelDemo, {
    .id = "graphics.shader_effect.sample_kernel",
    .title = "Image filtering with offset and weight arrays",
    .description = "Filters a single bright texel with authored three-tap kernels. Distinct vec2 offsets and scalar "
                   "weights update on the same shader, changing direction and spread; actual pixels verify each tap.",
    .apis = {"ShaderEffect::SetUniformVec2", "ShaderEffect::SetUniformVec2Array", "ShaderEffect::SetUniformFloatArray",
             "ShaderEffect::SetTexture", "GraphicsDevice::SamplerStates", "GraphicsDevice::DrawPrimitives"},
    .tags = {"shader", "uniform", "array", "filter", "cnaext"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::CustomEffects,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "GLSL only. Uses three valid taps and point sampling; does not establish invalid counts, "
                    "missing uniforms, other shader dialects or a multi-pass blur pipeline.",
})
