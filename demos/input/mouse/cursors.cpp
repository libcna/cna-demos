// SPDX-License-Identifier: MS-PL
// CNAEXT MouseCursor: the system cursor shapes, and a cursor made from a Texture2D.
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/MouseCursor.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    struct Tile
    {
        std::string_view label;
        MouseCursor& (*cursor)();
    };

    constexpr std::array<Tile, 12> SystemTiles = {{{"Arrow", &MouseCursor::getArrowProperty},
                                                   {"Crosshair", &MouseCursor::getCrosshairProperty},
                                                   {"Hand", &MouseCursor::getHandProperty},
                                                   {"IBeam", &MouseCursor::getIBeamProperty},
                                                   {"No", &MouseCursor::getNoProperty},
                                                   {"SizeAll", &MouseCursor::getSizeAllProperty},
                                                   {"SizeNESW", &MouseCursor::getSizeNESWProperty},
                                                   {"SizeNS", &MouseCursor::getSizeNSProperty},
                                                   {"SizeNWSE", &MouseCursor::getSizeNWSEProperty},
                                                   {"SizeWE", &MouseCursor::getSizeWEProperty},
                                                   {"Wait", &MouseCursor::getWaitProperty},
                                                   {"WaitArrow", &MouseCursor::getWaitArrowProperty}}};
    constexpr int Columns = 4;
    constexpr int TileCount = static_cast<int>(SystemTiles.size()) + 1;  // + the custom cursor

    Rectangle TileBounds(int index)
    {
        return Rectangle(40 + (index % Columns) * 180, 60 + (index / Columns) * 100, 170, 90);
    }

    class CursorsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Initialize() override
        {
            setIsMouseVisibleProperty(true);
            DemoGame::Initialize();
        }

        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // A custom cursor: a 32x32 ring with a dot, its hot spot in the middle. The texture
            // must be SurfaceFormat::Color; transparent pixels stay transparent.
            constexpr int Size = 32;
            std::vector<Color> pixels(Size * Size, Color::Transparent);
            for (int y = 0; y < Size; ++y)
            {
                for (int x = 0; x < Size; ++x)
                {
                    const int dx = x - Size / 2;
                    const int dy = y - Size / 2;
                    const int distance = dx * dx + dy * dy;
                    if ((distance >= 11 * 11 && distance <= 14 * 14) || distance <= 2 * 2)
                        pixels[static_cast<std::size_t>(y * Size + x)] = Color::Magenta;
                }
            }
            cursorImage_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), Size, Size);
            cursorImage_->SetData(pixels.data(), static_cast<int>(pixels.size()));
            custom_.emplace(MouseCursor::FromTexture2D(*cursorImage_, Size / 2, Size / 2));
            leftWasDown_ = Mouse::GetState().getLeftButtonProperty() == ButtonState::Pressed;
        }

        void UnloadContent() override
        {
            // There is no current-cursor getter. Only replace a cursor this demo installed, and
            // use the documented stock Arrow so a custom cursor is never left dangling.
            if (wroteCursor_)
                Mouse::SetCursor(MouseCursor::getArrowProperty());
            custom_->Dispose();
        }

        void Update(GameTime& gameTime) override
        {
            const MouseState mouse = Mouse::GetState();
            int hovered = -1;
            for (int i = 0; i < TileCount; ++i)
            {
                if (TileBounds(i).Contains(mouse.getXProperty(), mouse.getYProperty()))
                    hovered = i;
            }
            hovered_ = hovered;
            const bool leftDown = mouse.getLeftButtonProperty() == ButtonState::Pressed;
            if (!Context().Automated() && leftDown && !leftWasDown_ && hovered >= 0)
            {
                if (hovered < static_cast<int>(SystemTiles.size()))
                    Mouse::SetCursor(SystemTiles[static_cast<std::size_t>(hovered)].cursor());
                else
                    Mouse::SetCursor(*custom_);
                wroteCursor_ = true;
            }
            leftWasDown_ = leftDown;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Click a tile to explicitly change the cursor", Vector2(40.0f, 20.0f),
                                     Color::White);
            for (int i = 0; i < TileCount; ++i)
            {
                const Rectangle bounds = TileBounds(i);
                const bool custom = i == static_cast<int>(SystemTiles.size());
                spriteBatch_->Draw(*pixel_, bounds, i == hovered_ ? Color::SteelBlue : Color::DimGray);
                const std::string label(custom ? "FromTexture2D" : SystemTiles[static_cast<std::size_t>(i)].label);
                spriteBatch_->DrawString(*font_, label,
                                         Vector2(static_cast<float>(bounds.X + 10), static_cast<float>(bounds.Y + 10)),
                                         Color::White);
                if (custom)
                    spriteBatch_->Draw(*cursorImage_, Vector2(static_cast<float>(bounds.X + 10),
                                                              static_cast<float>(bounds.Y + 40)),
                                       Color::White);
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        int hovered_ = -2;
        bool leftWasDown_ = false;
        bool wroteCursor_ = false;
        std::unique_ptr<Texture2D> cursorImage_;
        std::optional<MouseCursor> custom_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(CursorsDemo, {
    .id = "input.mouse.cursors",
    .title = "Cursor shapes (CNAEXT)",
    .description = "Clicking a tile in an attended run explicitly calls Mouse::SetCursor for twelve system shapes or "
                   "one FromTexture2D cursor. Writes occur only on click, and a custom cursor is replaced by stock "
                   "Arrow before disposal.",
    .apis = {"Mouse::SetCursor", "MouseCursor::FromTexture2D", "MouseCursor::Arrow", "MouseCursor::Crosshair",
             "MouseCursor::Hand", "MouseCursor::IBeam", "MouseCursor::No", "MouseCursor::SizeAll",
             "MouseCursor::SizeNESW", "MouseCursor::SizeNS", "MouseCursor::SizeNWSE", "MouseCursor::SizeWE",
             "MouseCursor::Wait", "MouseCursor::WaitArrow", "MouseCursor::Dispose"},
    .tags = {"mouse", "cursor", "pointer", "hand", "custom-cursor", "cnaext", "explicit-consent"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::Mouse,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = {"Unattended runs never call SetCursor. CNA has no current-cursor getter, so after an explicit "
                     "change shutdown uses stock Arrow as a safe fallback and does not claim exact restoration."},
})
