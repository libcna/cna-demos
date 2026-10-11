// SPDX-License-Identifier: MS-PL
// ContentTypeReader<T>: reading your own type from an .xnb file, the XNA way.
#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReaderManager.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Content;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // The game's own content type. (A Game demo: a console program's ContentManager would look
    // for the .xnb below the working directory before CNA's platform exists, CNA-DEMOS-ISSUE-014.)
    struct Level
    {
        std::string name;
        std::vector<std::string> rows;
        int gems = 0;
    };

    // In XNA a ContentTypeWriter in the content pipeline wrote the .xnb and named this reader in
    // the file's type-reader table; this reads the fields back in the same order. (Here the
    // .xnb comes from tools/generate_assets.py, which writes exactly those bytes.)
    class LevelReader final : public ContentTypeReader<Level>
    {
    public:
        LevelReader()
            : ContentTypeReader<Level>("CnaDemos.Level")
        {
        }

    protected:
        Level Read(ContentReader& input, std::optional<Level>) override
        {
            Level level;
            level.name = input.ReadString();
            const int rows = input.ReadInt32();
            for (int i = 0; i < rows; ++i)
            {
                level.rows.push_back(input.ReadString());
            }
            level.gems = input.ReadInt32();
            return level;
        }
    };

    class CustomTypeReaderDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // XNA finds readers by reflection; CNA has none, so the reader is registered under the
            // name the .xnb's type-reader table uses (without the assembly part).
            ContentTypeReaderManager::AddTypeCreator("CnaDemos.LevelReader",
                                                     [] { return std::make_unique<LevelReader>(); });

            level_ = getContentProperty().Load<Level>("levels/castle");
            ctx.Print("level \"{}\": {} rows, {} gems", level_.name, level_.rows.size(), level_.gems);
            ctx.Check(level_.name == "Castle" && level_.rows.size() == 5, "the reader rebuilt the level from the .xnb");
            int counted = 0;
            for (const std::string& row : level_.rows)
            {
                counted += static_cast<int>(std::count(row.begin(), row.end(), 'G'));
            }
            ctx.Check(level_.gems == counted && counted == 4, "the stored gem count matches the map");

            // Loaded once, cached like any other asset.
            const Level again = getContentProperty().Load<Level>("levels/castle");
            ctx.Check(again.rows == level_.rows, "a second Load returns the cached level");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(28, 30, 40));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Level \"" + level_.name + "\" from levels/castle.xnb, " +
                                                 std::to_string(level_.gems) + " gems",
                                     Vector2(40.0f, 40.0f), Color::White);
            float y = 110.0f;
            for (const std::string& row : level_.rows)
            {
                spriteBatch_->DrawString(*font_, row, Vector2(300.0f, y), Color::Gold, 0.0f, Vector2::Zero, 2.0f,
                                         SpriteEffects::None, 0.0f);
                y += 44.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        Level level_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(CustomTypeReaderDemo, {
    .id = "content.xnb.custom_type_reader",
    .title = "A custom ContentTypeReader",
    .description = "Reads a level of the game's own type from an .xnb file with a ContentTypeReader<Level> "
                   "subclass, registered with ContentTypeReaderManager::AddTypeCreator under the reader name in "
                   "the file's type-reader table, and loaded with ContentManager::Load<Level>.",
    .apis = {"ContentTypeReader::Read", "ContentTypeReader::ContentTypeReader",
             "ContentTypeReaderManager::AddTypeCreator", "ContentReader::ReadString", "ContentReader::ReadInt32",
             "ContentManager::Load"},
    .tags = {"content", "xnb", "custom", "reader", "level"},
    .frames = 3,
    .deterministic = true,
    .assets = {"levels/castle.xnb", "fonts/pixel_font.cnb"},
})
