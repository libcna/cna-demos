// SPDX-License-Identifier: MS-PL
// A copied cube resource survives source disposal and is sampled by a custom GLSL program.
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

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
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"
#include "System/NotSupportedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    std::array<std::uint8_t, 16> Dxt3HalfRed()
    {
        // Explicit alpha nibbles 8/15, followed by a solid RGB565 red DXT colour block.
        return {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
                0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    }

    std::array<std::uint8_t, 16> Dxt5QuarterBlue()
    {
        // Alpha endpoint zero is 64 and every selector chooses it; the colour block is solid blue.
        return {0x40, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    }

    constexpr const char* Vertex = R"(
layout(location = 0) in vec2 aPos;
uniform mat4 projection;
void main() { gl_Position = projection * vec4(aPos, 0.0, 1.0); }
)";
    constexpr const char* Fragment = R"(
uniform samplerCube retainedCube;
uniform vec3 direction;
uniform float lod;
out vec4 FragColor;
void main() { FragColor = textureLod(retainedCube, direction, lod); }
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
            TextureCube original(device, 4, true, SurfaceFormat::Color);
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
            const std::array<Color, 4> mipOne{Color::Lime, Color::Lime, Color::Lime, Color::Lime};
            const Color mipTwo = Color::Blue;
            for (int face = 0; face < 6; ++face)
            {
                original.SetData(static_cast<CubeMapFace>(face), 1, nullptr,
                                 mipOne.data(), 0, 4);
                original.SetData(static_cast<CubeMapFace>(face), 2, nullptr, &mipTwo, 0, 1);
            }
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
            replacement_.emplace(device, 4, true, SurfaceFormat::Color);
            facePixels.fill(Color::White);
            const std::array<Color, 4> whiteMip{Color::White, Color::White,
                                                Color::White, Color::White};
            for (int face = 0; face < 6; ++face)
            {
                replacement_->SetData(static_cast<CubeMapFace>(face), facePixels.data(), 16);
                replacement_->SetData(static_cast<CubeMapFace>(face), 1, nullptr,
                                      whiteMip.data(), 0, 4);
                replacement_->SetData(static_cast<CubeMapFace>(face), 2, nullptr,
                                      whiteMip.data(), 0, 1);
            }
            try
            {
                dxt3_.emplace(device, 4, false, SurfaceFormat::Dxt3);
                dxt5_.emplace(device, 4, false, SurfaceFormat::Dxt5);
                const auto dxt3Block = Dxt3HalfRed();
                const auto dxt5Block = Dxt5QuarterBlue();
                for (int face = 0; face < 6; ++face)
                {
                    dxt3_->SetData(static_cast<CubeMapFace>(face), dxt3Block.data(), 16);
                    dxt5_->SetData(static_cast<CubeMapFace>(face), dxt5Block.data(), 16);
                }
            }
            catch (const System::NotSupportedException&)
            {
                dxt3_.reset();
                dxt5_.reset();
                Context().Print("DXT3/DXT5 cube sampling is unavailable on this renderer.");
            }
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
                shader_->SetUniformFloat("lod", 0.0f);
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
                shader_->SetUniformFloat("lod", 0.0f);
                batch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointClamp,
                              nullptr, nullptr, shader_.get());
                batch_->Draw(*white_, Rectangle(80 + corner * 40, 90, 40, 40), Color::White);
                batch_->End();
            }
            // Both draws request LOD1.25 from authored Lime mip1 and Blue mip2. Point mip
            // selection keeps mip1; linear mip selection blends them with 3:1 weights.
            SamplerState mipPoint = SamplerState::LinearClamp;
            mipPoint.setFilterProperty(TextureFilter::LinearMipPoint);
            SamplerState mipLinear = SamplerState::PointClamp;
            mipLinear.setFilterProperty(TextureFilter::PointMipLinear);
            const std::array<std::pair<SamplerState*, Rectangle>, 2> mipSamples{{
                {&mipPoint, Rectangle(320, 160, 50, 50)},
                {&mipLinear, Rectangle(380, 160, 50, 50)}}};
            for (const auto& [sampler, destination] : mipSamples)
            {
                device.getSamplerStatesProperty()(1, *sampler);
                shader_->Apply();
                shader_->SetTexture(1, *survivor_);
                shader_->SetUniformInt("retainedCube", 1);
                shader_->SetUniformVec3("direction", 1.0f, 0.0f, 0.0f);
                shader_->SetUniformFloat("lod", 1.25f);
                batch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointClamp,
                              nullptr, nullptr, shader_.get());
                batch_->Draw(*white_, destination, Color::White);
                batch_->End();
            }
            // Unit one is already occupied by survivor_. Rebinding a different cube must replace
            // that object: overwrite only the lower-right of the +X card with the white cube.
            shader_->Apply();
            shader_->SetTexture(1, *replacement_);
            shader_->SetUniformInt("retainedCube", 1);
            shader_->SetUniformVec3("direction", 1.0f, 0.0f, 0.0f);
            shader_->SetUniformFloat("lod", 0.0f);
            batch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointClamp,
                          nullptr, nullptr, shader_.get());
            batch_->Draw(*white_, Rectangle(190, 160, 50, 50), Color::White);
            batch_->End();
            if (dxt3_ && dxt5_)
            {
                device.getSamplerStatesProperty()(1, SamplerState::PointClamp);
                const std::array<std::pair<TextureCube*, Rectangle>, 2> compressed{{
                    {&*dxt3_, Rectangle(600, 430, 60, 30)},
                    {&*dxt5_, Rectangle(700, 430, 60, 30)}}};
                for (const auto& [cube, destination] : compressed)
                {
                    shader_->Apply();
                    shader_->SetTexture(1, *cube);
                    shader_->SetUniformInt("retainedCube", 1);
                    shader_->SetUniformVec3("direction", 1.0f, 0.0f, 0.0f);
                    shader_->SetUniformFloat("lod", 0.0f);
                    batch_->Begin(SpriteSortMode::Deferred, &BlendState::NonPremultiplied,
                                  &SamplerState::PointClamp, nullptr, nullptr, shader_.get());
                    batch_->Draw(*white_, destination, Color::White);
                    batch_->End();
                }
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
                const std::array<Color, 4> expectedCorners{
                    Color::White, Color::Yellow, Color::Cyan, Color::Magenta};
                bool orientedCorners = true;
                for (int corner = 0; corner < 4; ++corner)
                    orientedCorners = orientedCorners &&
                        CnaDemos::Near(screen.At(100 + corner * 40, 110), expectedCorners[corner], 1);
                Context().Check(orientedCorners,
                                "+X cube directions preserve the uploaded top-left through bottom-right corner order");
                Context().Check(CnaDemos::Near(screen.At(345, 185), Color::Lime, 1) &&
                                    CnaDemos::Near(screen.At(405, 185), Color(0, 191, 64), 2),
                                "point versus linear mip filtering selects mip1 or blends mip1/mip2 at LOD1.25");
                Context().Check(screen.At(120, 185) == Color::Red && screen.At(215, 185) == Color::White,
                                "rebinding unit one replaces the red survivor cube with the white cube");
                if (dxt3_ && dxt5_)
                {
                    const Color dxt3 = screen.At(630, 445);
                    const Color dxt5 = screen.At(730, 445);
                    Context().Print("compressed alpha centers: DXT3 {}; DXT5 {}", dxt3.ToString(), dxt5.ToString());
                    Context().Check(CnaDemos::Near(dxt3, Color(136, 0, 0, 192), 1) &&
                                        CnaDemos::Near(dxt5, Color(0, 0, 64, 207), 1),
                                    "samplerCube decodes DXT3 explicit alpha and DXT5 interpolated alpha before blending");
                }
            }
            DemoGame::Draw(time);
        }
        std::array<Color, 6> colors_{Color::Red, Color::Lime, Color::Blue, Color::Gold, Color::Cyan, Color::Magenta};
        std::unique_ptr<ShaderEffect> shader_;
        std::optional<TextureCube> survivor_;
        std::optional<TextureCube> replacement_;
        std::optional<TextureCube> dxt3_;
        std::optional<TextureCube> dxt5_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
        std::optional<Texture2D> white_;
    };
}

CNA_DEMOS_GAME(CubeSamplerOwnershipDemo, {
    .id = "graphics.texture_cube.sampler_ownership",
    .title = "Copied cube lifetime and custom sampling",
    .description = "Samples a copied cube's faces and oriented +X corners, compares point and linear mip selection, "
                   "samples DXT3/DXT5 alpha, then replaces the cube already bound to the same ShaderEffect unit.",
    .apis = {"TextureCube::TextureCube", "TextureCube::SetData", "TextureCube::GetData", "GraphicsResource::Dispose", "GraphicsResource::Name",
             "ShaderEffect::SetTexture", "ShaderEffect::SetUniformInt", "ShaderEffect::SetUniformVec3",
             "ShaderEffect::SetUniformFloat", "SamplerState::Filter", "TextureFilter::LinearMipPoint",
             "TextureFilter::PointMipLinear"},
    .tags = {"texture", "cube", "shader", "ownership", "cnaext"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::CustomEffects,
    .frames = 1,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "GLSL source compilation and cube sampling required. DXT3/DXT5 sampling runs when both formats "
                    "are supported. Authored levels establish face selection, +X orientation, binding replacement, "
                    "explicit-LOD mip filtering and compressed alpha, not cube seams or physical GPU readback.",
})
