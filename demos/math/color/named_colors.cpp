// SPDX-License-Identifier: MS-PL
// XNA's 141 named colours as a chart: Color::CornflowerBlue and the rest, and what is special about
// Color::Transparent in XNA 4.0.
#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <memory>
#include <optional>
#include <set>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    struct Named
    {
        const char* name;
        const Color* color;
    };

    // Every named colour of XNA 4.0, in the documentation's (alphabetical) order.
    const std::array<Named, 141> Colors = {{
        {"AliceBlue", &Color::AliceBlue},
        {"AntiqueWhite", &Color::AntiqueWhite},
        {"Aqua", &Color::Aqua},
        {"Aquamarine", &Color::Aquamarine},
        {"Azure", &Color::Azure},
        {"Beige", &Color::Beige},
        {"Bisque", &Color::Bisque},
        {"Black", &Color::Black},
        {"BlanchedAlmond", &Color::BlanchedAlmond},
        {"Blue", &Color::Blue},
        {"BlueViolet", &Color::BlueViolet},
        {"Brown", &Color::Brown},
        {"BurlyWood", &Color::BurlyWood},
        {"CadetBlue", &Color::CadetBlue},
        {"Chartreuse", &Color::Chartreuse},
        {"Chocolate", &Color::Chocolate},
        {"Coral", &Color::Coral},
        {"CornflowerBlue", &Color::CornflowerBlue},
        {"Cornsilk", &Color::Cornsilk},
        {"Crimson", &Color::Crimson},
        {"Cyan", &Color::Cyan},
        {"DarkBlue", &Color::DarkBlue},
        {"DarkCyan", &Color::DarkCyan},
        {"DarkGoldenrod", &Color::DarkGoldenrod},
        {"DarkGray", &Color::DarkGray},
        {"DarkGreen", &Color::DarkGreen},
        {"DarkKhaki", &Color::DarkKhaki},
        {"DarkMagenta", &Color::DarkMagenta},
        {"DarkOliveGreen", &Color::DarkOliveGreen},
        {"DarkOrange", &Color::DarkOrange},
        {"DarkOrchid", &Color::DarkOrchid},
        {"DarkRed", &Color::DarkRed},
        {"DarkSalmon", &Color::DarkSalmon},
        {"DarkSeaGreen", &Color::DarkSeaGreen},
        {"DarkSlateBlue", &Color::DarkSlateBlue},
        {"DarkSlateGray", &Color::DarkSlateGray},
        {"DarkTurquoise", &Color::DarkTurquoise},
        {"DarkViolet", &Color::DarkViolet},
        {"DeepPink", &Color::DeepPink},
        {"DeepSkyBlue", &Color::DeepSkyBlue},
        {"DimGray", &Color::DimGray},
        {"DodgerBlue", &Color::DodgerBlue},
        {"Firebrick", &Color::Firebrick},
        {"FloralWhite", &Color::FloralWhite},
        {"ForestGreen", &Color::ForestGreen},
        {"Fuchsia", &Color::Fuchsia},
        {"Gainsboro", &Color::Gainsboro},
        {"GhostWhite", &Color::GhostWhite},
        {"Gold", &Color::Gold},
        {"Goldenrod", &Color::Goldenrod},
        {"Gray", &Color::Gray},
        {"Green", &Color::Green},
        {"GreenYellow", &Color::GreenYellow},
        {"Honeydew", &Color::Honeydew},
        {"HotPink", &Color::HotPink},
        {"IndianRed", &Color::IndianRed},
        {"Indigo", &Color::Indigo},
        {"Ivory", &Color::Ivory},
        {"Khaki", &Color::Khaki},
        {"Lavender", &Color::Lavender},
        {"LavenderBlush", &Color::LavenderBlush},
        {"LawnGreen", &Color::LawnGreen},
        {"LemonChiffon", &Color::LemonChiffon},
        {"LightBlue", &Color::LightBlue},
        {"LightCoral", &Color::LightCoral},
        {"LightCyan", &Color::LightCyan},
        {"LightGoldenrodYellow", &Color::LightGoldenrodYellow},
        {"LightGray", &Color::LightGray},
        {"LightGreen", &Color::LightGreen},
        {"LightPink", &Color::LightPink},
        {"LightSalmon", &Color::LightSalmon},
        {"LightSeaGreen", &Color::LightSeaGreen},
        {"LightSkyBlue", &Color::LightSkyBlue},
        {"LightSlateGray", &Color::LightSlateGray},
        {"LightSteelBlue", &Color::LightSteelBlue},
        {"LightYellow", &Color::LightYellow},
        {"Lime", &Color::Lime},
        {"LimeGreen", &Color::LimeGreen},
        {"Linen", &Color::Linen},
        {"Magenta", &Color::Magenta},
        {"Maroon", &Color::Maroon},
        {"MediumAquamarine", &Color::MediumAquamarine},
        {"MediumBlue", &Color::MediumBlue},
        {"MediumOrchid", &Color::MediumOrchid},
        {"MediumPurple", &Color::MediumPurple},
        {"MediumSeaGreen", &Color::MediumSeaGreen},
        {"MediumSlateBlue", &Color::MediumSlateBlue},
        {"MediumSpringGreen", &Color::MediumSpringGreen},
        {"MediumTurquoise", &Color::MediumTurquoise},
        {"MediumVioletRed", &Color::MediumVioletRed},
        {"MidnightBlue", &Color::MidnightBlue},
        {"MintCream", &Color::MintCream},
        {"MistyRose", &Color::MistyRose},
        {"Moccasin", &Color::Moccasin},
        {"NavajoWhite", &Color::NavajoWhite},
        {"Navy", &Color::Navy},
        {"OldLace", &Color::OldLace},
        {"Olive", &Color::Olive},
        {"OliveDrab", &Color::OliveDrab},
        {"Orange", &Color::Orange},
        {"OrangeRed", &Color::OrangeRed},
        {"Orchid", &Color::Orchid},
        {"PaleGoldenrod", &Color::PaleGoldenrod},
        {"PaleGreen", &Color::PaleGreen},
        {"PaleTurquoise", &Color::PaleTurquoise},
        {"PaleVioletRed", &Color::PaleVioletRed},
        {"PapayaWhip", &Color::PapayaWhip},
        {"PeachPuff", &Color::PeachPuff},
        {"Peru", &Color::Peru},
        {"Pink", &Color::Pink},
        {"Plum", &Color::Plum},
        {"PowderBlue", &Color::PowderBlue},
        {"Purple", &Color::Purple},
        {"Red", &Color::Red},
        {"RosyBrown", &Color::RosyBrown},
        {"RoyalBlue", &Color::RoyalBlue},
        {"SaddleBrown", &Color::SaddleBrown},
        {"Salmon", &Color::Salmon},
        {"SandyBrown", &Color::SandyBrown},
        {"SeaGreen", &Color::SeaGreen},
        {"SeaShell", &Color::SeaShell},
        {"Sienna", &Color::Sienna},
        {"Silver", &Color::Silver},
        {"SkyBlue", &Color::SkyBlue},
        {"SlateBlue", &Color::SlateBlue},
        {"SlateGray", &Color::SlateGray},
        {"Snow", &Color::Snow},
        {"SpringGreen", &Color::SpringGreen},
        {"SteelBlue", &Color::SteelBlue},
        {"Tan", &Color::Tan},
        {"Teal", &Color::Teal},
        {"Thistle", &Color::Thistle},
        {"Tomato", &Color::Tomato},
        {"Transparent", &Color::Transparent},
        {"Turquoise", &Color::Turquoise},
        {"Violet", &Color::Violet},
        {"Wheat", &Color::Wheat},
        {"White", &Color::White},
        {"WhiteSmoke", &Color::WhiteSmoke},
        {"Yellow", &Color::Yellow},
        {"YellowGreen", &Color::YellowGreen}
    }};

    constexpr int Columns = 12;
    constexpr int CellWidth = 60;
    constexpr int CellHeight = 26;
    constexpr int Left = 40;
    constexpr int Top = 24;
    const Color Background(64, 64, 72);

    Rectangle Cell(std::size_t index)
    {
        return Rectangle(Left + static_cast<int>(index % Columns) * CellWidth,
                         Top + static_cast<int>(index / Columns) * CellHeight, CellWidth - 4, CellHeight - 4);
    }

    class NamedColorsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            std::set<std::string> names;
            int opaque = 0;
            for (const Named& named : Colors)
            {
                names.insert(named.name);
                opaque += named.color->getAProperty() == 255 ? 1 : 0;
            }
            ctx.Check(names.size() == 141, "141 named colours");
            // XNA 4.0 colours are premultiplied: Transparent is (0, 0, 0, 0), not System.Drawing's
            // transparent white. Every other named colour is opaque.
            ctx.Print("Transparent = {}", Color::Transparent.ToString());
            ctx.Check(Color::Transparent == Color(0, 0, 0, 0) && opaque == 140,
                      "Transparent is (0, 0, 0, 0); the other 140 are opaque");
            // The colour of every XNA project template's first frame.
            ctx.Check(Color::CornflowerBlue == Color(100, 149, 237), "CornflowerBlue is (100, 149, 237)");
            // As in .NET, some names share a value.
            ctx.Check(Color::Aqua == Color::Cyan && Color::Fuchsia == Color::Magenta, "Aqua = Cyan, Fuchsia = Magenta");
            selected_ = static_cast<std::size_t>(
                std::find_if(Colors.begin(), Colors.end(), [](const Named& n) { return n.color == &Color::CornflowerBlue; }) -
                Colors.begin());
        }

        void Update(GameTime& gameTime) override
        {
            // Arrow keys move the selection.
            const KeyboardState keys = Keyboard::GetState();
            const auto pressed = [&](Keys key) { return keys.IsKeyDown(key) && previous_.IsKeyUp(key); };
            const std::size_t count = Colors.size();
            if (pressed(Keys::Right)) selected_ = (selected_ + 1) % count;
            if (pressed(Keys::Left)) selected_ = (selected_ + count - 1) % count;
            if (pressed(Keys::Down)) selected_ = std::min(count - 1, selected_ + Columns);
            if (pressed(Keys::Up)) selected_ = selected_ >= Columns ? selected_ - Columns : selected_;
            previous_ = keys;
            DemoGame::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Background);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            const Rectangle frame = Cell(selected_);
            spriteBatch_->Draw(*pixel_, Rectangle(frame.X - 2, frame.Y - 2, frame.Width + 4, frame.Height + 4),
                               Color::White);
            for (std::size_t i = 0; i < Colors.size(); ++i)
            {
                spriteBatch_->Draw(*pixel_, Cell(i), *Colors[i].color);
            }
            const Named& named = Colors[selected_];
            spriteBatch_->DrawString(*font_, std::format("Color::{}  {}", named.name, named.color->ToString()),
                                     Vector2(40.0f, 440.0f), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                // Each swatch shows its colour; Transparent shows what is behind it.
                const CnaDemos::BackBufferPixels pixels(device);
                int shown = 0;
                for (std::size_t i = 0; i < Colors.size(); ++i)
                {
                    const Rectangle cell = Cell(i);
                    const Color expected = Colors[i].color == &Color::Transparent ? Background : *Colors[i].color;
                    shown += pixels.At(cell.X + cell.Width / 2, cell.Y + cell.Height / 2) == expected ? 1 : 0;
                }
                Context().Print("{} of {} swatches show their colour", shown, Colors.size());
                Context().Check(shown == static_cast<int>(Colors.size()),
                                "every swatch shows its colour (Transparent: the background)");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
        std::size_t selected_ = 0;
        KeyboardState previous_;
    };
}

CNA_DEMOS_GAME(NamedColorsDemo, {
    .id = "math.color.named_colors",
    .title = "Named colours",
    .description = "A chart of XNA's 141 named colours (arrow keys select one and show its value): Transparent is "
                   "(0, 0, 0, 0) because XNA 4.0 colours are premultiplied, CornflowerBlue is the project templates' "
                   "clear colour, and Aqua/Cyan and Fuchsia/Magenta are the same colour.",
    .apis = {
             "Color::AliceBlue", "Color::AntiqueWhite", "Color::Aqua", "Color::Aquamarine",
             "Color::Azure", "Color::Beige", "Color::Bisque", "Color::Black", "Color::BlanchedAlmond",
             "Color::Blue", "Color::BlueViolet", "Color::Brown", "Color::BurlyWood", "Color::CadetBlue",
             "Color::Chartreuse", "Color::Chocolate", "Color::Coral", "Color::CornflowerBlue",
             "Color::Cornsilk", "Color::Crimson", "Color::Cyan", "Color::DarkBlue", "Color::DarkCyan",
             "Color::DarkGoldenrod", "Color::DarkGray", "Color::DarkGreen", "Color::DarkKhaki",
             "Color::DarkMagenta", "Color::DarkOliveGreen", "Color::DarkOrange", "Color::DarkOrchid",
             "Color::DarkRed", "Color::DarkSalmon", "Color::DarkSeaGreen", "Color::DarkSlateBlue",
             "Color::DarkSlateGray", "Color::DarkTurquoise", "Color::DarkViolet", "Color::DeepPink",
             "Color::DeepSkyBlue", "Color::DimGray", "Color::DodgerBlue", "Color::Firebrick",
             "Color::FloralWhite", "Color::ForestGreen", "Color::Fuchsia", "Color::Gainsboro",
             "Color::GhostWhite", "Color::Gold", "Color::Goldenrod", "Color::Gray", "Color::Green",
             "Color::GreenYellow", "Color::Honeydew", "Color::HotPink", "Color::IndianRed",
             "Color::Indigo", "Color::Ivory", "Color::Khaki", "Color::Lavender", "Color::LavenderBlush",
             "Color::LawnGreen", "Color::LemonChiffon", "Color::LightBlue", "Color::LightCoral",
             "Color::LightCyan", "Color::LightGoldenrodYellow", "Color::LightGray", "Color::LightGreen",
             "Color::LightPink", "Color::LightSalmon", "Color::LightSeaGreen", "Color::LightSkyBlue",
             "Color::LightSlateGray", "Color::LightSteelBlue", "Color::LightYellow", "Color::Lime",
             "Color::LimeGreen", "Color::Linen", "Color::Magenta", "Color::Maroon",
             "Color::MediumAquamarine", "Color::MediumBlue", "Color::MediumOrchid",
             "Color::MediumPurple", "Color::MediumSeaGreen", "Color::MediumSlateBlue",
             "Color::MediumSpringGreen", "Color::MediumTurquoise", "Color::MediumVioletRed",
             "Color::MidnightBlue", "Color::MintCream", "Color::MistyRose", "Color::Moccasin",
             "Color::NavajoWhite", "Color::Navy", "Color::OldLace", "Color::Olive", "Color::OliveDrab",
             "Color::Orange", "Color::OrangeRed", "Color::Orchid", "Color::PaleGoldenrod",
             "Color::PaleGreen", "Color::PaleTurquoise", "Color::PaleVioletRed", "Color::PapayaWhip",
             "Color::PeachPuff", "Color::Peru", "Color::Pink", "Color::Plum", "Color::PowderBlue",
             "Color::Purple", "Color::Red", "Color::RosyBrown", "Color::RoyalBlue",
             "Color::SaddleBrown", "Color::Salmon", "Color::SandyBrown", "Color::SeaGreen",
             "Color::SeaShell", "Color::Sienna", "Color::Silver", "Color::SkyBlue", "Color::SlateBlue",
             "Color::SlateGray", "Color::Snow", "Color::SpringGreen", "Color::SteelBlue", "Color::Tan",
             "Color::Teal", "Color::Thistle", "Color::Tomato", "Color::Transparent", "Color::Turquoise",
             "Color::Violet", "Color::Wheat", "Color::White", "Color::WhiteSmoke", "Color::Yellow",
             "Color::YellowGreen"},
    .tags = {"color", "named-colors", "palette", "reference"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
