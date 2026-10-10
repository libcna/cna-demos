// SPDX-License-Identifier: MS-PL
// Asset names and load errors: ContentLoadException for a missing asset, either path separator.
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Content;

namespace
{
    void MissingAssets(CnaDemos::DemoContext& ctx)
    {
        ContentManager content(nullptr, "assets");

        // A missing asset is a ContentLoadException, so a game can offer a fallback.
        std::string message;
        try
        {
            (void)content.Load<Curve>("curves/no_such_curve");
        }
        catch (const ContentLoadException& e)
        {
            message = e.what();
        }
        ctx.Print("ContentLoadException: {}", message);
        ctx.Check(message.find("curves/no_such_curve") != std::string::npos,
                  "a missing asset throws ContentLoadException naming it");

        // Asset names are relative to RootDirectory, without an extension. XNA games written
        // on Windows use backslashes; they work as well.
        ctx.Check(content.getRootDirectoryProperty() == "assets", "RootDirectory is the folder asset names start from");
        ctx.Check(content.Load<Curve>("curves\\bounce").getKeysProperty().getCountProperty() == 3,
                  "\"curves\\\\bounce\" loads the same asset as \"curves/bounce\"");
    }
}

CNA_DEMOS_CONSOLE(MissingAssets, {
    .id = "content.content_manager.missing_assets",
    .title = "Asset names and missing assets",
    .description = "Loads an asset that does not exist and catches the ContentLoadException, and loads an existing one "
                   "with a Windows-style backslash in its name.",
    .apis = {"ContentManager::Load", "ContentManager::RootDirectory", "ContentLoadException"},
    .tags = {"content", "error", "exception", "path", "fallback"},
    .assets = {"curves/bounce.cnb"},
})
