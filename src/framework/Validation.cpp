// SPDX-License-Identifier: MS-PL
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <set>
#include <string>

#include "CnaDemos/Catalog.hpp"

namespace CnaDemos
{
    namespace
    {
        bool IsValidSegment(std::string_view segment)
        {
            if (segment.empty()
                || (!std::islower(static_cast<unsigned char>(segment.front()))
                    && !std::isdigit(static_cast<unsigned char>(segment.front()))))
            {
                return false;
            }
            return std::ranges::all_of(segment, [](unsigned char c) {
                return std::islower(c) || std::isdigit(c) || c == '_';
            });
        }

        bool IsValidId(std::string_view id)
        {
            if (id.empty() || !std::islower(static_cast<unsigned char>(id.front())))
            {
                return false;
            }
            int segments = 0;
            std::size_t start = 0;
            while (true)
            {
                const auto dot = id.find('.', start);
                const auto segment = id.substr(start, dot == std::string_view::npos ? id.npos : dot - start);
                if (!IsValidSegment(segment))
                {
                    return false;
                }
                ++segments;
                if (dot == std::string_view::npos)
                {
                    break;
                }
                start = dot + 1;
            }
            return segments >= 2;
        }

        // demos/graphics/sprite_batch/rotation.cpp -> "graphics.sprite_batch.rotation";
        // standalone/<id>/metadata.cpp -> "<id>".
        std::string IdFromSource(std::string_view source)
        {
            constexpr std::string_view demosRoot = "demos/";
            constexpr std::string_view standaloneRoot = "standalone/";
            if (source.starts_with(demosRoot) && source.ends_with(".cpp"))
            {
                std::string id(source.substr(demosRoot.size(), source.size() - demosRoot.size() - 4));
                std::ranges::replace(id, '/', '.');
                return id;
            }
            if (source.starts_with(standaloneRoot))
            {
                const auto rest = source.substr(standaloneRoot.size());
                return std::string(rest.substr(0, rest.find('/')));
            }
            return {};
        }
    }

    std::vector<std::string> ValidateRegistry(const std::string& assetRoot)
    {
        std::vector<std::string> problems;
        const auto report = [&problems](std::string_view id, const std::string& problem) {
            problems.push_back(std::string(id) + ": " + problem);
        };

        const auto demos = Registry::Instance().All();
        std::set<std::string_view> ids;
        for (const RegisteredDemo& demo : demos)
        {
            ids.insert(demo.entry.info.id);
        }

        std::string_view previous;
        for (const RegisteredDemo& demo : demos)
        {
            const DemoEntry& entry = demo.entry;
            const DemoInfo& info = entry.info;
            const std::string_view id = info.id;

            if (id == previous)
            {
                report(id, "duplicate ID (also registered by another source file)");
            }
            previous = id;

            if (!IsValidId(id))
            {
                report(id, "invalid ID: use two or more lower_snake_case segments joined by '.'");
            }

            const std::string expected = IdFromSource(entry.source);
            if (expected.empty())
            {
                report(id, "registered outside demos/ and standalone/ (" + entry.source + ")");
            }
            else if (entry.kind == DemoKind::Standalone)
            {
                if (id != expected || !entry.source.ends_with("/metadata.cpp"))
                {
                    report(id, "a standalone demo is registered in standalone/<id>/metadata.cpp; found "
                                   + entry.source);
                }
            }
            else if (id != expected && !(id.starts_with(expected) && id.size() > expected.size()
                                         && id[expected.size()] == '.'))
            {
                report(id, "ID does not match its source path " + entry.source + " (expected '" + expected
                               + "' or '" + expected + ".<name>')");
            }

            // A demo ID must be a leaf: the browser shows "graphics.sprite_batch" as a group.
            const auto child = ids.upper_bound(id);
            if (child != ids.end() && child->starts_with(id) && child->size() > id.size()
                && (*child)[id.size()] == '.')
            {
                report(id, "ID is also the group of another demo (" + std::string(*child) + ")");
            }

            if (info.title.empty())
            {
                report(id, "missing title");
            }
            else if (info.title.back() == '.')
            {
                report(id, "title must not end with a period");
            }
            if (info.description.size() < 20)
            {
                report(id, "description missing or too short");
            }
            else if (info.description.back() != '.')
            {
                report(id, "description must end with a period");
            }
            if (info.apis.empty())
            {
                report(id, "no CNA APIs listed in .apis");
            }
            for (const std::string_view tag : info.tags)
            {
                if (std::ranges::any_of(tag, [](unsigned char c) { return std::isupper(c) || std::isspace(c); }))
                {
                    report(id, "tag '" + std::string(tag) + "' must be lower case without spaces");
                }
            }
            if (info.frames < 0)
            {
                report(id, "frames must not be negative");
            }
            if (entry.kind == DemoKind::Console && (info.frames != 0 || info.deterministic))
            {
                report(id, "frames/deterministic apply to Game and standalone demos only");
            }
            if ((entry.kind == DemoKind::Game || entry.kind == DemoKind::Console) && demo.console == nullptr
                && demo.game == nullptr)
            {
                report(id, "no runnable body registered");
            }
            for (const std::string_view asset : info.assets)
            {
                const std::filesystem::path path = std::filesystem::path(assetRoot) / std::string(asset);
                if (!std::filesystem::exists(path))
                {
                    report(id, "asset not found: " + path.string());
                }
            }
        }
        return problems;
    }
}
