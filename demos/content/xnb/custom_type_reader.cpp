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
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

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
        float gravity = 0.0f;
        double parTime = 0.0;
        Color ambient;
        Matrix transform;
        Quaternion orientation;
        Vector2 spawn;
        Vector3 camera;
        Vector4 fog;
    };

    // Process-wide registration uses a capture-free factory; observations also outlive readers.
    struct ReaderObservation
    {
        int calls = 0;
        std::string assetName;
        ContentManager* owner = nullptr;
    };
    ReaderObservation observed;

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
            ++observed.calls;
            observed.assetName = input.getAssetNameProperty();
            observed.owner = input.getContentManagerProperty();
            Level level;
            level.name = input.ReadString();
            const int rows = input.ReadInt32();
            for (int i = 0; i < rows; ++i)
            {
                level.rows.push_back(input.ReadString());
            }
            level.gems = input.ReadInt32();
            level.gravity = input.ReadSingle();
            level.parTime = input.ReadDouble();
            level.ambient = input.ReadColor();
            level.transform = input.ReadMatrix();
            level.orientation = input.ReadQuaternion();
            level.spawn = input.ReadVector2();
            level.camera = input.ReadVector3();
            level.fog = input.ReadVector4();
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

            const int readsBefore = observed.calls;
            level_ = getContentProperty().Load<Level>("levels/castle");
            ctx.Check(observed.calls == readsBefore + 1, "the first Load invokes the custom reader once");
            ctx.Check(observed.assetName == "levels/castle" && observed.owner == &getContentProperty(),
                      "ContentReader carries the logical asset name and owning manager");
            ctx.Print("level \"{}\": {} rows, {} gems", level_.name, level_.rows.size(), level_.gems);
            ctx.Check(level_.name == "Castle" && level_.rows.size() == 5, "the reader rebuilt the level from the .xnb");
            int counted = 0;
            for (const std::string& row : level_.rows)
            {
                counted += static_cast<int>(std::count(row.begin(), row.end(), 'G'));
            }
            ctx.Check(level_.gems == counted && counted == 4, "the stored gem count matches the map");
            ctx.Check(level_.gravity == -1.5f && level_.parTime == 87.25,
                      "the reader preserves the authored single- and double-precision scalars");
            ctx.Check(level_.ambient == Color(17, 34, 51, 204),
                      "ReadColor preserves the authored RGBA byte order");
            ctx.Check(level_.transform == Matrix(1.0f, 2.0f, 3.0f, 4.0f,
                                                  5.0f, 6.0f, 7.0f, 8.0f,
                                                  9.0f, 10.0f, 11.0f, 12.0f,
                                                  13.0f, 14.0f, 15.0f, 16.0f),
                      "ReadMatrix preserves all sixteen row-major elements");
            ctx.Check(level_.orientation == Quaternion(0.25f, -0.5f, 0.75f, 1.0f),
                      "ReadQuaternion preserves X, Y, Z and W order");
            ctx.Check(level_.spawn == Vector2(1.25f, -2.5f) &&
                          level_.camera == Vector3(3.5f, 4.5f, -5.5f) &&
                          level_.fog == Vector4(-1.0f, 2.0f, -3.0f, 4.0f),
                      "the vector readers preserve every authored component in order");

            // Loaded once, cached like any other asset.
            const Level again = getContentProperty().Load<Level>("levels/castle");
            ctx.Check(again.rows == level_.rows && observed.calls == readsBefore + 1,
                      "a second Load returns the cached level without invoking its reader again");
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
    .description = "Reads a level of the game's own type and its authored scalar, color, matrix, quaternion and vector "
                   "fields from an .xnb file with a ContentTypeReader<Level> subclass registered under the reader name "
                   "in the file's type-reader table, then loads it with ContentManager::Load<Level>.",
    .apis = {"ContentTypeReader::Read", "ContentTypeReader::ContentTypeReader",
             "ContentTypeReaderManager::AddTypeCreator", "ContentReader::ReadString", "ContentReader::ReadInt32",
             "ContentReader::ReadSingle", "ContentReader::ReadDouble", "ContentReader::ReadColor",
             "ContentReader::ReadMatrix", "ContentReader::ReadQuaternion", "ContentReader::ReadVector2",
             "ContentReader::ReadVector3", "ContentReader::ReadVector4",
             "ContentManager::Load", "ContentReader::AssetName", "ContentReader::ContentManager",
             "ContentReader", "ContentTypeReaderManager"},
    .tags = {"content", "xnb", "custom", "reader", "level", "math"},
    .frames = 3,
    .deterministic = true,
    .assets = {"levels/castle.xnb", "fonts/pixel_font.cnb"},
})
