// SPDX-License-Identifier: MS-PL
// CNAEXT LooseFileContentTypeReader: teach ContentManager to load a game's own file format.
#include <algorithm>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/LooseFileContentTypeReader.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Content;

namespace
{
    // The game's own asset type: a tile map.
    struct Level
    {
        std::vector<std::string> rows;
        int gems = 0;
    };

    // Reads "*.level" text files: lines starting with "# " are comments, every other line is a row.
    class LevelReader final : public LooseFileContentTypeReader<Level>
    {
    public:
        int reads = 0;

        [[nodiscard]] std::vector<std::string> GetExtensions() const override { return {".level"}; }

        // `path` is the file ContentManager found below its RootDirectory.
        Level Read(const std::string& path, ContentManager&) override
        {
            ++reads;
            std::ifstream file(path);
            if (!file)
                throw std::runtime_error("cannot open " + path);  // Load reports it as ContentLoadException
            Level level;
            for (std::string line; std::getline(file, line);)
            {
                if (line.empty() || line.starts_with("# "))
                    continue;
                level.gems += static_cast<int>(std::count(line.begin(), line.end(), '*'));
                level.rows.push_back(line);
            }
            return level;
        }
    };

    void LooseFileReader(CnaDemos::DemoContext& ctx)
    {
        ContentManager content(nullptr, "assets");
        // Register once; from then on Load<Level> finds "levels/level1" as levels/level1.level,
        // caches it like any other asset, and reports failures as ContentLoadException.
        auto reader = std::make_unique<LevelReader>();
        LevelReader& readerRef = *reader;
        content.RegisterTypeReader<Level>(std::move(reader));

        const Level level = content.Load<Level>("levels/level1");
        for (const std::string& row : level.rows)
            ctx.Print("  {}", row);
        ctx.Check(level.rows.size() == 5 && level.rows.front() == "##########", "the level has its five rows");
        ctx.Check(level.gems == 2, "two gems are on the map");
        (void)content.Load<Level>("levels/level1");
        ctx.Check(readerRef.reads == 1, "the second Load is served from the cache");
    }
}

CNA_DEMOS_CONSOLE(LooseFileReader, {
    .id = "content.custom.loose_file_reader",
    .title = "A custom asset type (CNAEXT)",
    .description = "Registers a LooseFileContentTypeReader for a game's own \".level\" text format with "
                   "ContentManager::RegisterTypeReader, then loads a tile map with Load<Level>, cached like any asset.",
    .apis = {"ContentManager::RegisterTypeReader", "LooseFileContentTypeReader::Read",
             "LooseFileContentTypeReader::GetExtensions", "ContentManager::Load",
             "ContentLoadException"},
    .tags = {"content", "custom", "reader", "level", "file-format", "cnaext"},
    .assets = {"levels/level1.level"},
})
