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
            TextureCube original(device, 4, false, SurfaceFormat::Color);
            std::array<Color, 16> facePixels;
            for (std::size_t face = 0; face < colors_.size(); ++face)
            {
                facePixels.fill(colors_[face]);
                original.SetData(static_cast<CubeMapFace>(face), facePixels.data(), 16);
            }
            // Keep the +X centre red but author four distinct corners. GLSL's +X cube mapping is
            // s=(-Z/X+1)/2, t=(-Y/X+1)/2, so later directions can address these exact texel centres.
            facePixels.fill(Color::Red);
            facePixels[0] = Color::White;
            facePixels[3] = Color::Yellow;
            facePixels[12] = Color::Cyan;
            facePixels[15] = Color::Magenta;
            original.SetData(CubeMapFace::PositiveX, facePixels.data(), 16);
            survivor_.emplace(original); // actual CNA shared-resource value copy
            original.setNameProperty("source wrapper");
            survivor_->setNameProperty("surviving wrapper");
            Context().Check(original.getNameProperty() == "source wrapper" &&
                            survivor_->getNameProperty() == "surviving wrapper",
                            "shared resource wrappers retain independent names");
            facePixels.fill(Color::Orange);
            original.SetData(CubeMapFace::PositiveZ, facePixels.data(), 16);
            std::array<Color, 16> observed;
            survivor_->GetData(CubeMapFace::PositiveZ, observed.data(), 16);
            Context().Check(observed == facePixels,
                            "a complete Color face upload remains shared through the cube value copy");
            colors_[4] = Color::Orange;
            original.Dispose();
            survivor_->GetData(CubeMapFace::PositiveZ, observed.data(), 16);
            Context().Check(original.getIsDisposedProperty() && !survivor_->getIsDisposedProperty() &&
                            observed == facePixels,
                            "disposing the source wrapper leaves the copied cube alive with the shared edit");
            replacement_.emplace(device, 4, false, SurfaceFormat::Color);
            facePixels.fill(Color::White);
            for (int face = 0; face < 6; ++face)
                replacement_->SetData(static_cast<CubeMapFace>(face), facePixels.data(), 16);
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
            // Sample the four authored +X corners at exact texel centres. The small top swatches
            // retain the uploaded row order and leave the red centre of the face visible below.
            const std::array<Vector3, 4> cornerDirections{
                Vector3(1.0f, 0.75f, 0.75f), Vector3(1.0f, 0.75f, -0.75f),
                Vector3(1.0f, -0.75f, 0.75f), Vector3(1.0f, -0.75f, -0.75f)};
            for (int corner = 0; corner < 4; ++corner)
            {
                shader_->Apply();
                shader_->SetTexture(1, *survivor_);
                shader_->SetUniformInt("retainedCube", 1);
                const Vector3& direction = cornerDirections[corner];
                shader_->SetUniformVec3("direction", direction.X, direction.Y, direction.Z);
                batch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointClamp,
                              nullptr, nullptr, shader_.get());
                batch_->Draw(*white_, Rectangle(80 + corner * 40, 90, 40, 40), Color::White);
                batch_->End();
            }
            // Unit one is already occupied by survivor_. Rebinding a different cube must replace
            // that object: overwrite only the lower-right of the +X card with the white cube.
            shader_->Apply();
            shader_->SetTexture(1, *replacement_);
            shader_->SetUniformInt("retainedCube", 1);
            shader_->SetUniformVec3("direction", 1.0f, 0.0f, 0.0f);
            batch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointClamp,
                          nullptr, nullptr, shader_.get());
            batch_->Draw(*white_, Rectangle(190, 160, 50, 50), Color::White);
            batch_->End();
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
                const std::array<Color, 4> expectedCorners{
                    Color::White, Color::Yellow, Color::Cyan, Color::Magenta};
                bool orientedCorners = true;
                for (int corner = 0; corner < 4; ++corner)
                    orientedCorners = orientedCorners &&
                        CnaDemos::Near(screen.At(100 + corner * 40, 110), expectedCorners[corner], 1);
                Context().Check(orientedCorners,
                                "+X cube directions preserve the uploaded top-left through bottom-right corner order");
                Context().Check(screen.At(120, 185) == Color::Red && screen.At(215, 185) == Color::White,
                                "rebinding unit one replaces the red survivor cube with the white cube");
            }
            DemoGame::Draw(time);
        }
        std::array<Color, 6> colors_{Color::Red, Color::Lime, Color::Blue, Color::Gold, Color::Cyan, Color::Magenta};
        std::unique_ptr<ShaderEffect> shader_;
        std::optional<TextureCube> survivor_;
        std::optional<TextureCube> replacement_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
        std::optional<Texture2D> white_;
    };
}

CNA_DEMOS_GAME(CubeSamplerOwnershipDemo, {
    .id = "graphics.texture_cube.sampler_ownership",
    .title = "Copied cube lifetime and custom sampling",
    .description = "Edits a shared cube face, disposes the source wrapper, samples all six surviving faces plus the four oriented +X corners, and replaces the cube already bound to the same ShaderEffect unit.",
    .apis = {"TextureCube::TextureCube", "TextureCube::SetData", "TextureCube::GetData", "GraphicsResource::Dispose", "GraphicsResource::Name",
             "ShaderEffect::SetTexture", "ShaderEffect::SetUniformInt", "ShaderEffect::SetUniformVec3"},
    .tags = {"texture", "cube", "shader", "ownership", "cnaext"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::CustomEffects,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "GLSL source compilation and cube sampling required. Base-level faces establish ownership, face selection, +X orientation and binding replacement, not seams, mip filtering or physical GPU readback.",
})
