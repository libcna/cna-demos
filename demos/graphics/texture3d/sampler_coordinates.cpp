// SPDX-License-Identifier: MS-PL
// A real sampler3D: slice centers, interpolation between slices, and W addressing.
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
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture3D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureAddressMode.hpp"
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
    constexpr std::array<float, 5> Coordinates{-.25f, .25f, .375f, .75f, 1.25f};
    constexpr std::array<const char*, 5> Labels{"Z=-0.25", "Z=0.25", "Z=0.375", "Z=0.75", "Z=1.25"};
    constexpr std::array<const char*, 3> Rows{"PointClamp", "LinearClamp", "Linear / Wrap W"};
    constexpr const char* VertexSource = R"(
layout(location = 0) in vec3 aPosition;
void main() { gl_Position = vec4(aPosition, 1.0); }
)";
    constexpr const char* FragmentSource = R"(
uniform highp sampler3D volume;
uniform float sliceZ;
out vec4 FragColor;
void main() { FragColor = texture(volume, vec3(0.5, 0.5, sliceZ)); }
)";
    class VolumeSamplingDemo final : public CnaDemos::DemoGame
    {
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const auto dialect = device.GetShaderDialectEXT();
            const bool es = dialect == CNA::ShaderDialectEXT::GlslEs;
            if ((!es && dialect != CNA::ShaderDialectEXT::GlslDesktop) || !device.ExecutesShaderEffectSourceEXT())
                Context().Skip("volume sampling needs executable GLSL source");
            const std::string header = es ? "#version 300 es\nprecision highp float;\n" : "#version 330 core\n";
            shader_ = std::make_unique<ShaderEffect>(device, header + VertexSource, header + FragmentSource);
            if (!Context().Check(shader_->IsEffectValid(), "the custom sampler3D shader compiles"))
                throw std::runtime_error(shader_->GetCompileErrorEXT());
            volume_ = std::make_unique<Texture3D>(device, 2, 2, 2, false, SurfaceFormat::Color);
            // Z varies slowest. All four texels in each slice share a color, isolating W behavior.
            const std::array<Color, 8> texels{Color::Red, Color::Red, Color::Red, Color::Red,
                                             Color::Blue, Color::Blue, Color::Blue, Color::Blue};
            volume_->SetData(texels.data(), 8);
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
            const int cellWidth = full.getWidthProperty() / 5;
            const int rowHeight = (full.getHeightProperty() - 60) / 3;
            device.Clear(Color::Black);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            SamplerState wrapW = SamplerState::LinearClamp;
            wrapW.setAddressWProperty(TextureAddressMode::Wrap); // only the volume axis changes
            for (int row = 0; row < 3; ++row)
            {
                const auto& sampler = row == 0 ? SamplerState::PointClamp :
                                      row == 1 ? SamplerState::LinearClamp : wrapW;
                // Set the sampler on the SAME unit as the volume; SpriteBatch uses unit zero.
                device.getSamplerStatesProperty()(1, sampler);
                for (int column = 0; column < 5; ++column)
                {
                    device.setViewportProperty(Viewport(column * cellWidth + 12, row * rowHeight + 95,
                                                        cellWidth - 24, rowHeight - 50));
                    shader_->Apply();
                    shader_->SetTexture(1, *volume_);
                    shader_->SetUniformInt("volume", 1);
                    shader_->SetUniformFloat("sliceZ", Coordinates[column]);
                    device.DrawPrimitives(PrimitiveType::TriangleList, 0, 2);
                }
            }
            device.setViewportProperty(full);
            batch_->Begin();
            batch_->DrawString(*font_, "Texture3D: red slice at Z=0.25, blue at Z=0.75", Vector2(12, 12), Color::White);
            for (int column = 0; column < 5; ++column)
                batch_->DrawString(*font_, Labels[column], Vector2(static_cast<float>(column * cellWidth + 12), 40), Color::LightGray);
            for (int row = 0; row < 3; ++row)
                batch_->DrawString(*font_, Rows[row], Vector2(12, static_cast<float>(row * rowHeight + 72)), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels screen(device);
                const auto sample = [&](int row, int column) {
                    return screen.At(column * cellWidth + cellWidth / 2,
                                     row * rowHeight + 95 + (rowHeight - 50) / 2);
                };
                bool centers = true, clamped = true;
                for (int row = 0; row < 3; ++row)
                    centers = centers && CnaDemos::Near(sample(row, 1), Color::Red, 0) &&
                                         CnaDemos::Near(sample(row, 3), Color::Blue, 0);
                Context().Check(centers, "both authored slice centers retain their colors under all three sampler states");
                Context().Check(CnaDemos::Near(sample(0, 2), Color::Red, 0),
                                "point filtering selects the nearer red slice at Z=0.375");
                // At Z=.375, z*depth-.5=.25: 75% red and 25% blue, rounded to 8-bit channels.
                Context().Check(CnaDemos::Near(sample(1, 2), Color(191, 0, 64), 1) &&
                                CnaDemos::Near(sample(2, 2), Color(191, 0, 64), 1),
                                "linear filtering blends the two slices with the expected three-to-one weights");
                for (int row = 0; row < 2; ++row)
                    clamped = clamped && CnaDemos::Near(sample(row, 0), Color::Red, 0) &&
                                         CnaDemos::Near(sample(row, 4), Color::Blue, 0);
                Context().Check(clamped, "Clamp on W preserves the nearest endpoint color below zero and above one");
                Context().Check(CnaDemos::Near(sample(2, 0), Color::Blue, 0) &&
                                CnaDemos::Near(sample(2, 4), Color::Red, 0),
                                "Wrap on W repeats the opposite slice centers for the same out-of-range coordinates");
            }
            DemoGame::Draw(time);
        }
        std::unique_ptr<Texture3D> volume_;
        std::unique_ptr<ShaderEffect> shader_;
        std::optional<VertexDeclaration> declaration_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(VolumeSamplingDemo, {
    .id = "graphics.texture3d.sampler_coordinates",
    .title = "Sampling volume slices and the W axis",
    .description = "Binds a real two-slice Texture3D to an explicit custom sampler unit, compares point selection with "
                   "linear interpolation, and checks Clamp versus Wrap beyond the W coordinate range.",
    .apis = {"ShaderEffect::SetTexture", "ShaderEffect::SetUniformInt", "ShaderEffect::SetUniformFloat",
             "Texture3D::Texture3D", "Texture3D::SetData", "GraphicsDevice::SamplerStates", "SamplerState::PointClamp",
             "SamplerState::LinearClamp", "SamplerState::AddressW", "TextureAddressMode::Wrap"},
    .tags = {"texture", "volume", "shader", "sampler", "filtering", "cnaext"},
    .needs = CnaDemos::Need::Texture3D | CnaDemos::Need::CustomEffects,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "GLSL only. Backbuffer pixels establish actual sampling of a single-level Color volume; "
                    "no mip filtering, volume GetData/GPU readback, non-Color format or varying X/Y behavior is inferred.",
})
