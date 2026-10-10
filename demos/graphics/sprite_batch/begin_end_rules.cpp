// SPDX-License-Identifier: MS-PL
// The rules of SpriteBatch's Begin/End pair, shown by the exceptions breaking them throws.
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/InvalidOperationException.hpp"

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
    };
}

CNA_DEMOS_GAME(BeginEndRulesDemo, {
    .id = "graphics.sprite_batch.begin_end_rules",
    .title = "SpriteBatch Begin/End rules",
    .description = "Breaks SpriteBatch's rules on purpose -- Draw before Begin, End without Begin, Begin twice, an "
                   "Immediate batch beside another -- and shows each InvalidOperationException.",
    .apis = {"SpriteBatch::Begin", "SpriteBatch::End", "SpriteBatch::Draw", "SpriteSortMode::Immediate"},
    .tags = {"sprite", "errors", "rules"},
    .frames = 1,
})
