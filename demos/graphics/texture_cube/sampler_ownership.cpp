// SPDX-License-Identifier: MS-PL
// A copied cube resource survives source disposal and is sampled by a custom GLSL program.
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "CNA/ShaderDialectEXT.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr const char* Vertex = R"(
layout(location = 0) in vec2 aPos;
uniform mat4 projection;
void main() { gl_Position = projection * vec4(aPos, 0.0, 1.0); }
)";
    constexpr const char* Fragment = R"(
uniform samplerCube retainedCube;
uniform vec3 direction;
out vec4 FragColor;
void main() { FragColor = texture(retainedCube, direction); }
)";
    class CubeSamplerOwnershipDemo final : public CnaDemos::DemoGame
    {
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const auto dialect = device.GetShaderDialectEXT();
            const bool es = dialect == CNA::ShaderDialectEXT::GlslEs;
            if ((!es && dialect != CNA::ShaderDialectEXT::GlslDesktop) || !device.ExecutesShaderEffectSourceEXT())
                Context().Skip("the cube sampler demo needs executable GLSL source");
            const std::string header = es ? "#version 300 es\nprecision mediump float;\n" : "#version 330 core\n";
            shader_ = std::make_unique<ShaderEffect>(device, header + Vertex, header + Fragment);
            if (!Context().Check(shader_->IsEffectValid(), "the custom samplerCube program compiles"))
                throw std::runtime_error(shader_->GetCompileErrorEXT());
            TextureCube original(device, 2, false, SurfaceFormat::Color);
            std::array<Color, 4> facePixels;
            for (std::size_t face = 0; face < colors_.size(); ++face)
            {
                facePixels.fill(colors_[face]);
                original.SetData(static_cast<CubeMapFace>(face), facePixels.data(), 4);
            }
            survivor_.emplace(original); // actual CNA shared-resource value copy
            original.setNameProperty("source wrapper");
            survivor_->setNameProperty("surviving wrapper");
            Context().Check(original.getNameProperty() == "source wrapper" &&
                            survivor_->getNameProperty() == "surviving wrapper",
                            "shared resource wrappers retain independent names");
            facePixels.fill(Color::Orange);
            original.SetData(CubeMapFace::PositiveZ, facePixels.data(), 4);
            std::array<Color, 4> observed;
            survivor_->GetData(CubeMapFace::PositiveZ, observed.data(), 4);
            Context().Check(observed == facePixels,
                            "a complete Color face upload remains shared through the cube value copy");
            colors_[4] = Color::Orange;
            original.Dispose();
            survivor_->GetData(CubeMapFace::PositiveZ, observed.data(), 4);
            Context().Check(original.getIsDisposedProperty() && !survivor_->getIsDisposedProperty() &&
                            observed == facePixels,
                            "disposing the source wrapper leaves the copied cube alive with the shared edit");
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            white_.emplace(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            const std::array<Vector3, 6> directions{Vector3::UnitX, -Vector3::UnitX,
                Vector3::UnitY, -Vector3::UnitY, Vector3::UnitZ, -Vector3::UnitZ};
            const std::array<const char*, 6> labels{"+X", "-X", "+Y", "-Y", "+Z shared edit", "-Z"};
            for (int face = 0; face < 6; ++face)
            {
                shader_->Apply();
                // Unit zero belongs to SpriteBatch's ordinary Texture2D. The cube uses unit one.
                shader_->SetTexture(1, *survivor_);
                shader_->SetUniformInt("retainedCube", 1);
                const auto& direction = directions[face];
                shader_->SetUniformVec3("direction", direction.X, direction.Y, direction.Z);
                batch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointClamp,
                              nullptr, nullptr, shader_.get());
                batch_->Draw(*white_, Rectangle(80 + (face % 3) * 240, 90 + (face / 3) * 190, 160, 120), Color::White);
                batch_->End();
            }
            batch_->Begin();
            batch_->DrawString(*font_, "Copied cube after original disposal", Vector2(80, 20), Color::White);
            for (int face = 0; face < 6; ++face)
                batch_->DrawString(*font_, labels[face], Vector2(static_cast<float>(80 + (face % 3) * 240),
                                                              static_cast<float>(65 + (face / 3) * 190)), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels screen(device);
                bool allFaces = true;
                for (int face = 0; face < 6; ++face)
                    allFaces = allFaces && CnaDemos::Near(screen.At(160 + (face % 3) * 240,
                                                                 150 + (face / 3) * 190), colors_[face], 1);
                Context().Check(allFaces, "the custom sampler reads all six surviving faces including the shared +Z edit");
            }
            DemoGame::Draw(time);
        }
        std::array<Color, 6> colors_{Color::Red, Color::Lime, Color::Blue, Color::Gold, Color::Cyan, Color::Magenta};
        std::unique_ptr<ShaderEffect> shader_;
        std::optional<TextureCube> survivor_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
        std::optional<Texture2D> white_;
    };
}

CNA_DEMOS_GAME(CubeSamplerOwnershipDemo, {
    .id = "graphics.texture_cube.sampler_ownership",
    .title = "Copied cube lifetime and custom sampling",
    .description = "Edits a shared cube face, disposes the source wrapper and samples all six surviving faces through ShaderEffect's cube texture binding.",
    .apis = {"TextureCube::TextureCube", "TextureCube::SetData", "TextureCube::GetData", "GraphicsResource::Dispose", "GraphicsResource::Name",
             "ShaderEffect::SetTexture", "ShaderEffect::SetUniformInt", "ShaderEffect::SetUniformVec3"},
    .tags = {"texture", "cube", "shader", "ownership", "cnaext"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::CustomEffects,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "GLSL source compilation and cube sampling required. Solid base-level faces establish ownership and face selection, not orientation within a face, filtering or physical GPU readback.",
})
