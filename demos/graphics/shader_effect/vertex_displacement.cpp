// SPDX-License-Identifier: MS-PL
// Vertex texture fetch: a heightmap read in the vertex shader raises a flat grid into a hill.
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "CNA/Internal/Renderers/Common/IGraphicsRenderer.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using CNA::Internal::Renderers::ShaderDialectEXT;

namespace
{
    constexpr int Cells = 48;       // grid cells per side
    constexpr int HeightSize = 64;  // heightmap texels per side
    constexpr int HeightUnit = 1;   // sampler unit of the heightmap

    // The vertex shader moves each vertex up by the heightmap value under its texture coordinate.
    // (textureLod: a vertex shader has no derivatives to choose a mip level from.)
    constexpr const char* VertexBody = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
uniform mat4 World;
uniform mat4 View;
uniform mat4 Projection;
uniform sampler2D uHeight;
uniform float uScale;
out float vHeight;
void main()
{
    float h = textureLod(uHeight, aTexCoord, 0.0).r;
    vHeight = h;
    gl_Position = Projection * View * World * vec4(aPosition + vec3(0.0, h * uScale, 0.0), 1.0);
}
)";

    constexpr const char* FragmentBody = R"(
in float vHeight;
out vec4 FragColor;
void main()
{
    FragColor = vec4(mix(vec3(0.15, 0.45, 0.2), vec3(0.95, 0.95, 0.9), vHeight), 1.0);
}
)";

    const Color Sky(90, 130, 190);

    // Pixels in rows [y0, y1) that are not sky.
    int Covered(const CnaDemos::BackBufferPixels& p, int y0, int y1)
    {
        int count = 0;
        for (int y = y0; y < y1; ++y)
            for (int x = 0; x < p.Width(); ++x)
                if (!CnaDemos::Near(p.At(x, y), Sky, 2))
                    ++count;
        return count;
    }

    class VertexDisplacementDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const ShaderDialectEXT dialect = device.GetShaderDialectEXT();
            std::string header;
            if (dialect == ShaderDialectEXT::GlslEs)
                header = "#version 300 es\nprecision highp float;\nprecision highp sampler2D;\n";
            else if (dialect == ShaderDialectEXT::GlslDesktop)
                header = "#version 330 core\n";
            else
                Context().Skip("this demo carries GLSL; the renderer wants another shading language");
            if (!device.ExecutesShaderEffectSourceEXT())
                Context().Skip("this renderer accepts shader source but does not execute it");

            effect_ = std::make_unique<ShaderEffect>(device, header + VertexBody, header + FragmentBody);
            Context().Check(effect_->IsEffectValid(), "the GLSL compiled and linked");
            if (!effect_->IsEffectValid())
                Context().Print("compiler log:\n{}", effect_->GetCompileErrorEXT());
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 1.2f, 3.2f), Vector3(0.0f, 0.2f, 0.0f),
                                                          Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));

            // The heightmap: a round hill in the middle (any image would do -- a painted terrain).
            std::vector<Color> heights(HeightSize * HeightSize);
            for (int y = 0; y < HeightSize; ++y)
                for (int x = 0; x < HeightSize; ++x)
                {
                    const float dx = (x + 0.5f) / HeightSize - 0.5f;
                    const float dy = (y + 0.5f) / HeightSize - 0.5f;
                    const float r = std::sqrt(dx * dx + dy * dy) / 0.35f;
                    const float h = r < 1.0f ? 0.5f + 0.5f * std::cos(r * MathHelper::Pi) : 0.0f;
                    const auto v = static_cast<std::uint8_t>(h * 255.0f);
                    heights[y * HeightSize + x] = Color(v, v, v);
                }
            heightmap_ = std::make_unique<Texture2D>(device, HeightSize, HeightSize);
            heightmap_->SetData(heights.data(), static_cast<int>(heights.size()));

            // A flat grid on the XZ plane, 2 x 2 units, texture coordinates 0..1 across it.
            for (int z = 0; z <= Cells; ++z)
                for (int x = 0; x <= Cells; ++x)
                {
                    const float u = static_cast<float>(x) / Cells;
                    const float v = static_cast<float>(z) / Cells;
                    grid_.push_back(VertexPositionNormalTexture(Vector3(u * 2.0f - 1.0f, 0.0f, v * 2.0f - 1.0f),
                                                                Vector3::Up, Vector2(u, v)));
                }
            for (int z = 0; z < Cells; ++z)
                for (int x = 0; x < Cells; ++x)
                {
                    const auto i = static_cast<std::uint16_t>(z * (Cells + 1) + x);
                    const auto below = static_cast<std::uint16_t>(i + Cells + 1);
                    indices_.insert(indices_.end(), {i, below, static_cast<std::uint16_t>(i + 1),
                                                     static_cast<std::uint16_t>(i + 1), below,
                                                     static_cast<std::uint16_t>(below + 1)});
                }
        }

        void DrawGrid(float scale)
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Sky);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            effect_->Apply();
            effect_->SetTexture(HeightUnit, *heightmap_); // the heightmap on its own sampler unit
            effect_->SetUniformInt("uHeight", HeightUnit);
            effect_->SetUniformFloat("uScale", scale);
            device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, grid_.data(), 0,
                                             static_cast<int>(grid_.size()), indices_.data(), 0,
                                             static_cast<int>(indices_.size() / 3));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            if (FramesDrawn() == 0)
            {
                // Above the flat grid's far edge there is only sky; the hill rises into it.
                DrawGrid(0.0f);
                const CnaDemos::BackBufferPixels flat(device);
                DrawGrid(0.8f);
                const CnaDemos::BackBufferPixels hill(device);
                // The flat grid's far edge lies at row 226 of the 800 x 480 view; look above it.
                const int horizon = 220;
                const int flatCover = Covered(flat, 0, horizon);
                const int hillCover = Covered(hill, 0, horizon);
                Context().Print("non-sky pixels above the grid's far edge: flat grid {}, displaced grid {}", flatCover,
                                hillCover);
                Context().Check(flatCover == 0 && hillCover > 4000,
                                "the heightmap read in the vertex shader raises the grid into a hill");
            }
            DrawGrid(0.8f);
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<ShaderEffect> effect_;
        std::unique_ptr<Texture2D> heightmap_;
        std::vector<VertexPositionNormalTexture> grid_;
        std::vector<std::uint16_t> indices_;
    };
}

CNA_DEMOS_GAME(VertexDisplacementDemo, {
    .id = "graphics.shader_effect.vertex_displacement",
    .title = "Heightmap in the vertex shader",
    .description = "Binds a heightmap to a second sampler unit with ShaderEffect::SetTexture (CNAEXT) and reads it "
                   "in the GLSL vertex shader (vertex texture fetch): a flat grid becomes a hill without changing "
                   "a single vertex on the CPU.",
    .apis = {"ShaderEffect::SetTexture", "ShaderEffect::SetUniformInt", "ShaderEffect::SetUniformFloat",
             "ShaderEffect::ShaderEffect", "GraphicsDevice::DrawUserIndexedPrimitives"},
    .tags = {"3d", "shader", "glsl", "heightmap", "terrain", "vertex-texture", "cnaext"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::CustomEffects,
    .frames = 3,
    .deterministic = true,
    .restrictions = "GLSL only: renderers that take SPIR-V, HLSL, MSL or WGSL skip it.",
})
