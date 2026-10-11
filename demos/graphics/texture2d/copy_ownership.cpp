// SPDX-License-Identifier: MS-PL
// CNA value copies initially share texture storage; the upload overload determines what changes.
#include <array>
#include <memory>
#include <optional>
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Side = 8;
    using Pixels = std::array<Color, Side * Side>;

    class CopyOwnershipDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            Pixels checker;
            for (int y = 0; y < Side; ++y)
                for (int x = 0; x < Side; ++x)
                    checker[y * Side + x] = (x + y) % 2 == 0 ? Color::White : Color::CornflowerBlue;
            Texture2D original(device, Side, Side);
            original.SetData(checker.data(), static_cast<int>(checker.size()));
            original.setNameProperty("source");
            original.setTagProperty(this);
            edited_ = std::make_unique<Texture2D>(original); // actual CNA copy constructor
            Context().Check(edited_->getNameProperty() == "source" && edited_->getTagProperty() == this,
                            "a texture copy begins with the source wrapper's Name and Tag values");
            edited_->setNameProperty("edited copy");
            edited_->setTagProperty(nullptr);
            Context().Check(original.getNameProperty() == "source" && original.getTagProperty() == this,
                            "changing the copy's Name and Tag leaves the source wrapper unchanged");

            const Rectangle corner(0, 0, 1, 1);
            const Color gold = Color::Gold;
            edited_->SetData(0, &corner, &gold, 0, 1); // partial update: shared storage
            checker[0] = gold;
            Pixels sourcePixels{}, copyPixels{};
            original.GetData(sourcePixels.data(), static_cast<int>(sourcePixels.size()));
            edited_->GetData(copyPixels.data(), static_cast<int>(copyPixels.size()));
            Context().Check(sourcePixels == checker && copyPixels == checker,
                            "a partial rectangle upload through the copy changes both handles' pixels");

            Pixels replacement;
            for (int y = 0; y < Side; ++y)
                for (int x = 0; x < Side; ++x)
                    replacement[y * Side + x] = x % 2 == 0 ? Color::OrangeRed : Color::Gold;
            edited_->SetData(replacement.data(), static_cast<int>(replacement.size()));
            original.GetData(sourcePixels.data(), static_cast<int>(sourcePixels.size()));
            edited_->GetData(copyPixels.data(), static_cast<int>(copyPixels.size()));
            Context().Check(sourcePixels == checker && copyPixels == replacement,
                            "SetData(Color*, int) replaces only the writing copy's texture");

            survivor_ = std::make_unique<Texture2D>(original); // still shares the checker texture
            original.Dispose();
            survivor_->GetData(sourcePixels.data(), static_cast<int>(sourcePixels.size()));
            Context().Check(original.getIsDisposedProperty() && !survivor_->getIsDisposedProperty() && sourcePixels == checker,
                            "disposing one shared wrapper leaves its sibling alive with the same pixels");
            Context().Print("Left: shared checker survives source disposal. Right: full upload detached the edited copy.");
        }

        void Draw(const GameTime& gameTime) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color(24, 28, 36));
            batch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            batch_->DrawString(*font_, "Texture copies: shared edits and independent lifetimes", Vector2(48, 24), Color::White);
            batch_->DrawString(*font_, "Shared survivor", Vector2(48, 88), Color::White);
            batch_->DrawString(*font_, "Full upload copy", Vector2(432, 88), Color::White);
            batch_->Draw(*survivor_, Rectangle(48, 128, 288, 288), Color::White);
            batch_->Draw(*edited_, Rectangle(432, 128, 288, 288), Color::White);
            batch_->DrawString(*font_, "Original disposed; both remaining handles still draw.", Vector2(48, 438), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels screen(device);
                Context().Check(CnaDemos::Near(screen.At(66, 146), Color::Gold, 1) &&
                                CnaDemos::Near(screen.At(102, 146), Color::CornflowerBlue, 1) &&
                                CnaDemos::Near(screen.At(450, 146), Color::OrangeRed, 1) &&
                                CnaDemos::Near(screen.At(486, 146), Color::Gold, 1),
                                "both live handles render their distinct authored pixels after the source is disposed");
            }
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> edited_, survivor_;
    };
}

CNA_DEMOS_GAME(CopyOwnershipDemo, {
    .id = "graphics.texture2d.copy_ownership",
    .title = "Texture copies: shared edits and independent lifetimes",
    .description = "Copies a texture, edits its shared rectangle, detaches one copy with a full Color upload, "
                   "and renders both surviving textures after disposing the original wrapper; also checks Name/Tag isolation.",
    .apis = {"Texture2D::Texture2D", "Texture2D::SetData", "Texture2D::GetData", "GraphicsResource::Name",
             "GraphicsResource::Tag", "GraphicsResource::Dispose", "GraphicsResource::IsDisposed"},
    .tags = {"texture", "copy", "ownership", "lifetime", "cnaext"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "CNA C++ value-copy behavior for Color uploads; not every mip/raw-data overload or RenderTarget2D alias.",
})
