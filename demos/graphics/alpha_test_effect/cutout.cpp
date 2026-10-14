// SPDX-License-Identifier: MS-PL
// AlphaTestEffect: keep or discard whole pixels by alpha -- hard cut-outs that write depth correctly.
#include <array>
#include <memory>
#include <optional>
#include <stdexcept>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/AlphaTestEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/CompareFunction.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColorTexture.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionTexture.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // A textured quad in pixel space at depth z (orthographic camera, -Z into the screen).
    std::array<VertexPositionTexture, 6> Quad(float x, float y, float size, float z)
    {
        const VertexPositionTexture a(Vector3(x, y, z), Vector2(0, 0)), b(Vector3(x + size, y, z), Vector2(1, 0)),
            c(Vector3(x, y + size, z), Vector2(0, 1)), d(Vector3(x + size, y + size, z), Vector2(1, 1));
        return {a, b, c, b, d, c};
    }

    std::array<VertexPositionColorTexture, 6> ColoredQuad(float x, float y, float size, float z, Color color)
    {
        const VertexPositionColorTexture a(Vector3(x, y, z), color, Vector2(0, 0));
        const VertexPositionColorTexture b(Vector3(x + size, y, z), color, Vector2(1, 0));
        const VertexPositionColorTexture c(Vector3(x, y + size, z), color, Vector2(0, 1));
        const VertexPositionColorTexture d(Vector3(x + size, y + size, z), color, Vector2(1, 1));
        return {a, b, c, b, d, c};
    }

    class CutoutDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            circle_ = getContentProperty().Load<Texture2D>("textures/soft_circle");
            const Matrix projection = Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 10.0f);

            // Plain textured drawing: every pixel of the quad is drawn and writes depth.
            plain_ = std::make_unique<BasicEffect>(device);
            plain_->setTextureEnabledProperty(true);
            plain_->setTextureProperty(&circle_);
            plain_->setProjectionProperty(projection);

            // Alpha test: pixels whose alpha fails the comparison are discarded -- no colour, no depth.
            alphaTest_ = std::make_unique<AlphaTestEffect>(device);
            alphaTest_->setTextureProperty(&circle_);
            alphaTest_->setProjectionProperty(projection);
            alphaTest_->setAlphaFunctionProperty(CompareFunction::Greater);
            alphaTest_->setReferenceAlphaProperty(128);
            alphaTest_->setAlphaProperty(0.75f);
            alphaTest_->setVertexColorEnabledProperty(true);

            std::unique_ptr<Effect> cloneOwner(alphaTest_->Clone());
            auto* clone = dynamic_cast<AlphaTestEffect*>(cloneOwner.get());
            if (clone == nullptr)
                throw std::runtime_error("AlphaTestEffect::Clone returned the wrong runtime type");
            Context().Check(clone->getTextureProperty() == &circle_ &&
                                clone->getProjectionProperty() == projection &&
                                clone->getAlphaFunctionProperty() == CompareFunction::Greater &&
                                clone->getReferenceAlphaProperty() == 128 && clone->getAlphaProperty() == 0.75f &&
                                clone->getVertexColorEnabledProperty(),
                            "Clone retains texture, transform, threshold, alpha and vertex-color state");
            alphaClone_.reset(static_cast<AlphaTestEffect*>(cloneOwner.release()));
            alphaTest_->setAlphaProperty(0.25f);
            alphaTest_->setVertexColorEnabledProperty(false);
            Context().Check(alphaClone_->getAlphaProperty() == 0.75f &&
                                alphaClone_->getVertexColorEnabledProperty() &&
                                alphaTest_->getAlphaProperty() == 0.25f &&
                                !alphaTest_->getVertexColorEnabledProperty(),
                            "clone alpha and vertex-color state are independent from the source");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.getSamplerStatesProperty()(0, SamplerState::LinearClamp);

            // Each side: a near circle drawn first, then a far circle behind it, overlapping.
            DrawPair(*plain_, 40.0f);
            DrawColoredPair(*alphaClone_, 440.0f);

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                // Inside the far (orange) circle and in the transparent corner of the near quad.
                const Color plainCorner = p.At(40 + 222, 252);
                const Color testedCorner = p.At(440 + 222, 252);
                const Color testedCentre = p.At(440 + 140, 170);
                Context().Print("near corner over far circle: plain {}, alpha-tested {}; tinted centre {}",
                                plainCorner.ToString(), testedCorner.ToString(), testedCentre.ToString());
                Context().Check(plainCorner.getBProperty() == 255,
                                "plain: the near quad's transparent corner is drawn (white) and hides the far circle");
                Context().Check(testedCorner.getRProperty() > 80 && testedCorner.getGProperty() > 100 &&
                                    testedCorner.getBProperty() < 5,
                                "alpha test discards the near corner so the tinted orange far circle shows");
                Context().Check(testedCentre.getRProperty() >= 95 && testedCentre.getRProperty() <= 97 &&
                                    testedCentre.getGProperty() >= 190 && testedCentre.getGProperty() <= 192 &&
                                    testedCentre.getBProperty() >= 190 && testedCentre.getBProperty() <= 192,
                                "Alpha 0.75 premultiplies a half-red vertex tint at the opaque texture centre");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "BasicEffect (opaque)", Vector2(60.0f, 420.0f), Color::White);
            spriteBatch_->DrawString(*font_, "AlphaTestEffect, alpha > 128", Vector2(440.0f, 420.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

        void DrawColoredPair(AlphaTestEffect& effect, float x)
        {
            const Color cyanTint(128, 255, 255, 255);
            DrawQuad(effect, ColoredQuad(x + 50.0f, 80.0f, 180.0f, -2.0f, cyanTint), Color::White);
            DrawQuad(effect, ColoredQuad(x + 150.0f, 180.0f, 180.0f, -6.0f, cyanTint), Color::Orange);
        }

    private:
        template <typename TEffect>
        void DrawPair(TEffect& effect, float x)
        {
            DrawQuad(effect, Quad(x + 50.0f, 80.0f, 180.0f, -2.0f), Color::White);    // near, drawn first
            DrawQuad(effect, Quad(x + 150.0f, 180.0f, 180.0f, -6.0f), Color::Orange);  // far, behind it
        }

        template <typename TEffect, typename TVertex>
        void DrawQuad(TEffect& effect, const std::array<TVertex, 6>& quad, Color tint)
        {
            effect.setDiffuseColorProperty(tint.ToVector3());
            for (EffectPass& pass : effect.getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                getGraphicsDeviceProperty().DrawUserPrimitives(PrimitiveType::TriangleList, quad.data(), 0, 2);
            }
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D circle_;
        std::unique_ptr<BasicEffect> plain_;
        std::unique_ptr<AlphaTestEffect> alphaTest_;
        std::unique_ptr<AlphaTestEffect> alphaClone_;
    };
}

CNA_DEMOS_GAME(CutoutDemo, {
    .id = "graphics.alpha_test_effect.cutout",
    .title = "Alpha-tested cut-outs",
    .description = "Draws a white circle and, behind it, an orange one: with BasicEffect the near quad's transparent "
                   "corner is still drawn and hides the far circle, with AlphaTestEffect (alpha > 128) it is discarded.",
    .apis = {"AlphaTestEffect::AlphaTestEffect", "AlphaTestEffect::AlphaFunction", "AlphaTestEffect::ReferenceAlpha",
             "AlphaTestEffect::Texture", "AlphaTestEffect::DiffuseColor", "CompareFunction::Greater",
             "AlphaTestEffect::Projection", "AlphaTestEffect::Alpha",
             "AlphaTestEffect::VertexColorEnabled", "AlphaTestEffect::Clone"},
    .tags = {"3d", "alpha", "cutout", "foliage", "effect"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/soft_circle.png", "fonts/pixel_font.cnb"},
})
