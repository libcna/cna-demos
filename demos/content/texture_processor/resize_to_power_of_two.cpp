// SPDX-License-Identifier: MS-PL
// Content pipeline resizing: resizeToPowerOfTwo scales a 100x60 image to 128x64 at build time.
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbTextureCodec.hpp"
#include "CNA/Content/Pipeline/Texture2DContentPipeline.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace Pipeline = CNA::Content::Pipeline;
namespace Cnb = CNA::Content::Cnb;

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ResizeDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            // Below, actual XNB-targeted processing shows why Reach mipmaps may need resizing.
            // The source is 100x60; each side grows to the next power of two, stretching the image.
            texture_ = getContentProperty().Load<Texture2D>("textures/odd_size");
            Context().Print("odd_size: {}x{}", texture_.getWidthProperty(), texture_.getHeightProperty());
            Context().Check(texture_.getWidthProperty() == 128 && texture_.getHeightProperty() == 64,
                            "100x60 became 128x64");
            CheckTargetProfiles();
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // Drawn four times its size.
            spriteBatch_->Draw(texture_, Vector2(40.0f, 120.0f), std::nullopt, Color::White, 0.0f, Vector2::Zero, 4.0f,
                               SpriteEffects::None, 0.0f);
            spriteBatch_->DrawString(*font_,
                                     "odd_size.png (100x60), compiled with resizeToPowerOfTwo: " +
                                         std::to_string(texture_.getWidthProperty()) + "x" +
                                         std::to_string(texture_.getHeightProperty()),
                                     Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, "(the white frame shows the stretch)", Vector2(40.0f, 60.0f),
                                     Color::LightGray);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void CheckTargetProfiles()
        {
            auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
            Pipeline::RegisterTexture2DContentPipeline(*registry);
            const Pipeline::ContentPipeline pipeline(registry);
            Pipeline::ContentBuildRequest request;
            request.sourceRoot = std::filesystem::canonical(std::filesystem::path(TitleLocation::Path()) / "assets");
            request.source = "textures/odd_size.png";
            request.logicalName = "profile_probe";
            request.outputFormat = Pipeline::ContentOutputFormat::Xnb;
            request.environment.targetProfile = GraphicsProfile::Reach;
            request.parameters.Set(Pipeline::TextureGenerateMipmapsParameter, true);
            request.parameters.Set(Pipeline::TextureFormatParameter, std::string("Color"));
            Pipeline::ContentDependencyCollector dependencies;
            bool refused = false;
            try { (void)pipeline.ImportAndProcess(request, dependencies); }
            catch (const Pipeline::ContentPipelineError& error)
            {
                Context().Print("Expected Reach refusal: {}", error.what());
                refused = error.Stage() == Pipeline::ContentPipelineStage::Process &&
                    error.Component() == "CNA.TextureProcessor" &&
                    error.Source() == request.sourceRoot / request.source &&
                    error.LogicalName() == request.logicalName &&
                    std::string(error.what()).find("requires mipmapped Texture2D sizes to be powers of two") != std::string::npos;
            }
            Context().Check(refused, "XNB Reach processing refuses mipmaps for the original 100x60 source");

            // Inspect processed values directly: these XNB-targeted calls do not run a writer.
            const auto layout = [](const Cnb::CnbTextureData& texture, unsigned width, unsigned height,
                                   const std::vector<std::size_t>& expectedBytes)
            {
                if (texture.width != width || texture.height != height || texture.depth != 1 ||
                    texture.faceCount != 1 || texture.mipCount != expectedBytes.size() ||
                    texture.representations.size() != 1 ||
                    texture.representations[0].format != Cnb::CnbTextureFormat::Rgba8) return false;
                std::vector<std::size_t> actualBytes;
                for (const auto& level : texture.representations[0].levels) actualBytes.push_back(level.size());
                return actualBytes == expectedBytes;
            };
            request.environment.targetProfile = GraphicsProfile::HiDef;
            const auto hiDef = pipeline.ImportAndProcess(request, dependencies);
            const auto& original = hiDef.processed.Get<Cnb::CnbTextureData>();
            Context().Check(layout(original, 100, 60, {24000, 6000, 1500, 336, 72, 12, 4}),
                            "HiDef-targeted processing keeps 100x60 and produces all seven mip payloads");

            request.environment.targetProfile = GraphicsProfile::Reach;
            request.parameters.Set(Pipeline::TextureResizeToPowerOfTwoParameter, true);
            const auto resized = pipeline.ImportAndProcess(request, dependencies);
            const auto& powerOfTwo = resized.processed.Get<Cnb::CnbTextureData>();
            Context().Check(layout(powerOfTwo, 128, 64, {32768, 8192, 2048, 512, 128, 32, 8, 4}),
                            "explicit resize allows Reach-targeted processing with a complete 128x64 mip chain");
            std::vector<Color> pixels(128 * 64);
            texture_.GetData(pixels.data(), static_cast<int>(pixels.size()));
            bool samePixels = powerOfTwo.representations.size() == 1 &&
                !powerOfTwo.representations[0].levels.empty() &&
                powerOfTwo.representations[0].levels[0].size() == pixels.size() * 4;
            if (samePixels)
            {
                const auto& bytes = powerOfTwo.representations[0].levels[0];
                for (std::size_t i = 0; i < pixels.size(); ++i)
                    samePixels &= pixels[i] == Color(bytes[i * 4], bytes[i * 4 + 1], bytes[i * 4 + 2], bytes[i * 4 + 3]);
            }
            Context().Check(samePixels, "fresh resized level-zero pixels match the precompiled CNB texture drawn by this scene");

            request.parameters.Set(Pipeline::TextureResizeToPowerOfTwoParameter, false);
            request.parameters.Set(Pipeline::TextureGenerateMipmapsParameter, false);
            const auto noMips = pipeline.ImportAndProcess(request, dependencies);
            const auto& unscaled = noMips.processed.Get<Cnb::CnbTextureData>();
            Context().Check(layout(unscaled, 100, 60, {24000}),
                            "Reach-targeted processing accepts the non-power-of-two size when mipmaps are disabled");

            request.parameters.Set(Pipeline::TextureGenerateMipmapsParameter, true);
            request.outputFormat = Pipeline::ContentOutputFormat::Cnb;
            const auto cnb = pipeline.Build(request);
            const auto document = Cnb::CnbDocument::Parse(cnb.output.bytes, "profile probe CNB");
            const auto native = Cnb::DecodeTexture2DFromCnb(document);
            Context().Check(layout(native, 100, 60, {24000, 6000, 1500, 336, 72, 12, 4}) &&
                            original.representations.size() == 1 &&
                            native.representations[0].levels == original.representations[0].levels,
                            "CNB output has no XNB Reach size gate and preserves the original seven mip payloads");
            Context().Print("XNB processing: Reach needs resize or no mipmaps; HiDef keeps 100x60. CNB: {}x{}, {} levels.",
                            native.width, native.height, native.mipCount);
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D texture_;
    };
}

CNA_DEMOS_GAME(ResizeDemo, {
    .id = "content.texture_processor.resize_to_power_of_two",
    .title = "Resize to a power of two",
    .description = "Loads a 100x60 image compiled with the TextureProcessor parameter resizeToPowerOfTwo: the "
                   "texture is 128x64 and drawn enlarged; contrasts Reach/HiDef mipmap processing and CNB output.",
    .apis = {"ContentManager::Load", "Texture2D::Width", "Texture2D::Height", "Texture2D::GetData",
             "CNA::Content::Pipeline::ContentPipeline::ImportAndProcess",
             "CNA::Content::Pipeline::ContentBuildEnvironment::targetProfile",
             "CNA::Content::Pipeline::TextureResizeToPowerOfTwoParameter"},
    .tags = {"content", "pipeline", "power-of-two", "resize", "texture"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "textures/odd_size.cnb", "textures/odd_size.png"},
    .restrictions = "Profile checks inspect canonical processor outputs for XNB and complete CNB bytes for the native control. Rendering uses the existing precompiled CNB with the project's HiDef device.",
})
