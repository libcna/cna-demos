// SPDX-License-Identifier: MS-PL
// Screen orientation: the orientations a game supports, the one it is in, and the change event.
// On a phone the device's rotation picks one; on a desktop CNA can emulate it with the arrow keys.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/DisplayOrientation.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    std::string Name(DisplayOrientation orientation)
    {
        switch (orientation)
        {
            case DisplayOrientation::Default: return "Default";
            case DisplayOrientation::LandscapeLeft: return "LandscapeLeft";
            case DisplayOrientation::LandscapeRight: return "LandscapeRight";
            case DisplayOrientation::Portrait: return "Portrait";
            default: return "several";
        }
    }

    class OrientationDemo final : public CnaDemos::DemoGame
    {
    public:
        OrientationDemo()
        {
            // A portrait game: taller than wide, Portrait only. (With the default
            // SupportedOrientations, the back buffer's shape alone would say the same.)
            Graphics().setPreferredBackBufferWidthProperty(480);
            Graphics().setPreferredBackBufferHeightProperty(800);
            Graphics().setSupportedOrientationsProperty(DisplayOrientation::Portrait);

            // A desktop does not rotate: CNA's extension makes the window behave like a device
            // that does, before the graphics device is created (Up asks for Portrait, Left and
            // Right for the landscapes -- granted only when SupportedOrientations allows it).
            GameWindow& window = getWindowProperty();
            window.setKeyboardOrientationEmulationEnabledEXT(true);
            window.OrientationChanged += [this](System::Object*, const System::EventArgs&) { ++changes_; };
        }

    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            const auto& pp = getGraphicsDeviceProperty().getPresentationParametersProperty();
            ctx.Print("starts in {}, back buffer {}x{}", Name(getWindowProperty().getCurrentOrientationProperty()),
                      pp.getBackBufferWidthProperty(), pp.getBackBufferHeightProperty());
            ctx.Check(getWindowProperty().getCurrentOrientationProperty() == DisplayOrientation::Portrait,
                      "a Portrait-only game starts in Portrait");
            ctx.Check(pp.getBackBufferWidthProperty() < pp.getBackBufferHeightProperty(),
                      "the back buffer is taller than wide");
            changesAtStart_ = changes_;
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            if (++updates_ == 3)
            {
                // A landscape-only minigame: the game now supports LandscapeLeft only, and the
                // window turns.
                Graphics().setSupportedOrientationsProperty(DisplayOrientation::LandscapeLeft);
                Graphics().ApplyChanges();
                const auto& pp = getGraphicsDeviceProperty().getPresentationParametersProperty();
                Context().Print("landscape only: {}, back buffer {}x{}, OrientationChanged x{}",
                                Name(getWindowProperty().getCurrentOrientationProperty()),
                                pp.getBackBufferWidthProperty(), pp.getBackBufferHeightProperty(), changes_);
                Context().Check(getWindowProperty().getCurrentOrientationProperty() == DisplayOrientation::LandscapeLeft,
                                "with only LandscapeLeft supported, the window turns to LandscapeLeft");
                Context().Check(changes_ == changesAtStart_ + 1, "the turn raised OrientationChanged once");
                Context().Check(pp.getBackBufferWidthProperty() > pp.getBackBufferHeightProperty(),
                                "the back buffer is now wider than tall");
                Context().Check(getPresentationParametersOrientation() == DisplayOrientation::LandscapeLeft,
                                "PresentationParameters::DisplayOrientation follows");
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(28, 36, 52));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, Name(getWindowProperty().getCurrentOrientationProperty()),
                                     Vector2(20.0f, 20.0f), Color::White);
            spriteBatch_->DrawString(*font_, "keys: Up Portrait, Left/Right Landscape", Vector2(20.0f, 50.0f),
                                     Color::LightGray);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        DisplayOrientation getPresentationParametersOrientation()
        {
            return getGraphicsDeviceProperty().getPresentationParametersProperty().getDisplayOrientationProperty();
        }

        int updates_ = 0;
        int changes_ = 0;
        int changesAtStart_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(OrientationDemo, {
    .id = "game.window.orientation",
    .title = "Screen orientation",
    .description = "A Portrait-only game (GraphicsDeviceManager::SupportedOrientations, a taller-than-wide back "
                   "buffer) starts in Portrait; supporting LandscapeLeft only turns the window, swaps the back "
                   "buffer's sides and raises GameWindow::OrientationChanged. On a desktop, CNA's keyboard "
                   "emulation lets the arrow keys request an orientation.",
    .apis = {"GraphicsDeviceManager::SupportedOrientations", "GameWindow::CurrentOrientation",
             "GameWindow::OrientationChanged", "PresentationParameters::DisplayOrientation", "DisplayOrientation",
             "GameWindow::KeyboardOrientationEmulationEnabledEXT"},
    .tags = {"window", "orientation", "portrait", "landscape", "mobile"},
    .needs = CnaDemos::Need::Window,
    .frames = 10,
    .assets = {"fonts/pixel_font.cnb"},
})
