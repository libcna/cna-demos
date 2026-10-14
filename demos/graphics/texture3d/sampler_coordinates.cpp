// SPDX-License-Identifier: MS-PL
// A real sampler3D: XYZ coordinates, slice interpolation, W addressing, and authored mip filtering.
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
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
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"
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
uniform vec3 coordinate;
uniform float lod;
out vec4 FragColor;
void main() { FragColor = textureLod(volume, coordinate, lod); }
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
            // X varies fastest and Z slowest. The four slice-zero corners establish U/V
            // orientation while every central texel stays red, preserving the isolated W tests.
            std::array<Color, 32> texels;
            std::fill_n(texels.begin(), 16, Color::Red);
            std::fill_n(texels.begin() + 16, 16, Color::Blue);
            texels[0] = Color::White;
            texels[3] = Color::Yellow;
            texels[12] = Color::Cyan;
            texels[15] = Color::Magenta;
            const std::array<Color, 4> mipOne{Color::Lime, Color::Lime, Color::Lime, Color::Lime};
            const Color mipTwo = Color::Blue;
            {
                // Stage an upload, then transfer ownership into the scene. No sampler references
                // either temporary; shader binding happens only after both have been destroyed.
                Texture3D uploaded(device, 4, 4, 2, true, SurfaceFormat::Color);
                uploaded.SetData(texels.data(), 32);
                uploaded.SetData(1, 0, 0, 2, 2, 0, 1, mipOne.data(), 0, 4);
                uploaded.SetData(2, 0, 0, 1, 1, 0, 1, &mipTwo, 0, 1);
                Texture3D staged(std::move(uploaded));
                uploaded.Dispose();
                std::array<Color, 32> observed;
                observed.fill(Color::Black);
                staged.GetData(observed.data(), 32);
                Context().Check(!staged.getIsDisposedProperty() && observed == texels,
                                "move construction keeps the uploaded volume alive after the source wrapper is disposed");

                // A distinct occupied destination makes assignment's replacement observable.
                volume_ = std::make_unique<Texture3D>(device, 1, 1, 1, false, SurfaceFormat::Color);
                const Color placeholder = Color::Green;
                volume_->SetData(&placeholder, 1);
                *volume_ = std::move(staged);
                staged.Dispose();
                observed.fill(Color::Black);
                volume_->GetData(observed.data(), 32);
                Context().Check(!volume_->getIsDisposedProperty() && volume_->getWidthProperty() == 4 &&
                                volume_->getHeightProperty() == 4 && volume_->getDepthProperty() == 2 &&
                                observed == texels,
                                "move assignment replaces an occupied destination and retains both slices after the staging wrapper is disposed");
            } // both moved-from wrappers are destroyed before any sampler3D draw

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
                    shader_->SetUniformVec3("coordinate", .5f, .5f, Coordinates[column]);
                    shader_->SetUniformFloat("lod", 0.0f);
                    device.DrawPrimitives(PrimitiveType::TriangleList, 0, 2);
                }
            }
            const auto drawSample = [&](const Viewport& viewport, const SamplerState& sampler,
                                        float x, float y, float z, float lod) {
                device.setViewportProperty(viewport);
                device.getSamplerStatesProperty()(1, sampler);
                shader_->Apply();
                shader_->SetTexture(1, *volume_);
                shader_->SetUniformInt("volume", 1);
                shader_->SetUniformVec3("coordinate", x, y, z);
                shader_->SetUniformFloat("lod", lod);
                device.DrawPrimitives(PrimitiveType::TriangleList, 0, 2);
            };
            // Four small cards occupy the corners of the red slice-centre cell without covering
            // its centre sample. Exact texel-centre coordinates expose both U and V orientation.
            constexpr int swatch = 20;
            const int left = cellWidth + 20;
            const int right = cellWidth * 2 - 40;
            const int top = 103;
            const int bottom = rowHeight + 17;
            drawSample(Viewport(left, top, swatch, swatch), SamplerState::PointClamp,
                       .125f, .125f, .25f, 0.0f);
            drawSample(Viewport(right, top, swatch, swatch), SamplerState::PointClamp,
                       .875f, .125f, .25f, 0.0f);
            drawSample(Viewport(left, bottom, swatch, swatch), SamplerState::PointClamp,
                       .125f, .875f, .25f, 0.0f);
            drawSample(Viewport(right, bottom, swatch, swatch), SamplerState::PointClamp,
                       .875f, .875f, .25f, 0.0f);
            // Both cards request LOD1.25 from authored Lime mip1 and Blue mip2. Point mip
            // selection keeps mip1; linear mip selection blends the levels with 3:1 weights.
            SamplerState mipPoint = SamplerState::LinearClamp;
            mipPoint.setFilterProperty(TextureFilter::LinearMipPoint);
            SamplerState mipLinear = SamplerState::PointClamp;
            mipLinear.setFilterProperty(TextureFilter::PointMipLinear);
            const int mipTop = rowHeight + 103;
            drawSample(Viewport(left, mipTop, swatch, swatch), mipPoint,
                       .5f, .5f, .5f, 1.25f);
            drawSample(Viewport(right, mipTop, swatch, swatch), mipLinear,
                       .5f, .5f, .5f, 1.25f);
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
                const std::array<Color, 4> expectedCorners{
                    Color::White, Color::Yellow, Color::Cyan, Color::Magenta};
                const std::array<std::pair<int, int>, 4> cornerPixels{{
                    {left + swatch / 2, top + swatch / 2},
                    {right + swatch / 2, top + swatch / 2},
                    {left + swatch / 2, bottom + swatch / 2},
                    {right + swatch / 2, bottom + swatch / 2}}};
                bool orientedXY = true;
                for (std::size_t corner = 0; corner < cornerPixels.size(); ++corner)
                    orientedXY = orientedXY && CnaDemos::Near(
                        screen.At(cornerPixels[corner].first, cornerPixels[corner].second),
                        expectedCorners[corner], 1);
                Context().Check(orientedXY,
                                "slice-zero U/V coordinates preserve the uploaded top-left through bottom-right corner order");
                Context().Check(CnaDemos::Near(screen.At(left + swatch / 2, mipTop + swatch / 2),
                                              Color::Lime, 1) &&
                                    CnaDemos::Near(screen.At(right + swatch / 2, mipTop + swatch / 2),
                                                  Color(0, 191, 64), 2),
                                "point versus linear mip filtering selects mip1 or blends mip1/mip2 at LOD1.25");
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
    .description = "Binds a moved two-slice Texture3D to an explicit sampler unit, verifies U/V/W coordinates, compares "
                   "point and linear filtering within and between authored mip levels, and checks W wrapping.",
    .apis = {"ShaderEffect::SetTexture", "ShaderEffect::SetUniformInt", "ShaderEffect::SetUniformFloat",
             "ShaderEffect::SetUniformVec3", "SamplerState::Filter", "TextureFilter::LinearMipPoint",
             "TextureFilter::PointMipLinear",
             "Texture3D::Texture3D", "Texture3D::SetData", "Texture3D::GetData", "GraphicsResource::Dispose",
             "GraphicsDevice::SamplerStates", "SamplerState::PointClamp",
             "SamplerState::LinearClamp", "SamplerState::AddressW", "TextureAddressMode::Wrap"},
    .tags = {"texture", "volume", "shader", "sampler", "filtering", "ownership", "cnaext"},
    .needs = CnaDemos::Need::Texture3D | CnaDemos::Need::CustomEffects,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "GLSL only. Backbuffer pixels establish Color-volume coordinate, addressing and explicit-LOD filtering "
                    "behavior; public GetData checks retained base payloads after moves. No direct GPU readback, active-binding "
                    "move, external-reference rebinding, cross-device assignment or non-Color sampling is inferred.",
})
