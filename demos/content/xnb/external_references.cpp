// SPDX-License-Identifier: MS-PL
// A card XNB references a loose texture and a compiled label relative to its own directory.
#include <any>
#include <array>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReaderManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Content;

namespace
{
    struct Label { std::string title; int rank = 0; };
    using LabelPtr = std::shared_ptr<Label>;
    struct Card { std::optional<Texture2D> image, absentImage; std::any label, absentLabel; };
    int labelReads = 0;
    class LabelReader final : public ContentTypeReader<LabelPtr>
    {
    public:
        LabelReader() : ContentTypeReader<LabelPtr>("CnaDemos.ExternalCardLabel") {}
    protected:
        LabelPtr Read(ContentReader& input, std::optional<LabelPtr>) override
        {
            ++labelReads;
            auto label = std::make_shared<Label>();
            label->title = input.ReadString(); label->rank = input.ReadInt32();
            return label;
        }
    };
    class CardReader final : public ContentTypeReader<Card>
    {
    public:
        CardReader() : ContentTypeReader<Card>("CnaDemos.ExternalCard") {}
    protected:
        Card Read(ContentReader& input, std::optional<Card>) override
        {
            Card card;
            card.image = input.ReadExternalReference<Texture2D>(); // normal Load<Texture2D> lookup includes PNG
            card.absentImage = input.ReadExternalReference<Texture2D>();
            card.label = input.ReadExternalReference(); // type-erased route requires a compiled XNB child
            card.absentLabel = input.ReadExternalReference();
            return card;
        }
    };

    class ExternalReferencesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            ContentTypeReaderManager::AddTypeCreator("CnaDemos.ExternalCardReader", [] { return std::make_unique<CardReader>(); });
            ContentTypeReaderManager::AddTypeCreator("CnaDemos.ExternalCardLabelReader", [] { return std::make_unique<LabelReader>(); });
            auto& content = getContentProperty();
            auto& ctx = Context();
            batch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(content.Load<SpriteFont>("fonts/pixel_font"));
            labelReads = 0;
            first_ = content.Load<Card>("external/entries/card_a");
            second_ = content.Load<Card>("external/entries/card_b");
            ready_ = ctx.Check(first_.image && second_.image, "both cards resolve the sibling PNG through typed external references");
            if (!ready_) return;
            ctx.Check(first_.image->getWidthProperty() == 2 && first_.image->getHeightProperty() == 1,
                      "the sibling image is resolved from the parent asset directory, not the content root");
            ctx.Check(!first_.absentImage && !second_.absentImage && !first_.absentLabel.has_value() && !second_.absentLabel.has_value(),
                      "empty external strings produce nullopt textures and empty type-erased values");
            const auto* firstLabel = std::any_cast<LabelPtr>(&first_.label);
            const auto* secondLabel = std::any_cast<LabelPtr>(&second_.label);
            ready_ = ctx.Check(firstLabel && *firstLabel && secondLabel && *secondLabel,
                               "the compiled child is safely unboxed as its actual custom label type");
            if (!ready_) return;
            ctx.Check(*firstLabel == *secondLabel && labelReads == 1 && (*firstLabel)->title == "Knight" && (*firstLabel)->rank == 4,
                      "two different parent assets reuse the owning manager's type-erased child cache");
            title_ = (*firstLabel)->title;
            std::array<Color, 2> pixels{};
            first_.image->GetData(pixels.data(), 2);
            ctx.Check(pixels[0] == Color::Red && pixels[1] == Color::Blue, "the external image preserves its authored pixels");
            const Color gold = Color::Gold; const Rectangle firstPixel(0, 0, 1, 1);
            first_.image->SetData(0, &firstPixel, &gold, 0, 1); // partial edit preserves shared storage
            second_.image->GetData(pixels.data(), 2);
            auto directlyLoaded = content.Load<Texture2D>("external/images/banner");
            std::array<Color, 2> directPixels{}; directlyLoaded.GetData(directPixels.data(), 2);
            ctx.Check(pixels == directPixels && pixels[0] == gold && pixels[1] == Color::Blue,
                      "slash and backslash sibling paths resolve to the same typed cache as a direct owner Load");
            for (const char* name : {"escape", "absolute", "missing"})
            {
                bool refused = false, outside = false;
                try { (void)content.Load<Card>(std::string("external/entries/") + name); }
                catch (const ContentLoadException& error)
                {
                    refused = true;
                    outside = std::string(error.what()).find("outside the content root") != std::string::npos;
                    ctx.Print("{} reference refused: {}", name, error.what());
                }
                const bool isMissing = std::string(name) == "missing";
                ctx.Check(refused && (isMissing ? !outside : outside), std::string(name) + " external reference is rejected by the appropriate loader boundary");
            }
        }
        void Draw(const GameTime& gameTime) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color(24, 28, 36));
            if (ready_)
            {
                batch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
                batch_->DrawString(*font_, "XNB cards: sibling texture + compiled label", Vector2(40, 32), Color::White);
                batch_->DrawString(*font_, title_ + " A", Vector2(64, 128), Color::White);
                batch_->DrawString(*font_, title_ + " B", Vector2(448, 128), Color::White);
                batch_->Draw(*first_.image, Rectangle(64, 180, 256, 128), Color::White);
                batch_->Draw(*second_.image, Rectangle(448, 180, 256, 128), Color::White);
                batch_->DrawString(*font_, "One shared texture edit updates both cards.", Vector2(40, 360), Color::White);
                batch_->End();
                if (FramesDrawn() == 0)
                {
                    const CnaDemos::BackBufferPixels screen(device);
                    Context().Check(CnaDemos::Near(screen.At(100, 220), Color::Gold, 1) && CnaDemos::Near(screen.At(260, 220), Color::Blue, 1) &&
                                    CnaDemos::Near(screen.At(490, 220), Color::Gold, 1) && CnaDemos::Near(screen.At(650, 220), Color::Blue, 1),
                                    "both separately loaded cards render the updated shared external image");
                }
            }
            DemoGame::Draw(gameTime);
        }
    private:
        Card first_, second_;
        bool ready_ = false;
        std::string title_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(ExternalReferencesDemo, {
    .id = "content.xnb.external_references",
    .title = "Load an XNB card's external resources",
    .description = "Resolves sibling loose textures and compiled custom labels through typed and type-erased external references, "
                   "checks null references and owner caches, rejects unsafe or missing paths, and renders shared image edits.",
    .apis = {"ContentReader::ReadExternalReference", "ContentManager::Load", "ContentTypeReader::Read",
             "ContentTypeReaderManager::AddTypeCreator", "Texture2D::GetData", "Texture2D::SetData"},
    .tags = {"content", "xnb", "external-reference", "cache", "paths"},
    .frames = 3,
    .deterministic = true,
    .assets = {"external/entries/card_a.xnb", "external/entries/card_b.xnb", "external/entries/escape.xnb",
               "external/entries/absolute.xnb", "external/entries/missing.xnb", "external/labels/knight.xnb",
               "external/images/banner.png", "fonts/pixel_font.cnb"},
    .restrictions = "Absolute/escaping path refusal is CNA hardening; arbitrary custom T is not assumed supported by the templated external-reference overload.",
})
