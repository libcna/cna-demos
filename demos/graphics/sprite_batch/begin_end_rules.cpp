// SPDX-License-Identifier: MS-PL
// The rules of SpriteBatch's Begin/End pair, shown by the exceptions breaking them throws.
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/EventArgs.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class BeginEndRulesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            SpriteBatch batch(device);
            SpriteBatch other(device);
            Texture2D texture(device, 1, 1);

            Expect("Draw before Begin", [&] { batch.Draw(texture, Vector2::Zero, Color::White); });
            Expect("End without Begin", [&] { batch.End(); });

            batch.Begin();
            Expect("Begin twice without End", [&] { batch.Begin(); });
            batch.End();

            // Several deferred batches may be open at once on one device...
            batch.Begin();
            other.Begin();
            other.End();
            Context().Check(true, "two Deferred batches can be open at the same time");
            // ...but an Immediate batch draws straight to the device and must be alone.
            Expect("Immediate while another batch is open",
                   [&] { other.Begin(SpriteSortMode::Immediate, BlendState::AlphaBlend); });
            batch.End();

            // A SpriteBatch is a GraphicsResource too. Dispose abandons an open queue, raises the
            // event once and makes every batching entry point refuse further use.
            SpriteBatch disposable(device);
            int disposingCount = 0;
            bool disposingSenderMatches = false;
            bool eventPrecedesDisposedFlag = false;
            disposable.Disposing += [&](System::Object* sender, const System::EventArgs&) {
                ++disposingCount;
                disposingSenderMatches = sender == &disposable;
                eventPrecedesDisposedFlag = !disposable.getIsDisposedProperty();
            };
            disposable.Begin();
            disposable.Dispose();
            Context().Check(disposingCount == 1 && disposingSenderMatches && eventPrecedesDisposedFlag &&
                                disposable.getIsDisposedProperty(),
                            "Dispose raises Disposing before publishing the disposed state");
            ExpectDisposed("Begin after Dispose", [&] { disposable.Begin(); });
            ExpectDisposed("End after Dispose", [&] { disposable.End(); });
            ExpectDisposed("Draw after Dispose", [&] { disposable.Draw(texture, Vector2::Zero, Color::White); });
            disposable.Dispose();
            Context().Check(disposingCount == 1, "a second Dispose raises no event");

            // Disposing an open deferred batch must also retire the device-level active-batch
            // accounting; otherwise an unrelated Immediate batch is poisoned for this device.
            immediateRecovered_ = true;
            try
            {
                other.Begin(SpriteSortMode::Immediate, BlendState::AlphaBlend);
                other.End();
            }
            catch (const System::InvalidOperationException& e)
            {
                immediateRecovered_ = false;
                Context().Print("Immediate after disposing an open batch -> InvalidOperationException: {}", e.what());
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            SpriteBatch queued(device);
            queued.Begin();
            {
                Texture2D temporary(device, 1, 1);
                const Color orange = Color::Orange;
                temporary.SetData(&orange, 1);
                queued.Draw(temporary, Rectangle(100, 100, 120, 80), Color::White);
            }
            // Deferred SpriteBatch retains the native texture payload; the C++ wrapper may leave
            // scope before the flush boundary without creating a dangling reference.
            queued.End();

            const auto& presentation = device.getPresentationParametersProperty();
            const int width = presentation.getBackBufferWidthProperty();
            const int height = presentation.getBackBufferHeightProperty();
            std::vector<Color> pixels(static_cast<std::size_t>(width) * height);
            device.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
            Context().Check(pixels[static_cast<std::size_t>(140) * width + 160] == Color::Orange &&
                                pixels[static_cast<std::size_t>(40) * width + 40] == Color::Black,
                            "a queued texture survives its caller wrapper until deferred End");
            Context().Check(immediateRecovered_,
                            "disposing an open batch releases the device's active-batch accounting");

            Game::Draw(gameTime);
        }

    private:
        void Expect(std::string_view what, const std::function<void()>& action)
        {
            try
            {
                action();
                Context().Check(false, std::string(what) + " throws InvalidOperationException");
            }
            catch (const System::InvalidOperationException& e)
            {
                Context().Print("{:<38} -> InvalidOperationException: {}", what, e.what());
                Context().Check(true, std::string(what) + " throws InvalidOperationException");
            }
        }

        void ExpectDisposed(std::string_view what, const std::function<void()>& action)
        {
            try
            {
                action();
                Context().Check(false, std::string(what) + " throws ObjectDisposedException");
            }
            catch (const System::ObjectDisposedException& e)
            {
                Context().Print("{:<38} -> ObjectDisposedException: {}", what, e.what());
                Context().Check(true, std::string(what) + " throws ObjectDisposedException");
            }
        }

        bool immediateRecovered_ = false;
    };
}

CNA_DEMOS_GAME(BeginEndRulesDemo, {
    .id = "graphics.sprite_batch.begin_end_rules",
    .title = "SpriteBatch Begin/End rules",
    .description = "Exercises SpriteBatch's Begin/End errors, explicit disposal and a deferred texture whose caller "
                   "wrapper leaves scope before End.",
    .apis = {"SpriteBatch::Begin", "SpriteBatch::End", "SpriteBatch::Draw", "SpriteBatch::Dispose",
             "SpriteSortMode::Immediate", "GraphicsResource::Disposing", "GraphicsResource::IsDisposed",
             "GraphicsDevice::GetBackBufferData"},
    .tags = {"sprite", "errors", "rules", "dispose", "lifetime"},
    .frames = 1,
    .deterministic = true,
    .restrictions = "CNA-DEMOS-ISSUE-082: disposing an open batch leaves the device's active-batch counter set.",
    .knownIssue = "CNA-DEMOS-ISSUE-082",
})
