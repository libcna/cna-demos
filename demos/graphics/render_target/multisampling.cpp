// SPDX-License-Identifier: MS-PL
// A multisampled render target: smooth (anti-aliased) triangle edges with MultiSampleCount 4.
#include <array>
#include <memory>
#include <optional>
#include <set>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/CullMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Size = 100;

    class MultisamplingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            aliased_ = std::make_unique<RenderTarget2D>(device, Size, Size, false, SurfaceFormat::Color, DepthFormat::None);
            // The count is a request: the device may round it to what it supports.
            smooth_ = std::make_unique<RenderTarget2D>(device, Size, Size, false, SurfaceFormat::Color, DepthFormat::None,
                                                       4, RenderTargetUsage::DiscardContents);
            rasterOff_ = std::make_unique<RenderTarget2D>(device, Size, Size, false, SurfaceFormat::Color,
                                                          DepthFormat::None, 4, RenderTargetUsage::DiscardContents);
            masked_ = std::make_unique<RenderTarget2D>(device, Size, Size, false, SurfaceFormat::Color,
                                                       DepthFormat::None, 4, RenderTargetUsage::DiscardContents);
            Context().Print("requested 4 samples, got {}", smooth_->getMultiSampleCountProperty());
            Context().Check(smooth_->getMultiSampleCountProperty() > 1, "the target is multisampled");

            smoothRasterizer_.setCullModeProperty(CullMode::None);
            smoothRasterizer_.setMultiSampleAntiAliasProperty(true);
            rasterOffState_.setCullModeProperty(CullMode::None);
            rasterOffState_.setMultiSampleAntiAliasProperty(false);
            noCoverage_.setMultiSampleMaskProperty(0);

            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, Size, Size, 0.0f, 0.0f, 1.0f));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const std::array<VertexPositionColor, 3> sliver = {
                VertexPositionColor(Vector3(10.0f, 90.0f, 0.0f), Color::White),
                VertexPositionColor(Vector3(50.0f, 8.0f, 0.0f), Color::White),
                VertexPositionColor(Vector3(92.0f, 70.0f, 0.0f), Color::White)};
            Render(device, *aliased_, smoothRasterizer_, fullCoverage_, sliver);
            Render(device, *smooth_, smoothRasterizer_, fullCoverage_, sliver);
            Render(device, *rasterOff_, rasterOffState_, fullCoverage_, sliver);
            Render(device, *masked_, smoothRasterizer_, noCoverage_, sliver);
            // Unbinding resolves the samples into one colour per pixel.
            device.SetRenderTarget(nullptr);

            if (FramesDrawn() == 0)
            {
                const std::size_t aliasedGreys = Greys(*aliased_);
                const std::size_t smoothGreys = Greys(*smooth_);
                const std::size_t rasterOffGreys = Greys(*rasterOff_);
                const std::size_t maskedGreys = Greys(*masked_);
                Context().Print("distinct grey levels: no target MSAA {}, enabled {}, rasterizer off {}, mask 0 {}",
                                aliasedGreys, smoothGreys, rasterOffGreys, maskedGreys);
                Context().Check(aliasedGreys == 2, "without multisampling an edge pixel is all or nothing");
                Context().Check(smoothGreys > 2, "with multisampling edge pixels are partly covered");
                if (device.GetGraphicsRendererName() == "SOFTWARE")
                    Context().Check(rasterOffGreys == 2,
                                    "MultiSampleAntiAlias false disables partial sample coverage on SOFTWARE");
                else
                    Context().Check(rasterOffGreys > 2,
                                    "OpenGL ES cannot disable multisampling per rasterizer state");
                Context().Check(maskedGreys == 1, "MultiSampleMask 0 rejects every covered sample");
            }

            // Magnified 3x with point sampling, so the edge pixels are visible.
            device.Clear(Color::DarkSlateBlue);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*aliased_, Rectangle(60, 80, 3 * Size, 3 * Size), Color::White);
            spriteBatch_->Draw(*smooth_, Rectangle(440, 80, 3 * Size, 3 * Size), Color::White);
            spriteBatch_->DrawString(*font_, "MultiSampleCount 0", Vector2(60.0f, 400.0f), Color::White);
            spriteBatch_->DrawString(*font_, "MultiSampleCount 4", Vector2(440.0f, 400.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        void Render(GraphicsDevice& device, RenderTarget2D& target, const RasterizerState& rasterizer,
                    const BlendState& blend, const std::array<VertexPositionColor, 3>& vertices)
        {
            device.SetRenderTarget(&target);
            device.Clear(Color::Black);
            device.setRasterizerStateProperty(rasterizer);
            device.setBlendStateProperty(blend);
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices.data(), 0, 1);
            }
        }

        static std::size_t Greys(const RenderTarget2D& target)
        {
            std::array<Color, Size * Size> pixels{};
            target.GetData(pixels.data(), static_cast<int>(pixels.size()));
            std::set<int> levels;
            for (const Color& c : pixels)
                levels.insert(c.getRProperty());
            return levels.size();
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<RenderTarget2D> aliased_;
        std::unique_ptr<RenderTarget2D> smooth_;
        std::unique_ptr<RenderTarget2D> rasterOff_;
        std::unique_ptr<RenderTarget2D> masked_;
        std::unique_ptr<BasicEffect> effect_;
        RasterizerState smoothRasterizer_;
        RasterizerState rasterOffState_;
        BlendState fullCoverage_;
        BlendState noCoverage_;
    };
}

CNA_DEMOS_GAME(MultisamplingDemo, {
    .id = "graphics.render_target.multisampling",
    .title = "Multisampled render target",
    .description = "Draws the same triangle into plain and multisampled targets, then contrasts rasterizer-level "
                   "anti-alias control and a zero sample-coverage mask before resolving.",
    .apis = {"RenderTarget2D::RenderTarget2D", "RenderTarget2D::MultiSampleCount",
             "RasterizerState::MultiSampleAntiAlias", "BlendState::MultiSampleMask"},
    .tags = {"render-target", "msaa", "anti-aliasing", "3d"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::MultiSampling,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
