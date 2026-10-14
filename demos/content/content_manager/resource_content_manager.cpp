// SPDX-License-Identifier: MS-PL
// ResourceContentManager: content compiled into the executable instead of shipped as files.
#include <any>
#include <cmath>
#include <cstdint>
#include <exception>
#include <optional>
#include <string_view>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ResourceContentManager.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveLoopType.hpp"
#include "System/Resources/ResourceManager.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Content;

namespace
{
    // An .xnb file as a binary resource of the program -- what a .resx would embed in an XNA game.
    // It holds a Curve (CurveReader): PreLoop Constant, PostLoop Cycle, keys (0, 0), (0.5, 1), (1, 0).
    const std::vector<std::uint8_t> BounceXnb = {
        0x58, 0x4e, 0x42, 0x77, 0x05, 0x00, 0x85, 0x00, 0x00, 0x00, 0x01, 0x2b, 0x4d, 0x69, 0x63, 0x72,
        0x6f, 0x73, 0x6f, 0x66, 0x74, 0x2e, 0x58, 0x6e, 0x61, 0x2e, 0x46, 0x72, 0x61, 0x6d, 0x65, 0x77,
        0x6f, 0x72, 0x6b, 0x2e, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2e, 0x43, 0x75, 0x72, 0x76,
        0x65, 0x52, 0x65, 0x61, 0x64, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
    };

    void ResourceContent(CnaDemos::DemoContext& ctx)
    {
        // The resource family: no reflection in C++, so a lookup function stands in for the
        // assembly's resource table. Binary resources are byte vectors (.NET's byte[]).
        System::Resources::ResourceManager resources(
            "CnaDemos.EmbeddedContent",
            [](std::string_view, std::string_view, std::string_view) -> std::optional<std::string> { return std::nullopt; },
            [](std::string_view, std::string_view culture, std::string_view name) -> std::optional<std::any> {
                if (culture.empty() && name == "Bounce")
                    return std::any(BounceXnb);
                return std::nullopt;
            });

        // No content directory involved: assets come from the resource family, by name. CNA now
        // routes this Load<T> through OpenStream, but does so before registering the built-in XNB
        // readers; a clean process therefore cannot resolve CurveReader (CNA-DEMOS-ISSUE-012).
        ResourceContentManager content(nullptr, &resources);
        std::optional<Curve> bounce;
        try
        {
            bounce.emplace(content.Load<Curve>("Bounce"));
        }
        catch (const std::exception& error)
        {
            ctx.Print("Load<Curve>(\"Bounce\") failed: {}", error.what());
        }
        ctx.Check(bounce.has_value(), "Load reads the asset from the resource family (CNA-DEMOS-ISSUE-012)");
        if (bounce)
        {
            ctx.Print("embedded curve: {} keys, value at 0.25 = {:.3f}, at 0.5 = {:.3f}, at 1.5 (cycled) = {:.3f}",
                      bounce->getKeysProperty().getCountProperty(), bounce->Evaluate(0.25f), bounce->Evaluate(0.5f),
                      bounce->Evaluate(1.5f));
            ctx.Check(bounce->getKeysProperty().getCountProperty() == 3 &&
                          bounce->getPostLoopProperty() == CurveLoopType::Cycle,
                      "the Curve came out of the embedded .xnb");
            ctx.Check(std::abs(bounce->Evaluate(0.5f) - 1.0f) < 1e-5f && std::abs(bounce->Evaluate(1.5f) - 1.0f) < 1e-5f,
                      "it evaluates like the file's curve (PostLoop Cycle repeats it)");
        }

        // A name the family does not have is a ContentLoadException, like a missing file.
        bool missing = false;
        try
        {
            (void)content.Load<Curve>("NoSuchResource");
        }
        catch (const ContentLoadException& error)
        {
            missing = true;
            ctx.Print("missing resource: {}", error.what());
        }
        catch (const std::exception& error)
        {
            ctx.Print("missing resource, not a ContentLoadException: {}", error.what());
        }
        ctx.Check(missing, "an unknown resource name is a ContentLoadException");
    }
}

CNA_DEMOS_CONSOLE(ResourceContent, {
    .id = "content.content_manager.resource_content_manager",
    .title = "Content embedded in the program",
    .description = "Loads a Curve from an .xnb held as a binary resource of the program through a "
                   "ResourceContentManager over a System::Resources::ResourceManager -- no content directory -- "
                   "and shows that an unknown resource name is a ContentLoadException.",
    .apis = {"ResourceContentManager::ResourceContentManager", "ResourceContentManager::OpenStream",
             "ContentManager::Load", "ContentLoadException",
             "ResourceManager::ResourceManager", "Curve::Evaluate", "Curve::PostLoop"},
    .tags = {"content", "resources", "embedded", "xnb"},
    .knownIssue = "CNA-DEMOS-ISSUE-012",
    // XNB reader registration is process-global. Isolation prevents an earlier content demo from
    // hiding CNA-DEMOS-ISSUE-012 by registering CurveReader before this cold-start workflow.
    .ownProcess = true,
})
