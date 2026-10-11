// SPDX-License-Identifier: MS-PL
// A game's JSON assets: named CNJ factories produce the same C++ enemy type.
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"

using namespace Microsoft::Xna::Framework::Content;

namespace
{
    struct Enemy { std::string name; int health; };

    void CnjLoader(CnaDemos::DemoContext& ctx)
    {
        // CNJ registration belongs to this manager, unlike the shared CNB loader registry.
        // Captured counters outlive the manager that owns these callbacks.
        int ordinaryReads = 0, eliteReads = 0;
        ContentManager content(nullptr, "assets/custom");
        content.RegisterCnjLoader<Enemy>("CnaDemos.Enemy",
            [&ordinaryReads](const std::string& text, ContentManager&) {
                // CNA supplies raw JSON; a game chooses its own parser for the payload.
                // nlohmann-json is already a documented build prerequisite of this project.
                const auto data = nlohmann::json::parse(text).at("data");
                Enemy enemy{data.at("name").get<std::string>(), data.at("health").get<int>()};
                ++ordinaryReads;
                return enemy;
            });
        content.RegisterCnjLoader<Enemy>("CnaDemos.EliteEnemy",
            [&eliteReads](const std::string& text, ContentManager&) {
                const auto data = nlohmann::json::parse(text).at("data");
                // Elite assets author a base health; this game's elite rule doubles it.
                Enemy enemy{data.at("name").get<std::string>(), 2 * data.at("baseHealth").get<int>()};
                ++eliteReads;
                return enemy;
            });

        const Enemy guard = content.Load<Enemy>("guard");
        const Enemy elite = content.Load<Enemy>("elite");
        ctx.Print("{}: {} HP; {}: {} HP", guard.name, guard.health, elite.name, elite.health);
        ctx.Check(guard.name == "Mossling" && guard.health == 12,
                  "the ordinary factory reads the name and health from the CNJ payload");
        ctx.Check(elite.name == "Stone Guard" && elite.health == 100,
                  "a second CNJ type selects a different factory for the same C++ Enemy");
        (void)content.Load<Enemy>("guard");
        (void)content.Load<Enemy>("elite");
        ctx.Check(ordinaryReads == 1 && eliteReads == 1, "both named factories use the manager's asset cache");

        bool duplicateRejected = false;
        try
        {
            content.RegisterCnjLoader<Enemy>("CnaDemos.Enemy",
                [](const std::string&, ContentManager&) -> Enemy {
                    throw std::logic_error("Duplicate registration must never replace the original factory");
                });
        }
        catch (const std::logic_error&) { duplicateRejected = true; }
        ctx.Check(duplicateRejected, "duplicate named registration is rejected instead of replacing a factory");

        bool payloadRejected = false;
        try { (void)content.Load<Enemy>("missing_health"); }
        catch (const ContentLoadException& error)
        {
            payloadRejected = true;
            ctx.Print("Invalid enemy payload: {}", error.what());
        }
        ctx.Check(payloadRejected && ordinaryReads == 1,
                  "a missing required payload field reaches the caller as ContentLoadException");

        content.Unload();
        const Enemy reloaded = content.Load<Enemy>("guard");
        ctx.Check(reloaded.health == 12 && ordinaryReads == 2,
                  "Unload clears cached assets but retains this manager's factory registrations");
        ContentManager unregistered(nullptr, "assets/custom");
        bool isolated = false;
        try { (void)unregistered.Load<Enemy>("guard"); }
        catch (const ContentLoadException&) { isolated = true; }
        ctx.Check(isolated, "a different manager needs its own CNJ registration");
    }
}

CNA_DEMOS_CONSOLE(CnjLoader, {
    .id = "content.custom.cnj_loader",
    .title = "Custom CNJ enemy definitions (CNAEXT)",
    .description = "Registers two named CNJ factories for one game-owned Enemy type, parses their JSON payloads, "
                   "checks caching and manager-local registration, and rejects a missing required field.",
    .apis = {"ContentManager::RegisterCnjLoader", "ContentManager::Load", "ContentManager::Unload",
             "ContentLoadException"},
    .tags = {"content", "custom", "cnj", "json", "cnaext"},
    .assets = {"custom/guard.cnj", "custom/elite.cnj", "custom/missing_health.cnj"},
})
