// SPDX-License-Identifier: MS-PL
// OcclusionQuery: how many pixels of an object passed the depth test (e.g. is the sun behind a wall?).
#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/OcclusionQuery.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    std::array<VertexPositionColor, 6> Square(Vector3 c, float half, Color color)
    {
        const VertexPositionColor tl(Vector3(c.X - half, c.Y + half, c.Z), color),
            tr(Vector3(c.X + half, c.Y + half, c.Z), color), bl(Vector3(c.X - half, c.Y - half, c.Z), color),
            br(Vector3(c.X + half, c.Y - half, c.Z), color);
        return {tl, tr, bl, tr, br, bl};
    }

    template<class Exception, class Action>
    void ExpectRefusal(CnaDemos::DemoContext& context, const char* label, Action action)
    {
        bool refused = false;
        try { action(); }
        catch (const Exception& error)
        {
            refused = true;
            context.Print("{}: {}", label, error.what());
        }
        context.Check(refused, label);
    }

    class OcclusionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            // Ordering rules are independent of how quickly this renderer completes a query.
            // This scratch query is ended before disposal; the visible lights below use their own queries.
            OcclusionQuery scratch(device);
            Context().Check(!scratch.getIsCompleteProperty(), "a fresh query has no completed result");
            ExpectRefusal<System::InvalidOperationException>(Context(), "PixelCount before Begin is refused",
                [&] { (void)scratch.getPixelCountProperty(); });
            ExpectRefusal<System::InvalidOperationException>(Context(), "End without Begin is refused",
                [&] { scratch.End(); });
            scratch.Begin();
            ExpectRefusal<System::InvalidOperationException>(Context(), "a repeated Begin is refused without ending the active query",
                [&] { scratch.Begin(); });
            scratch.End();
            // Deliberately do not observe IsComplete here. Completion speed cannot bypass this guard.
            ExpectRefusal<System::InvalidOperationException>(Context(), "reuse before observing IsComplete is refused",
                [&] { scratch.Begin(); });
            scratch.Dispose();
            Context().Check(scratch.getIsDisposedProperty() && !scratch.HasRenderer(),
                            "explicit disposal marks the query disposed and releases its renderer");
            ExpectRefusal<System::ObjectDisposedException>(Context(), "Begin on a disposed query is refused",
                [&] { scratch.Begin(); });
            ExpectRefusal<System::ObjectDisposedException>(Context(), "PixelCount on a disposed query is refused",
                [&] { (void)scratch.getPixelCountProperty(); });

            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            hidden_ = std::make_unique<OcclusionQuery>(device);
            open_ = std::make_unique<OcclusionQuery>(device);
            moving_ = std::make_unique<OcclusionQuery>(device);
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            time_ += static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(20, 24, 40));
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.setDepthStencilStateProperty(DepthStencilState::Default);

            // The wall goes first, so it is in the depth buffer when the lights are drawn.
            DrawSquare(Square(Vector3(-1.2f, 0.0f, 0.5f), 1.4f, Color::SlateGray));

            if (FramesDrawn() == 0)
            {
                // Two fixed lights for the checks: one completely behind the wall, one in the open.
                Measure(*hidden_, Square(Vector3(-1.2f, 0.0f, -1.0f), 0.4f, Color::Yellow));
                Measure(*open_, Square(Vector3(2.4f, 0.0f, -1.0f), 0.4f, Color::Yellow));
            }
            // A light passing behind the wall, measured every frame once the last answer is in.
            const float x = 3.2f * std::sin(time_ * 1.5f);
            if (moving_->getIsCompleteProperty() || !movingStarted_)
            {
                if (movingStarted_)
                {
                    visible_ = moving_->getPixelCountProperty();
                }
                movingStarted_ = true;
                Measure(*moving_, Square(Vector3(x, 1.5f, -1.0f), 0.4f, Color::Gold));
            }
            else
            {
                DrawSquare(Square(Vector3(x, 1.5f, -1.0f), 0.4f, Color::Gold));
            }

            // The answer arrives later than the draw (the GPU works behind the CPU): poll
            // IsComplete on later frames rather than waiting for it.
            if (!checked_ && hidden_->getIsCompleteProperty() && open_->getIsCompleteProperty())
            {
                checked_ = true;
                const int hidden = hidden_->getPixelCountProperty();
                const int open = open_->getPixelCountProperty();
                // CNA extension: where the backend can only answer "any" or "none" (OpenGL ES 3.0,
                // WebGL 2), PixelCount is 0 or 1 and isPixelCountPreciseEXT says so.
                precise_ = open_->isPixelCountPreciseEXT();
                Context().Print("behind the wall: {} pixels, in the open: {} pixels (precise: {})", hidden, open,
                                precise_);
                Context().Check(hidden == 0, "a light completely behind the wall has no visible pixels");
                if (precise_)
                {
                    Context().Check(open > 100, "a light in the open has visible pixels (a real tally)");
                }
                else
                {
                    Context().Check(open == 1, "a light in the open is visible (the count is a 0/1 flag here)");
                }
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_,
                                     std::string("Moving light, visible pixels: ") + std::to_string(visible_) +
                                         (precise_ ? "" : " (a 0/1 flag here)"),
                                     Vector2(20.0f, 16.0f), Color::White);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

        void EndRun() override
        {
            if (Context().Automated())
            {
                Context().Check(checked_, "both queries completed during the run");
            }
            DemoGame::EndRun();
        }

    private:
        void Measure(OcclusionQuery& query, const std::array<VertexPositionColor, 6>& square)
        {
            query.Begin();
            DrawSquare(square);
            query.End();
        }

        void DrawSquare(const std::array<VertexPositionColor, 6>& square)
        {
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                getGraphicsDeviceProperty().DrawUserPrimitives(PrimitiveType::TriangleList, square.data(), 0, 2);
            }
        }

        float time_ = 0.0f;
        int visible_ = 0;
        bool movingStarted_ = false;
        bool checked_ = false;
        bool precise_ = true;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<OcclusionQuery> hidden_;
        std::unique_ptr<OcclusionQuery> open_;
        std::unique_ptr<OcclusionQuery> moving_;
    };
}

CNA_DEMOS_GAME(OcclusionDemo, {
    .id = "graphics.occlusion_query.visible_pixels",
    .title = "Occlusion queries",
    .description = "Counts the visible pixels of lights drawn behind a wall with OcclusionQuery::Begin/End: zero "
                   "for one hidden completely, many for one in the open, and a live count for one passing behind "
                   "the wall, read when IsComplete says the answer has arrived; isPixelCountPreciseEXT tells a real "
                   "tally from the 0/1 flag OpenGL ES 3.0 can only give. A scratch query demonstrates ordering and disposal refusals.",
    .apis = {"OcclusionQuery::OcclusionQuery", "OcclusionQuery::Begin", "OcclusionQuery::End",
             "OcclusionQuery::IsComplete", "OcclusionQuery::PixelCount", "OcclusionQuery::isPixelCountPreciseEXT",
             "OcclusionQuery::Dispose", "GraphicsResource::Dispose", "GraphicsResource::IsDisposed",
             "Microsoft::Xna::Framework::Graphics::OcclusionQuery::HasRenderer"},
    .tags = {"3d", "occlusion", "query", "visibility", "lens-flare"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::OcclusionQuery,
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
})
